/******************************************************************************
 * FOTA.h
 *
 * Created on: 22 May 2020
 * Author    : Gokce Alparslan
 ******************************************************************************/
#ifndef INC_FOTA_H_
#define INC_FOTA_H_
#include "stdint.h"

#define APPLICATION_START_ADDRESS     0x0800C000 //(FLASH_BASE+ 48*1024)
#define START_ADDRESS_OF_NEW_FIRMWARE 0x0800C000

enum Fota_States
{
	GET_THE_FIRST_DATA_FROM_SERVER,
	ARRANGE_THE_DATA,
	WRITE_NEW_FIRMWARE_ON_FLASH,
	GET_THE_LAST_OF_DATA_FROM_SERVER,
    EXECUTE_THE_NEW_FIRMWARE
};

typedef struct
{
	uint8_t  State_of_FOTA;
	uint32_t last_used_flash_address;

}FOTA_datas_t;

typedef void (*pFunction)(void);

void FOTA_main();

void initial_arrangement();

void endiannes_arrangement(uint32_t Buffer_index);

void Flash_erase();

uint8_t convertChartoHex(uint8_t *x);

extern FOTA_datas_t FOTA;

#endif /* INC_FOTA_H_ */
