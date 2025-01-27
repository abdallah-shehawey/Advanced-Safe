/*
 * SECURITY_program.c
 *
 *  Author : Abdallah Abdelmoemen Shehawey
 *  Layer  : APP_Layer
 */

#include <util/delay.h>

#include "../STD_TYPES.h"
#include "../STD_MACROS.h"

#include "SECURITY_config.h"
#include "SECURITY_interface.h"

#include "../../MCAL_Layer/DIO/DIO_interface.h"
#include "../../MCAL_Layer/EEPROM/EEPROM_interface.h"
#include "../../MCAL_Layer/USART/USART_interface.h"
#include "../../HAL_Layer/CLCD/CLCD_interface.h"

/* Global variables */
volatile u8 Error_State, KPD_Press;
volatile u8 UserName[20];
volatile u8 UserName_Length = 0, PassWord_Length = 0;
volatile u8 Tries = Tries_Max;
volatile u8 Check[21];
volatile u8 UserName_Check_Flag = 1, PassWord_Check_Flag = 1;
volatile u8 Current_User = 0;
volatile u8 User_Count = 0;

/* Helper functions for EEPROM access */
static u16 Get_User_Base_Address(u8 user_index)
{
	return EEPROM_USER_START + (user_index * USER_BLOCK_SIZE);
}

static void Read_Username(u8 user_index, u8 *username, u8 *length)
{
	u16 base_addr = Get_User_Base_Address(user_index);
	*length = EEPROM_FunReadName(base_addr + USER_NAME_LENGTH_OFFSET);
	for (u8 i = 0; i < *length; i++)
	{
		username[i] = EEPROM_FunReadName(base_addr + USER_NAME_START_OFFSET + i);
	}
	username[*length] = '\0';
}

static void Write_Username(u8 user_index, u8 *username, u8 length)
{
	u16 base_addr = Get_User_Base_Address(user_index);
	EEPROM_FunWriteName(base_addr + USER_NAME_LENGTH_OFFSET, length);
	for (u8 i = 0; i < length; i++)
	{
		EEPROM_FunWriteName(base_addr + USER_NAME_START_OFFSET + i, username[i]);
	}
}

static void Read_Password(u8 user_index, u8 *password, u8 *length)
{
	u16 base_addr = Get_User_Base_Address(user_index);
	*length = EEPROM_FunReadName(base_addr + USER_PASS_LENGTH_OFFSET);
	for (u8 i = 0; i < *length; i++)
	{
		password[i] = EEPROM_FunReadName(base_addr + USER_PASS_START_OFFSET + i);
	}
	password[*length] = '\0';
}

static void Write_Password(u8 user_index, u8 *password, u8 length)
{
	u16 base_addr = Get_User_Base_Address(user_index);
	EEPROM_FunWriteName(base_addr + USER_PASS_LENGTH_OFFSET, length);
	for (u8 i = 0; i < length; i++)
	{
		EEPROM_FunWriteName(base_addr + USER_PASS_START_OFFSET + i, password[i]);
	}
}

void EEPROM_vInit(void)
{
	/* Read number of users */
	User_Count = EEPROM_FunReadName(EEPROM_UserCount_Location);
	if (User_Count == 0xFF)
	{ // First time initialization
		User_Count = 0;
		EEPROM_FunWriteName(EEPROM_UserCount_Location, User_Count);
	}

	/* Read number of tries left */
	if (EEPROM_FunReadName(EEPROM_NoTries_Location) != NOTPRESSED)
	{
		Tries = EEPROM_FunReadName(EEPROM_NoTries_Location);
		if (Tries == 0)
		{
			Error_TimeOut();
		}
	}
}

