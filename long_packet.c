#include "long_packet.h"
#include "SEGGER_RTT.h"
#include <string.h>

ble_user_mem_block_t m_user_block;
uint8_t m_queued[SIZE_QUEUED];

void long_packet_on_ble_event(ble_nus_t *p_ble_nus, ble_evt_t * p_ble_evt)
{
	char data[SIZE_QUEUED];
	uint16_t i;
	uint16_t len = 0;
	
	uint16_t value_offset;
	uint16_t value_len;
	
	m_user_block.p_mem = m_queued;
	m_user_block.len = SIZE_QUEUED;
	
	switch(p_ble_evt->header.evt_id)
	{
		case BLE_EVT_USER_MEM_REQUEST :
			SEGGER_RTT_printf(0, "Request\n");
			sd_ble_user_mem_reply(p_ble_nus->conn_handle, &m_user_block);
			break;
			
		case BLE_EVT_USER_MEM_RELEASE :
			SEGGER_RTT_printf(0, "Release\n");
			memset(data, 0, SIZE_QUEUED);
			for(i = 0; m_queued[i] != 0; i += SIZE_HEADER + value_len)
			{
				value_offset = m_queued[i + MEM_OFFSET];
				value_len = m_queued[i + MEM_LEN];
				memcpy(data + value_offset, &m_queued[i + SIZE_HEADER], value_len);
				len += value_len;
			}
					
			p_ble_nus->data_handler(p_ble_nus, (uint8_t*)data, len);
			break;
			
		default :
				
			break;	
	}
}

void displayRx(ble_nus_t *p_ble_nus)
{
	uint32_t err_code;
	ble_gatts_value_t p_ble_gatts_value;
	err_code = sd_ble_gatts_value_get(p_ble_nus->conn_handle, p_ble_nus->tx_handles.value_handle, &p_ble_gatts_value);
	SEGGER_RTT_printf(0, "Valeur Rx : %s\nErr Code : %d\n", p_ble_gatts_value.p_value, err_code);
}
