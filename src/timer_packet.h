#include "app_timer.h"

#define APP_TIMER_PRESCALER             0                                           /**< Value of the RTC1 PRESCALER register. */
#define APP_TIMER_OP_QUEUE_SIZE         4                                           /**< Size of timer operation queues. */
#define APP_TIMER_MAX_TIMERS            0

#define TIMER_INTERVAL APP_TIMER_TICKS(1000, APP_TIMER_PRESCALER)

uint32_t timer_init(void);
void timeout_handler(void * p_context);
void timer_start(void);
void timer_stop(void);
void timer_restart(void);
uint32_t timer_get_ticks(void);
uint32_t timer_ticks_to_ms(uint32_t ticks);



