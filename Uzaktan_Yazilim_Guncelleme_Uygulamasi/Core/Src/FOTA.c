/*********************************************************************************************************************************
 * FOTA.c
 *
 * Created on: 22 May 2020
 * Author    : Gokce Alparslan
 ********************************************************************************************************************************/
#include "FOTA.h"
#include "stdint.h"
#include "ESP8266.h"

FOTA_datas_t FOTA = {0};

/*********************************************************************************************************************************
 * FIRMWARE OVER THE AIR
 * If there is not a new firmware, Fota state is GET_THE_FIRST_DATA_FROM_SERVER.
 * Arrange the data taken from server according to memory of stm -little endiannes
 * Write the first part on flash
 * Get the last of the data, arrange and write.
 * Execute the new firmware.
 ********************************************************************************************************************************/
void FOTA_main()
{
	static uint32_t Buffer_index;

	switch (FOTA.State_of_FOTA)
	{
	case GET_THE_FIRST_DATA_FROM_SERVER:
	{
		break;
	}
	case ARRANGE_THE_DATA:
	{
    	FOTA.last_used_flash_address = OFF;

		endiannes_arrangement(Buffer_index);

	    FOTA.State_of_FOTA = WRITE_NEW_FIRMWARE_ON_FLASH;

	    break;
    }
    case WRITE_NEW_FIRMWARE_ON_FLASH:
    {
        FOTA.last_used_flash_address = ESP_datas.flash_son_adres_eki;

        if( OFF == FOTA.last_used_flash_address) //for first entrance
        {
        	Flash_erase();
        }

        for (int i = 0; i < Buffer_index; i++)
        {
        	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
    		        		  START_ADDRESS_OF_NEW_FIRMWARE + FOTA.last_used_flash_address,
					     	  *(uint32_t*)&ESP_datas.new_firmware_datas_u32[i]);

		    FOTA.last_used_flash_address = FOTA.last_used_flash_address + 4;
        }

       if (OFF == ESP_datas.all_firmware_have_received)
       {
    	   ESP_datas.flash_son_adres_eki = FOTA.last_used_flash_address;

           FOTA.State_of_FOTA = GET_THE_LAST_OF_DATA_FROM_SERVER;

           break;
       }
       else
       {
           HAL_FLASH_Lock();

           FOTA.State_of_FOTA = EXECUTE_THE_NEW_FIRMWARE;

           break;
       }
	}
    case GET_THE_LAST_OF_DATA_FROM_SERVER:
    {
    	ESP_datas.SGTD_index = 0;

    	memset(ESP_datas.new_firmware_datas,0,sizeof(ESP_datas.new_firmware_datas));

    	memset(ESP_datas.Received_Data, 0, sizeof(ESP_datas.Received_Data));
    	memset(ESP_datas.last_line_of_Received_Data, 0,sizeof(ESP_datas.last_line_of_Received_Data));
    	memset(ESP_datas.new_firmware_datas_u32, 0,sizeof(ESP_datas.new_firmware_datas_u32));

    	HAL_UART_Receive_IT(&huart2, &ESP_datas.rx_data, 1);

    	send_command_to_server(COMMAND_TYPE_CLOSE_AT_CIPSEND_3);

    	FOTA.State_of_FOTA = GET_THE_FIRST_DATA_FROM_SERVER;

      break;
    }
    case EXECUTE_THE_NEW_FIRMWARE: /*check if there is a new firmware or not*/
    {
    	SystemInit();

    	pFunction Jump_To_Application = *( (pFunction*) (APPLICATION_START_ADDRESS + 4) );

    	__set_CONTROL(0);

        SysTick->CTRL = 0; //disable SysTick

        HAL_TIM_Base_Stop_IT(&htim3);  //TİMER IT KAPAT

        HAL_UART_AbortReceive_IT(&huart2);  //UART IT KAPAT

        SCB->VTOR = APPLICATION_START_ADDRESS;

        __set_MSP( *( (uint32_t*) APPLICATION_START_ADDRESS ) );

        Jump_To_Application();
    }
	} /*SWITCH*/
}

/*********************************************************************************************************************************
 * Start connection with server
 * Listen the ESP8266
 ********************************************************************************************************************************/
