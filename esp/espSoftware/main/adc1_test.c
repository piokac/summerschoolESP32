/* ADC1 Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "sdkconfig.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_types.h"
#include "soc/uart_struct.h"
#include "soc/timer_group_struct.h"

#include "nvs_flash.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"
#include "driver/uart.h"
#include "math.h"


//For ADC
#define ADC1 6 /*!< ADC1 channel 6 is GPIO34 */
#define ADC2 7 /*!< ADC1 channel 7 is GPIO35 */

//-------------------------

//For UART - there are
#define ECHO_TEST_TXD  (1)
#define ECHO_TEST_RXD  (3)
#define ECHO_TEST_RTS  (18)
#define ECHO_TEST_CTS  (19)

#define BUF_SIZE (1024)
//-------------------------

//For Timer
#define TIMER_INTR_SEL TIMER_INTR_LEVEL  /*!< Timer level interrupt */
#define TIMER_GROUP    TIMER_GROUP_0     /*!< Test on timer group 0 */
#define TIMER_DIVIDER   16               /*!< Hardware timer clock divider */
#define TIMER_SCALE    (TIMER_BASE_CLK / TIMER_DIVIDER)  /*!< used to calculate counter value */
#define TIMER_FINE_ADJ   (1.4*(TIMER_BASE_CLK / TIMER_DIVIDER)/1000000) /*!< used to compensate alarm value */
//#define TIMER_INTERVAL0_SEC   (3.4179)   /*!< test interval for timer 0 */
//#define TEST_WITHOUT_RELOAD   0   /*!< example of auto-reload mode */
xQueueHandle timer_queue;
#define TEST_WITH_RELOAD   1      /*!< example without auto-reload mode */
#define DIVIDER_TO_MS 1000.0
int TIMER_INTERVAL1_SEC=2;
//--------------------------

//For setting pin level
#define pin1 13
#define pin2 4


#define ADC_BUFFER_SIZE 2000
int adc1data[ADC_BUFFER_SIZE];
int adc2data[ADC_BUFFER_SIZE];
volatile int adc_head = 0;
volatile int adc_tail = 0;

typedef struct {
	int type;                  /*!< event type */
	int group;                 /*!< timer group */
	int idx;                   /*!< timer number */
	uint64_t counter_val;      /*!< timer counter value */
	double time_sec;           /*!< calculated time from counter value */
} timer_event_t;

void initialize_adcs()
{
	adc1_config_width(ADC_WIDTH_12Bit);
	adc1_config_channel_atten(ADC1,ADC_ATTEN_11db);
	adc1_config_channel_atten(ADC2,ADC_ATTEN_11db);
}

void adcgetdata()
{
	adc1data[adc_head]=adc1_get_voltage(ADC1);
	adc2data[adc_head]=adc1_get_voltage(ADC2);
	adc_head++;
	adc_head%=ADC_BUFFER_SIZE;
}

void refresh_timer(int timerGroup, int TimerIndex)
{
	timer_pause(timerGroup, TimerIndex); //stopping timer
	timer_set_counter_value(timerGroup, TimerIndex, 0x00000000ULL); // setting starting value
	/*Set alarm value*/
	timer_set_alarm_value(timerGroup, TimerIndex, (double)(TIMER_INTERVAL1_SEC/DIVIDER_TO_MS) * TIMER_SCALE); // setting a time when timer hits end value(when timer hits end value it start a alarm task)
	timer_start(timerGroup, TimerIndex); //starting timer
}

void setPinLevel(int level1, int level2)
{
	gpio_pad_select_gpio(pin1);
	gpio_set_direction(pin1, GPIO_MODE_OUTPUT);
	gpio_set_level(pin1, level1);
	gpio_pad_select_gpio(pin2);
	gpio_set_direction(pin2, GPIO_MODE_OUTPUT);
	gpio_set_level(pin2, level2);
}

/*
static void timer_example_evt_task(void *arg)
{
	while(1) {
		timer_event_t evt;
		xQueueReceive(timer_queue, &evt, portMAX_DELAY);
		uint64_t timer_val;
		timer_get_counter_value(evt.group, evt.idx, &timer_val);
		double time;
		timer_get_counter_time_sec(evt.group, evt.idx, &time);
		blink();
	}
}
 */

