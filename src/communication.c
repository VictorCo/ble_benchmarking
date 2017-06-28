#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "communication.h"
#include "communication_error.h"
#include "SEGGER_RTT.h"
#include "timer_packet.h"
#include "ble_nus.h"
#include "long_packet.h"
#include "def.h"


//Les indices du tableau doivent être dans le même ordre que que les enums TYPE_CMD_NAME et TYPE_PARAM_NAME
//Voir dans communication.h
//En procédant ainsi nous pouvous atteindre m_def_input[TEST_SPEED_DOWN] etc...
const c_def_input m_def_input[N_INPUT] = 
    {
        {TEST_SPEED_DOWN,           0, {"test_speed_down",          "tsd"   }},
        {TEST_SPEED_UP,             0, {"test_speed_up",            "tsu"   }},
        {SET_PARAM,                 0, {"set_param",                "sp"    }},
        {GET_TIME,                  0, {"get_time",                 "gt"    }},
        {GET_PARAMS,                0, {"get_params",               "gp"    }},
        {GET_CONNECTION_SECURITY,   0, {"get_connection_security",  "gcs"   }},
        {START,                     0, {"start",                    "s"     }},
        {CONTINUE,                  1, {"continue",                 "c"     }},
        {STOP,                      0, {"stop",                     "e"     }},
        {MODIFY,                    2, {"modify",                   "m"     }}
    };
    

    
c_info_t m_info_t = 
    {
        .b_timestamp_start = false,
        .b_timestamp_available = false,
        .b_stop_down = false,
        .remaining_byte = 0,
        .conn_params =
            {
                .min_conn_interval = MIN_CONN_INTERVAL,
                .max_conn_interval = MAX_CONN_INTERVAL,
                .slave_latency = SLAVE_LATENCY,
                .conn_sup_timeout = CONN_SUP_TIMEOUT
            },
        .current_conn_params =
            {
                .min_conn_interval = MIN_CONN_INTERVAL,
                .max_conn_interval = MAX_CONN_INTERVAL,
                .slave_latency = SLAVE_LATENCY,
                .conn_sup_timeout = CONN_SUP_TIMEOUT
            }
    };
    
const c_def_conn_param m_def_conn_param[N_CON_PARAM] =
    {
        {M_CON_INTERVAL_MIN,    (uint16_t *)&m_info_t.conn_params.min_conn_interval,        (uint16_t *)&m_info_t.current_conn_params.min_conn_interval},
        {M_CON_INTERVAL_MAX,    (uint16_t *)&m_info_t.conn_params.max_conn_interval,        (uint16_t *)&m_info_t.current_conn_params.max_conn_interval},
        {M_CON_SLAVE_LATENCY,   (uint16_t *)&m_info_t.conn_params.slave_latency,            (uint16_t *)&m_info_t.current_conn_params.slave_latency},       
        {M_CON_TIMEOUT,         (uint16_t *)&m_info_t.conn_params.conn_sup_timeout,         (uint16_t *)&m_info_t.current_conn_params.conn_sup_timeout}
    };


