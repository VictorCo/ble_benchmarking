#include <stdint.h>
#include "ble_nus.h"

#define MESSAGE_TYPE_SOF 			's'
#define MESSAGE_TYPE_EOF 			'e'
#define MESSAGE_TYPE_CMD 			'c'
#define MESSAGE_TYPE_PARAM 		'p'

#define MESSAGE_ERROR_INIT 					-1
#define MESSAGE_ERROR_NUMBER_WORD		-2
#define MESSAGE_ERROR_LEN_WORD 			-3
#define MESSAGE_ERROR_OPEN_WORD 		-4
#define MESSAGE_SUCCES 1

#define MAX_WORD  0x80			//le maximum d'action speciale disponible

#define TYPE_PARAM 0x1
#define TYPE_CMD   0x2
#define TYPE_OTHER 0x10
#define MASK_WORD  0xF		/*les 4 premiers bits sont utilisés pour reconnaitre un debut ou fin d'une action spéciale
														bit 1 : parametre
														bit 2 : commande
														bit 3 : à définir
														bit 4 : à définir
													*/

typedef enum
{
	test
}type_param;

typedef struct
{
	int length;
	char *start;
	int type;
}c_word_t;

typedef struct
{
	uint8_t length;
	char *start;
	char *end;
	int nWord;
	c_word_t word[MAX_WORD];
}c_msg_t;


void communication_start(char *s, int length, ble_nus_t *p_nus);
void display_param(c_msg_t *p_msg);
int parse(c_msg_t *p_msg);
