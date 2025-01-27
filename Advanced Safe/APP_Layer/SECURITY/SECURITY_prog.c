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
volatile u8 Is_Admin = 0;
volatile u8 System_Status = 0;
volatile u8 Security_Level = SECURITY_LEVEL_MEDIUM;

/* Helper functions for EEPROM access */
static u16 Get_User_Base_Address(u8 user_index)
{
	return EEPROM_USER_START + (user_index * USER_BLOCK_SIZE);
}

static void Read_Username(u8 user_index, volatile u8 *username, volatile u8 *length)
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

/* Password validation */
u8 Is_Password_Valid(u8 *password, u8 length)
{
	if (length < PASSWORD_MIN_LENGTH)
		return 0;

	u8 has_upper = 0, has_lower = 0, has_number = 0, has_special = 0;

	for (u8 i = 0; i < length; i++)
	{
		if (password[i] >= 'A' && password[i] <= 'Z')
			has_upper = 1;
		else if (password[i] >= 'a' && password[i] <= 'z')
			has_lower = 1;
		else if (password[i] >= '0' && password[i] <= '9')
			has_number = 1;
		else
			has_special = 1;
	}

	if (PASS_NEED_UPPER && !has_upper)
		return 0;
	if (PASS_NEED_LOWER && !has_lower)
		return 0;
	if (PASS_NEED_NUMBER && !has_number)
		return 0;
	if (PASS_NEED_SPECIAL && !has_special)
		return 0;

	return 1;
}

/* Username validation */
u8 Is_Username_Exists(u8 *username, u8 length)
{
	u8 stored_username[21];
	u8 stored_length;

	for (u8 i = 0; i < User_Count; i++)
	{
		if (i == Current_User)
			continue; // Skip current user when changing username

		Read_Username(i, stored_username, &stored_length);
		if (length == stored_length)
		{
			u8 match = 1;
			for (u8 j = 0; j < length; j++)
			{
				if (username[j] != stored_username[j])
				{
					match = 0;
					break;
				}
			}
			if (match)
				return 1;
		}
	}
	return 0;
}

/* Event logging */
void Log_Event(u8 event_type, u8 user_index)
{
	CLCD_vClearScreen();
	switch (event_type)
	{
	case EVENT_LOGIN_SUCCESS:
		CLCD_vSendString((u8 *)"Login: Success");
		break;
	case EVENT_LOGIN_FAIL:
		CLCD_vSendString((u8 *)"Login: Failed");
		break;
	case EVENT_PASS_CHANGE:
		CLCD_vSendString((u8 *)"Pass Changed");
		break;
	case EVENT_USER_CHANGE:
		CLCD_vSendString((u8 *)"User Changed");
		break;
	case EVENT_USER_DELETE:
		CLCD_vSendString((u8 *)"User Deleted");
		break;
	case EVENT_USER_CREATE:
		CLCD_vSendString((u8 *)"User Created");
		break;
	}
	_delay_ms(1000);
}

/* User management functions */
void Change_Username(void)
{
	CLCD_vClearScreen();
	CLCD_vSendString((u8 *)"New Username:");

	u8 new_username[21];
	u8 new_length = 0;

	while (1)
	{
		Error_State = USART_RecieveDataFuncName(&KPD_Press);
		if (Error_State == OK)
		{
			if (KPD_Press == 0x0D || KPD_Press == 0x0F)
			{ // Enter
				if (new_length >= USERNAME_MIN_LENGTH)
					break;
			}
			else if (KPD_Press == 0x08)
			{ // Backspace
				if (new_length > 0)
				{
					new_length--;
					CLCD_vSendCommand(CLCD_SHIFT_CURSOR_LEFT);
					CLCD_vSendData(' ');
					CLCD_vSendCommand(CLCD_SHIFT_CURSOR_LEFT);
				}
			}
			else if (new_length < USERNAME_MAX_LENGTH)
			{
				new_username[new_length++] = KPD_Press;
				CLCD_vSendData(KPD_Press);
			}
		}
	}

	new_username[new_length] = '\0';

	if (Is_Username_Exists(new_username, new_length))
	{
		CLCD_vClearScreen();
		CLCD_vSendString((u8 *)"Username exists!");
		_delay_ms(1000);
		return;
	}

	Write_Username(Current_User, new_username, new_length);
	UserName_Length = new_length;
	for (u8 i = 0; i < new_length; i++)
	{
		UserName[i] = new_username[i];
	}
	UserName[new_length] = '\0';

	Log_Event(EVENT_USER_CHANGE, Current_User);
}

