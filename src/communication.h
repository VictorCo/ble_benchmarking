#ifndef COMMUNICATION_H
#define COMMUNICATION_H

#include <stdint.h>
#include "ble_nus.h"

#define MESSAGE_TYPE_SOF            's'
#define MESSAGE_TYPE_EOF            'e'
#define MESSAGE_TYPE_PARAM          '-'
#define MESSAGE_SEPARATOR           ' '

#define MAX_WORD        0x10        //le maximum de mot possible en une commande

#define TYPE_NULL       0x0
#define TYPE_CMD        0x1
#define TYPE_PARAM      0x2
#define TYPE_ATTRIBUTE  0x4
#define TYPE_OTHER      0x10

#define N_NAME          0x2         //nombre de nom different possible pour un input

#define N_CON_PARAM 4       //nombre de parametre de connexion           
#define M_CON_INTERVAL_MIN  "interval_min"
#define M_CON_INTERVAL_MAX  "interval_max"
#define M_CON_SLAVE_LATENCY "slave_latency"
#define M_CON_TIMEOUT       "timeout"

#define RESULT_MSG_STOP_TIMER       "Stop timer"
#define RESULT_MSG_GET_TIME_UP      "UP get time : "
#define RESULT_MSG_GET_TIME_DOWN    "DOWN get time : "
#define RESULT_MSG_GET_TIME_BYTE    "Octet : "


// Les commandes disponible
enum TYPE_CMD_NAME
{
    TEST_SPEED_DOWN = 0,
    TEST_SPEED_UP,
    SET_PARAM,
    GET_TIME,
    GET_PARAMS,
    GET_CONNECTION_SECURITY,
    N_TYPE_CMD
};

//Les parametres disponible
enum TYPE_PARAM_NAME
{
    START = N_TYPE_CMD,
    CONTINUE,
    STOP,
    MODIFY,
    N_TYPE_PARAM,
    N_INPUT = N_TYPE_PARAM
};


typedef struct
{
    uint8_t type;
    uint8_t n_attribute;
    const char* const name[N_NAME];  //une commande, parametre attribut peut avoir N_NAME nom(s)
                                     //pour le moment : son nom complet + un raccourci
}c_def_input;

typedef struct
{
    char *param_name;
    uint16_t *param_value;
}c_def_conn_param;


typedef struct
{
    uint8_t type;       //cmd, param, attr...
    uint8_t name;       //test_speed_down, continue...
    uint16_t length;
    char *start;
}c_word_t;

typedef struct
{
    c_word_t word[MAX_WORD];
    uint8_t nWord;
    uint16_t length;
    char *start;
    char *end;
}c_msg_t;

typedef struct
{
    uint32_t timestamp_start;     //valeur du timestamp au moment de start un test
    uint32_t timestamp;           //valeur du timestamp entre start et stop
    uint8_t timer_name;           //TEST_SPEED_UP ou TEST_SPEED_DOWN
    uint16_t byte_number;         //nombre d'octet echangé dans une instance de test complete
    uint16_t remaining_byte;      //nombre d'octets restant a envoyer en DOWN dans le cas BLE_ERROR_NO_TX_PACKETS
    uint16_t total_byte;          //nombre total d'octet à transferer (utilisé dans le test_down)
    bool b_timestamp_start;       //savoir si un timer est deja lance
    bool b_timestamp_available;   //un timer est disponible pour la commande GET_TIME ? 
                                  //si un autre timer est lance on laisse la valeur de l'ancien timer disponible.
    bool b_stop_down;             //on stop le timer down avant de recevoir toutes les données ?
    
    ble_gap_conn_params_t conn_params;
}c_info_t;


void communication_start(char *s, int length, ble_nus_t *p_nus);
uint8_t parse(c_msg_t *p_msg);
uint8_t skip_separator(char *s);
uint8_t get_type(char *s, uint8_t old_type);
bool check_input_word_exist(c_msg_t *p_msg_t);
bool check_argument_number(const c_word_t *p_param, uint8_t nb_argument); //regarde si un parametre passe contient dans le message transmis le bon nombre d'arguments
void communication_run(const c_msg_t *p_msg_t, ble_nus_t *p_nus);
void continue_send_byte_up(ble_nus_t *p_nus);
void communication_update_params(ble_evt_t *p_evt);
int8_t get_conn_param(char *s);

#endif