void communication_start(char *s, int length, ble_nus_t *p_nus)
{
    c_msg_t msg;
    int err_code;
    char mot[SIZE_QUEUED+1];
    char *c_lower;

    memcpy(mot, s, length);
    memset(&msg, 0, sizeof(c_msg_t));
    msg.length = length;
    msg.start = mot;
    msg.end = mot + length-1;
    *(msg.end+1) = '\0';
    
    for (c_lower = msg.start ; *c_lower; ++c_lower) *c_lower = tolower(*c_lower);
        
    err_code = parse(&msg);
    
    if(err_code == MESSAGE_SUCCES)
    {
        
        if(check_input_word_exist(&msg)
           && p_nus->is_notification_enabled)
        {
            communication_run(&msg, p_nus);
            //send_long_packet(p_nus, s, length);
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

/*
Permet de savoir si on atteint un séparateur de mot
Si oui renvoi le nombre de séparateur à la suite
*/

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
Si la fonction renvoie true on est assuré d'avoir la bonne synthaxe d'une commande
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
    int8_t conn_param;
    uint16_t modify_value;
    uint32_t err_code;
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
                    m_info_t.byte_number = 0;
                    m_info_t.n_packet = 0;
                    ble_nus_string_send(p_nus, (uint8_t *)RESULT_MSG_START_TIMER, strlen(RESULT_MSG_START_TIMER), false);
                    m_info_t.timestamp_start = timer_get_ticks();
                }
                
                else if(param->name == STOP)
                {
                    if(!m_info_t.b_timestamp_start)
                    {
                        NRF_LOG_PRINTF("timer non lance\n");
                        return;
                    }
                    
                    if(m_info_t.remaining_byte)
                    {
                        m_info_t.b_stop_down = true;
                        NRF_LOG_PRINTF("Il reste des bits a UP\nStop mis en attente\n");
                        return;
                    }
                    
                    if ( ble_nus_string_send(p_nus, (uint8_t *)RESULT_MSG_STOP_TIMER, strlen(RESULT_MSG_STOP_TIMER), false) != NRF_SUCCESS)
                    {
                        return;
                    }
                    
                    m_info_t.b_stop_down = false;
                    m_info_t.b_timestamp_start = false;
                    m_info_t.b_timestamp_available = true;
                    
                    m_info_t.timestamp = timer_ticks_to_ms( timer_get_ticks() - m_info_t.timestamp_start); 

                    //DEBUG===
                    if(m_info_t.timer_name == TEST_SPEED_UP)
                        NRF_LOG_PRINTF("(UP) temps ecoule : %d\n", m_info_t.timestamp );
                    
                    else if(m_info_t.timer_name == TEST_SPEED_DOWN)
                        NRF_LOG_PRINTF("(DOWN) temps ecoule : %d\n", m_info_t.timestamp );
                    //========
                }
                
                else if(param->name == CONTINUE)
                {
                    if(m_info_t.b_timestamp_start)
                    {
                        if(cmd->name == TEST_SPEED_UP)
                        {
                            m_info_t.byte_number += p_msg_t->length;
                            m_info_t.n_packet++;
                            i++;
                        }
                    
                        else if(cmd->name == TEST_SPEED_DOWN)
                        {
                            m_info_t.total_byte = (m_info_t.remaining_byte) ? 
                                atoi(p_msg_t->word[++i].start) + m_info_t.total_byte
                                : atoi(p_msg_t->word[++i].start);
                            
                            m_info_t.remaining_byte = m_info_t.total_byte - m_info_t.remaining_byte;
                            
                            if(m_info_t.remaining_byte)
                            {
                                send_dummy_data(p_nus, &m_info_t, m_info_t.remaining_byte);
                            }
                        }                        
                    }
                }
                
                break;
            
            case SET_PARAM :
                if(param->name == MODIFY)
                {
                    modify_option = &p_msg_t->word[++i];
                    modify_value = atoi(p_msg_t->word[++i].start);
                    
                    if( (conn_param = get_conn_param(modify_option->start)) < 0 )
                    {
                        NRF_LOG_PRINTF("Nom de parametre de connexion inconnu\n");
                        return;
                    }
                    
                   *(m_def_conn_param[conn_param].param_value) = MSEC_TO_UNITS(modify_value, UNIT_1_25_MS);
                    //err_code = sd_ble_gap_ppcp_set(&m_info_t.conn_params);
                    
                    //===============================
                    ble_gap_conn_params_t test; 
                    sd_ble_gap_ppcp_get(&test);
                    NRF_LOG_PRINTF("DEBUG test imin : %d\n", test.min_conn_interval);
                    NRF_LOG_PRINTF("DEBUG test imax : %d\n", test.max_conn_interval);
                    NRF_LOG_PRINTF("DEBUG test slave: %d\n", test.slave_latency);
                    NRF_LOG_PRINTF("DEBUG test timeout: %d\n", test.conn_sup_timeout);
                    //===============================
                    if (sd_ble_gap_conn_param_update(p_nus->conn_handle,&m_info_t.conn_params) == NRF_SUCCESS )
                    {
                        *(m_def_conn_param[conn_param].param_value_current) = *(m_def_conn_param[conn_param].param_value);
                    }
                    NRF_LOG_PRINTF("==== Modify : %s\n", m_def_conn_param[conn_param].param_name);
                    NRF_LOG_PRINTF("Value : %d\n", modify_value);
                    NRF_LOG_PRINTF("Value modifie : %d\n", *(m_def_conn_param[conn_param].param_value));
                    NRF_LOG_PRINTF("err_code : %d ====\n", err_code);
                    
                }
                break;
            
            case GET_TIME :
                if(m_info_t.b_timestamp_available)
                {
                    //Type de test : UP? DOWN?
                    if(m_info_t.timer_name == TEST_SPEED_UP)
                        sprintf(data_send, "%s>%d", RESULT_MSG_GET_TIME_UP, m_info_t.timestamp);
                    else if(m_info_t.timer_name == TEST_SPEED_DOWN)
                        sprintf(data_send, "%s>%d", RESULT_MSG_GET_TIME_DOWN, m_info_t.timestamp);
                    
                    ble_nus_string_send(p_nus, (uint8_t *)data_send, strlen(data_send), false);
                    
                    //nombre octet
                    sprintf(data_send, "%s>%d", RESULT_MSG_GET_TIME_BYTE, m_info_t.byte_number);
                    ble_nus_string_send(p_nus, (uint8_t *)data_send, strlen(data_send), false);
                    
                    //debit
                    //ble_nus_string_send(p_nus, (uint8_t *)RESULT_MSG_GET_TIME_DEBIT, strlen(RESULT_MSG_GET_TIME_DEBIT), false);
                    sprintf(data_send, "%s>%f", RESULT_MSG_GET_TIME_DEBIT, (float)m_info_t.byte_number/(float)m_info_t.timestamp);
                    ble_nus_string_send(p_nus, (uint8_t *)data_send, strlen(data_send), false);
                    
                    //nombre de packet
                    //ble_nus_string_send(p_nus, (uint8_t *)RESULT_MSG_GET_N_PACKET, strlen(RESULT_MSG_GET_N_PACKET), false);
                    sprintf(data_send, "%s>%d", RESULT_MSG_GET_N_PACKET, m_info_t.n_packet);
                    ble_nus_string_send(p_nus, (uint8_t *)data_send, strlen(data_send), false);
                }
                
                else
                   NRF_LOG_PRINTF("timer non dispo\n"); 
                break;
                
            case GET_PARAMS :  
                if(sd_ble_gap_ppcp_get(&m_info_t.conn_params) == NRF_SUCCESS )
                {
                    //interval min
                    //ble_nus_string_send(p_nus, (uint8_t *)RESULT_MSG_GET_CON_INTERVAL_MIN, strlen(RESULT_MSG_GET_CON_INTERVAL_MIN), false);
                    sprintf(data_send, "%s>%d", RESULT_MSG_GET_CON_INTERVAL_MIN, m_info_t.current_conn_params.min_conn_interval);
                    ble_nus_string_send(p_nus, (uint8_t *)data_send, strlen(data_send), false);
                    
                    //interval max
                    //ble_nus_string_send(p_nus, (uint8_t *)RESULT_MSG_GET_CON_INTERVAL_MAX, strlen(RESULT_MSG_GET_CON_INTERVAL_MAX), false);
                    sprintf(data_send, "%s>%d", RESULT_MSG_GET_CON_INTERVAL_MAX, m_info_t.current_conn_params.max_conn_interval);
                    ble_nus_string_send(p_nus, (uint8_t *)data_send, strlen(data_send), false);
                    
                    //slave_latency
                    //ble_nus_string_send(p_nus, (uint8_t *)RESULT_MSG_GET_CON_SLAVE_LATENCY, strlen(RESULT_MSG_GET_CON_SLAVE_LATENCY), false);
                    sprintf(data_send, "%s>%d", RESULT_MSG_GET_CON_SLAVE_LATENCY, m_info_t.current_conn_params.slave_latency);
                    ble_nus_string_send(p_nus, (uint8_t *)data_send, strlen(data_send), false);
                    
                    //timeout
                    //ble_nus_string_send(p_nus, (uint8_t *)RESULT_MSG_GET_CON_TIMEOUT, strlen(RESULT_MSG_GET_CON_TIMEOUT), false);
                    sprintf(data_send, "%s>%d", RESULT_MSG_GET_CON_TIMEOUT, m_info_t.current_conn_params.conn_sup_timeout);
                    ble_nus_string_send(p_nus, (uint8_t *)data_send, strlen(data_send), false);
                }
                break;
                
            case GET_CONNECTION_SECURITY :
                
                break;
        }
       
    }

}