void Change_Password(void)
{
	CLCD_vClearScreen();
	CLCD_vSendString((u8 *)"Current Pass:");

	// First verify current password
	u8 temp_pass[21];
	u8 pass_length = 0;

	while (1)
	{
		Error_State = USART_RecieveDataFuncName(&KPD_Press);
		if (Error_State == OK)
		{
			if (KPD_Press == 0x0D || KPD_Press == 0x0F)
				break;
			else if (KPD_Press == 0x08)
			{
				if (pass_length > 0)
				{
					pass_length--;
					CLCD_vSendCommand(CLCD_SHIFT_CURSOR_LEFT);
					CLCD_vSendData(' ');
					CLCD_vSendCommand(CLCD_SHIFT_CURSOR_LEFT);
				}
			}
			else if (pass_length < PASSWORD_MAX_LENGTH)
			{
				temp_pass[pass_length++] = KPD_Press;
				CLCD_vSendData('*');
			}
		}
	}

	u8 stored_pass[21];
	u8 stored_length;
	Read_Password(Current_User, stored_pass, &stored_length);

	if (pass_length != stored_length)
	{
		CLCD_vClearScreen();
		CLCD_vSendString((u8 *)"Wrong Password!");
		_delay_ms(1000);
		return;
	}

	for (u8 i = 0; i < pass_length; i++)
	{
		if (temp_pass[i] != stored_pass[i])
		{
			CLCD_vClearScreen();
			CLCD_vSendString((u8 *)"Wrong Password!");
			_delay_ms(1000);
			return;
		}
	}

	// Get new password
	CLCD_vClearScreen();
	CLCD_vSendString((u8 *)"New Password:");
	CLCD_vSetPosition(2, 1);
	CLCD_vSendString((u8 *)"Min len: ");
	CLCD_vSendIntNumber(PASSWORD_MIN_LENGTH);

	pass_length = 0;
	while (1)
	{
		Error_State = USART_RecieveDataFuncName(&KPD_Press);
		if (Error_State == OK)
		{
			if (KPD_Press == 0x0D || KPD_Press == 0x0F)
			{
				if (pass_length >= PASSWORD_MIN_LENGTH)
					break;
			}
			else if (KPD_Press == 0x08)
			{
				if (pass_length > 0)
				{
					pass_length--;
					CLCD_vSendCommand(CLCD_SHIFT_CURSOR_LEFT);
					CLCD_vSendData(' ');
					CLCD_vSendCommand(CLCD_SHIFT_CURSOR_LEFT);
				}
			}
			else if (pass_length < PASSWORD_MAX_LENGTH)
			{
				temp_pass[pass_length++] = KPD_Press;
				CLCD_vSendData('*');
			}
		}
	}

	if (!Is_Password_Valid(temp_pass, pass_length))
	{
		CLCD_vClearScreen();
		CLCD_vSendString((u8 *)"Weak Password!");
		CLCD_vSetPosition(2, 1);
		CLCD_vSendString((u8 *)"Need: A,a,1,@");
		_delay_ms(2000);
		return;
	}

	Write_Password(Current_User, temp_pass, pass_length);
	Log_Event(EVENT_PASS_CHANGE, Current_User);
}

