#ifndef BEE_DEEP_SLEEP_H
#define BEE_DEEP_SLEEP_H

void deep_sleep_task(void *args);
void deep_sleep_register_rtc_timer_wakeup(void);

#endif 