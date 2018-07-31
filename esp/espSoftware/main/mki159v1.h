/*
 * mki159v1.h
 *
 *  Created on: 2 mar 2018
 *      Author: Kuba
 */

#ifndef MAIN_MKI159V1_H_
#define MAIN_MKI159V1_H_


typedef unsigned char uint8_t;

void initAG();
void initGyro();
void initAccel();
void initMagnet();
void readA();
void readG();
void readM();
void writeAG(uint8_t address, uint8_t data);
void writeM(uint8_t address, uint8_t data);
void setPinLevel(int level1, int level2);

void AllowReadAndSendBySPI();

void test_spi();

#endif /* MAIN_MKI159V1_H_ */