u8 Delete_User(void)
{
	CLCD_vClearScreen();
	CLCD_vSendString((u8 *)"Enter Pass to");
	CLCD_vSetPosition(2, 1);
	CLCD_vSendString((u8 *)"Delete Account");

	u8 temp_pass[21];
	u8 pass_length = 0;

	while (1)
	{
		Error_State = USART_RecieveDataFuncName(&KPD_Press);
		if (Error_State == OK)
		{
			if (KPD_Press == 0x0D || KPD_Press == 0x0F)
				break;
			else if (KPD_Press == 0x08)
			{
				if (pass_length > 0)
				{
					pass_length--;
					CLCD_vSendCommand(CLCD_SHIFT_CURSOR_LEFT);
					CLCD_vSendData(' ');
					CLCD_vSendCommand(CLCD_SHIFT_CURSOR_LEFT);
				}
			}
			else if (pass_length < PASSWORD_MAX_LENGTH)
			{
				temp_pass[pass_length++] = KPD_Press;
				CLCD_vSendData('*');
			}
		}
	}

	u8 stored_pass[21];
	u8 stored_length;
	Read_Password(Current_User, stored_pass, &stored_length);

	if (pass_length != stored_length)
	{
		CLCD_vClearScreen();
		CLCD_vSendString((u8 *)"Wrong Password!");
		_delay_ms(1000);
		return 0;
	}

	for (u8 i = 0; i < pass_length; i++)
	{
		if (temp_pass[i] != stored_pass[i])
		{
			CLCD_vClearScreen();
			CLCD_vSendString((u8 *)"Wrong Password!");
			_delay_ms(1000);
			return 0;
		}
	}

	// Move all users after current user one position back
	for (u8 i = Current_User; i < User_Count - 1; i++)
	{
		u8 next_user = i + 1;
		u8 username[21], password[21];
		u8 uname_len, pass_len;

		Read_Username(next_user, username, &uname_len);
		Read_Password(next_user, password, &pass_len);

		Write_Username(i, username, uname_len);
		Write_Password(i, password, pass_len);
	}

	User_Count--;
	EEPROM_FunWriteName(EEPROM_UserCount_Location, User_Count);

	Log_Event(EVENT_USER_DELETE, Current_User);
	return 1;
}

void Delete_User_By_Admin(u8 user_index)
{
	if (!Is_Admin || user_index >= User_Count)
		return;

	// Move all users after deleted user one position back
	for (u8 i = user_index; i < User_Count - 1; i++)
	{
		u8 next_user = i + 1;
		u8 username[21], password[21];
		u8 uname_len, pass_len;

		Read_Username(next_user, username, &uname_len);
		Read_Password(next_user, password, &pass_len);

		Write_Username(i, username, uname_len);
		Write_Password(i, password, pass_len);
	}

	User_Count--;
	EEPROM_FunWriteName(EEPROM_UserCount_Location, User_Count);

	Log_Event(EVENT_USER_DELETE, user_index);
}

void List_Users(void)
{
	if (!Is_Admin)
		return;

	CLCD_vClearScreen();
	CLCD_vSendString((u8 *)"Users List:");

	for (u8 i = 0; i < User_Count; i++)
	{
		if (i > 3)
		{
			CLCD_vSetPosition(4, 1);
			CLCD_vSendString((u8 *)"More...");
			_delay_ms(2000);
			CLCD_vClearScreen();
			CLCD_vSendString((u8 *)"Users List:");
			i = i - 3;
		}

		u8 username[21];
		u8 length;
		Read_Username(i, username, &length);

		CLCD_vSetPosition(i % 3 + 1, 1);
		CLCD_vSendIntNumber(i + 1);
		CLCD_vSendString((u8 *)": ");
		CLCD_vSendString(username);
	}

	_delay_ms(3000);
}

