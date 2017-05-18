#include <string.h>
#include "communication.h"
#include "communication_error.h"
#include "SEGGER_RTT.h"
#include "timer_packet.h"
#include "ble_nus.h"
#include "long_packet.h"


const c_def_input m_def_input[N_INPUT] = 
    {
        {TEST_SPEED_DOWN,   0, {"test_speed_down",  "tsd"   }},
        {TEST_SPEED_UP,     0, {"test_speed_up",    "tsu"   }},
        {SET_PARAM,         0, {"set_param",        "sp"    }},
        {GET_TIME,          0, {"get_time",         "gt"    }},
        {START,             0, {"start",            "s"     }},
        {CONTINUE,          0, {"continue",         "c"     }},
        {STOP,              0, {"stop",             "e"     }},
        {MODIFY,            2, {"modify",           "m"     }}
    };
    
c_info_t m_info_t = 
    {
        .b_timestamp_start = false,
        .b_timestamp_available = false
    };


void communication_start(char *s, int length, ble_nus_t *p_nus)
{
    c_msg_t msg;
    int err_code;
    char mot[SIZE_QUEUED];

    memcpy(mot, s, length);
    memset(&msg, 0, sizeof(c_msg_t));
    msg.length = length;
    msg.start = mot;
    msg.end = mot + length-1;

    err_code = parse(&msg);
    
    if(err_code == MESSAGE_SUCCES)
    {
        
        if(check_input_word_exist(&msg)
           && p_nus->is_notification_enabled)
        {
            communication_run(&msg, p_nus);
            send_long_packet(p_nus, s, length);
        }
    }
    
    else
    {
        communication_error_display(err_code);
    }
}

uint8_t parse(c_msg_t *p_msg)
{
    c_word_t *word_current = NULL;
    uint8_t old_type = TYPE_NULL;
    uint8_t skip;

    if(*p_msg->start != MESSAGE_TYPE_SOF
    || *p_msg->end != MESSAGE_TYPE_EOF)
    {
        return MESSAGE_ERROR_INIT;
    }
	
    for(char * i = p_msg->start+1; i != p_msg->end; i++)
    {
        skip = skip_separator(i);
        if(skip)      //si on atteint un nouveau type de mot
        {
            if(old_type != TYPE_NULL)      //on ferme l'ancien type si il existe
            {
                word_current->length = i - word_current->start;
                word_current->start[word_current->length] = '\0';
                if(!word_current->length)
                {
                    return MESSAGE_ERROR_LEN_WORD;
                }
                
                word_current = NULL;
            }
            i += skip;
            if(i == p_msg->end)
            {
                break;
            }
                
            if(++p_msg->nWord > MAX_WORD)
            {
                return MESSAGE_ERROR_NUMBER_WORD;
            }
              
            word_current = &p_msg->word[p_msg->nWord-1];
            word_current->type = get_type(i, old_type);
            word_current->start = i;  
            //si c'est un parametre on deplace le curseur de 1 pour le '-'
            if(word_current->type == TYPE_PARAM)
            {
                word_current->start++; 
            }
            old_type = word_current->type;     
        }
        
        if(!word_current)
        {
            return MESSAGE_ERROR_OPEN_WORD;
        }
    }
	
    if(word_current)			//si un parametre n'a pas été fermé, erreur
    {
        return MESSAGE_ERROR_CLOSE_WORD;
    }
    
    if(!p_msg->nWord)           
    {
        return MESSAGE_ERROR_NO_CMD;
    }
    return MESSAGE_SUCCES;
}

uint8_t skip_separator(char *s)
{
    uint8_t skip = 0;
    while(*s == MESSAGE_SEPARATOR)
    {
        s++;
        skip++;
    }
    
    return skip;
}

uint8_t get_type(char *s, uint8_t old_type)
{
    if(old_type == TYPE_NULL)
    {
        return TYPE_CMD;
    }
    
    if(*s == MESSAGE_TYPE_PARAM)
    {
        return TYPE_PARAM;
    }
            
    return TYPE_ATTRIBUTE;
}

