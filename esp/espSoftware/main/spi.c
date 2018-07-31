/*
 * spi.c
 *
 *  Created on: 2 mar 2018
 *      Author: Kuba
 */


#include "driver/spi_master.h"
#include "soc/gpio_reg.h"
#include "esp_spi_flash.h"
#include "spi.h"
#include <stddef.h>
#include <string.h>
#include "esp_log.h"

#define GPIO_MOSI 23
#define GPIO_MISO 19
#define GPIO_SCLK 18
#define GPIO_CSAG 13
#define GPIO_CSM 4

spi_device_handle_t handleAG; //create slave instance
spi_device_handle_t handleM;



void remove_spi_device(spi_device_handle_t handle)
{
	spi_bus_remove_device(handle);
}

void spi_send_AG(const uint8_t *data, size_t data_length)
{
	spi_transaction_t t;
	memset(&t, 0, sizeof(spi_transaction_t));
	t.length=data_length*8;
	t.tx_buffer=data;
	t.rx_buffer=NULL;
	spi_device_transmit(handleAG, &t);  //Transmit!
}

void spi_send_M(const uint8_t *data, size_t data_length) // 255 bytes is max length, for more change uint8_t data_length to ex. int data_length
{
	spi_transaction_t t;
	memset(&t, 0, sizeof(spi_transaction_t));
	t.length=data_length*8;
	t.tx_buffer=data;
	t.rx_buffer=NULL;
	spi_device_transmit(handleM, &t);
}

void spi_receive_AG(uint8_t *recdata, size_t recdata_length)
{
	spi_transaction_t t;
	memset(&t, 0, sizeof(spi_transaction_t));
	t.length=recdata_length*8;
	t.tx_buffer=NULL;
	t.rx_buffer=recdata;
	spi_device_transmit(handleAG, &t);
}

void spi_receive_M(uint8_t *recdata, size_t recdata_length)
{
	spi_transaction_t t;
	memset(&t, 0, sizeof(spi_transaction_t));
	t.length=recdata_length*8;
	t.tx_buffer=NULL;
	t.rx_buffer=recdata;
	spi_device_transmit(handleM, &t);
}

void spi_init()
{
	esp_err_t ret;
	/*
	spi_bus_config_t busconfiguration={
			.mosi_io_num=GPIO_MOSI,
			.miso_io_num=GPIO_MISO,
			.sclk_io_num=GPIO_SCLK,
			.quadwp_io_num=-1,
			.quadhd_io_num=-1
	};
	 */
	/*
	spi_bus_config_t bus_config;
	bus_config.sclk_io_num = GPIO_SCLK; // CLK
	bus_config.mosi_io_num = GPIO_MOSI; // MOSI
	bus_config.miso_io_num = GPIO_MISO; // MISO
	bus_config.quadwp_io_num = -1; // Not used
	bus_config.quadhd_io_num = -1; // Not used
	ESP_LOGI("SPI", "... Initializing bus.");
	ESP_ERROR_CHECK(spi_bus_initialize(HSPI_HOST, &bus_config, 1));
	 */


	/*
	spi_device_interface_config_t deviceconfigurationAG={
			//.command_bits=0,
			//.address_bits=0,
			//.dummy_bits=0,
			.clock_speed_hz=10000,
			//.duty_cycle_pos=128,        //50% duty cycle
			.mode=0,
			.spics_io_num=GPIO_CSAG,
			//.cs_ena_posttrans=3,        //Keep the CS low 3 cycles after transaction, to stop slave from missing the last bit when CS has less propagation delay than CLK
			.queue_size=1

	};
	 */
	//spi_device_handle_t spi;
	spi_bus_config_t buscfg={
			.miso_io_num=GPIO_MISO,
			.mosi_io_num=GPIO_MOSI,
			.sclk_io_num=GPIO_SCLK,
			.quadwp_io_num=-1,
			.quadhd_io_num=-1
	};
	spi_device_interface_config_t AGcfg={
			.clock_speed_hz=1000*1000,               //Clock out at 1 MHz
			.mode=0,                                //SPI mode 0
			.spics_io_num=GPIO_CSAG,               //CS pin
			.queue_size=1,                          //We want to be able to queue 1 transaction at a time
	};

	spi_device_interface_config_t Mcfg={
			.clock_speed_hz=1000*1000,               //Clock out at 1 MHz
			.mode=0,                                //SPI mode 0
			.spics_io_num=GPIO_CSM,               //CS pin
			.queue_size=1,                         //We want to be able to queue 1 transaction at a time
	};

	//Initialize the SPI bus
	ret=spi_bus_initialize(HSPI_HOST, &buscfg, 0);
	assert(ret==ESP_OK);
	//Attach the LCD to the SPI bus
	ret=spi_bus_add_device(HSPI_HOST, &AGcfg, &handleAG);
	ret=spi_bus_add_device(HSPI_HOST, &Mcfg, &handleM);
	assert(ret==ESP_OK);
	ESP_LOGI("SPI","AG and M added");
	//spi_device_interface_config_t devcfgM=config_spi_M();

	//Initialize the SPI bus and add the device we want to send stuff to.
	//spi_bus_initialize(HSPI_HOST, &busconfiguration, 1);

	//spi_bus_add_device(HSPI_HOST, &deviceconfigurationAG, &handleAG);
	//spi_bus_add_device(HSPI_HOST, &devcfgM, &handleM);

	//xSemaphoreGive(rdySem);

	/*
	char data[3];
	spi_transaction_t trans_desc;
	trans_desc.address = 0;
	trans_desc.command = 0;
	trans_desc.flags = 0;
	trans_desc.length = 3 * 8;

	trans_desc.rxlength = 0;
	trans_desc.tx_buffer = data;
	trans_desc.rx_buffer = data;
	data[0] = 0x12;
	data[1] = 0x34;
	data[2] = 0x56;
	ESP_LOGI("SPI", "... Transmitting.");
	ESP_ERROR_CHECK(spi_device_transmit(spi, &trans_desc));
	ESP_LOGI("SPI", "... Removing device.");
	ESP_ERROR_CHECK(spi_bus_remove_device(spi));
	ESP_LOGI("SPI", "... Freeing bus.");
	ESP_ERROR_CHECK(spi_bus_free(HSPI_HOST));
	ESP_LOGD("SPI", "<< test_spi_task");
	 */
}


void send_line_finish()
{
	spi_transaction_t *rtrans;
	spi_device_get_trans_result(handleAG, &rtrans, portMAX_DELAY);
}




