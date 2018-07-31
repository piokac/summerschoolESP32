/*
 * timer.h
 *
 *  Created on: 15 mar 2018
 *      Author: Kuba
 */

#ifndef MAIN_TIMER_H_
#define MAIN_TIMER_H_

#define TIMER_INTR_SEL TIMER_INTR_LEVEL  /*!< Timer level interrupt */
//#define TIMER_GROUP    TIMER_GROUP_1     /*!< Test on timer group 0 */
#define TIMER_DIVIDER   16               /*!< Hardware timer clock divider */
#define TIMER_SCALE    (TIMER_BASE_CLK / TIMER_DIVIDER)  /*!< used to calculate counter value */
#define TEST_WITH_RELOAD   1
#define DIVIDER_TO_MS 1000.0

//void IRAM_ATTR adc1timer1alarm(void *para);
/**
 *\brief update a timer when it's period(interval) changed
 *\param timerGroup
 *\param TimerIndex
 */
void refresh_timer(int timerGroup, int TimerIndex);
void initializeTIMER();
void example_tg0_timer1_init();

#endif /* MAIN_TIMER_H_ */
