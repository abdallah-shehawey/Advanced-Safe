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

#define NOTPRESSED 0xFF

#define EEPROM_UserNameStatus 0x10
#define EEPROM_PassWordStatus  0x11
#define EEPROM_NoTries_Location      0x12
#define EEPROM_USNL_Location    0x13
#define EEPROM_PWL_Location      0x14

#define EEPROM_UserNameStartLocation 0x21
#define EEPROM_UserNameEndLocation   0x40

#define EEPROM_PassWordStartLocation  0x41
#define EEPROM_PassWordEndLocation    0x60

volatile u8 KPD_Press, Error_State = 1,UserName_Length = 0, PassWord_Length = 0;
volatile u8 Check[21] ;
volatile u8 UserName_Check_Flag = 1, PassWord_Check_Flag = 1;
volatile u8 Max_Tries = 3;
/*
 * USNL ==> User Name Length
 * PWL   ==> PassWord Length
 */

void Clear_Char              (void);
void UserName_Set     (void);
void PassWord_Set        (void);
void UserName_Check  (void);
void PassWord_Check    (void);
void Sign_In                   (void);
void Error_TimeOut(void);


void main(void)
{
	UserName_Length = EEPROM_vRead(EEPROM_USNL_Location);
	PassWord_Length = EEPROM_vRead(EEPROM_PWL_Location);

	if(EEPROM_vRead(EEPROM_NoTries_Location) != NOTPRESSED)
	{
		Max_Tries = EEPROM_vRead(EEPROM_NoTries_Location);
	}
	else
	{

	}
	/* Initialize CLCD On PORTB And 4Bit Mode And Connected on Low Nibble */
	CLCD_vInit();

	/* Initialize USART to communicate with laptop */
	USART_vInit();

	CLCD_vSetPosition(1, 7);
	CLCD_vSendString("Welcome");
	_delay_ms(500);

	//if it's first login
	if(EEPROM_vRead(EEPROM_UserNameStatus) == NOTPRESSED)
	{
		CLCD_vSetPosition(2, 1);
		CLCD_vSendString("Please Sign UP");
		CLCD_vSendExtraChar(4, 15);
		CLCD_vSetPosition(4, 16);
		CLCD_vSendString(" : OK");
		while(1)
		{
			//wait in error page until press enter
			Error_State = USART_u8ReceiveData(&KPD_Press);
			if (Error_State == OK)
			{
				if (KPD_Press == 0x0D || KPD_Press == 0x0F)
				{
					break;
				}
			}
		}
		UserName_Set();
	}
	if(EEPROM_vRead(EEPROM_PassWordStatus) == NOTPRESSED)
	{
		PassWord_Set();
		CLCD_vClearScreen();
		CLCD_vSendString("Saved Successfully");
		CLCD_vSendExtraChar(4, 15);
		CLCD_vSetPosition(4, 16);
		CLCD_vSendString(" : OK");
		while(1)
		{
			//wait in error page until press enter
			Error_State = USART_u8ReceiveData(&KPD_Press);
			if (Error_State == OK)
			{
				if (KPD_Press == 0x0D || KPD_Press == 0x0F)
				{
					break;
				}
			}
		}
		CLCD_vClearScreen();
	}
	/*___________________________________________________________________________________________________________________*/

	if(Max_Tries == 0)
	{
		Error_TimeOut();
	}
	else
	{

	}

	while (1)
	{
		Sign_In();
		_delay_ms(1000);
	}
}

//======================================================================================================================================//

