#include "ble.h"
#include "ble_gap.h"
#include "ble_nus.h"
#include "SEGGER_RTT.h"
#include "nrf_log.h"

#include "security_mode.h"

void security_mode_on_ble_event(ble_nus_t *p_ble_nus, ble_evt_t * p_ble_evt)
{
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_SEC_PARAMS_REQUEST :
//            sd_ble_gap_sec_params_reply(p_ble_nus->conn_handle,
//                                                sec_status,
//                                                p_ble_evt->evt.gap_evt.params.sec_params_request.peer_params,
//                                                ble_gap_sec_keyset_t const *  	p_sec_keyset 
//                                            ) 	
            NRF_LOG_PRINTF("demande de securite\n");
            break;
        
        default :
            break;
        
    }

}
