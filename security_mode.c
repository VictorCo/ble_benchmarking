#include <stdlib.h>
#include <string.h>
#include "ble.h"
#include "ble_gap.h"
#include "ble_nus.h"
#include "SEGGER_RTT.h"
#include "nrf_log.h"

#include "security_mode.h"

void security_mode_on_ble_event(ble_nus_t *p_ble_nus, ble_evt_t * p_ble_evt)
{
    ble_gap_sec_params_t sec;
    uint32_t err_code;
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_SEC_PARAMS_REQUEST :
            
            memset(&sec, 0, sizeof(ble_gap_sec_params_t));

            sec.bond         = SEC_PARAM_BOND;
            sec.mitm         = SEC_PARAM_MITM;
            sec.io_caps      = SEC_PARAM_IO_CAPABILITIES;
            sec.oob          = SEC_PARAM_OOB;
            sec.min_key_size = SEC_PARAM_MIN_KEY_SIZE;
            sec.max_key_size = SEC_PARAM_MAX_KEY_SIZE;
            err_code = sd_ble_gap_sec_params_reply(p_ble_nus->conn_handle,
                                        BLE_GAP_SEC_STATUS_SUCCESS,
                                        &sec,
                                        NULL 
                                            ); 	
            NRF_LOG_PRINTF("demande de securite\n");
            NRF_LOG_PRINTF("sec err_code : %d\n", err_code);
            break;
        
        default :
            break;
        
    }

}
