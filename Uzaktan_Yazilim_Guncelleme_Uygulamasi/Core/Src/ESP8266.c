/********************************************************************************************************************************
 * ESP8266.c
 *
 * Created on: 22 May 2020
 * Author    : Gokce Alparslan
 ********************************************************************************************************************************/
#include "ESP8266.h"
#include "main.h"

ESP_datas_t ESP_datas = {0};

void RX_Callback()
{
	static uint32_t linenumber = 0;

	uint32_t* p_last_line_of_Received_Data;

	get_data_from_wifi_module(&huart2,p_last_line_of_Received_Data,linenumber);  //pointer yap

	check_message_type(p_last_line_of_Received_Data);
}

/********************************************************************************************************************************
 * Receiving data one byte one byte over server and put inside Received_Data
 * Received_data can be used as terminal?
 ********************************************************************************************************************************/
void get_data_from_wifi_module(UART_HandleTypeDef *huartx,uint32_t* p_last_line_of_Received_Data,uint8_t linenumber_u32)
{
	uint8_t  rx_data;

	HAL_UART_Receive_IT(huartx, &rx_data, 1);

	ESP_datas.Received_Data[linenumber_u32][ESP_datas.rx_index] = rx_data;

	ESP_datas.last_line_of_Received_Data[ESP_datas.rx_index] = rx_data;

	ESP_datas.rx_index++;

	p_last_line_of_Received_Data = ((uint32_t*)&ESP_datas.last_line_of_Received_Data);
}

/********************************************************************************************************************************
 * Esp_message : +IPD:.....
 * -2- START BITS
 * -1- STOP BITS IN EVERY PART OF MESSAGE.
 * -0- LAST STOP BIT OF MESSAGE.All messages have received.
 *
 *     -2-      | + IPD:..... |       -1-        | + IPD:                     -1-        |      -0-
 *   START BITS | X BYTE DATA | PACKET_STOP_BITS | X BYTE DATA - .... - PACKET_STOP_BITS | LAST_STOP_BITS
 ********************************************************************************************************************************/