void UserName_Set(void)
{
	if (User_Count >= MAX_USERS)
	{
		CLCD_vClearScreen();
		CLCD_vSendString("Max Users Reached");
		_delay_ms(1000);
		return;
	}

	CLCD_vClearScreen();
	CLCD_vSendString("Set UserName");
	CLCD_vSetPosition(2, 1);
	CLCD_vSendString("Max chars: ");
	CLCD_vSendIntNumber(USERNAME_MAX_LENGTH);

	UserName_Length = 0;
	u8 temp_username[21];

	do
	{
		if (UserName_Length != 0)
		{
			CLCD_vClearScreen();
			CLCD_vSendString("Min Length: ");
			CLCD_vSendIntNumber(USERNAME_MIN_LENGTH);
			_delay_ms(1000);
			CLCD_vClearScreen();
			CLCD_vSendString("Set UserName");
			UserName_Length = 0;
		}

		while (1)
		{
			Error_State = USART_RecieveDataFuncName(&KPD_Press);

			if (Error_State == OK)
			{
				if (KPD_Press == 0x0D || KPD_Press == 0x0F)
				{ // Enter key
					if (UserName_Length >= USERNAME_MIN_LENGTH)
						break;
				}
				else if (KPD_Press == 0x08)
				{ // Backspace
					if (UserName_Length > 0)
					{
						UserName_Length--;
					}
				}
				else if (UserName_Length < USERNAME_MAX_LENGTH)
				{
					temp_username[UserName_Length] = KPD_Press;
					UserName_Length++;
				}
			}
		}
	} while (UserName_Length < USERNAME_MIN_LENGTH);

	temp_username[UserName_Length] = '\0';
	Write_Username(User_Count, temp_username, UserName_Length);

	for (u8 i = 0; i < UserName_Length; i++)
	{
		UserName[i] = temp_username[i];
	}
	UserName[UserName_Length] = '\0';
}

void PassWord_Set(void)
{
	CLCD_vClearScreen();
	CLCD_vSendString("Set Password");
	CLCD_vSetPosition(2, 1);
	CLCD_vSendString("Max chars: ");
	CLCD_vSendIntNumber(PASSWORD_MAX_LENGTH);

	PassWord_Length = 0;
	u8 temp_password[21];

	do
	{
		if (PassWord_Length != 0)
		{
			CLCD_vClearScreen();
			CLCD_vSendString("Min Length: ");
			CLCD_vSendIntNumber(PASSWORD_MIN_LENGTH);
			_delay_ms(1000);
			CLCD_vClearScreen();
			CLCD_vSendString("Set Password");
			PassWord_Length = 0;
		}

		while (1)
		{
			Error_State = USART_RecieveDataFuncName(&KPD_Press);

			if (Error_State == OK)
			{
				if (KPD_Press == 0x0D || KPD_Press == 0x0F)
				{ // Enter key
					if (PassWord_Length >= PASSWORD_MIN_LENGTH)
						break;
				}
				else if (KPD_Press == 0x08)
				{ // Backspace
					if (PassWord_Length > 0)
					{
						PassWord_Length--;
					}
				}
				else if (PassWord_Length < PASSWORD_MAX_LENGTH)
				{
					temp_password[PassWord_Length] = KPD_Press;
					CLCD_vSendData('*'); // Show * for password
					PassWord_Length++;
				}
			}
		}
	} while (PassWord_Length < PASSWORD_MIN_LENGTH);

	Write_Password(User_Count, temp_password, PassWord_Length);
	User_Count++;
	EEPROM_FunWriteName(EEPROM_UserCount_Location, User_Count);
}

