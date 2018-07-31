/*
 * adc.h
 *
 *  Created on: 15 mar 2018
 *      Author: Kuba
 */

#ifndef MAIN_ADC_H_
#define MAIN_ADC_H_

#define ADC_BUFFER_SIZE 2000 /*!< circular buffer size */
#define ADC1 6 /*!< ADC1 channel 6 is GPIO34 */
#define ADC2 7 /*!< ADC1 channel 7 is GPIO35 */



/**
 *\brief Initialize ADCs with defined pins
 */
void initialize_adcs();


/**
 *\brief functions gets data from adcs, put values into cyclic buffers and update cyclic buffer head, line "adc_head%=ADC_BUFFER_SIZE" checks if adc_head is bigger than size and if it is then it's reload it to correct value smaller than buffer size
 */
void adcgetdata();



void setPinLevel(int level1, int level2);

#endif /* MAIN_ADC_H_ */
