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

#include "../HAL_Layer/CLCD/CLCD_interface.h"

#include "SECURITY/SECURITY_interface.h"

volatile u8 Error_State, KPD_Press;
extern u8 UserName[20];
extern u8 UserName_Length;
extern u8 User_Count;

void Display_Menu(void)
{
	CLCD_vClearScreen();
	CLCD_vSendString("1:Sign In");
	CLCD_vSetPosition(2, 1);
	CLCD_vSendString("2:New User");
	// Display current users count
	CLCD_vSetPosition(3, 1);
	CLCD_vSendString("Users:");
	CLCD_vSendIntNumber(User_Count);
	CLCD_vSendString("/");
	CLCD_vSendIntNumber(MAX_USERS);
}

void main(void)
{
	/* Initialize CLCD On PORTB And 4Bit Mode */
	CLCD_vInit();
	/* Initialize USART to communicate with laptop */
	USART_vInit();
	/* Initialize EEPROM */
	EEPROM_vInit();
	_delay_ms(500);

	while (1)
	{
		Display_Menu();

		// Wait for user choice from terminal
		while (1)
		{
			Error_State = USART_RecieveDataFuncName(&KPD_Press);
			if (Error_State == OK)
			{
				if (KPD_Press == '1' || KPD_Press == '2')
				{
					break;
				}
			}
		}

		if (KPD_Press == '1')
		{
			// Sign In
			Sign_In();
			CLCD_vClearScreen();
			CLCD_vSetPosition(2, 7);
			CLCD_vSendString("Welcome");
			CLCD_vSetPosition(3, ((20 - UserName_Length) / 2) + 1);
			CLCD_vSendString(UserName);
			_delay_ms(2000);
		}
		else if (KPD_Press == '2')
		{
			// New User Registration
			if (User_Count >= MAX_USERS)
			{
				CLCD_vClearScreen();
				CLCD_vSendString("EEPROM Full!");
				CLCD_vSetPosition(2, 1);
				CLCD_vSendString("Max Users Reached");
				_delay_ms(2000);
				continue;
			}

			UserName_Set();
			PassWord_Set();

			CLCD_vClearScreen();
			CLCD_vSendString("User Registered!");
			CLCD_vSetPosition(2, 1);
			CLCD_vSendString("Space Left: ");
			CLCD_vSendIntNumber(MAX_USERS - User_Count);
			_delay_ms(2000);
		}
	}
}

//======================================================================================================================================//
