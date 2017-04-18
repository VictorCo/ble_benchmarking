#include <stdint.h>
#include "ble_nus.h"

#define MESSAGE_TYPE_SOF 's'
#define MESSAGE_TYPE_EOF 'e'
#define MESSAGE_TYPE_CMD 'c'
#define MESSAGE_TYPE_PARAM 'p'

#define MESSAGE_ERROR -1
#define MESSAGE_ERROR_PARAM -2
#define MESSAGE_SUCCES 1

#define MAX_PARAM 8

typedef enum
{
	test
}type_param;

typedef struct
{
	int length;
	char *start;
	char *type;
}param_t;

typedef struct
{
	uint8_t length;
	char *start;
	char *end;
	int nParam;
	param_t param[MAX_PARAM];
	char *cmd;
}msg_t;


void communication_start(char *s, int length, ble_nus_t *p_nus);
int static parse(msg_t *p_msg);
void display_param(msg_t *p_msg);