void check_message_type(const uint32_t* const p_last_line_of_Received_Data) //todo const ekle .asagida verilere pointer ile ulas
{
	static uint32_t linenumber_u32    = 0;
	static uint32_t scan_index_u32    = 0;
	static uint8_t  veri_akisi_u8     = 0;
	static uint8_t  stop_bit_type_u8  = 0;
	static uint8_t  response_u8       = 0;

	check_first_response(response_u8);

	if (IPD == response_u8)   //GELEN VERİLER IPD Lİ Son_Satir_Receive_Data[0]
	{
		check_start_and_stop_bits(veri_akisi_u8,scan_index_u32);

	    if(ON == veri_akisi_u8)   //ALINAN PAKET
	    {
	    	for (int i = ESP_datas.Start_byte_index; i < ESP_datas.Stop_byte_index ; i++)
	    	{
	    		check_stop_bits(stop_bit_type_u8);

	    		ESP_datas.new_firmware_datas[ESP_datas.SGTD_index] = ESP_datas.last_line_of_Received_Data[i];

	    		if(LAST_STOP_BITS == stop_bit_type_u8) //SON pakette geldi
	    		{
	    			ESP_datas.all_firmware_have_received = OFF;

	    			ESP_datas.arastop = 0;

	    			FOTA.State_of_FOTA = ON;    //Buradaki işlemler bitti main e geç:::Verileri_duzenleme_ aşaması

	    			break;
	    		}
	    		else if(PACKET_STOP_BITS == stop_bit_type_u8)//SGTD'yı dolduracak kadar byte geldi(1paket)
	    		{
	    		 	ESP_datas.all_firmware_have_received = ON;

	    		 	ESP_datas.arastop = 3;  //ham paketi stop_byte ve start_byte'dan ayırırız ancak arastop_paket kalır

	    		 	FOTA.State_of_FOTA = ON;   //Buradaki işlemler bitti main e geç:::Verileri_duzenleme_ aşaması

	    		 	break;
	    		}

	    		ESP_datas.SGTD_index++;

            }//for (i = ESP_datas.Start_byte_index; i < ESP_datas.Stop_byte_index ; i++)

	    	if (FOTA.State_of_FOTA != ON)
	    	{
	    		ESP_datas.requestment_type_u8 = NEW_FIRMWARE_REQUEST;

	    		memset(ESP_datas.Received_Data, 0, sizeof(ESP_datas.Received_Data));

	    		memset(ESP_datas.last_line_of_Received_Data, 0,sizeof(ESP_datas.last_line_of_Received_Data));

	    		HAL_UART_Transmit(&huart2,(uint8_t*) "AT+CIPSEND=3\r\n",strlen("AT+CIPSEND=3\r\n"), 100);

	    	    ESP_datas.connection_status_with_server = ON;
	    	}

	    	ESP_datas.Start_byte_index = 0;

	    	ESP_datas.Stop_byte_index = 0;

	    	scan_index_u32 = 0;

	    	veri_akisi_u8 = OFF;

	    }//if(veri_akisi_u8 == ON)

	    if ( (ESP_datas.last_line_of_Received_Data[8]  == 'n') && ////////////////////////////////
	    	 (ESP_datas.last_line_of_Received_Data[9]  == 'e') &&
	    	 (ESP_datas.last_line_of_Received_Data[10] == 'w') &&
	    	 (ESP_datas.last_line_of_Received_Data[20] == 'h')    )// yes or no NEW hex! .byte byte almaya basla
	    {
	    	uint8_t a,b,c,d;

	    	ESP_datas.Firmware_version_on_board_u32 = *(uint32_t*)0x08008000;

	        a =((uint8_t) (convertChartoHex(&ESP_datas.last_line_of_Received_Data[11])) << 4) | (convertChartoHex(&ESP_datas.last_line_of_Received_Data[12])) ;
	        b =((uint8_t) (convertChartoHex(&ESP_datas.last_line_of_Received_Data[13])) << 4) | (convertChartoHex(&ESP_datas.last_line_of_Received_Data[14])) ;
	        c =((uint8_t) (convertChartoHex(&ESP_datas.last_line_of_Received_Data[15])) << 4) | (convertChartoHex(&ESP_datas.last_line_of_Received_Data[16])) ;
	        d =((uint8_t) (convertChartoHex(&ESP_datas.last_line_of_Received_Data[17])) << 4) | (convertChartoHex(&ESP_datas.last_line_of_Received_Data[18])) ;
	        uint64_t versiyon= (((uint64_t) a << 24) | ((uint32_t) b << 16) | ((uint16_t) c << 8) | d);

	        ESP_datas.Firmware_version_on_server_u32 = versiyon;

	        if( ESP_datas.Firmware_version_on_board_u32 == 0xFFFFFFFF || ESP_datas.Firmware_version_on_board_u32 != ESP_datas.Firmware_version_on_server_u32 )  //YENİ KOD VAR
	        {
	        	memset(ESP_datas.Received_Data, 0, sizeof(ESP_datas.Received_Data));

	        	memset(ESP_datas.last_line_of_Received_Data, 0, sizeof(ESP_datas.last_line_of_Received_Data));

	        	memset(ESP_datas.new_firmware_datas, 0, sizeof(ESP_datas.new_firmware_datas));

	        	ESP_datas.requestment_type_u8 = NEW_FIRMWARE_REQUEST;

	        	HAL_UART_Transmit(&huart2,(uint8_t*) "AT+CIPSEND=3\r\n",strlen("AT+CIPSEND=3\r\n"),100);

	        	ESP_datas.connection_status_with_server = ON;
			}
			else
			{
				FOTA.State_of_FOTA = EXECUTE_THE_NEW_FIRMWARE;
			}
	    }//if ( (Son_Satir_Receive_Data[8]  == 'n')...


		}//if (IPD == response_u8)
        else if(HEX_OR_TEST == response_u8) //hex veya test gönder
        {
        	reset_terminal_datas(&linenumber_u32,ESP_datas.requestment_type_u8);

        	scan_index_u32 = 0;

        	if(ESP_ADJUSTMENT_REQUEST == ESP_datas.requestment_type_u8)
        	{
        		send_command_to_server(COMMAND_TYPE_TEST);
        	}

        	if(NEW_FIRMWARE_REQUEST == ESP_datas.requestment_type_u8)
        	{
        		send_command_to_server(COMMAND_TYPE_HEX);
        	}
        }
        else if(WIFI_GOT_IP == response_u8)
        {
	    	reset_terminal_datas(&linenumber_u32,ESP_ADJUSTMENT_REQUEST);

	    	send_command_to_server(COMMAND_TYPE_AT_CIPSTART);
        }
	    else if(BUSY == response_u8 )
	    {
	    	reset_terminal_datas(&linenumber_u32,ESP_ADJUSTMENT_REQUEST);

	    	send_command_to_server(COMMAND_TYPE_CLOSE_AT_CIPSEND);

	    	ESP_datas.connection_status_with_server = ON;
	    }

	    if (END_OF_THE_LINE == ESP_datas.rx_data)
	    {
	    	ESP_datas.rx_index = 0;

	    	linenumber_u32++;

	    	if (linenumber_u32 > 3)  // 3 satır dolmasını bekle, aşağıda esp den gelen yanıtları kıyasla ve ne yapılacağına karar ver.
	    	{
	    		if( (ESP_datas.last_line_of_Received_Data[0] == 'E') &&
	    			(ESP_datas.last_line_of_Received_Data[1] == 'R') &&
	    		    (ESP_datas.last_line_of_Received_Data[2] == 'R') &&
	    		    (ESP_datas.last_line_of_Received_Data[3] == 'O') &&
	    		    (ESP_datas.last_line_of_Received_Data[4] == 'R')   )
	    		{
	    			check_connected_or_invalid_link(response_u8,linenumber_u32);

	    			if(ALREADY_CONNECTED == response_u8)
				    {
				    	reset_terminal_datas(&linenumber_u32,ESP_ADJUSTMENT_REQUEST);

				    	send_command_to_server(COMMAND_TYPE_CLOSE_AT_CIPSEND);

				    	ESP_datas.connection_status_with_server = ON;
				    }
				    else if(LINK_IS_NOT_VALID == response_u8)
				    {
				    	reset_terminal_datas(&linenumber_u32,ESP_ADJUSTMENT_REQUEST);

				    	send_command_to_server(COMMAND_TYPE_AT_CIPSTART);
				    }
	    		 }//if (Son_Satir_Receive_Data[0] == 'E'...ERROR
			    else if( (ESP_datas.last_line_of_Received_Data[0] == 'O') &&
			     		 (ESP_datas.last_line_of_Received_Data[1] == 'K')    )
			    {
			    	check_connect_or_close_situation(response_u8,linenumber_u32);

			    	if(CONNECT == response_u8)
			        {
				    	reset_terminal_datas(&linenumber_u32,ESP_ADJUSTMENT_REQUEST);

				    	send_command_to_server(COMMAND_TYPE_CLOSE_AT_CIPSEND);

				    	ESP_datas.connection_status_with_server = ON;
			        }
			        else if(CLOSE == response_u8)
			        {
				    	reset_terminal_datas(&linenumber_u32,ESP_ADJUSTMENT_REQUEST);

				    	send_command_to_server(COMMAND_TYPE_AT_CIPSTART);
			        }
			    }//else if( (Son_Satir_Receive_Data[0] == 'O')...OK



	    	}//if (linenumber_u32 > 3)
		}//if (rx_data == 0x0A)
}


