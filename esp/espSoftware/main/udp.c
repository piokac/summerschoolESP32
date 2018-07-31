/*
 * udp.c
 *
 *  Created on: 9 sie 2017
 *      Author: Kuba
 */
//#include "adc1_test.c"
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include <sys/socket.h>
#include "driver/timer.h"
#include "udp.h"
#include "timer.h"
#include "adc.h"
#include "esp_err.h"

EventGroupHandle_t udp_event_group;

int total_data = 0;
int success_pack = 0;
extern volatile int connection_mode;
extern volatile int sending_data;
extern int TIMER_INTERVAL1_SEC;

int mysocket; //not static because used in main
static int TCPsocket = 0; //static because used only in udp.c
static struct sockaddr_in server_addr; //static because used only in udp.c

struct sockaddr_in remote_addr;//not static because used in main
//static unsigned int socklen;//not static because used in main



static esp_err_t event_handler(void *ctx, system_event_t *event)
{
	switch(event->event_id) {
	case SYSTEM_EVENT_STA_START:
		esp_wifi_connect();
		break;
	case SYSTEM_EVENT_STA_DISCONNECTED:
		esp_wifi_connect();
		xEventGroupClearBits(udp_event_group, WIFI_CONNECTED_BIT);
		break;
	case SYSTEM_EVENT_STA_CONNECTED:
		break;
	case SYSTEM_EVENT_STA_GOT_IP:
		ESP_LOGI(TAG, "event_handler:SYSTEM_EVENT_STA_GOT_IP!");
		ESP_LOGI(TAG, "got ip:%s\n",
				ip4addr_ntoa(&event->event_info.got_ip.ip_info.ip));
		xEventGroupSetBits(udp_event_group, WIFI_CONNECTED_BIT);
		break;
	case SYSTEM_EVENT_AP_STACONNECTED:
		ESP_LOGI(TAG, "station:"MACSTR" join,AID=%d\n",
				MAC2STR(event->event_info.sta_connected.mac),
				event->event_info.sta_connected.aid);
		xEventGroupSetBits(udp_event_group, WIFI_CONNECTED_BIT);
		break;
	case SYSTEM_EVENT_AP_STADISCONNECTED:
		ESP_LOGI(TAG, "station:"MACSTR"leave,AID=%d\n",
				MAC2STR(event->event_info.sta_disconnected.mac),
				event->event_info.sta_disconnected.aid);
		xEventGroupClearBits(udp_event_group, WIFI_CONNECTED_BIT);
		break;
	default:
		break;
	}
	return ESP_OK;
}


//wifi_init_sta
void wifi_init_sta()
{
	udp_event_group = xEventGroupCreate();

	tcpip_adapter_init();
	ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL) );

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	wifi_config_t wifi_config = {
			.sta = {
					.ssid = EXAMPLE_DEFAULT_SSID,
					.password = EXAMPLE_DEFAULT_PWD
			},
	};

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
	ESP_ERROR_CHECK(esp_wifi_start() );

	ESP_LOGI(TAG, "wifi_init_sta finished.");
	ESP_LOGI(TAG, "connect to ap SSID:%s password:%s \n",
			EXAMPLE_DEFAULT_SSID,EXAMPLE_DEFAULT_PWD);
}
//wifi_init_softap
void wifi_init_softap()
{
	udp_event_group = xEventGroupCreate();

	tcpip_adapter_init();
	ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	ESP_ERROR_CHECK(esp_wifi_init(&cfg));
	wifi_config_t wifi_config = {
			.ap = {
					.ssid = EXAMPLE_DEFAULT_SSID,
					.ssid_len=0,
					.max_connection=EXAMPLE_MAX_STA_CONN,
					.password = EXAMPLE_DEFAULT_PWD,
					.authmode=WIFI_AUTH_WPA_WPA2_PSK
			},
	};
	if (strlen(EXAMPLE_DEFAULT_PWD) ==0) {
		wifi_config.ap.authmode = WIFI_AUTH_OPEN;
	}

	ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
	ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_config));
	ESP_ERROR_CHECK(esp_wifi_start());

	ESP_LOGI(TAG, "wifi_init_softap finished.SSID:%s password:%s \n",
			EXAMPLE_DEFAULT_SSID, EXAMPLE_DEFAULT_PWD);
}

//create a udp server socket. return ESP_OK:success ESP_FAIL:error
esp_err_t create_udp_server()
{
	ESP_LOGI(TAG, "create_udp_server() port:%d", EXAMPLE_DEFAULT_PORT);
	mysocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (mysocket < 0) {
		show_socket_error_reason(mysocket);
		return ESP_FAIL;
	}
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(EXAMPLE_DEFAULT_PORT);
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(mysocket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		show_socket_error_reason(mysocket);
		close(mysocket);
		return ESP_FAIL;
	}
	return ESP_OK;
}

