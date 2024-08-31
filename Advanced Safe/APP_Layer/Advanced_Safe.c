/*
 * Smart_Home.c
 *
 *  Created on: Aug 29, 2024
 *      Author: Mega
 */

#define F_CPU 8000000UL
#include <util/delay.h>

#include "STD_TYPES.h"
#include "STD_MACROS.h"

#include "../MCAL_Layer/DIO/DIO_interface.h"
#include "../MCAL_Layer/EEPROM/EEPROM_interface.h"
#include "../MCAL_Layer/USART/USART_interface.h"

#include "../HAL_Layer/CLCD/CLCD_interface.h"

#include "Security.h"

volatile u8 UserName_Length = 0, PassWord_Length = 0;
volatile u8 Error_State, KPD_Press;
volatile u8 Max_Tries = Tries_Max;

void main(void)
{

	/* Initialize CLCD On PORTB And 4Bit Mode And Connected on Low Nibble */
	CLCD_vInit();

	/* Initialize USART to communicate with laptop */
	USART_vInit();

	CLCD_vSetPosition(1, 7);
	CLCD_vSendString("Welcome");
	_delay_ms(500);

	EEPROM_Check();

	while (1)
	{
		Sign_In();
		_delay_ms(1000);
	}
}

//======================================================================================================================================//