void send_command_to_server(uint8_t command_type)
{
	uint16_t size = 0;

	uint8_t *pData = 0;

	switch(command_type)
	{
	case COMMAND_TYPE_HEX:
	{
		pData = (uint8_t*)"hex";

		size = strlen("hex");

		break;
	}
	case COMMAND_TYPE_TEST:
	{
		pData = (uint8_t*)"test";

		size = strlen("test");

		break;
	}
	case COMMAND_TYPE_AT_CIPSTART:
	{
		pData = (uint8_t*)"AT+CIPSTART=\"TCP\",\"192.168.0.28\",1234\r\n";

		size = strlen("AT+CIPSTART=\"TCP\",\"192.168.0.28\",1234\r\n");

		break;
	}
	case COMMAND_TYPE_CLOSE_AT_CIPSEND:
	{
		pData = (uint8_t*)"AT+CIPSEND=4\r\n";

		size = strlen("AT+CIPSEND=4\r\n");

		break;
	}
	case COMMAND_TYPE_CLOSE_AT_CIPSEND_3:
	{
		pData = (uint8_t*)"AT+CIPSEND=3\r\n";

		size = strlen("AT+CIPSEND=3\r\n");

		break;
	}
	}//switch(command_type)

	HAL_UART_Transmit(&huart2,pData,size,100);
}

