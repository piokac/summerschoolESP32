#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/adc.h"
//#include "sdkconfig.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_types.h"
#include "soc/uart_struct.h"
#include "soc/timer_group_struct.h"


#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "esp_err.h"
#include <errno.h>
#include <sys/socket.h>
#include "freertos/event_groups.h"

#include "nvs_flash.h"
#include "driver/periph_ctrl.h"
#include "driver/timer.h"
#include "driver/uart.h"
#include "math.h"


#include "freertos/semphr.h"
#include "esp_event.h"
#include "soc/rtc_cntl_reg.h"
#include "rom/cache.h"
//#include "driver/spi_master.h"
//#include "esp_spi_flash.h"
//#include "soc/gpio_reg.h"
#include "esp_intr_alloc.h"

#include "udp.h"
#include "mki159v1.h"
#include "spi.h"
#include "timer.h"
#include "adc.h"

//-------------------------

//For UART - there are
#define ECHO_TEST_TXD  (1)
#define ECHO_TEST_RXD  (3)
#define ECHO_TEST_RTS  (18)
#define ECHO_TEST_CTS  (19)


//-------------------------

//For Timer


extern int TIMER_INTERVAL1_SEC;  /*!< Timer's period */
//--------------------------


volatile int connection_status=0;
volatile int connection_mode=1; //0-uart , 1 - udp



//volatile int pin1levelTEST=0;

volatile int uart_num;

extern int adc1data[ADC_BUFFER_SIZE]; /*!< current adc1 value */
extern int adc2data[ADC_BUFFER_SIZE]; /*!< current adc2 value */
extern volatile int adc_head; /*!< buffer head */
extern volatile int adc_tail; /*!< buffer tail */
extern int mysocket;
extern int success_pack;
extern int total_data;
extern EventGroupHandle_t udp_event_group;
extern struct sockaddr_in remote_addr;

volatile int sending_data=0;//0 not sending, 1-sending


volatile int tester_i=0;

extern volatile int spi_send;

extern volatile uint8_t accelData[6];
extern volatile uint8_t gyroData[6];
extern volatile uint8_t magnetData[6];
extern volatile int ImuWhichReadingsAreOn[3];
/**
 *
 *\brief this task establish a UDP connection and receive data from UDP
 */
static void udp_conn(void *pvParameters)
{
	ESP_LOGI(TAG, "udp_conn start.");
	xEventGroupWaitBits(udp_event_group, WIFI_CONNECTED_BIT,false, true, portMAX_DELAY);
	/*create udp socket*/
	int socket_ret;
	socket_ret = create_udp_client();
	if(socket_ret == ESP_FAIL) {
		ESP_LOGI(TAG, "create udp socket error,stop.");
		vTaskDelete(NULL);
	}
	ESP_LOGI(TAG, "create tcp client start.");
	create_tcp_client();

	/*initialize adc and timer after connecting*/
	if(connection_status<2)
	{
		connection_status++;
		if(connection_status==2)
		{
			spi_init();
			initAG();
			initMagnet();
			setPinLevel(1,1);
			initialize_adcs();
			initializeTIMER();
		}
	}
	/*create a task to tx/rx data*/
	TaskHandle_t tx_rx_task;
	xTaskCreate(&sendUDP_recvTCP_data, "send_recv_data", 4096, NULL, 4, &tx_rx_task);

	/*waiting udp connected success*/
	xEventGroupWaitBits(udp_event_group, UDP_CONNCETED_SUCCESS,false, true, portMAX_DELAY);
	int bps;
	while (1) {
		total_data = 0;
		//vTaskDelay(3000 / portTICK_RATE_MS);//every 3s
		bps = total_data;

		if (total_data <= 0) {
			int err_ret = check_connected_socket();
			if (err_ret == -1) {  //-1 reason: low level netif error
				ESP_LOGW(TAG, "udp send & recv stop.\n");
				break;
			}
		}
#if EXAMPLE_ESP_UDP_PERF_TX
		ESP_LOGI(TAG, "udp send %d byte per sec! total pack: %d \n", bps, success_pack);
#else
		ESP_LOGI(TAG, "udp recv %d byte per sec! total pack: %d \n", bps, success_pack);
#endif /*EXAMPLE_ESP_UDP_PERF_TX*/
	}
	close_socket();
	vTaskDelete(tx_rx_task);
	vTaskDelete(NULL);
}
/**
 *
 *\brief function used to control ESP settings, it contains communication protocol(frame which start with # and ends with *)
 */