void UserName_Check(void)
{
	CLCD_vClearScreen();
	CLCD_vSendString("Enter Username");

	u8 CheckLength = 0;
	UserName_Check_Flag = 0;

	while (1)
	{
		Error_State = USART_RecieveDataFuncName(&KPD_Press);

		if (Error_State == OK)
		{
			if (KPD_Press == 0x0D || KPD_Press == 0x0F)
			{ // Enter key
				break;
			}
			else if (KPD_Press == 0x08)
			{ // Backspace
				if (CheckLength > 0)
				{
					CheckLength--;
				}
			}
			else if (CheckLength < USERNAME_MAX_LENGTH)
			{
				Check[CheckLength] = KPD_Press;
				CheckLength++;
			}
		}
	}
	Check[CheckLength] = '\0';

	// Check against all stored usernames
	u8 stored_username[21];
	u8 stored_length;

	for (u8 i = 0; i < User_Count; i++)
	{
		Read_Username(i, stored_username, &stored_length);

		if (CheckLength == stored_length)
		{
			u8 match = 1;
			for (u8 j = 0; j < CheckLength; j++)
			{
				if (Check[j] != stored_username[j])
				{
					match = 0;
					break;
				}
			}
			if (match)
			{
				UserName_Check_Flag = 1;
				Current_User = i;
				break;
			}
		}
	}
}

void PassWord_Check(void)
{
	CLCD_vClearScreen();
	CLCD_vSendString("Enter Password");

	u8 CheckLength = 0;
	PassWord_Check_Flag = 0;

	while (1)
	{
		Error_State = USART_RecieveDataFuncName(&KPD_Press);

		if (Error_State == OK)
		{
			if (KPD_Press == 0x0D || KPD_Press == 0x0F)
			{ // Enter key
				break;
			}
			else if (KPD_Press == 0x08)
			{ // Backspace
				if (CheckLength > 0)
				{
					CheckLength--;
				}
			}
			else if (CheckLength < PASSWORD_MAX_LENGTH)
			{
				Check[CheckLength] = KPD_Press;
				CLCD_vSendData('*'); // Show * for password
				CheckLength++;
			}
		}
	}

	// Check password for current user
	u8 stored_password[21];
	u8 stored_length;

	Read_Password(Current_User, stored_password, &stored_length);

	if (CheckLength == stored_length)
	{
		PassWord_Check_Flag = 1;
		for (u8 i = 0; i < CheckLength; i++)
		{
			if (Check[i] != stored_password[i])
			{
				PassWord_Check_Flag = 0;
				break;
			}
		}
	}
}

void Sign_In(void)
{
	while (1)
	{
		UserName_Check();
		if (UserName_Check_Flag)
		{
			PassWord_Check();
		}

		if (UserName_Check_Flag == 0 || PassWord_Check_Flag == 0)
		{
			CLCD_vClearScreen();
			CLCD_vSendString("Invalid Login");

			Tries--;
			EEPROM_FunWriteName(EEPROM_NoTries_Location, Tries);

			if (Tries > 0)
			{
				CLCD_vSetPosition(2, 1);
				CLCD_vSendString("Tries Left: ");
				CLCD_vSendIntNumber(Tries);
				_delay_ms(1000);
			}
			else
			{
				Error_TimeOut();
			}
		}
		else
		{
			CLCD_vClearScreen();
			CLCD_vSendString("Login Success!");
			_delay_ms(1000);

			// Reset tries on successful login
			EEPROM_FunWriteName(EEPROM_NoTries_Location, NOTPRESSED);
			Tries = Tries_Max;

			// Read and display username
			Read_Username(Current_User, UserName, &UserName_Length);
			break;
		}
	}
}

void Error_TimeOut(void)
{
	CLCD_vClearScreen();
	CLCD_vSendString("Time out: ");

	for (u8 i = 5; i > 0; i--)
	{
		CLCD_vSetPosition(1, 10);
		CLCD_vSendIntNumber(i);
		_delay_ms(1000);
	}

	EEPROM_FunWriteName(EEPROM_NoTries_Location, NOTPRESSED);
	Tries = Tries_Max;
}

void Clear_Char(void)
{
	CLCD_vSendCommand(CLCD_SHIFT_CURSOR_LEFT);
	CLCD_vSendData(' ');
	CLCD_vSendCommand(CLCD_SHIFT_CURSOR_LEFT);
}
