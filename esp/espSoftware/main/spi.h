/*
 * spi.h
 *
 *  Created on: 2 mar 2018
 *      Author: Kuba
 */
#include "driver/spi_master.h"

#ifndef MAIN_SPI_H_
#define MAIN_SPI_H_


void remove_spi_device(spi_device_handle_t handle);
void spi_init();
void spi_send_AG(const uint8_t *data, size_t data_length);
void spi_send_M(const uint8_t *data, size_t data_length);
void spi_receive_AG(uint8_t *recdata,size_t recdata_length);
void spi_receive_M(uint8_t *recdata,size_t recdata_length);
void send_line_finish();
//static void send_test() ;

#endif /* MAIN_SPI_H_ */