void UserName_Set(void)
{
	EEPROM_vWrite(EEPROM_USNL_Location, 0x00);
	UserName_Length = EEPROM_vRead(EEPROM_USNL_Location);
	CLCD_vClearScreen();
	CLCD_vSetPosition(1, 5);
	CLCD_vSendString("Set UserName");
	CLCD_vSetPosition(2, 1);
	CLCD_vSendString("Maximum char : 20");
	CLCD_vSendExtraChar(4, 15);  // To Send Enter Symbol
	CLCD_vSetPosition(4, 16);
	CLCD_vSendString(" : OK");

	CLCD_vSetPosition(3, 1);
	UserName_Length = 0;
	/*Get username from user*/
	do
	{
		//if username is less than 5 char
		if (UserName_Length != 0)
		{
			CLCD_vClearScreen();
			CLCD_vSendString ("UserName Must be");
			CLCD_vSetPosition(2, 1);
			CLCD_vSendString("More than 5 Char");
			CLCD_vSendExtraChar(4, 1);
			CLCD_vSetPosition(4, 2);
			CLCD_vSendString(" : Exit");
			while(1)
			{
				//wait in error page until press enter
				Error_State = USART_u8ReceiveData(&KPD_Press);
				if (Error_State == OK)
				{
					if (KPD_Press == 0x0D || KPD_Press == 0x0F)
					{
						break;
					}
				}
			}
			CLCD_vClearScreen();
			CLCD_vSetPosition(1, 5);
			CLCD_vSendString("Set UserName");
			CLCD_vSetPosition(2, 1);
			CLCD_vSendString("Maximum char : 20");
			CLCD_vSendExtraChar(4, 15); // To Send Enter Symbol
			CLCD_vSetPosition(4, 16);
			CLCD_vSendString(" : OK");
			CLCD_vSetPosition(3, 1);
			UserName_Length = 0;
		}
		CLCD_vSendCommand(CLCD_DISPLAYON_CURSORON);
		//get user name from user by using Keypoard
		while (1)
		{
			//get input from Laptop
			Error_State = USART_u8ReceiveData(&KPD_Press);
			//if user name length is valid
			if (Error_State == OK && UserName_Length == 0)
			{
				if (KPD_Press == 0x0D || KPD_Press == 0x0F)
				{

				}
				else if (KPD_Press == 0x08)
				{

				}
				else
				{
					CLCD_vSendData(KPD_Press);
					EEPROM_vWrite(EEPROM_UserNameStartLocation + UserName_Length, KPD_Press);
					UserName_Length++;
				}
			}
			else if (Error_State == OK && UserName_Length < 20)
			{
				if (KPD_Press == 0x0D || KPD_Press == 0x0F)
				{
					CLCD_vSendCommand(CLCD_DISPLAYON_CURSOROFF);
					break;
				}
				else if (KPD_Press == 0x08)
				{
					Clear_Char();
					UserName_Length--;
				}
				else
				{
					CLCD_vSendData(KPD_Press);
					EEPROM_vWrite(EEPROM_UserNameStartLocation + UserName_Length, KPD_Press);
					UserName_Length++;
				}
			}
			//if user name length is more than 20 do no thing exept enter and delete
			else if (Error_State == OK && UserName_Length >= 20)
			{
				if (KPD_Press == 0x0D || KPD_Press == 0x0F)
				{
					CLCD_vSendCommand(CLCD_DISPLAYON_CURSOROFF);
					break;
				}
				else if (KPD_Press == 0x08)
				{
					Clear_Char();
					UserName_Length--;
				}
				else
				{

				}
			}
			else
			{

			}
		}
	}while(UserName_Length <= 5);
	/*___________________________________________________________________________________________________________________*/

	for (u8 i = 0; i < 20 - (UserName_Length - 1); i++)
	{
		EEPROM_vWrite(EEPROM_UserNameEndLocation - i, 0xFF);
	}
	EEPROM_vWrite(EEPROM_UserNameStatus, 0x00);
	EEPROM_vWrite(EEPROM_USNL_Location, UserName_Length);
}

//======================================================================================================================================//

