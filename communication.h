#include <stdint.h>
#include "ble_nus.h"

#define MESSAGE_TYPE_SOF            's'
#define MESSAGE_TYPE_EOF            'e'
#define MESSAGE_TYPE_PARAM          '-'
#define MESSAGE_SEPARATOR           ' '

#define MAX_WORD        0x10        //le maximum d'action speciale disponible

#define TYPE_NULL       0x0
#define TYPE_CMD        0x1
#define TYPE_PARAM      0x2
#define TYPE_ATTRIBUTE  0x4
#define TYPE_OTHER      0x10
#define MASK_WORD       0xF         /*les 4 premiers bits sont utilisés pour reconnaitre un debut ou fin d'une action spéciale
                                        -> bit 1 : commande
                                        -> bit 2 : parametre
                                        -> bit 3 : attribut
                                        -> bit 4 : à définir
                                    */
// Les commandes disponible
enum TYPE_CMD_NAME
{
    TEST_SPEED_DOWN = 0,
    TEST_SPEED_UP,
    SET_PARAM,
    GET_TIME,
    NB_TYPE_PARAM
};

//Les parametres disponible
enum TYPE_PARAM_NAME
{
	START = 0,
    CONTINUE,
    STOP,
    MODIFY
};

typedef struct
{
    uint8_t type;       //cmd, param, attr...
    uint8_t length;
    char *start;
}c_word_t;

typedef struct
{
    c_word_t word[MAX_WORD];
    uint8_t nWord;
    uint8_t length;
    char *start;
    char *end;
}c_msg_t;


void communication_start(char *s, int length, ble_nus_t *p_nus);
void display_param(c_msg_t *p_msg);
uint8_t parse(c_msg_t *p_msg);
uint8_t skip_separator(char *s);
uint8_t get_type(char *s, uint8_t old_type);    //obtient le type du mot suivant la position dans la chaine