static void uart_communication()
{
	uart_num = UART_NUM_0;
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

	if(connection_status<2)
	{
		connection_status++;
		if(connection_status==2)
		{
			spi_init();
			initAG();
			initMagnet();
			setPinLevel(1,1);
			initialize_adcs();
			initializeTIMER();

		}
	}
	uint8_t* data = (uint8_t*) malloc(BUF_SIZE); // uint8_t address of first element in data table

	while(1)
	{
		int len = uart_read_bytes(uart_num, data, BUF_SIZE, 20 / portTICK_RATE_MS);//lenght of incoming data
		if(len>0)
		{
			char cmd; // command to execute, f means change frequency of blinking
			//char test_msg[100]; //char table used to test, contains a text from sprintf below function
			int size;
			connection_mode=0;
			ESP_LOGI("UART", "received");
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
							onoffSending();
							ESP_LOGI("UART", "SENDING STATUS: %d \n", sending_data);
							//while(adc_tail!=adc_head)
							//{
								//int frameLength=0;
								//char frameData[20];
								//char frame[30];
								//sprintf(frameData," G %d %d*",adc1data[adc_tail],adc2data[adc_tail]);
								//double frameDataLength=0;
								//frameDataLength=strlen(frameData)+1;
								//double LogOfFrameDataLength= log((double)frameDataLength+1.0)/log(10.0); // it equals log of base of 10th of frameDataLength
								//int frameLengthNumberOfDigits = 0; //frame: #frameLengthNumberOfDigits G %d %d*
								//frameLengthNumberOfDigits = floor(LogOfFrameDataLength) + 1.0;
								//frameLength = frameDataLength + frameLengthNumberOfDigits;
								//sprintf(frame,"#%d G %d %d*",frameLength,adc1data[adc_tail],adc2data[adc_tail]);
								//uart_write_bytes(uart_num, frame, strlen(frame));
								//adc_tail++;
								//adc_tail%=ADC_BUFFER_SIZE;
								//onoffSending();
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

						case 'w':
						{
							//sscanf((char *)&data[i], "#%*d %*c %s %s %s %SCNd16*", SSID, wifipassword, addressIP, &udpport);
							//reconnect ....
							break;
						}

						}
						i=i+size-1;
					}
				}
				//uart_write_bytes(uart_num, (const char*) test_msg, strlen((char*)test_msg));//sending test_msg to display by UART
			}
		}
		//if(len==0)
		//{
		//	connection_mode=1;
		//}

	}
}


//kalibruj co sekunde