//create a udp client socket. return ESP_OK:success ESP_FAIL:error
esp_err_t create_udp_client()
{
	ESP_LOGI(TAG, "create_udp_client()");
	ESP_LOGI(TAG, "connecting to %s:%d",
			EXAMPLE_DEFAULT_SERVER_IP, EXAMPLE_DEFAULT_PORT);
	mysocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (mysocket < 0) {
		show_socket_error_reason(mysocket);
		return ESP_FAIL;
	}
	/*for client remote_addr is also server_addr*/
	remote_addr.sin_family = AF_INET;
	remote_addr.sin_port = htons(EXAMPLE_DEFAULT_PORT);
	remote_addr.sin_addr.s_addr = inet_addr(EXAMPLE_DEFAULT_SERVER_IP);

	return ESP_OK;
}

esp_err_t create_tcp_client()
{
	ESP_LOGI(TAGTCP, "client socket....serverip:port=%s:%d\n",
			EXAMPLE_DEFAULT_SERVER_IP, EXAMPLE_DEFAULT_TCPPORT );
	TCPsocket = socket(AF_INET, SOCK_STREAM, 0);
	if (TCPsocket < 0) {
		show_socket_error_reason(TCPsocket);
		return ESP_FAIL;
	}
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(EXAMPLE_DEFAULT_TCPPORT );
	server_addr.sin_addr.s_addr = inet_addr(EXAMPLE_DEFAULT_SERVER_IP);
	ESP_LOGI(TAGTCP, "connecting to server...");
	if (connect(TCPsocket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		show_socket_error_reason(TCPsocket);
		return ESP_FAIL;
	}
	ESP_LOGI(TAGTCP, "connect to server success!");
	return ESP_OK;
}

void onoffSending()
{
	sending_data=!sending_data;
}


void sendUDP_recvTCP_data(void *pvParameters)
{
	uint8_t* TCPdatabuff = (uint8_t*) malloc(BUF_SIZE);
	//char databuff[EXAMPLE_DEFAULT_PKTSIZE];
	while (1) {
		int len = recv(TCPsocket, TCPdatabuff, BUF_SIZE, 0);
		if(len>0)
		{
			char cmd;
			int size;
			ESP_LOGI(TAGTCP, "dataget");
			connection_mode=1;
			for(int i=0;i<len;i++)
			{
				if((char)TCPdatabuff[i]=='#')
				{
					sscanf((char *)&TCPdatabuff[i],"#%d %c",&size,&cmd);
					if((char)TCPdatabuff[i+size-1]== '*')
					{
						switch(cmd)
						{
						case 'f':
						{
							sscanf((char *)&TCPdatabuff[i],"#%*d %*c %d*",&TIMER_INTERVAL1_SEC);
							//sprintf(test_msg,"%d - %c - %d\n",size,cmd,TIMER_INTERVAL1_SEC); can't use data table( data is char table), because then we overwrite information in this table
							refresh_timer(TIMER_GROUP_0,TIMER_1); //if frequency(TIMER_INTERVAL1_SEC) has changed, restart timer
							break;
						}
						case 'g':
						{
							onoffSending();
							ESP_LOGI("WIFI", "SENDING STATUS: %d \n", sending_data);




							/*while(adc_tail!=adc_head)
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
								// sending by udp
								len = sendto(mysocket, frame, strlen(frame), 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr));
								adc_tail++;
								adc_tail%=ADC_BUFFER_SIZE;
							}
							 */
							break;
						}
						case 's':
						{
							int pin1level=0;
							int pin2level=0;
							sscanf((char *)&TCPdatabuff[i],"#%*d %*c %d %d*",&pin1level,&pin2level);
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
			}
		}
	}
}


int get_socket_error_code(int socket)
{
	int result;
	u32_t optlen = sizeof(int);
	if(getsockopt(socket, SOL_SOCKET, SO_ERROR, &result, &optlen) == -1) {
		if(socket==mysocket)
		{
			ESP_LOGE(TAG, "getsockopt failed");
		}
		else if(socket==TCPsocket)
		{
			ESP_LOGE(TAGTCP, "getsockopt failed");
		}
		return -1;
	}
	return result;
}

int show_socket_error_reason(int socket)
{
	int err = get_socket_error_code(socket);
	if(socket==mysocket)
	{
		ESP_LOGW(TAG, "socket error %d %s", err, strerror(err));
	}
	else if(socket==TCPsocket)
	{
		ESP_LOGW(TAGTCP, "socket error %d %s", err, strerror(err));
	}
	return err;
}

int check_connected_socket()
{
	int ret;
	ESP_LOGD(TAG, "check connect_socket");
	ret = get_socket_error_code(mysocket);
	if(ret != 0) {
		ESP_LOGW(TAG, "socket error %d %s", ret, strerror(ret));
	}
	return ret;
}

void close_socket()
{
	close(mysocket);
	close(TCPsocket);
}