void check_start_and_stop_bits(uint8_t veri_akisi_u8, uint32_t scan_index_u32)
{
    static uint8_t  Flag_STOP_bit_u8  = 0;
    static uint8_t  Flag_START_bit_u8 = 0;

	if((ESP_datas.last_line_of_Received_Data[scan_index_u32-2] == '-')&&
       (ESP_datas.last_line_of_Received_Data[scan_index_u32-1] == '2')&&
       (ESP_datas.last_line_of_Received_Data[scan_index_u32]   == '-')   )
    {
		Flag_STOP_bit_u8 = ON;
    }
    else if ((ESP_datas.last_line_of_Received_Data[scan_index_u32-2] == '-') &&
			 (ESP_datas.last_line_of_Received_Data[scan_index_u32-1] == '1') &&
			 (ESP_datas.last_line_of_Received_Data[scan_index_u32]   == '-')    )
	{
    	Flag_START_bit_u8 = ON;
	}
	if (ON == Flag_STOP_bit_u8)     //her 176 byte için stop bitini bekle
	{
		veri_akisi_u8 = ON;

		ESP_datas.Stop_byte_index = scan_index_u32 - 2;

		Flag_START_bit_u8 = OFF;

		Flag_STOP_bit_u8  = OFF;
	}
	scan_index_u32++;

	if (ON == Flag_START_bit_u8)     //her 176byte için start bitini bekle
	{
		ESP_datas.Start_byte_index = scan_index_u32;

		Flag_START_bit_u8 = OFF;
	}
}



void check_stop_bits(uint8_t stop_bits_type_u8)
{
	if ((ESP_datas.new_firmware_datas[ESP_datas.SGTD_index - 2] == '-') &&
		(ESP_datas.new_firmware_datas[ESP_datas.SGTD_index - 1] == '0') &&
		(ESP_datas.new_firmware_datas[ESP_datas.SGTD_index]     == '-')   ) //SON pakette geldi
	{
		stop_bits_type_u8 = LAST_STOP_BITS;
	}
    else if( (ESP_datas.new_firmware_datas[ESP_datas.SGTD_index - 2] == '-') &&
		     (ESP_datas.new_firmware_datas[ESP_datas.SGTD_index - 1] == '6') &&
			 (ESP_datas.new_firmware_datas[ESP_datas.SGTD_index]     == '-')   )//SGTD'yı dolduracak kadar byte geldi(1paket)
	{
    	stop_bits_type_u8 = PACKET_STOP_BITS;
	}
}

void check_first_response(uint8_t first_response_u8)
{
	if (ESP_datas.last_line_of_Received_Data[0] == '+')
	{
		first_response_u8 = IPD;
	}
	else if(ESP_datas.last_line_of_Received_Data[0] == '>') //hex veya test gönder
	{
		first_response_u8 = HEX_OR_TEST;
	}
	else if( (ESP_datas.last_line_of_Received_Data[0] == 'W') &&
			 (ESP_datas.last_line_of_Received_Data[1] == 'I') &&
			 (ESP_datas.last_line_of_Received_Data[2] == 'F') &&
			 (ESP_datas.last_line_of_Received_Data[3] == 'I') &&
			 (ESP_datas.last_line_of_Received_Data[5] == 'G') &&
			 (ESP_datas.last_line_of_Received_Data[6] == 'O')   )
	{
		first_response_u8 = WIFI_GOT_IP;
	}
	else if( (ESP_datas.last_line_of_Received_Data[0] == 'b') &&
			 (ESP_datas.last_line_of_Received_Data[1] == 'u') &&
			 (ESP_datas.last_line_of_Received_Data[2] == 's') &&
			 (ESP_datas.last_line_of_Received_Data[3] == 'y')   )
	{
		first_response_u8 = BUSY;
	}
}

