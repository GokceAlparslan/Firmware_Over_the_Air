/******************************************************************************
 * ESP8266.h
 *
 * Created on: 22 May 2020
 * Author    : Gokce Alparslan
 ******************************************************************************/

#ifndef INC_ESP8266_H_
#define INC_ESP8266_H_

#include "stdint.h"
#include "main.h"
#include "FOTA.h"
//#include "string.h"

#define ON              1
#define OFF             0
#define END_OF_THE_LINE 0x0A   // \r\n

typedef struct
{
	uint8_t  rx_data;
	uint32_t rx_index;
	uint8_t  Received_Data[200][200];
	uint8_t  last_line_of_Received_Data[200];
	uint8_t  message_type_u8;

	uint8_t  new_firmware_datas[10000];
	uint32_t new_firmware_datas_u32[1200];

	uint8_t  Start_byte_index;
	uint8_t  Stop_byte_index;

	uint32_t arastop;
	uint32_t SGTD_index;

	uint8_t  all_firmware_have_received;

	uint8_t  requestment_type_u8;
	uint8_t  connection_status_with_server;

	uint32_t Firmware_version_on_board_u32;
	uint32_t Firmware_version_on_server_u32;

	uint32_t flash_son_adres_eki;
}ESP_datas_t;

enum ESP_adjustment_or_firmware_request
{
	ESP_ADJUSTMENT_REQUEST = 1,
	NEW_FIRMWARE_REQUEST   = 2
};

enum Message_stop_bits_type
{
	PACKET_STOP_BITS,
	LAST_STOP_BITS,
};

enum Command_type
{
	COMMAND_TYPE_HEX,
	COMMAND_TYPE_TEST,
	COMMAND_TYPE_AT_CIPSTART,
	COMMAND_TYPE_CLOSE_AT_CIPSEND,
	COMMAND_TYPE_CLOSE_AT_CIPSEND_3
};

enum Response_type
{
	IPD,
	HEX_OR_TEST,
	WIFI_GOT_IP,
	BUSY,
	ALREADY_CONNECTED,
	LINK_IS_NOT_VALID,
	CONNECT,
	CLOSE
};

void RX_Callback();

void get_data_from_wifi_module(UART_HandleTypeDef *huartx,uint32_t* p_last_line_of_Received_Data,uint8_t linenumber_u32);

void check_message_type(const uint32_t* const p_last_line_of_Received_Data);

void send_command_to_server(uint8_t command_type);

void check_start_and_stop_bits(uint8_t veri_akisi_u8, uint32_t scan_index_u32);

void check_stop_bits(uint8_t stop_bits_type_u8);

void check_first_response(uint8_t first_response_u8);

void check_connected_or_invalid_link(uint8_t already_connected_or_invalid_link_u8,uint32_t linenumber_u32);

void check_connect_or_close_situation(uint8_t connect_or_close_u8,uint32_t linenumber_u32);

void reset_terminal_datas(uint32_t* linenumber,uint8_t requestment_type_u8);

extern ESP_datas_t ESP_datas;

#endif /* INC_ESP8266_H_ */


