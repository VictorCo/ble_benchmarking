#include "ble.h"
#include "ble_nus.h"

#define SIZE_QUEUED 512
#define SIZE_PACKET 24
#define SIZE_HEADER 6
#define SIZE_DATA SIZE_PACKET - SIZE_HEADER
#define START_PACKET 0xE

typedef struct
{
	uint16_t	len;
	uint8_t 	*data;
}l_value_t;



void long_packet_on_ble_event(ble_nus_t *p_ble_nus, ble_evt_t * p_ble_evt);
void displayRx(ble_nus_t *p_ble_nus);
	