void check_connected_or_invalid_link(uint8_t already_connected_or_invalid_link_u8,uint32_t linenumber_u32)
{
	if( (ESP_datas.Received_Data[linenumber_u32 - 3][0] == 'A') &&
        (ESP_datas.Received_Data[linenumber_u32 - 3][1] == 'L') &&
        (ESP_datas.Received_Data[linenumber_u32 - 3][2] == 'R') &&
        (ESP_datas.Received_Data[linenumber_u32 - 3][3] == 'E') &&
        (ESP_datas.Received_Data[linenumber_u32 - 3][4] == 'A') &&
        (ESP_datas.Received_Data[linenumber_u32 - 3][5] == 'D')   )
    {

		already_connected_or_invalid_link_u8 = ALREADY_CONNECTED;
    }
    else if((ESP_datas.Received_Data[linenumber_u32 - 3][22] == 'A') &&   //esp sıfırdan başladığında hatalı
            (ESP_datas.Received_Data[linenumber_u32 - 3][23] == 'L') &&
            (ESP_datas.Received_Data[linenumber_u32 - 3][24] == 'R') &&
            (ESP_datas.Received_Data[linenumber_u32 - 3][25] == 'E') &&
            (ESP_datas.Received_Data[linenumber_u32 - 3][26] == 'A') &&
            (ESP_datas.Received_Data[linenumber_u32 - 3][27] == 'D')   )
    {
		already_connected_or_invalid_link_u8 = ALREADY_CONNECTED;
    }
    else if((ESP_datas.Received_Data[linenumber_u32 - 3][0] == 'L') &&
            (ESP_datas.Received_Data[linenumber_u32 - 3][1] == 'I') &&
            (ESP_datas.Received_Data[linenumber_u32 - 3][2] == 'N')   )//LINK İS NOT VALİD
    {
		already_connected_or_invalid_link_u8 = LINK_IS_NOT_VALID;
    }
}


void check_connect_or_close_situation(uint8_t connect_or_close_u8,uint32_t linenumber_u32)
{
	if( (ESP_datas.Received_Data[linenumber_u32 - 3][0] == 'C' ) &&
        (ESP_datas.Received_Data[linenumber_u32 - 3][1] == 'O' ) &&
        (ESP_datas.Received_Data[linenumber_u32 - 3][2] == 'N' ) &&
        (ESP_datas.Received_Data[linenumber_u32 - 3][3] == 'N' ) &&
        (ESP_datas.Received_Data[linenumber_u32 - 3][4] == 'E' ) &&
        (ESP_datas.Received_Data[linenumber_u32 - 3][5] == 'C' ) &&
        (ESP_datas.Received_Data[linenumber_u32 - 3][6] == 'T' ) &&
        (ESP_datas.Received_Data[linenumber_u32 - 3][7] == '\r') &&
        (ESP_datas.Received_Data[linenumber_u32 - 3][8] == '\n')    )//CONNECT
    {
		connect_or_close_u8 = CONNECT;
    }
    else if( (ESP_datas.last_line_of_Received_Data[0] == 'C') &&
    		 (ESP_datas.last_line_of_Received_Data[1] == 'L') &&
    		 (ESP_datas.last_line_of_Received_Data[2] == 'O') &&
    		 (ESP_datas.last_line_of_Received_Data[3] == 'S') &&
    		 (ESP_datas.last_line_of_Received_Data[4] == 'E')   )
    {
    	connect_or_close_u8 = CLOSE;
    }
}

void reset_terminal_datas(uint32_t* linenumber_u32,uint8_t requestment_type_u8)
{
	memset(ESP_datas.Received_Data, 0, sizeof(ESP_datas.Received_Data));

	memset(ESP_datas.last_line_of_Received_Data,0, sizeof(ESP_datas.last_line_of_Received_Data));

	*linenumber_u32 = 0;

   	ESP_datas.requestment_type_u8 = requestment_type_u8;
}
