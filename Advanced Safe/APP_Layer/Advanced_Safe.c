/*
 * Advanced Safe.c
 *
 *  Created on: Aug 29, 2024
 *      Author: Abdallah Abdelmoemen Shehawey
 */

#define F_CPU 8000000UL
#include <util/delay.h>

#include "STD_TYPES.h"
#include "STD_MACROS.h"

#include "../MCAL_Layer/DIO/DIO_interface.h"
#include "../MCAL_Layer/EEPROM/EEPROM_interface.h"
#include "../MCAL_Layer/USART/USART_interface.h"
#include "../HAL_Layer/KPD/KPD_interface.h"

#include "../HAL_Layer/CLCD/CLCD_interface.h"

#include "SECURITY/SECURITY_interface.h"

volatile u8 Error_State, KPD_Press                  ;

void main(void)
{
	/* Initialize CLCD On PORTB And 4Bit Mode And Connected on Low Nibble */
	CLCD_vInit();
	/* Initialize USART to communicate with laptop */
	USART_vInit();

	KPD_vInit();

	_delay_ms(500);
	/*
	First check if it first sign in or not
	check if there is time out or not
	check how many tries left
	 */
	EEPROM_vInit();
	while (1)
	{
		Sign_In();
		_delay_ms(1000);
	}
}

//======================================================================================================================================//
