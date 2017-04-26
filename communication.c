#include <string.h>
#include "communication.h"
#include "SEGGER_RTT.h"
#include "timer_packet.h"


void communication_start(char *s, int length, ble_nus_t *p_nus)
{
	c_msg_t msg;
	int err_code;
	char mot[length+1];
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
	ble_nus_string_send(p_nus, (uint8_t *)"envoie donne abcdefghiklmnop", 28);
	if(err_code == MESSAGE_SUCCES)
	{
		timer_restart();
		SEGGER_RTT_printf(0, "Envoie conforme au protocole\n", err_code);
	}
}

int parse(c_msg_t *p_msg)
{
	c_word_t *word_current = NULL;
	uint8_t type;
	
	if (*p_msg->start != MESSAGE_TYPE_SOF
	|| 	*p_msg->end != MESSAGE_TYPE_EOF)
	{
		return MESSAGE_ERROR_INIT;
	}
	
	for(char * i = p_msg->start+1; i != p_msg->end; i++)
	{
		switch(*i)
		{
			case MESSAGE_TYPE_PARAM :
				type = TYPE_PARAM;
				break;
			
			
			case MESSAGE_TYPE_CMD :
				type = TYPE_CMD;
                break;
					
			default :
				type = TYPE_OTHER;
                break;
		}
		
		if(type & MASK_WORD) //action spe
		{
			if(!word_current)	//ouverture
			{
				if(++p_msg->nWord >= MAX_WORD)
				{
					return MESSAGE_ERROR_NUMBER_WORD;
				}
				
				word_current = &p_msg->word[p_msg->nWord-1];
				word_current->type = type;
				word_current->start = i+1;
			}
			
			else if(word_current->type & type)  //fermeture
			{
				word_current->length = i - word_current->start;
				
				if(word_current->length <= 0)
				{
					return MESSAGE_ERROR_LEN_WORD;
				}
				word_current = NULL;
			}
		}
	}
	
	if(word_current)			//si un parametre n'a pas été fermé, erreur
	{
		return MESSAGE_ERROR_OPEN_WORD;
	}
		
	display_param(p_msg);
	return MESSAGE_SUCCES;
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
	


