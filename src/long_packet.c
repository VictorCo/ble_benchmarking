#include <string.h>
#include "nrf_error.h"
#include "long_packet.h"
#include "ble.h"
#include "ble_nus.h"
#include "app_error.h"
#include "SEGGER_RTT.h"

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
            NRF_LOG("Request\n");
            sd_ble_user_mem_reply(p_ble_nus->conn_handle, &m_user_block);
            break;
            
        case BLE_EVT_USER_MEM_RELEASE :
            NRF_LOG("Release\n");
            memset(data, 0, SIZE_QUEUED);
            for(i = 0; m_queued[i] != 0; i += SIZE_HEADER + value_len)
            {
                value_offset = *((uint16_t *)&m_queued[i + MEM_OFFSET]);
                value_len = *((uint16_t *)&m_queued[i + MEM_LEN]);
                memcpy(data + value_offset, &m_queued[i + SIZE_HEADER], value_len);
                len += value_len;
            }
            p_ble_nus->data_handler(p_ble_nus, (uint8_t*)data, len);
            break;
            
            
        case BLE_EVT_TX_COMPLETE :
//            p_ble_nus->available_channel++;
        
//            if(p_ble_nus->available_channel == p_ble_nus->max_channel)
//            if(p_ble_nus->available_channel)
            continue_send_byte_up(p_ble_nus);

            break;
        
        case BLE_GATTS_EVT_RW_AUTHORIZE_REQUEST :
            NRF_LOG_PRINTF("Request\n");
        
            break;
            
        default :
                
            break;	
    }
}

void send_long_packet(ble_nus_t *p_nus, char *s, int length)
{
    uint32_t pos = 0;
    uint32_t length_packet;
    uint32_t offset = length;
    //uint32_t err_code;

    for(; pos != length; offset = length - pos)
    {
        length_packet = ( offset > BLE_NUS_MAX_DATA_LEN ) ? BLE_NUS_MAX_DATA_LEN : offset;
        //do{
        //err_code = ble_nus_string_send(p_nus, (uint8_t*)&s[pos], length_packet);
        //}while(err_code == BLE_ERROR_NO_TX_BUFFERS);
        //SEGGER_RTT_printf(0, "send long packet erreur type : %d\n", err_code);
        //APP_ERROR_CHECK(err_code);
        pos += length_packet;
    }
}

void send_dummy_data(ble_nus_t *p_nus, c_info_t *p_info, int length)
{
#define SIZE BLE_NUS_MAX_DATA_LEN
    char *packet[SIZE];
    memset(packet,0x31,SIZE);
    uint16_t i;
    uint8_t extra = length%SIZE;
    uint16_t err_code;
    
    if(extra)
    {
        err_code = ble_nus_string_send(p_nus, (uint8_t*)&packet, extra, true);
        p_info->n_packet++;
    }
    
    if(length > SIZE)
    {
        for(i = 0; i < length; i+=SIZE)
        {
            err_code = ble_nus_string_send(p_nus, (uint8_t *)packet, SIZE, true);
            if(err_code == BLE_ERROR_NO_TX_PACKETS)
            {
                p_info->remaining_byte = length - extra - i;
                p_info->byte_number = p_info->total_byte - p_info->remaining_byte;
                return;
            }
            p_info->n_packet++;
        }
    }
    p_info->byte_number += length;
    p_info->remaining_byte = 0;
}


