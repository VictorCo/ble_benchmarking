#include "long_packet.h"
#include "SEGGER_RTT.h"

ble_user_mem_block_t m_user_block;
uint8_t m_queued[SIZE_QUEUED];

void long_packet_on_ble_event(ble_nus_t *p_ble_nus, ble_evt_t * p_ble_evt)
{
	uint32_t err_code;
	
	m_user_block.p_mem = m_queued;
	m_user_block.len = SIZE_QUEUED;
	
	switch (p_ble_evt->header.evt_id)
  {
			case BLE_EVT_USER_MEM_REQUEST :
				SEGGER_RTT_printf(0, "Request\n");
				SEGGER_RTT_printf(0, "taille = %d\n", p_ble_evt->evt.common_evt.params.user_mem_request.type);
				err_code = sd_ble_user_mem_reply(p_ble_nus->conn_handle, &m_user_block);
				SEGGER_RTT_printf(0, "Mem req : %d\n", err_code);
			break;
			
			case BLE_EVT_USER_MEM_RELEASE :
				SEGGER_RTT_printf(0, "Release\n");
				SEGGER_RTT_printf(0, "taille = %d\n", *p_ble_evt->evt.common_evt.params.user_mem_release.mem_block.p_mem);
			
			default :
			break;
	}

}

void getRx(ble_nus_t *p_ble_nus)
{
	uint32_t err_code;
	ble_gatts_value_t p_ble_gatts_value;
	err_code = sd_ble_gatts_value_get(p_ble_nus->conn_handle, p_ble_nus->tx_handles.value_handle, &p_ble_gatts_value);
	SEGGER_RTT_printf(0, "Valeur Rx : %s\nErr Code : %d\n", p_ble_gatts_value.p_value, err_code);
	
}
