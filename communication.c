#include <string.h>
#include "communication.h"
#include "SEGGER_RTT.h"


void communication_start(char *s, int length, ble_nus_t *p_nus)
{
	msg_t msg;
	int err_code;
	char mot[length+1];
	for (int i = 0; i < length; i++)
		mot[i] = *(s+i);
	
	mot[length] = '\0';
	
	memset(&msg, 0, sizeof(msg_t));
	msg.length = length;
	msg.start = mot;
	msg.end = mot + length-1;
	
	err_code = parse(&msg);
	ble_nus_string_send(p_nus, (uint8_t *)"envoie donne", 12);
	if (err_code == MESSAGE_SUCCES)
			SEGGER_RTT_printf(0, "Envoie conforme au protocole", err_code);
	
	
}

int static parse(msg_t *p_msg)
{
	param_t *param_current;
	
	if (*p_msg->start != MESSAGE_TYPE_SOF
	|| 	*p_msg->end != MESSAGE_TYPE_EOF)
	{
		return MESSAGE_ERROR;
	}
	
	
	SEGGER_RTT_printf(0, "start addr = %d\n", p_msg->start);
	SEGGER_RTT_printf(0, "fin addr = %d\n", p_msg->end);
	for (char * i = p_msg->start+1; i != p_msg->end; i++)
	{
		SEGGER_RTT_printf(0, "valeur = %c // addr = %d\n", *i, i);
		switch (*i)
		{
			case MESSAGE_TYPE_PARAM :
				if (++p_msg->nParam >= MAX_PARAM)
					return MESSAGE_ERROR_PARAM;
				
				param_current = &p_msg->param[p_msg->nParam-1];
				param_current->start = i;
				SEGGER_RTT_printf(0, "Current : %d \\ %d\n", param_current->start, p_msg->param[p_msg->nParam-1].start);
				SEGGER_RTT_printf(0, "Parametre\n");
				
			break;
			
			case MESSAGE_TYPE_CMD :
				SEGGER_RTT_printf(0, "Commande\n");
			break;
			
			default :
				SEGGER_RTT_printf(0, "Autre\n");
			break;
		
		}		
	}
		
	display_param(p_msg);
	
	return MESSAGE_SUCCES;
}

void display_param(msg_t *p_msg)
{
	param_t *p = &p_msg->param[0];
	for (int i = 0; i < p_msg->nParam; i++, p = &p_msg->param[i])
		SEGGER_RTT_printf(0, "***\nParam n : %d\nAddr param : %d\nlen : %d\nAddr : %d\n", i, p,&p->length, p->start);
}
	


