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

	timer_start();
	app_timer_cnt_get(&m_timer_value);
    APP_ERROR_CHECK(err_code);
	return m_timer_value;
}

void timer_start(void)
{
	app_timer_start(m_timer_id, TIMER_INTERVAL, NULL);
}

void timer_restart(void)
{
	app_timer_stop(m_timer_id);
	timer_start();
}

void timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
}

uint32_t timer_update(void)
{
	uint32_t timer_old = m_timer_value;
    uint32_t ms = m_timer_value/(APP_TIMER_CLOCK_FREQ/1000);
	app_timer_cnt_get(&m_timer_value);
	SEGGER_RTT_printf(0, 
                      "timer : %d\nms : %d\n",
                      ms/1000, ms);
	
	return m_timer_value - timer_old;
}