void PassWord_Set(void)
{
	CLCD_vClearScreen();
	CLCD_vSetPosition(1, 5);
	CLCD_vSendString("Set PassWord");
	CLCD_vSetPosition(2, 1);
	CLCD_vSendString("Maximum char : 20");
	CLCD_vSendExtraChar(4, 15);  // To Send Enter Symbol
	CLCD_vSetPosition(4, 16);
	CLCD_vSendString(" : OK");

	CLCD_vSetPosition(3, 1);
	PassWord_Length = 0;
	/*Get username from user*/
	do
	{
		//if username is less than 5 char
		if (PassWord_Length != 0)
		{
			CLCD_vClearScreen();
			CLCD_vSendString ("PassWord Must be");
			CLCD_vSetPosition(2, 1);
			CLCD_vSendString("More than 5 Char");
			CLCD_vSendExtraChar(4, 1);
			CLCD_vSetPosition(4, 2);
			CLCD_vSendString(" : Exit");
			while(1)
			{
				Error_State = USART_u8ReceiveData(&KPD_Press);
				if (Error_State == OK)
				{
					if (KPD_Press == 0x0D || KPD_Press == 0x0F)
					{
						break;
					}
				}
			}
			CLCD_vClearScreen();
			CLCD_vSetPosition(1, 5);
			CLCD_vSendString("Set PassWord");
			CLCD_vSetPosition(2, 1);
			CLCD_vSendString("Maximum char : 20");
			CLCD_vSendExtraChar(4, 15); // To Send Enter Symbol
			CLCD_vSetPosition(4, 16);
			CLCD_vSendString(" : OK");
			CLCD_vSetPosition(3, 1);
			PassWord_Length = 0;
		}
		CLCD_vSendCommand(CLCD_DISPLAYON_CURSORON);
		//get user name from user by using Keypoard
		while (1)
		{
			//get input from Laptop
			Error_State = USART_u8ReceiveData(&KPD_Press);
			//if user name length is valid
			if (Error_State == OK && PassWord_Length == 0)
			{
				if (KPD_Press == 0x0D || KPD_Press == 0x0F)
				{

				}
				else if (KPD_Press == 0x08)
				{

				}
				else
				{
					CLCD_vSendData(KPD_Press);
					EEPROM_vWrite(EEPROM_PassWordStartLocation + PassWord_Length, KPD_Press);
					PassWord_Length++;
				}
			}
			else if (Error_State == OK && PassWord_Length < 20)
			{
				if (KPD_Press == 0x0D || KPD_Press == 0x0F)
				{
					CLCD_vSendCommand(CLCD_DISPLAYON_CURSOROFF);
					break;
				}
				else if (KPD_Press == 0x08)
				{
					Clear_Char();
					PassWord_Length--;
				}
				else
				{
					CLCD_vSendData(KPD_Press);
					EEPROM_vWrite(EEPROM_PassWordStartLocation + PassWord_Length, KPD_Press);
					PassWord_Length++;
				}
			}
			//if user name length is more than 20 do no thing exept enter and delete
			else if (Error_State == OK && PassWord_Length >= 20)
			{
				if (KPD_Press == 0x0D || KPD_Press == 0x0F)
				{
					CLCD_vSendCommand(CLCD_DISPLAYON_CURSOROFF);
					break;
				}
				else if (KPD_Press == 0x08)
				{
					Clear_Char();
					PassWord_Length--;
				}
				else
				{

				}
			}
			else
			{

			}
		}
	}while(PassWord_Length <= 5);
	/*___________________________________________________________________________________________________________________*/

	for (u8 i = 0; i < 20 - (PassWord_Length - 1); i++)
	{
		EEPROM_vWrite(EEPROM_PassWordEndLocation - i, 0xFF);
	}
	EEPROM_vWrite(EEPROM_PassWordStatus, 0x00);
	EEPROM_vWrite(EEPROM_PWL_Location, PassWord_Length);
}

//======================================================================================================================================//

void UserName_Check(void)
{
	CLCD_vClearScreen();
	CLCD_vSendString("Check UserName");
	CLCD_vSetPosition(2, 1);
	CLCD_vSendCommand(CLCD_DISPLAYON_CURSORON);
	u8 CheckLength = 0;
	UserName_Check_Flag = 1;
	while(1)
	{
		Error_State = USART_u8ReceiveData(&KPD_Press);
		//if user name length is valid
		if (Error_State == OK && CheckLength == 0)
		{
			if (KPD_Press == 0x0D || KPD_Press == 0x0F)
			{

			}
			else if (KPD_Press == 0x08)
			{

			}
			else
			{
				CLCD_vSendData(KPD_Press);
				Check[CheckLength] = KPD_Press;
				CheckLength++;
			}
		}
		else if (Error_State == OK && CheckLength < 20)
		{
			if (KPD_Press == 0x0D || KPD_Press == 0x0F)
			{
				CLCD_vSendCommand(CLCD_DISPLAYON_CURSOROFF);
				break;
			}
			else if (KPD_Press == 0x08)
			{
				Clear_Char();
				CheckLength--;
			}
			else
			{
				CLCD_vSendData(KPD_Press);
				Check[CheckLength] = KPD_Press;
				CheckLength++;
			}
		}
		//if user name length is more than 20 do no thing exept enter and delete
		else if (Error_State == OK && CheckLength>= 20)
		{
			if (KPD_Press == 0x0D || KPD_Press == 0x0F)
			{
				CLCD_vSendCommand(CLCD_DISPLAYON_CURSOROFF);
				break;
			}
			else if (KPD_Press == 0x08)
			{
				Clear_Char();
				CheckLength--;
			}

			else
			{

			}
		}
		else
		{

		}
	}

	/*___________________________________________________________________________________________________________________*/

	//Check if UserName is correct or not
	if (CheckLength == UserName_Length)
	{
		for(u8 i = 0; i < UserName_Length; i++)
		{
			if (Check[i] != EEPROM_vRead(EEPROM_UserNameStartLocation + i))
			{
				UserName_Check_Flag = 0;
			}
			else
			{

			}
		}
	}
	else if (CheckLength != UserName_Length)
	{
		UserName_Check_Flag = 0;
	}
	else
	{

	}
}

