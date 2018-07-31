/*
 * mki159v1.c
 *
 *  Created on: 2 mar 2018
 *      Author: Kuba
 */
#include "mki159v1.h"
#include "spi.h"
#include <string.h>
#include "esp_log.h"
#include "adc.h"
//#include "spi.c"

#define CTRL_REG1_G 0b00010000 //0x10
#define CTRL_REG2_G 0b00010001 //0x11
#define CTRL_REG3_G 0b00010010 //0x12
#define CTRL_REG4   0b00011110 //0x1E
#define CTRL_REG5_XL 0b00011111 //0x1F
#define CTRL_REG6_XL 0b00100000 //0x20
#define CTRL_REG7_XL 0b00100001 //0x21
#define CTRL_REG9 0b00100011 //0x22
#define CTRL_REG1_M 0b00100000 //0x20
#define CTRL_REG2_M 0b00100001 //0x21
#define CTRL_REG3_M 0b00100010 //0x22
#define CTRL_REG4_M 0b00100011
#define CTRL_REG5_M 0b00100100

uint8_t OUT_X_G[]={0b00011000, 0b00011001}; //0x18 0x19
uint8_t OUT_X_M[]={0b00101000, 0b00101001};//0x28 0x29
//http://www.st.com/resource/en/datasheet/lsm9ds1.pdf
// z rozdzialu register mapping
uint8_t OUT_X_XL[] = {0b00101000, 0b00101001}; //0x28 0x29

volatile uint8_t gyroData[6], accelData[6], magnetData[6];

//extern spi_device_handle_t handleAG; //create slave instance

volatile int spi_send=1; //allow for sending with SPI when 1

volatile int ImuWhichReadingsAreOn[3]={1, 1, 1}; // first element is accelerometer, second is gyroscope, third is magnetometer - 0 mean reading OFF, 1 mean reading ON



//esp gpio 13 - a/g
//esp gpio 4 - m
void initAG()
{
	initGyro();
	initAccel();
}

void initGyro()
{
	//ESP_LOGI("initGyro","initGYROstart");
	writeAG(CTRL_REG1_G, 0xc0);
	writeAG(CTRL_REG2_G, 0x00);
	writeAG(CTRL_REG3_G, 0x00);
	writeAG(CTRL_REG4, 0x38);
	ESP_LOGI("GYRO","inited");
}

void initAccel()
{
	writeAG(CTRL_REG5_XL, 0x38);
	writeAG(CTRL_REG6_XL, 0xb7);
	writeAG(CTRL_REG7_XL, 0x00);
	writeAG(CTRL_REG9, 0x04);
	ESP_LOGI("ACCEL","inited");
}

void initMagnet()
{
	writeM(CTRL_REG1_M, 0x1c);
	writeM(CTRL_REG2_M, 0x60);
	//writeM(CTRL_REG3_M, 0x86);
	writeM(CTRL_REG3_M, 0x80); // Continuous conversion mode
	//writeM(CTRL_REG3_M, 0x82);
	writeM(CTRL_REG4_M, 0x00);
	writeM(CTRL_REG5_M, 0x00);
	ESP_LOGI("MAGNET","inited");
}


void readG()
{
	uint8_t address = (OUT_X_G[0] | 0x80);

	//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);
	setPinLevel(0,1);

	// HAL_SPI_Transmit(&hspi1, &address, 1, -1);
	spi_send_AG(&address, 1);

	//HAL_SPI_Receive(&hspi1, gyroData, 6, -1);
	spi_receive_AG(gyroData,6);

	//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);
	setPinLevel(1,1);
}

void readA()
{
	uint8_t address = (OUT_X_XL[0] | 0x80);

	//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);
	setPinLevel(0,1);

	//HAL_SPI_Transmit(&hspi1, &address, 1, -1);
	spi_send_AG(&address, 1);

	//HAL_SPI_Receive(&hspi1, accelData, 6, -1);
	spi_receive_AG(accelData,6);

	// HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);
	setPinLevel(1,1);
}

void AllowReadAndSendBySPI()
{
	if(spi_send==1)
	{
		spi_send=0;
	}
}

/*
void test_spi()
{

		uint8_t address = (OUT_X_XL[0] | 0x80);
		setPinLevel(0,1);

		spi_transaction_t trans_desc;
		memset(&trans_desc, 0, sizeof(spi_transaction_t));
		//memset(&trans_desc, 0, sizeof(spi_transaction_t));
		//memset(&trans_desc, 0, sizeof(trans_desc));
		trans_desc.address = 0;
		trans_desc.command = 0;
		trans_desc.flags = 0;
		trans_desc.length = 1 * 8;

		trans_desc.rxlength = 0;
		trans_desc.tx_buffer = &address;
		trans_desc.rx_buffer = NULL;

		ESP_LOGI("SPI", "... Transmitting.");
		ESP_ERROR_CHECK(spi_device_transmit(handleAG, &trans_desc));
		//ESP_LOGI("SPI", "... Removing device.");
		//ESP_ERROR_CHECK(spi_bus_remove_device(handleAG));
		//ESP_LOGI("SPI", "... Freeing bus.");
		//ESP_ERROR_CHECK(spi_bus_free(HSPI_HOST));
		ESP_LOGI("SPI", "sent");

		spi_transaction_t trans_recv;
		memset(&trans_recv, 0, sizeof(spi_transaction_t));
		//memset(&trans_recv, 0, sizeof(spi_transaction_t));
		trans_recv.address = 0;
		trans_recv.command = 0;
		trans_recv.flags = 0;
		trans_recv.length = 1 * 8;

		trans_recv.rxlength = 0 ;
		trans_recv.tx_buffer = NULL;
		trans_recv.rx_buffer = accelData;
		ESP_LOGI("SPI", "... Transmitting.");
		ESP_ERROR_CHECK(spi_device_transmit(handleAG, &trans_recv));
		//ESP_LOGI("SPI", "... Removing device.");
		//ESP_ERROR_CHECK(spi_bus_remove_device(handleAG));
		//ESP_LOGI("SPI", "... Freeing bus.");
		//ESP_ERROR_CHECK(spi_bus_free(HSPI_HOST));
		ESP_LOGI("SPI", "received");
		setPinLevel(1,1);
		ESP_LOGI("SPI received: ", "accelData: %hhu : %hhu : %hhu :%hhu : %hhu : %hhu \n", accelData[0],accelData[1],accelData[2],accelData[3],accelData[4],accelData[5]);

}

 */
void readM()
{
	uint8_t address = (OUT_X_M[0] | 0xc0); //by³o 0x80

	//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
	setPinLevel(1,0);

	//HAL_SPI_Transmit(&hspi1, &address, 1, -1);
	spi_send_M(&address, 1);

	//HAL_SPI_Receive(&hspi1, magnetData, 6, -1);
	spi_receive_M(magnetData,6);

	//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
	setPinLevel(1,1);

}

void writeAG(uint8_t address, uint8_t data)
{
	uint8_t newData[2] = {address, data};

	//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);
	setPinLevel(0,1);
	//ESP_LOGI("writeAG","writeAGbeforeSENDING");
	//HAL_SPI_Transmit(&hspi1, newData, 2, -1);
	spi_send_AG(newData, 2);

	//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_SET);
	setPinLevel(1,1);
}

void writeM(uint8_t address, uint8_t data)
{
	uint8_t newData[2] = {address, data};

	//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
	setPinLevel(1,0);

	//HAL_SPI_Transmit(&hspi1, newData, 2, -1);
	spi_send_M(newData, 2);

	//HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
	setPinLevel(1,1);
}
