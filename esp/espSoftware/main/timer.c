/*
 * timer.c
 *
 *  Created on: 15 mar 2018
 *      Author: Kuba
 */

#include <stdio.h>
#include "esp_types.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "soc/timer_group_struct.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"
#include "timer.h"
#include "adc.h"
#include "mki159v1.h"


xQueueHandle timer_queue;

int TIMER_INTERVAL1_SEC=20;

typedef struct {
	int type;                  /*!< event type */
	int group;                 /*!< timer group */
	int idx;                   /*!< timer number */
	uint64_t counter_val;      /*!< timer counter value */
	double time_sec;           /*!< calculated time from counter value */
} timer_event_t;

/**
 *
 *\brief function executed when alarm hits end value(interval value) used to get data from adc
 */
void IRAM_ATTR adc1timer1alarm(void *para)
{
	int timer_idx = (int) para;
	uint32_t intr_status = TIMERG0.int_st_timers.val;
	timer_event_t evt;
	TIMERG0.hw_timer[timer_idx].update = 1;
	/*We don't call a API here because they are not declared with IRAM_ATTR*/
	TIMERG0.int_clr_timers.t1 = 1;
	uint64_t timer_val = ((uint64_t) TIMERG0.hw_timer[timer_idx].cnt_high) << 32
			| TIMERG0.hw_timer[timer_idx].cnt_low;
	double time = (double) timer_val / (TIMER_BASE_CLK / TIMERG0.hw_timer[timer_idx].config.divider);
	/*Post an event to out example task*/
	evt.type = TEST_WITH_RELOAD;
	evt.group = 0;
	evt.idx = timer_idx;
	evt.counter_val = timer_val;
	evt.time_sec = time;
	xQueueSendFromISR(timer_queue, &evt, NULL);
	/*For a auto-reload timer, we still need to set alarm_en bit if we want to enable alarm again.*/
	TIMERG0.hw_timer[timer_idx].config.alarm_en = 1;
	//adcgetdata();
	AllowReadAndSendBySPI();
}



void refresh_timer(int timerGroup, int TimerIndex)
{
	timer_pause(timerGroup, TimerIndex); //stopping timer
	timer_set_counter_value(timerGroup, TimerIndex, 0x00000000ULL); // setting starting value
	/*Set alarm value*/
	timer_set_alarm_value(timerGroup, TimerIndex, (double)(TIMER_INTERVAL1_SEC/DIVIDER_TO_MS) * TIMER_SCALE); // setting a time when timer hits end value(when timer hits end value it start a alarm task)
	timer_start(timerGroup, TimerIndex); //starting timer
}


/**
 *
 *\brief initiation of timer, important to select function called after timer period elapesed and this period
 */
void example_tg0_timer1_init()
{
	int timer_group = TIMER_GROUP_0;
	int timer_idx = TIMER_1;
	timer_config_t config;
	config.alarm_en = 1;
	config.auto_reload = 1;
	config.counter_dir = TIMER_COUNT_UP;
	config.divider = TIMER_DIVIDER;
	config.intr_type = TIMER_INTR_SEL;
	config.counter_en = TIMER_PAUSE;
	/*Configure timer*/
	timer_init(timer_group, timer_idx, &config);
	/*Stop timer counter*/
	timer_pause(timer_group, timer_idx);
	/*Load counter value */
	timer_set_counter_value(timer_group, timer_idx, 0x00000000ULL);
	/*Set alarm value*/
	timer_set_alarm_value(timer_group, timer_idx, (double)(TIMER_INTERVAL1_SEC/DIVIDER_TO_MS) * TIMER_SCALE);//TIMER_INTERVAL1_SEC * TIMER_SCALE is a end value
	/*Enable timer interrupt*/
	timer_enable_intr(timer_group, timer_idx);
	/*Set ISR handler*/
	timer_isr_register(timer_group, timer_idx, adc1timer1alarm, (void*) timer_idx, ESP_INTR_FLAG_IRAM, NULL); // timer_group0_isr is a alarm function(executed when timer hits end value)
	/*Start timer counter*/
	timer_start(timer_group, timer_idx);
}


void initializeTIMER()
{
	timer_queue = xQueueCreate(10, sizeof(timer_event_t));
	example_tg0_timer1_init();//first initiation of the timer(with auto-reload - after hitting end value it starts counting from 0)
}
