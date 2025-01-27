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

/* These variables are already declared as extern in SECURITY_interface.h */
extern volatile u8 Error_State;
extern volatile u8 User_Count;
extern volatile u8 Is_Admin;
extern volatile u8 Current_User;

/* Constants */
#define INPUT_TIMEOUT_MS 30000 // 30 seconds timeout
#define DISPLAY_DELAY_MS 2000	 // 2 seconds display time
#define SCROLL_DELAY_MS 300		 // 300ms scroll delay

/* Function prototypes */
void Display_Welcome(void);
void Display_Init_Status(void);
u8 Wait_For_Input(u8 *input, u16 timeout_ms);
void Display_Error(const u8 *message);
void Scroll_Text(const u8 *text, u8 row);

void Display_Welcome(void)
{
	CLCD_vClearScreen();
	CLCD_vSendString((u8 *)"   Welcome to");
	CLCD_vSetPosition(2, 1);
	CLCD_vSendString((u8 *)" Advanced Safe");
	_delay_ms(DISPLAY_DELAY_MS);

	CLCD_vClearScreen();
	CLCD_vSendString((u8 *)"Developed by:");
	CLCD_vSetPosition(2, 1);
	CLCD_vSendString((u8 *)"Abdallah Shehawey");
	_delay_ms(DISPLAY_DELAY_MS);
}

void Display_Init_Status(void)
{
	CLCD_vClearScreen();
	CLCD_vSendString((u8 *)"Initializing");
	CLCD_vSetPosition(2, 1);

	// Initialize LCD
	CLCD_vSendString((u8 *)"LCD...");
	CLCD_vSendString((u8 *)"OK");
	_delay_ms(500);

	// Initialize USART
	CLCD_vSetPosition(2, 1);
	CLCD_vSendString((u8 *)"USART...");
	USART_vInit();
	CLCD_vSendString((u8 *)"OK");
	_delay_ms(500);

	// Initialize EEPROM
	CLCD_vSetPosition(2, 1);
	CLCD_vSendString((u8 *)"EEPROM...");
	EEPROM_vInit();
	CLCD_vSendString((u8 *)"OK");
	_delay_ms(500);
}

u8 Wait_For_Input(u8 *input, u16 timeout_ms)
{
	u16 elapsed = 0;
	while (elapsed < timeout_ms)
	{
		Error_State = USART_RecieveDataFuncName(input);
		if (Error_State == OK)
		{
			return 1; // Input received
		}
		_delay_ms(1);
		elapsed++;
	}
	return 0; // Timeout
}

void Display_Error(const u8 *message)
{
	CLCD_vClearScreen();
	CLCD_vSendString((u8 *)"Error:");
	CLCD_vSetPosition(2, 1);
	CLCD_vSendString((u8 *)message);
	_delay_ms(DISPLAY_DELAY_MS);
}

void Scroll_Text(const u8 *text, u8 row)
{
	u8 length = 0;
	while (text[length] != '\0')
		length++;

	if (length <= 16)
	{
		CLCD_vSetPosition(row, 1);
		CLCD_vSendString((u8 *)text);
		return;
	}

	for (u8 i = 0; i <= length - 16; i++)
	{
		CLCD_vSetPosition(row, 1);
		for (u8 j = 0; j < 16; j++)
		{
			if (i + j < length)
			{
				CLCD_vSendData(text[i + j]);
			}
			else
			{
				CLCD_vSendData(' ');
			}
		}
		_delay_ms(SCROLL_DELAY_MS);
	}
}

void Display_Menu(void)
{
	CLCD_vClearScreen();
	CLCD_vSendString((u8 *)"1:Sign In");
	CLCD_vSetPosition(2, 1);
	CLCD_vSendString((u8 *)"2:New User");
	CLCD_vSetPosition(3, 1);
	CLCD_vSendString((u8 *)"Users:");
	CLCD_vSendIntNumber(User_Count);
	CLCD_vSendString((u8 *)"/");
	CLCD_vSendIntNumber(MAX_USERS);

	// Show system status
	CLCD_vSetPosition(4, 1);
	if (Tries < Tries_Max)
	{
		CLCD_vSendString((u8 *)"Tries Left: ");
		CLCD_vSendIntNumber(Tries);
	}
	else
	{
		CLCD_vSendString((u8 *)"System Ready");
	}
}

int main(void)
{
  CLCD_vInit();
	// Show welcome screen
	Display_Welcome();

	// Initialize and show status
	Display_Init_Status();

	u8 choice;

	while (1)
	{
		Display_Menu();

		// Wait for input with timeout
		if (!Wait_For_Input(&choice, INPUT_TIMEOUT_MS))
		{
			Display_Error((u8 *)"Input Timeout!");
			continue;
		}

		if (choice == '1')
		{
			Sign_In();

			// Check if this is the admin user (first user)
			Is_Admin = (Current_User == 0);

			// Show appropriate menu based on user type
			if (Is_Admin)
			{
				Admin_Menu();
			}
			else
			{
				User_Menu();
			}
		}
		else if (choice == '2')
		{
			if (User_Count >= MAX_USERS)
			{
				CLCD_vClearScreen();
				CLCD_vSendString((u8 *)"EEPROM Full!");
				CLCD_vSetPosition(2, 1);
				CLCD_vSendString((u8 *)"Max Users: ");
				CLCD_vSendIntNumber(MAX_USERS);
				_delay_ms(DISPLAY_DELAY_MS);
				continue;
			}

			// Check if username exists
			UserName_Set();

			// Set password with complexity requirements
			PassWord_Set();

			// Log the event
			Log_Event(EVENT_USER_CREATE, User_Count - 1);

			CLCD_vClearScreen();
			Scroll_Text((u8 *)"User Created Successfully!", 1);
			CLCD_vSetPosition(2, 1);
			CLCD_vSendString((u8 *)"Space Left: ");
			CLCD_vSendIntNumber(MAX_USERS - User_Count);
			_delay_ms(DISPLAY_DELAY_MS);
		}
		else
		{
			Display_Error((u8 *)"Invalid Choice!");
		}
	}

	return 0;
}