int8_t get_conn_param(char *s)
{
    int i;
    
    if(s == NULL)
        return -1;
    
    for(i=0; i < N_CON_PARAM; i++)
    {
        if(strcmp(s, m_def_conn_param[i].param_name) == 0)
            return i;
    }
    
    return -1;
    
}

//quand long_packet.c recoit l'event BLE_EVT_TX_COMPLETE
//on regarde si on doit encore upload des données
void continue_send_byte_up(ble_nus_t *p_nus)
{
    if(m_info_t.timestamp_start)
    {
        if(m_info_t.timer_name == TEST_SPEED_DOWN)
        {
            if(m_info_t.remaining_byte)
            {
                send_dummy_data(p_nus, &m_info_t, m_info_t.remaining_byte);
            }
        }
        
        if(m_info_t.b_stop_down)   //si on a recu la commande de stopper le timer alors qu'on avait pas toute les données
        {
            //on renvoie une instruction comme quoi on veux stop le timer
            
            communication_start("s tsd -e e", 10, p_nus);
        }
    }
}

void communication_update_params(ble_evt_t *p_evt)
{
    switch(p_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONN_PARAM_UPDATE :
            //m_info_t.conn_params = p_evt->evt.gap_evt.params.conn_param_update.conn_params;
//        sd_ble_gap_conn_param_update(p_evt->evt.gap_evt.conn_handle, &p_evt->evt.gap_evt.params.conn_param_update_request.conn_params);
        NRF_LOG_PRINTF("UPDATE\n");
            break;
        
        
        default :
            break;
    }
}