void app_main()
{

	//A UART read/write example without event queue;

	//xTaskCreate(timer_example_evt_task, "timer_evt_task", 2048, NULL, 5, NULL); thread which is waiting to execute when there is a timer alarm(I didn't create it, I used a alarm function for blinking)
#if EXAMPLE_ESP_WIFI_MODE_AP
	ESP_LOGI(TAG, "EXAMPLE_ESP_WIFI_MODE_AP");
	wifi_init_softap();
#else /*EXAMPLE_ESP_WIFI_MODE_AP*/
	ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
	wifi_init_sta();
#endif
	xTaskCreate(&udp_conn, "udp_conn", 2048, NULL, 10, NULL);
	xTaskCreate(&uart_communication, "uart_communication", 4096, NULL, 5, NULL);
	//char frame[EXAMPLE_DEFAULT_PKTSIZE];

	//problem z readA() kiedy chce to dac w adcdataget;
	//uint16_t test_tab[15];
	//for(int i=0;i<15;i++)
	//{
	//test_tab[i]=i;
	//}
	//while(1)
	//{
	//	send_line_finish();
	//	spi_send_test(test_tab);
	//if(connection_mode==1)
	//{
	//	char frameData[20];
	//	sprintf(frameData," A %c*",accelData[0]);
	//	sendto(mysocket, frameData, 20, 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr));
	//
	//	}
	//}
	//}


	//while(1)
	//{
	//if(spi_send==0)
	//{
	//if((ImuWhichReadingsAreOn[0] || ImuWhichReadingsAreOn[1] || ImuWhichReadingsAreOn[2]))
	//	{
	//char imuFrameData[100];
	//char imuAccelData[25];
	//char imuGyroData[25];
	//char imuMagnetData[25];
	//int j=0;


	//if(ImuWhichReadingsAreOn[0]==1)
	//{
	//readA();
	//sprintf(imuAccelData,"%hhu %hhu %hhu %hhu %hhu %hhu", accelData[0],accelData[1],accelData[2],accelData[3],accelData[4],accelData[5]);
	/*
					char *pointerToArray = imuFrameData + strlen(imuFrameData);
					for (int i = 0 ; i<6 ; i++)
					{
						pointerToArray += sprintf(pointerToArray, " %hhu", accelData[i]);
					}
	 */
	//}
	//	if(ImuWhichReadingsAreOn[1]==1)
	//	{
	//readG();
	//	sprintf(imuGyroData,"%hhu %hhu %hhu %hhu %hhu %hhu",gyroData[0],gyroData[1],gyroData[2],gyroData[3],gyroData[4],gyroData[5]);
	//	char *pointerToArray = imuFrameData + strlen(imuFrameData);
	//	for (int i = 0 ; i<6 ; i++)
	//	{
	//		pointerToArray += sprintf(pointerToArray, " %hhu", gyroData[i]);
	//	}

	//	}
	//	if(ImuWhichReadingsAreOn[2]==1)
	//	{
	//readM();
	//	sprintf(imuMagnetData,"%hhu %hhu %hhu %hhu %hhu %hhu", magnetData[0],magnetData[1],magnetData[2],magnetData[3],magnetData[4],magnetData[5]);
	//char *pointerToArray = imuFrameData + strlen(imuFrameData);
	//for (int i = 0 ; i<6 ; i++)
	//	{
	//	pointerToArray += sprintf(pointerToArray, " %hhu", magnetData[i]);
	//	}
	//}



	//sprintf(imuFrameData,"%s %s %s",imuAccelData, imuGyroData, imuMagnetData);
	//imuFrameData[0]='g';
	while(1)
	{
		//ESP_LOGI("test", "in_while \n");
		if(spi_send==0)
		{
			//ESP_LOGI("test", "in_spi_send \n");
			readA();
			readG();
			readM();

			unsigned char frame[26];
			frame[0]='#';
			frame[1]=26;
			frame[2]=' ';
			frame[3]='G';
			frame[4]=' ';

			for(int i=0;i<6;i++)
			{
				frame[i+5]=accelData[i];
			}

			frame[11]=' ';

			for(int i=0;i<6;i++)
			{
				frame[i+12]=gyroData[i];
			}

			frame[18]=' ';

			for(int i=0;i<6;i++)
			{
				frame[i+19]=magnetData[i];
			}

			frame[25]='*';
			//ESP_LOGI("test", "SD!!!! \n");
			if(sending_data)
			{
				//ESP_LOGI("test", "in_sending_data \n");
				if(connection_mode==1)
				{
					sendto(mysocket, frame, 26, 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr));
					//ESP_LOGI(TAG, "frame: %s \n", frame);
				}
				else if(connection_mode==0)
				{
					 ESP_LOGI("UART:", "CONN_MODE \n");
					//uart_write_bytes(uart_num, (char)frame, BUF_SIZE);
				}
			}

			spi_send=1;
		}


		//if(sending_data)
		//{
			//unsigned char frame[9];
			//if(adc_tail!=adc_head)
			//{

				//bitowe wysylanie, liczby do 1 bajta
				/*
				frame[0]='#';
				frame[1]=9;
				frame[2]=' ';
				frame[3]='A';
				frame[4]=' ';
				frame[5]=adc1data[adc_tail];
				frame[6]=' ';
				frame[7]=adc2data[adc_tail];
				frame[8]='*';
				*/

				//wysylanie znak po znaku

				//int frameLength=0;
				//char frameData[20];
				//sprintf(frameData," G %d %d*",adc1data[adc_tail],adc2data[adc_tail]);
				//double frameDataLength=0;
				//frameDataLength=strlen(frameData)+1;
				//double LogOfFrameDataLength= log((double)frameDataLength+1.0)/log(10.0); // it equals log of base of 10th of frameDataLength
				//int frameLengthNumberOfDigits = 0; //frame: #frameLengthNumberOfDigits G %d %d*
				//frameLengthNumberOfDigits = floor(LogOfFrameDataLength) + 1.0;
				//frameLength = frameDataLength + frameLengthNumberOfDigits;
				//sprintf(frame,"#%d G %d %d*",frameLength,adc1data[adc_tail],adc2data[adc_tail]);
				// sending by udp
				//if(connection_mode==1)
				//{
					//sendto(mysocket, frame, 9, 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr));
					//ESP_LOGI(TAG, "frame: %s \n", frame);

				//}
				//else if(connection_mode==0)
				//{
					//uart_write_bytes(uart_num, frame, strlen(frame));
				//}
				//adc_tail++;
				//adc_tail%=ADC_BUFFER_SIZE;
			//}
		//}
	}

	/*
			if(sending_data)
			{
				if(adc_tail!=adc_head)
				{
					//if(spi_send==0)
					//{
					//	test_spi();
					//wyswietla 0 albo 255, na logic analyzerze sprawdzic czy CS sie zmienia, nie chce wywolac sie w timerze ta funckja
					//	spi_send=1;
					//	}
					//char frame[EXAMPLE_DEFAULT_PKTSIZE];
					int frameLength=0;
					char frameData[20];
					sprintf(frameData," G %d %d*",adc1data[adc_tail],adc2data[adc_tail]);
					double frameDataLength=0;
					frameDataLength=strlen(frameData)+1;
					double LogOfFrameDataLength= log((double)frameDataLength+1.0)/log(10.0); // it equals log of base of 10th of frameDataLength
					int frameLengthNumberOfDigits = 0; //frame: #frameLengthNumberOfDigits G %d %d*
					frameLengthNumberOfDigits = floor(LogOfFrameDataLength) + 1.0;
					frameLength = frameDataLength + frameLengthNumberOfDigits;
					sprintf(frame,"#%d G %d %d*",frameLength,adc1data[adc_tail],adc2data[adc_tail]);
					// sending by udp
					if(connection_mode==1)
					{
						sendto(mysocket, frame, EXAMPLE_DEFAULT_PKTSIZE, 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr));
						//ESP_LOGI(TAG, "frame: %s \n", frame);

					}
					else if(connection_mode==0)
					{
						uart_write_bytes(uart_num, frame, strlen(frame));
					}
					adc_tail++;
					adc_tail%=ADC_BUFFER_SIZE;
				}
			}
	 */
}