//======================================================================================================================================//

void PassWord_Check(void)
{
	CLCD_vClearScreen();
	CLCD_vSendString("Check PassWord");
	CLCD_vSetPosition(2, 1);
	CLCD_vSendCommand(CLCD_DISPLAYON_CURSORON);
	u8 CheckLength = 0;
	PassWord_Check_Flag = 1;

	while(1)
	{
		Error_State = USART_u8ReceiveData(&KPD_Press);
		//if user name length is valid
		if (Error_State == OK && CheckLength == 0)
		{
			if (KPD_Press == 0x0D || KPD_Press == 0x0F)
			{

			}
			else if (KPD_Press == 0x08)
			{

			}
			else
			{
				CLCD_vSendData(KPD_Press);
				Check[CheckLength] = KPD_Press;
				CheckLength++;
			}
		}
		else if (Error_State == OK && CheckLength < 20)
		{
			if (KPD_Press == 0x0D || KPD_Press == 0x0F)
			{
				CLCD_vSendCommand(CLCD_DISPLAYON_CURSOROFF);
				break;
			}
			else if (KPD_Press == 0x08)
			{
				Clear_Char();
				CheckLength--;
			}
			else
			{
				CLCD_vSendData(KPD_Press);
				Check[CheckLength] = KPD_Press;
				CheckLength++;
			}
		}
		//if user name length is more than 20 do no thing exept enter and delete
		else if (Error_State == OK && CheckLength>= 20)
		{
			if (KPD_Press == 0x0D || KPD_Press == 0x0F)
			{
				CLCD_vSendCommand(CLCD_DISPLAYON_CURSOROFF);
				break;
			}
			else if (KPD_Press == 0x08)
			{
				Clear_Char();
				CheckLength--;
			}

			else
			{

			}
		}
		else
		{

		}
	}

	/*___________________________________________________________________________________________________________________*/

	//Check if UserName is correct or not
	if (CheckLength == PassWord_Length)
	{
		for(u8 i = 0; i < PassWord_Length; i++)
		{
			if (Check[i] != EEPROM_vRead(EEPROM_PassWordStartLocation + i))
			{
				PassWord_Check_Flag = 0;
			}
			else
			{

			}
		}
	}
	else if (CheckLength != PassWord_Length)
	{
		PassWord_Check_Flag = 0;
	}
	else
	{

	}
}

//======================================================================================================================================//

void Sign_In(void)
{
	UserName_Check();
	PassWord_Check();
	if(UserName_Check_Flag ==0 || PassWord_Check_Flag ==0)
	{
		CLCD_vClearScreen();
		CLCD_vSendString("Invalid Username");
		CLCD_vSetPosition(2, 1);
		CLCD_vSendString("or Password");
		Max_Tries--;
		EEPROM_vWrite(EEPROM_NoTries_Location, Max_Tries);

		if (Max_Tries > 0)
		{
			CLCD_vSetPosition(3, 1);
			CLCD_vSendString("Tries Left : ");
			CLCD_vSendData(Max_Tries + 48);
		}
		else	if (Max_Tries == 0)
		{
			_delay_ms(500);
			Error_TimeOut();
		}
	}
	else
	{
		CLCD_vClearScreen();
		CLCD_vSendString("Successfully");
		CLCD_vSetPosition(2, 1);
		CLCD_vSendString("Sign in");
		_delay_ms(5000);
	}

}

//======================================================================================================================================//

void Error_TimeOut(void)
{
	CLCD_vClearScreen();
	CLCD_vSendString("Time out :  ");

	for (u8 i = 6; i > 0 ; i-- )
	{
		CLCD_vSendCommand(CLCD_SHIFT_CURSOR_LEFT);
		CLCD_vSendData(i + 47);
		_delay_ms(1000);
	}
	EEPROM_vWrite(EEPROM_NoTries_Location, NOTPRESSED);
	Max_Tries = 3;
}

//======================================================================================================================================//

void Clear_Char()
{
	CLCD_vSendCommand(CLCD_SHIFT_CURSOR_LEFT);
	CLCD_vSendData(' ');
	CLCD_vSendCommand(CLCD_SHIFT_CURSOR_LEFT);

}















