void User_Menu(void)
{
	while (1)
	{
		CLCD_vClearScreen();
		CLCD_vSendString((u8 *)"1:Change Pass");
		CLCD_vSetPosition(2, 1);
		CLCD_vSendString((u8 *)"2:Change User");
		CLCD_vSetPosition(3, 1);
		CLCD_vSendString((u8 *)"3:Delete Account");
		CLCD_vSetPosition(4, 1);
		CLCD_vSendString((u8 *)"4:Logout");

		while (1)
		{
			Error_State = USART_RecieveDataFuncName(&KPD_Press);
			if (Error_State == OK)
			{
				if (KPD_Press >= '1' && KPD_Press <= '4')
					break;
			}
		}

		switch (KPD_Press)
		{
		case '1':
			Change_Password();
			break;
		case '2':
			Change_Username();
			break;
		case '3':
			if (Delete_User())
				return;
			break;
		case '4':
			return;
		}
	}
}

void Admin_Menu(void)
{
	while (1)
	{
		CLCD_vClearScreen();
		CLCD_vSendString((u8 *)"1:List Users");
		CLCD_vSetPosition(2, 1);
		CLCD_vSendString((u8 *)"2:Delete User");
		CLCD_vSetPosition(3, 1);
		CLCD_vSendString((u8 *)"3:User Menu");
		CLCD_vSetPosition(4, 1);
		CLCD_vSendString((u8 *)"4:Logout");

		while (1)
		{
			Error_State = USART_RecieveDataFuncName(&KPD_Press);
			if (Error_State == OK)
			{
				if (KPD_Press >= '1' && KPD_Press <= '4')
					break;
			}
		}

		switch (KPD_Press)
		{
		case '1':
			List_Users();
			break;
		case '2':
		{
			CLCD_vClearScreen();
			CLCD_vSendString((u8 *)"User Number:");

			u8 user_num = 0;
			while (1)
			{
				Error_State = USART_RecieveDataFuncName(&KPD_Press);
				if (Error_State == OK)
				{
					if (KPD_Press >= '0' && KPD_Press <= '9')
					{
						user_num = user_num * 10 + (KPD_Press - '0');
						CLCD_vSendData(KPD_Press);
					}
					else if (KPD_Press == 0x0D || KPD_Press == 0x0F)
					{
						if (user_num > 0 && user_num <= User_Count)
							break;
					}
				}
			}
			Delete_User_By_Admin(user_num - 1);
			break;
		}
		case '3':
			User_Menu();
			break;
		case '4':
			return;
		}
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
		CLCD_vSendString((u8 *)"Max Users Reached");
		_delay_ms(1000);
		return;
	}

	CLCD_vClearScreen();
	CLCD_vSendString((u8 *)"Set UserName");
	CLCD_vSetPosition(2, 1);
	CLCD_vSendString((u8 *)"Max chars: ");
	CLCD_vSendIntNumber(USERNAME_MAX_LENGTH);

	UserName_Length = 0;
	u8 temp_username[21];

	do
	{
		if (UserName_Length != 0)
		{
			CLCD_vClearScreen();
			CLCD_vSendString((u8 *)"Min Length: ");
			CLCD_vSendIntNumber(USERNAME_MIN_LENGTH);
			_delay_ms(1000);
			CLCD_vClearScreen();
			CLCD_vSendString((u8 *)"Set UserName");
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
	CLCD_vSendString((u8 *)"Set Password");
	CLCD_vSetPosition(2, 1);
	CLCD_vSendString((u8 *)"Max chars: ");
	CLCD_vSendIntNumber(PASSWORD_MAX_LENGTH);

	PassWord_Length = 0;
	u8 temp_password[21];

	do
	{
		if (PassWord_Length != 0)
		{
			CLCD_vClearScreen();
			CLCD_vSendString((u8 *)"Min Length: ");
			CLCD_vSendIntNumber(PASSWORD_MIN_LENGTH);
			_delay_ms(1000);
			CLCD_vClearScreen();
			CLCD_vSendString((u8 *)"Set Password");
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
	CLCD_vSendString((u8 *)"Enter Username");

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
	CLCD_vSendString((u8 *)"Enter Password");

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
			CLCD_vSendString((u8 *)"Invalid Login");

			Tries--;
			EEPROM_FunWriteName(EEPROM_NoTries_Location, Tries);

			if (Tries > 0)
			{
				CLCD_vSetPosition(2, 1);
				CLCD_vSendString((u8 *)"Tries Left: ");
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
			CLCD_vSendString((u8 *)"Login Success!");
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
	CLCD_vSendString((u8 *)"Time out: ");

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

u8 System_GetStatus(void)
{
	return EEPROM_FunReadName(EEPROM_SYSTEM_STATUS);
}

void System_SetStatus(u8 status)
{
	EEPROM_FunWriteName(EEPROM_SYSTEM_STATUS, status);
}

u8 System_GetSecurityLevel(void)
{
	return Security_Level;
}

void System_SetSecurityLevel(u8 level)
{
	if (level <= SECURITY_LEVEL_HIGH)
	{
		Security_Level = level;
	}
}

u8 Calculate_Checksum(void)
{
	u8 checksum = 0;
	for (u16 addr = EEPROM_START_ADDRESS; addr < EEPROM_CHECKSUM_Location; addr++)
	{
		checksum ^= EEPROM_FunReadName(addr);
	}
	return checksum;
}

void Update_Checksum(void)
{
	u8 checksum = Calculate_Checksum();
	EEPROM_FunWriteName(EEPROM_CHECKSUM_Location, checksum);
}

u8 Verify_System_Integrity(void)
{
	u8 stored_checksum = EEPROM_FunReadName(EEPROM_CHECKSUM_Location);
	u8 calculated_checksum = Calculate_Checksum();
	return (stored_checksum == calculated_checksum);
}

void Create_Backup(void)
{
	// Copy main data to backup section
	for (u16 addr = EEPROM_START_ADDRESS; addr < EEPROM_BACKUP_START; addr++)
	{
		u8 data = EEPROM_FunReadName(addr);
		EEPROM_FunWriteName(EEPROM_BACKUP_START + addr, data);
	}

	// Set backup valid flag
	System_SetStatus(System_GetStatus() | SYSTEM_BACKUP_VALID);
	Log_Event(EVENT_BACKUP_CREATE, Current_User);
}

u8 Restore_Backup(void)
{
	if (!(System_GetStatus() & SYSTEM_BACKUP_VALID))
	{
		return 0;
	}

	// Restore data from backup section
	for (u16 addr = EEPROM_START_ADDRESS; addr < EEPROM_BACKUP_START; addr++)
	{
		u8 data = EEPROM_FunReadName(EEPROM_BACKUP_START + addr);
		EEPROM_FunWriteName(addr, data);
	}

	Log_Event(EVENT_BACKUP_RESTORE, Current_User);
	return 1;
}

void Factory_Reset(void)
{
	// Clear all user data
	for (u16 addr = EEPROM_START_ADDRESS; addr <= EEPROM_END_ADDRESS; addr++)
	{
		EEPROM_FunWriteName(addr, 0xFF);
	}

	// Reset system status
	System_SetStatus(SYSTEM_INITIALIZED);
	User_Count = 0;
	Tries = Tries_Max;

	Log_Event(EVENT_SYSTEM_RESET, 0);
}

void System_Lock(void)
{
	System_SetStatus(System_GetStatus() | SYSTEM_LOCKED);
	Log_Event(EVENT_SYSTEM_LOCK, Current_User);
}

void System_Unlock(void)
{
	System_SetStatus(System_GetStatus() & ~SYSTEM_LOCKED);
	Log_Event(EVENT_SYSTEM_UNLOCK, Current_User);
}

u8 Is_System_Locked(void)
{
	return (System_GetStatus() & SYSTEM_LOCKED) != 0;
}

u8 Get_User_Type(u8 user_index)
{
	if (user_index == 0)
		return USER_TYPE_ADMIN;
	if (user_index >= User_Count)
		return USER_TYPE_GUEST;
	return USER_TYPE_NORMAL;
}

u8 Set_User_Type(u8 user_index, u8 type)
{
	if (user_index == 0 || user_index >= User_Count)
		return 0;
	if (type > USER_TYPE_SUPER)
		return 0;
	// Store user type in EEPROM (implementation needed)
	return 1;
}

void Sign_Out(void)
{
	Current_User = 0xFF;
	Is_Admin = 0;
}

u8 Verify_Password(u8 *password, u8 length)
{
	if (!Is_Password_Valid(password, length))
		return 0;

	u8 stored_pass[21];
	u8 stored_length;
	Read_Password(Current_User, stored_pass, &stored_length);

	if (length != stored_length)
		return 0;

	for (u8 i = 0; i < length; i++)
	{
		if (password[i] != stored_pass[i])
			return 0;
	}

	return 1;
}

void Super_Admin_Menu(void)
{
	while (1)
	{
		CLCD_vClearScreen();
		CLCD_vSendString((u8 *)"1:Backup");
		CLCD_vSetPosition(2, 1);
		CLCD_vSendString((u8 *)"2:Restore");
		CLCD_vSetPosition(3, 1);
		CLCD_vSendString((u8 *)"3:Factory Reset");
		CLCD_vSetPosition(4, 1);
		CLCD_vSendString((u8 *)"4:Back");

		u8 choice;
		while (1)
		{
			Error_State = USART_RecieveDataFuncName(&choice);
			if (Error_State == OK)
			{
				if (choice >= '1' && choice <= '4')
					break;
			}
		}

		switch (choice)
		{
		case '1':
			Create_Backup();
			break;
		case '2':
			if (!Restore_Backup())
			{
				CLCD_vClearScreen();
				CLCD_vSendString((u8 *)"No Valid Backup!");
				_delay_ms(2000);
			}
			break;
		case '3':
			CLCD_vClearScreen();
			CLCD_vSendString((u8 *)"Are you sure?");
			CLCD_vSetPosition(2, 1);
			CLCD_vSendString((u8 *)"1:Yes 2:No");

			while (1)
			{
				Error_State = USART_RecieveDataFuncName(&choice);
				if (Error_State == OK)
				{
					if (choice == '1')
					{
						Factory_Reset();
						return;
					}
					else if (choice == '2')
						break;
				}
			}
			break;
		case '4':
			return;
		}
	}
}

void Maintenance_Menu(void)
{
	if (!Is_Admin)
		return;

	while (1)
	{
		CLCD_vClearScreen();
		CLCD_vSendString((u8 *)"1:System Status");
		CLCD_vSetPosition(2, 1);
		CLCD_vSendString((u8 *)"2:Security Level");
		CLCD_vSetPosition(3, 1);
		CLCD_vSendString((u8 *)"3:Verify System");
		CLCD_vSetPosition(4, 1);
		CLCD_vSendString((u8 *)"4:Back");

		u8 choice;
		while (1)
		{
			Error_State = USART_RecieveDataFuncName(&choice);
			if (Error_State == OK)
			{
				if (choice >= '1' && choice <= '4')
					break;
			}
		}

		switch (choice)
		{
		case '1':
			CLCD_vClearScreen();
			CLCD_vSendString((u8 *)"Status: ");
			if (System_GetStatus() & SYSTEM_LOCKED)
				CLCD_vSendString((u8 *)"Locked");
			else
				CLCD_vSendString((u8 *)"Unlocked");
			_delay_ms(2000);
			break;

		case '2':
			CLCD_vClearScreen();
			CLCD_vSendString((u8 *)"Security Level:");
			CLCD_vSetPosition(2, 1);
			CLCD_vSendString((u8 *)"1:Low 2:Med 3:High");

			while (1)
			{
				Error_State = USART_RecieveDataFuncName(&choice);
				if (Error_State == OK)
				{
					if (choice >= '1' && choice <= '3')
					{
						System_SetSecurityLevel(choice - '1');
						break;
					}
				}
			}
			break;

		case '3':
			CLCD_vClearScreen();
			CLCD_vSendString((u8 *)"Verifying...");
			if (Verify_System_Integrity())
				CLCD_vSendString((u8 *)"OK");
			else
				CLCD_vSendString((u8 *)"FAIL");
			_delay_ms(2000);
			break;

		case '4':
			return;
		}
	}
}
