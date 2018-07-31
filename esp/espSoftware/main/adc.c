/*
 * adc.c
 *
 *  Created on: 15 mar 2018
 *      Author: Kuba
 */
#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "adc.h"
#include "mki159v1.h"
#include <esp_log.h>

int adc1data[ADC_BUFFER_SIZE]; /*!< current adc1 value */
int adc2data[ADC_BUFFER_SIZE]; /*!< current adc2 value */
volatile int adc_head = 0; /*!< buffer head */
volatile int adc_tail = 0; /*!< buffer tail */
volatile int i_test=0;
volatile int i_grupa=0;


//For setting pin level
#define pin1 13 /*!< GPIO pin1 */
#define pin2 4  /*!< GPIO pin2 */





void initialize_adcs()
{
	adc1_config_width(ADC_WIDTH_12Bit);
	adc1_config_channel_atten(ADC1,ADC_ATTEN_11db);
	adc1_config_channel_atten(ADC2,ADC_ATTEN_11db);
}

void adcgetdata()
{

	/*
	if(i_test>=255)
	{
		i_grupa++;
		i_test=0;
	}
	if(i_grupa>=255)
	{
	i_grupa=0;
	}
	//readA();
	adc1data[adc_head]=i_test++;
	//adc2data[adc_head]=adc1_get_voltage(ADC2);
	adc2data[adc_head]=i_grupa;
	adc_head++;
	adc_head%=ADC_BUFFER_SIZE;
	*/
}

/**
 *\brief change pin level
 *\param level1 pin1 level
 *\param level2 pin2 level
 */
void setPinLevel(int level1, int level2)
{
	gpio_pad_select_gpio(pin1);
	gpio_set_direction(pin1, GPIO_MODE_OUTPUT);
	gpio_set_level(pin1, level1);
	//ESP_LOGI("PIN13",": %d", level1);
	gpio_pad_select_gpio(pin2);
	gpio_set_direction(pin2, GPIO_MODE_OUTPUT);
	gpio_set_level(pin2, level2);
}