/*
Regarde si les commandes, parametres et attributs ont la bonne synthaxe
Si un attribut est présent sans parametre avant -> erreur
Gère si un paramètre doit avoir 0, 1, ou plusieurs paramètres
Si la fonction renvoie true on est assusré d'avoir la bonne synthaxe d'une commande
*/
bool check_input_word_exist(c_msg_t *p_msg_t)
{
    int i,j,k;
    bool find;
    
    for(i = 0; i < p_msg_t->nWord; i++)
    {
        if(  !((TYPE_CMD | TYPE_PARAM) & p_msg_t->word[i].type ) )
        {
            NRF_LOG_PRINTF("commande ou param requis\n");
            return false;
        }
        for(j = 0, find = false; j < N_INPUT; j++)
        {
            for(k = 0; k < N_NAME; k++)
            {
                if(strcmp(p_msg_t->word[i].start, m_def_input[j].name[k]) == 0)
                {
                    NRF_LOG_PRINTF("%s trouve\n", m_def_input[j].name[k]);
                    p_msg_t->word[i].name = m_def_input[j].type;
                    
                    if( (i + m_def_input[j].n_attribute) > p_msg_t->nWord
                        || !check_argument_number(&p_msg_t->word[i], m_def_input[j].n_attribute) )
                    {
                        NRF_LOG_PRINTF("il manque des arguments\n");
                        return false;
                    }
                    i += m_def_input[j].n_attribute;
                    find = true;
                    break;
                }
            }
            
            if(find)
            {
                break;
            }
        }
        if(j == N_INPUT)
        {
            NRF_LOG_PRINTF("param inconnu\n");
            return false;
        }
    }
    return true;
}

/*
Depuis un paramètre donné, regarde si il y a le bon nombre d'argument attendu
*/
bool check_argument_number(const c_word_t *p_param, uint8_t nb_argument)
{
    int i;
    for(i = 0; i < nb_argument; i++)
    {
        if( !((p_param+i+1)->type == TYPE_ATTRIBUTE) )
        {
            return false;
        }
    }
    
    return true;
}

void communication_run(const c_msg_t *p_msg_t, ble_nus_t *p_nus)
{
    const c_word_t *cmd = p_msg_t->word;    //le premier mot est forcement la commande
    const c_word_t *param = NULL;
    const c_word_t *modify_option;
    const c_word_t *modify_value;
    char data_send[16];
    int i;
    
    for(i = 0; i < p_msg_t->nWord; i ++)
    {
        param = &p_msg_t->word[i];
        switch (cmd->name)
        {
            case TEST_SPEED_UP :
            case TEST_SPEED_DOWN :
                
                //si un timer est deja lancé on regarde si la commande porte sur le meme type de timer
                if(m_info_t.b_timestamp_start)
                {
                    if(m_info_t.timer_name != cmd->name)
                    {
                        NRF_LOG_PRINTF("Pas le bon type de timer\n");
                        return;
                    }
                }
            
                if(param->name == START)
                {
                    if(m_info_t.b_timestamp_start)
                    {
                        NRF_LOG_PRINTF("Un timer est deja lance\n");
                        return;
                    }
                    m_info_t.timer_name = cmd->name;
                    m_info_t.b_timestamp_start = true;
                    m_info_t.timestamp_start = timer_get_ticks();
                }
                
                else if(param->name == STOP)
                {
                    if(!m_info_t.b_timestamp_start)
                    {
                        NRF_LOG_PRINTF("timer non lance\n");
                        return;
                    }
                    m_info_t.b_timestamp_start = false;
                    m_info_t.b_timestamp_available = true;
                    m_info_t.timestamp = timer_ticks_to_ms( timer_get_ticks() - m_info_t.timestamp_start);
                    
                    if(m_info_t.timer_name == TEST_SPEED_UP)
                        NRF_LOG_PRINTF("(UP) temps ecoule : %d\n", m_info_t.timestamp );
                    
                    else if(m_info_t.timer_name == TEST_SPEED_DOWN)
                        NRF_LOG_PRINTF("(DOWN) temps ecoule : %d\n", m_info_t.timestamp );
                }
                break;
            
            case SET_PARAM :
                if(param->name == MODIFY)
                {
                    modify_option = &p_msg_t->word[++i];
                    modify_value = &p_msg_t->word[++i];
                    
                    
                    
                }
                break;
            
            case GET_TIME :
                if(m_info_t.b_timestamp_available)
                {

                    if(m_info_t.timer_name == TEST_SPEED_UP)
                        ble_nus_string_send(p_nus, (uint8_t *)RESULT_MSG_GET_TIME_UP, strlen(RESULT_MSG_GET_TIME_UP));
                    else if(m_info_t.timer_name == TEST_SPEED_DOWN)
                        ble_nus_string_send(p_nus, (uint8_t *)RESULT_MSG_GET_TIME_DOWN, strlen(RESULT_MSG_GET_TIME_DOWN));
                    
                    sprintf(data_send, "%d", m_info_t.timestamp);
                    ble_nus_string_send(p_nus, (uint8_t *)data_send, strlen(data_send));
                }
                
                else
                   NRF_LOG_PRINTF("timer non dispo\n"); 
                break;
            
        }
       
    }

}	


