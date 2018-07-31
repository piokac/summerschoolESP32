/*
 * udp.h
 *
 *  Created on: 9 sie 2017
 *      Author: Kuba
 */

#ifndef MAIN_UDP_H_
#define MAIN_UDP_H_


#ifdef __cplusplus
extern "C" {
#endif

/*test options*/
#define EXAMPLE_ESP_WIFI_MODE_AP 0 /*!< TRUE:AP FALSE:STA */
#define EXAMPLE_ESP_UDP_MODE_SERVER 0 /*!< TRUE:server FALSE:client */
#define EXAMPLE_ESP_UDP_PERF_TX 1 /*!< TRUE:send FALSE:receive */
#define EXAMPLE_PACK_BYTE_IS 97 /*!< 'a'*/
/*AP info and tcp_server info*/
#define EXAMPLE_DEFAULT_SSID "vnet-5c36c7" /*!< default SSID used to tests */
#define EXAMPLE_DEFAULT_PWD "C0C6875C36C7" /*!< default password used to tests */
#define EXAMPLE_DEFAULT_PORT 7755 /*!< default port used to tests, port type is uint16_t */
#define EXAMPLE_DEFAULT_TCPPORT 7756 /*!< default port used to tests, port type is uint16_t */
#define EXAMPLE_DEFAULT_SERVER_IP "192.168.0.19" /*!< default server ip used to tests */

#define EXAMPLE_MAX_STA_CONN 1 /*!< how many sta can be connected(AP mode) */
#define TAG "udp:"
#define TAGTCP "tcp:"


#define WIFI_CONNECTED_BIT BIT0
#define UDP_CONNCETED_SUCCESS BIT1

#define BUF_SIZE (1024)
#define EXAMPLE_DEFAULT_PKTSIZE 1024 /*!< default pktsize*/


/**
 *\brief using esp as station
 */
void wifi_init_sta();

//using esp as softap
void wifi_init_softap();

//create a udp server socket. return ESP_OK:success ESP_FAIL:error
esp_err_t create_udp_server();

/**
 *\brief create a udp client socket. return ESP_OK:success ESP_FAIL:erro
 */
esp_err_t create_udp_client();

esp_err_t create_tcp_client();
void sendUDP_recvTCP_data(void *pvParameters);

/**
 *\brief send or recv data task, not used here sending by udp is in tcp function
 */
//void send_recv_data(void *pvParameters);

//get socket error code. return: error code
int get_socket_error_code(int socket);

//show socket error code. return: error code
int show_socket_error_reason(int socket);

//check connected socket. return: error code
int check_connected_socket();

//close all socket
void close_socket();

void onoffSending();




#ifdef __cplusplus
}
#endif


#endif /* MAIN_UDP_H_ */
