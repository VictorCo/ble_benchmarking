#include "ble.h"
#include "ble_nus.h"

#define SIZE_QUEUED         128
#define SIZE_HEADER           6

#define MEM_OFFSET          0x2
#define MEM_LEN             0x4

typedef struct
{
    uint16_t    len;
    uint8_t     *data;
}l_value_t;



void long_packet_on_ble_event(ble_nus_t *p_ble_nus, ble_evt_t * p_ble_evt);
void send_long_packet(ble_nus_t *p_ble_nus, char *s, int length);
void displayRx(ble_nus_t *p_ble_nus);
	

