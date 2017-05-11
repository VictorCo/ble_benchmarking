#include "timer_packet.h"
#include "app_timer.h"
#include "SEGGER_RTT.h"

APP_TIMER_DEF(m_timer_id);
uint32_t m_timer_value;

uint32_t timer_init(void)
{
    uint32_t err_code;

    err_code = app_timer_create(&m_timer_id,
                                APP_TIMER_MODE_REPEATED, 
                                timeout_handler);

    //timer_start();
    APP_ERROR_CHECK(err_code);
    return err_code;
}

void timer_start(void)
{
    uint32_t err_code;
    err_code = app_timer_start(m_timer_id, TIMER_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
}

void timer_stop(void)
{
    uint32_t err_code;
    err_code = app_timer_stop(m_timer_id);
    APP_ERROR_CHECK(err_code);
}

void timer_restart(void)
{
    timer_stop();
    timer_start();
}

void timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
}

uint32_t timer_get_ticks(void)
{
    app_timer_cnt_get(&m_timer_value);
    return m_timer_value;
}

uint32_t timer_ticks_to_ms(uint32_t ticks)
{
    return ticks/(APP_TIMER_CLOCK_FREQ/1000);
}