void IRAM_ATTR adc1timer1alarm(void *para)//function executed when alarm hits end value(end value is TIMER_INTERVAL1_SEC * TIMER_SCALE)
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
	adcgetdata();
}


static void example_tg0_timer1_init()
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



static void echo_task()
{
	const int uart_num = UART_NUM_0;
	uart_config_t uart_config = {
			.baud_rate = 115200,
			.data_bits = UART_DATA_8_BITS,
			.parity = UART_PARITY_DISABLE,
			.stop_bits = UART_STOP_BITS_1,
			.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
			.rx_flow_ctrl_thresh = 122,
	};

	uart_param_config(uart_num, &uart_config);

	uart_set_pin(uart_num, ECHO_TEST_TXD, ECHO_TEST_RXD,  UART_PIN_NO_CHANGE,  UART_PIN_NO_CHANGE);

	uart_driver_install(uart_num, BUF_SIZE * 2, 0, 0, NULL, 0);


	uint8_t* data = (uint8_t*) malloc(BUF_SIZE); // uint8_t address of first element in data table

	while(1)
	{
		int len = uart_read_bytes(uart_num, data, BUF_SIZE, 20 / portTICK_RATE_MS);//lenght of incoming data
		char cmd; // command to execute, f means change frequency of blinking
		//char test_msg[100]; //char table used to test, contains a text from sprintf below function
		int size;
		if(len>0)
		{
			for(int i=0;i<len;i++)
			{
				if((char)data[i]=='#')
				{
					sscanf((char *)&data[i],"#%d %c",&size,&cmd);
					if((char)data[i+size-1]== '*')
					{
						switch(cmd)
						{
						case 'f':
						{
							sscanf((char *)&data[i],"#%*d %*c %d*",&TIMER_INTERVAL1_SEC);
							//sprintf(test_msg,"%d - %c - %d\n",size,cmd,TIMER_INTERVAL1_SEC); can't use data table( data is char table), because then we overwrite information in this table
							refresh_timer(TIMER_GROUP_0,TIMER_1); //if frequency(TIMER_INTERVAL1_SEC) has changed, restart timer
							break;
						}
						case 'g':
						{
							while(adc_tail!=adc_head)
							{
								int frameLength=0;
								char frameData[20];
								char frame[30];
								sprintf(frameData," G %d %d*",adc1data[adc_tail],adc2data[adc_tail]);
								double frameDataLength=0;
								frameDataLength=strlen(frameData)+1;
								double LogOfFrameDataLength= log((double)frameDataLength+1.0)/log(10.0); // it equals log of base of 10th of frameDataLength
								int frameLengthNumberOfDigits = 0; //frame: #frameLengthNumberOfDigits G %d %d*
								frameLengthNumberOfDigits = floor(LogOfFrameDataLength) + 1.0;
								frameLength = frameDataLength + frameLengthNumberOfDigits;
								sprintf(frame,"#%d G %d %d*",frameLength,adc1data[adc_tail],adc2data[adc_tail]);
								uart_write_bytes(uart_num, frame, strlen(frame));
								adc_tail++;
								adc_tail%=ADC_BUFFER_SIZE;
							}
							break;
						}
						case 's':
						{
							int pin1level=0;
							int pin2level=0;
							sscanf((char *)&data[i],"#%*d %*c %d %d*",&pin1level,&pin2level);
							setPinLevel(pin1level,pin2level);
							break;
						}
						}
						i=i+size-1;
					}
				}
				//uart_write_bytes(uart_num, (const char*) test_msg, strlen((char*)test_msg));//sending test_msg to display by UART
			}
		}

	}
}


void app_main()
{

	//A UART read/write example without event queue;
	xTaskCreate(echo_task, "uart_echo_task", 1024, NULL, 10, NULL);// I disabled echo message
	timer_queue = xQueueCreate(10, sizeof(timer_event_t));
	initialize_adcs();
	example_tg0_timer1_init();//first initiation of the timer(with auto-reload - after hitting end value it starts counting from 0)
	//xTaskCreate(timer_example_evt_task, "timer_evt_task", 2048, NULL, 5, NULL); thread which is waiting to execute when there is a timer alarm(I didn't create it, I used a alarm function for blinking)
}





