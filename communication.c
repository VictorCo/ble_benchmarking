#include <string.h>
#include "communication.h"
#include "communication_error.h"
#include "SEGGER_RTT.h"
#include "timer_packet.h"
#include "ble_nus.h"
#include "long_packet.h"


void communication_start(char *s, int length, ble_nus_t *p_nus)
{
    c_msg_t msg;
    int err_code;
    char mot[SIZE_QUEUED+1];
    for(int i = 0; i < length; i++)
    {
        mot[i] = *(s+i);
    }
    mot[length] = '\0';

    memset(&msg, 0, sizeof(c_msg_t));
    msg.length = length;
    msg.start = mot;
    msg.end = mot + length-1;

    err_code = parse(&msg);
    
    if(err_code == MESSAGE_SUCCES)
    {
        if(p_nus->is_notification_enabled)
        {
            send_long_packet(p_nus, s, length);
        }
        timer_restart();
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
        if( (skip = skip_separator(i)) )      //si on atteint un nouveau type de mot
        {
            if(old_type != TYPE_NULL)      //on ferme l'ancien type si il existe
            {
                word_current->length = i - word_current->start;
                
                if (!word_current->length)
                {
                    return MESSAGE_ERROR_LEN_WORD;
                }
                
                word_current = NULL;
            }
            i += skip;
            if (i == p_msg->end)
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
            if (word_current->type == TYPE_PARAM)
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
		
    display_param(p_msg);
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
    if (old_type == TYPE_NULL)
    {
        return TYPE_CMD;
    }
    
    if (*s == MESSAGE_TYPE_PARAM)
    {
        return TYPE_PARAM;
    }
            
    return TYPE_ATTRIBUTE;
}

void display_param(c_msg_t *p_msg)
{
    c_word_t *p = &p_msg->word[0];
    for(int i = 0; i < p_msg->nWord; i++, p = &p_msg->word[i])
    {
        SEGGER_RTT_printf(0,
                          "***\nWord n : %d\nAddr param : %d\ntype : %x, len : %d\nAddr start: %d\n",
                          i, p,p->type, p->length, p->start);
    }
}
	