void initial_arrangement()
{
	  HAL_TIM_Base_Start_IT(&htim3);

	  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_15, 1);

	  HAL_UART_Receive_IT(&huart2, &ESP_datas.rx_data, 1);

  	  send_command_to_server(COMMAND_TYPE_AT_CIPSTART);

	  ESP_datas.connection_status_with_server = OFF;

	  FOTA.State_of_FOTA = GET_THE_FIRST_DATA_FROM_SERVER; //silme
}

void endiannes_arrangement(uint32_t Buffer_index)
{
    uint8_t  a,b,c,d;

    for (int i = 0; i < (ESP_datas.SGTD_index-2 - ESP_datas.arastop); i = i + 8)  //bufferların içindeki veriler sektörlerdeki gibi 64bitlik hale getirilip svalueinsector e kaydedilir. (SGTD_index-2 ile bufferdaki stop biti ayıklanır.)
    {
        a = ((uint8_t) (convertChartoHex(&ESP_datas.new_firmware_datas[i]))     << 4) | (convertChartoHex(&ESP_datas.new_firmware_datas[i + 1]));

        b = ((uint8_t) (convertChartoHex(&ESP_datas.new_firmware_datas[i + 2])) << 4) | (convertChartoHex(&ESP_datas.new_firmware_datas[i + 3]));

        c = ((uint8_t) (convertChartoHex(&ESP_datas.new_firmware_datas[i + 4])) << 4) | (convertChartoHex(&ESP_datas.new_firmware_datas[i + 5]));

        d = ((uint8_t) (convertChartoHex(&ESP_datas.new_firmware_datas[i + 6])) << 4) | (convertChartoHex(&ESP_datas.new_firmware_datas[i + 7]));

        ESP_datas.new_firmware_datas_u32[Buffer_index] = (((uint64_t) a << 24) | ((uint32_t) b << 16) | ((uint16_t) c << 8) | d);

        Buffer_index++;
    }
 }

void Flash_erase()
{
	  HAL_FLASH_Unlock();
	  FLASH_Erase_Sector(FLASH_SECTOR_2,  VOLTAGE_RANGE_3);
	  FLASH_Erase_Sector(FLASH_SECTOR_3,  VOLTAGE_RANGE_3);
	  FLASH_Erase_Sector(FLASH_SECTOR_4,  VOLTAGE_RANGE_3);
	  FLASH_Erase_Sector(FLASH_SECTOR_5,  VOLTAGE_RANGE_3);
	  FLASH_Erase_Sector(FLASH_SECTOR_6,  VOLTAGE_RANGE_3);
	  FLASH_Erase_Sector(FLASH_SECTOR_7,  VOLTAGE_RANGE_3);
	  FLASH_Erase_Sector(FLASH_SECTOR_8,  VOLTAGE_RANGE_3);
	  FLASH_Erase_Sector(FLASH_SECTOR_9,  VOLTAGE_RANGE_3);
	  FLASH_Erase_Sector(FLASH_SECTOR_10, VOLTAGE_RANGE_3);
	  FLASH_Erase_Sector(FLASH_SECTOR_11, VOLTAGE_RANGE_3);

	  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,0x08008000, *(uint32_t*) &ESP_datas.Firmware_version_on_server_u32); //yeni kodun versiyonunu kaydet
}

uint8_t convertChartoHex(uint8_t *x)
{
	uint8_t hexresult;
    if(*x=='0'){ hexresult = 0x0;}
    if(*x=='1'){ hexresult = 0x1;}
    if(*x=='2'){ hexresult = 0x2;}
    if(*x=='3'){ hexresult = 0x3;}
    if(*x=='4'){ hexresult = 0x4;}
    if(*x=='5'){ hexresult = 0x5;}
    if(*x=='6'){ hexresult = 0x6;}
    if(*x=='7'){ hexresult = 0x7;}
    if(*x=='8'){ hexresult = 0x8;}
    if(*x=='9'){ hexresult = 0x9;}
    if(*x=='A'){ hexresult = 0xA;}
    if(*x=='B'){ hexresult = 0xB;}
    if(*x=='C'){ hexresult = 0xC;}
    if(*x=='D'){ hexresult = 0xD;}
    if(*x=='E'){ hexresult = 0xE;}
    if(*x=='F'){ hexresult = 0xF;}

    return hexresult;
}
