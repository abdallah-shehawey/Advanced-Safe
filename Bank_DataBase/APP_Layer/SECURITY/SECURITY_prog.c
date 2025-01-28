/*
 * SECURITY_program.c
 *
 * Author : Abdallah Abdelmoemen Shehawey
 * Layer  : APP_Layer
 * Description: Implements security features including user authentication,
 *             password management, and access control for the system
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

/* Global Variables - System State */
volatile u8 Error_State;             // Current operation error state
volatile u8 KPD_Press;               // Last keypad press value
volatile u8 UserName[20];            // Current username buffer
volatile u8 UserName_Length = 0;     // Length of current username
volatile u8 PassWord_Length = 0;     // Length of current password
volatile u8 Tries = Tries_Max;       // Remaining login attempts
volatile u8 Check[21];               // Buffer for verification operations
volatile u8 UserName_Check_Flag = 1; // Username verification flag
volatile u8 PassWord_Check_Flag = 1; // Password verification flag
volatile u8 Current_User = 0;        // Index of currently active user
volatile u8 User_Count = 0;          // Total number of registered users
volatile u8 Is_Admin = 0;            // Admin privilege flag

//=====================================================================================//

/* EEPROM Access Helper Functions */

/**
 * @brief Calculates the base EEPROM address for a user's data block
 * @param user_index Index of the user (0 to MAX_USERS-1)
 * @return Base address for the user's data in EEPROM
 */
static u16 Get_User_Base_Address(u8 user_index)
{
  return EEPROM_USER_START + (user_index * USER_BLOCK_SIZE);
}

/**
 * @brief Reads a username from EEPROM
 * @param user_index Index of the user to read
 * @param username Buffer to store the username
 * @param length Pointer to store username length
 */
static void Read_Username(u8 user_index, volatile u8 *username, volatile u8 *length)
{
  u16 base_addr = Get_User_Base_Address(user_index);
  *length = EEPROM_vRead(base_addr + USER_NAME_LENGTH_OFFSET);
  for (u8 i = 0; i < *length; i++)
  {
    username[i] = EEPROM_vRead(base_addr + USER_NAME_START_OFFSET + i);
  }
  username[*length] = '\0';
}

/**
 * @brief Writes a username to EEPROM
 * @param user_index Index of the user to write
 * @param username Username to store
 * @param length Length of the username
 */
static void Write_Username(u8 user_index, u8 *username, u8 length)
{
  u16 base_addr = Get_User_Base_Address(user_index);
  EEPROM_vWrite(base_addr + USER_NAME_LENGTH_OFFSET, length);
  for (u8 i = 0; i < length; i++)
  {
    EEPROM_vWrite(base_addr + USER_NAME_START_OFFSET + i, username[i]);
  }
}

/**
 * @brief Reads a password from EEPROM
 * @param user_index Index of the user
 * @param password Buffer to store the password
 * @param length Pointer to store password length
 */
static void Read_Password(u8 user_index, u8 *password, u8 *length)
{
  u16 base_addr = Get_User_Base_Address(user_index);
  *length = EEPROM_vRead(base_addr + USER_PASS_LENGTH_OFFSET);
  for (u8 i = 0; i < *length; i++)
  {
    password[i] = EEPROM_vRead(base_addr + USER_PASS_START_OFFSET + i);
  }
  password[*length] = '\0';
}

/**
 * @brief Writes a password to EEPROM
 * @param user_index Index of the user
 * @param password Password to store
 * @param length Length of the password
 */
static void Write_Password(u8 user_index, u8 *password, u8 length)
{
  u16 base_addr = Get_User_Base_Address(user_index);
  EEPROM_vWrite(base_addr + USER_PASS_LENGTH_OFFSET, length);
  for (u8 i = 0; i < length; i++)
  {
    EEPROM_vWrite(base_addr + USER_PASS_START_OFFSET + i, password[i]);
  }
}

/**
 * @brief Validates password complexity requirements
 * @param password Password to validate
 * @param length Length of the password
 * @return true if password meets all requirements, false otherwise
 * @details Checks for:
 *         - Minimum length
 *         - Uppercase letters (if required)
 *         - Lowercase letters (if required)
 *         - Numbers (if required)
 *         - Special characters (if required)
 */
bool Is_Password_Valid(u8 *password, u8 length)
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
    return false;
  if (PASS_NEED_LOWER && !has_lower)
    return false;
  if (PASS_NEED_NUMBER && !has_number)
    return false;
  if (PASS_NEED_SPECIAL && !has_special)
    return false;

  return true;
}

/**
 * @brief Checks if a username already exists in the system
 * @param username Username to check
 * @param length Length of the username
 * @return true if username exists, false otherwise
 */
bool Is_Username_Exists(u8 *username, u8 length)
{
  u8 stored_username[21];
  u8 stored_length;

  for (u8 i = 0; i < User_Count; i++)
  {
    if (i == Current_User)
      continue; // Skip current user when changing username

    Read_Username(i, stored_username, &stored_length);

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
      return true;
  }
  return false;
}

/**
 * @brief Logs system events and displays them on LCD
 * @param event_type Type of event (login, password change, etc.)
 * @param user_index Index of user involved in the event
 */
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

//=====================================================================================//

/* User management functions */
/**
 * @brief Changes username for current user
 * @details Prompts for new username, validates it's not taken,
 *          and updates EEPROM storage
 */
void Change_Username(void)
{
  CLCD_vClearScreen();
  CLCD_vSendString((u8 *)"New Username:");
  CLCD_vSetPosition(2, 1);

  u8 new_username[21];
  u8 new_length = 0;
  do
  {
    while (1)
    {
      Error_State = USART_u8ReceiveData(&KPD_Press);
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
      CLCD_vClearScreen();
      CLCD_vSendString((u8 *)"New Username:");
      CLCD_vSetPosition(2, 1);
    }
  } while (Is_Username_Exists(new_username, new_length));

  //=====================================================================================//

  Write_Username(Current_User, new_username, new_length);
  UserName_Length = new_length;
  for (u8 i = 0; i < new_length; i++)
  {
    UserName[i] = new_username[i];
  }
  UserName[new_length] = '\0';

  Log_Event(EVENT_USER_CHANGE, Current_User);
}

//=====================================================================================//

/**
 * @brief Changes password for current user
 * @details Prompts for current password, validates it,
 *          then allows setting new password with complexity requirements
 */
void Change_Password(void)
{
  u8 password_flag = 1;
  CLCD_vClearScreen();
  CLCD_vSendString((u8 *)"Current Pass:");
  CLCD_vSetPosition(2, 1);

  // First verify current password
  u8 temp_pass[21];
  u8 pass_length = 0;
  do
  {
    password_flag = 1;
    while (1)
    {
      Error_State = USART_u8ReceiveData(&KPD_Press);
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
          CLCD_vSendData(KPD_Press);
          _delay_ms(200);
          Clear_Char();
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
      password_flag = 0;
      _delay_ms(1000);
      CLCD_vClearScreen();
      CLCD_vSendString((u8 *)"Current Pass:");
      CLCD_vSetPosition(2, 1);
    }
    else
    {
      for (u8 i = 0; i < pass_length; i++)
      {
        if (temp_pass[i] != stored_pass[i])
        {
          CLCD_vClearScreen();
          CLCD_vSendString((u8 *)"Wrong Password!");
          password_flag = 0;
          _delay_ms(1000);
          CLCD_vClearScreen();
          CLCD_vSendString((u8 *)"Current Pass:");
          CLCD_vSetPosition(2, 1);
        }
      }
    }
  } while (password_flag == 0);
  // Get new password
  CLCD_vClearScreen();
  CLCD_vSendString((u8 *)"New Password:");
  CLCD_vSetPosition(2, 1);
  CLCD_vSendString((u8 *)"Min len: ");
  CLCD_vSendIntNumber(PASSWORD_MIN_LENGTH);

  do
  {
    pass_length = 0;
    while (1)
    {
      Error_State = USART_u8ReceiveData(&KPD_Press);
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
          CLCD_vSendData(KPD_Press);
          _delay_ms(200);
          Clear_Char();
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
      CLCD_vClearScreen();
      CLCD_vSendString((u8 *)"New Password:");
      CLCD_vSetPosition(2, 1);
      CLCD_vSendString((u8 *)"Min len: ");
      CLCD_vSendIntNumber(PASSWORD_MIN_LENGTH);
    }
  } while (!Is_Password_Valid(temp_pass, pass_length));

  Write_Password(Current_User, temp_pass, pass_length);
  Log_Event(EVENT_PASS_CHANGE, Current_User);
}

//=====================================================================================//

/**
 * @brief Deletes current user account
 * @return true if deletion successful, false if cancelled
 * @details Requires password confirmation before deletion
 */
bool Delete_User(void)
{
  CLCD_vClearScreen();
  CLCD_vSendString((u8 *)"Enter Pass to");
  CLCD_vSetPosition(2, 1);
  CLCD_vSendString((u8 *)"Delete Account");
  CLCD_vSetPosition(3, 1);

  u8 temp_pass[21];
  u8 pass_length = 0;

  while (1)
  {
    Error_State = USART_u8ReceiveData(&KPD_Press);
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
      return false;
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
  EEPROM_vWrite(EEPROM_UserCount_Location, User_Count);

  Log_Event(EVENT_USER_DELETE, Current_User);
  return true;
}

//=====================================================================================//

/**
 * @brief Admin function to delete any user account
 * @param user_index Index of user to delete
 * @details Only available to admin users, no confirmation needed
 */
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
  EEPROM_vWrite(EEPROM_UserCount_Location, User_Count);

  Log_Event(EVENT_USER_DELETE, user_index);
}

//=====================================================================================//

/**
 * @brief Lists all registered users
 * @details Displays usernames of all registered users on LCD
 */
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

//=====================================================================================//

/**
 * @brief Displays and handles regular user menu
 * @details Options include:
 *         - Change username
 *         - Change password
 *         - Delete account
 *         - Sign out
 */
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
      Error_State = USART_u8ReceiveData(&KPD_Press);
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

/**
 * @brief Displays and handles admin menu
 * @details Additional options beyond user menu:
 *         - List all users
 *         - Delete other users
 *         - Factory reset
 */
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
    CLCD_vSendString((u8 *)"4:Factory Reset");

    while (1)
    {
      Error_State = USART_u8ReceiveData(&KPD_Press);
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
        Error_State = USART_u8ReceiveData(&KPD_Press);
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
      Factory_Reset();
      break;
    case 0x08:
      return;
    }
  }
}

//=====================================================================================//

/**
 * @brief Initializes EEPROM for first use
 * @details Sets up initial state if EEPROM is empty
 */
void EEPROM_vInit(void)
{
  /* Read number of users */
  User_Count = EEPROM_vRead(EEPROM_UserCount_Location);
  if (User_Count == 0xFF)
  { // First time initialization
    User_Count = 0;
    EEPROM_vWrite(EEPROM_UserCount_Location, User_Count);
  }

  /* Read number of tries left */
  if (EEPROM_vRead(EEPROM_NoTries_Location) != NOTPRESSED)
  {
    Tries = EEPROM_vRead(EEPROM_NoTries_Location);
    if (Tries == 0)
    {
      Error_TimeOut();
    }
  }
}

//=====================================================================================//

/**
 * @brief Sets up new username
 * @details Handles input and validation of new username,
 *          ensures uniqueness and proper length
 */
void UserName_Set(void)
{
  USART_u8SendData(0x0D);
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
    UserName_Length = 0;
    do
    {
      if (UserName_Length != 0)
      {
        CLCD_vClearScreen();
        CLCD_vSendString((u8 *)"Min Length: ");
        CLCD_vSendIntNumber(USERNAME_MIN_LENGTH);
        _delay_ms(1000);
        CLCD_vClearScreen();
        CLCD_vSendString((u8 *)"Re Set UserName");
        CLCD_vSetPosition(2, 1);
        CLCD_vSendString((u8 *)"Max chars: ");
        UserName_Length = 0;
      }
      CLCD_vSetPosition(3, 1);
      while (1)
      {
        Error_State = USART_u8ReceiveData(&KPD_Press);
        if (Error_State == OK)
        {
          if (KPD_Press == 0x0D || KPD_Press == 0x0F)
          { // Enter key
            if (UserName_Length >= USERNAME_MIN_LENGTH || UserName_Length < USERNAME_MIN_LENGTH)
            {
              break;
            }
            else
            {
            }
          }
          else if (KPD_Press == 0x08)
          { // Backspace
            if (UserName_Length > 0)
            {
              UserName_Length--;
              Clear_Char();
            }
          }
          else if (UserName_Length < USERNAME_MAX_LENGTH)
          {
            temp_username[UserName_Length] = KPD_Press;
            CLCD_vSendData(KPD_Press);
            UserName_Length++;
          }
        }
      }
    } while (UserName_Length < USERNAME_MIN_LENGTH);
    if (Is_Username_Exists(temp_username, UserName_Length))
    {
      CLCD_vClearScreen();
      CLCD_vSendString((u8 *)"Username exists!");
      _delay_ms(1000);
      CLCD_vClearScreen();
      CLCD_vSendString((u8 *)"Set UserName");
      CLCD_vSetPosition(2, 1);
      CLCD_vSendString((u8 *)"Max chars: ");
    }
  } while (Is_Username_Exists(temp_username, UserName_Length));
  temp_username[UserName_Length] = '\0';
  Write_Username(User_Count, temp_username, UserName_Length);

  for (u8 i = 0; i < UserName_Length; i++)
  {
    UserName[i] = temp_username[i];
  }
  UserName[UserName_Length] = '\0';
}

//=====================================================================================//

/**
 * @brief Sets up new password
 * @details Handles input and validation of new password,
 *          enforces complexity requirements
 */
void PassWord_Set(void)
{
  USART_u8SendData(0x0D);
  CLCD_vClearScreen();
  CLCD_vSendString((u8 *)"Set Password");
  CLCD_vSetPosition(2, 1);
  CLCD_vSendString((u8 *)"Max chars: ");
  CLCD_vSendIntNumber(PASSWORD_MAX_LENGTH);

  PassWord_Length = 0;
  u8 temp_password[21];
  do
  {
    PassWord_Length = 0;
    do
    {
      if (PassWord_Length != 0)
      {
        CLCD_vClearScreen();
        CLCD_vSendString((u8 *)"Min Length: ");
        CLCD_vSendIntNumber(PASSWORD_MIN_LENGTH);
        _delay_ms(1000);
        CLCD_vClearScreen();
        CLCD_vSendString((u8 *)"Re Set Password");
        CLCD_vSetPosition(2, 1);
        CLCD_vSendString((u8 *)"Max chars: ");
        CLCD_vSendIntNumber(PASSWORD_MAX_LENGTH);
        PassWord_Length = 0;
      }
      CLCD_vSetPosition(3, 1);
      while (1)
      {
        Error_State = USART_u8ReceiveData(&KPD_Press);

        if (Error_State == OK)
        {
          if (KPD_Press == 0x0D || KPD_Press == 0x0F)
          { // Enter key
            if (PassWord_Length >= PASSWORD_MIN_LENGTH || PassWord_Length < PASSWORD_MIN_LENGTH)
              break;
          }
          else if (KPD_Press == 0x08)
          { // Backspace
            if (PassWord_Length > 0)
            {
              PassWord_Length--;
              Clear_Char();
            }
          }
          else if (PassWord_Length < PASSWORD_MAX_LENGTH)
          {
            temp_password[PassWord_Length] = KPD_Press;
            CLCD_vSendData(KPD_Press);
            _delay_ms(200);
            Clear_Char();
            CLCD_vSendData('*'); // Show * for password
            PassWord_Length++;
          }
        }
      }
    } while (PassWord_Length < PASSWORD_MIN_LENGTH);
    if (!Is_Password_Valid(temp_password, PassWord_Length))
    {
      CLCD_vClearScreen();
      CLCD_vSendString((u8 *)"Weak Password!");
      CLCD_vSetPosition(2, 1);
      CLCD_vSendString((u8 *)"Need: A,a,1,@");
      _delay_ms(2000);
      CLCD_vClearScreen();
      CLCD_vSendString((u8 *)"Set Password");
      CLCD_vSetPosition(2, 1);
      CLCD_vSendString((u8 *)"Max chars: ");
      CLCD_vSendIntNumber(PASSWORD_MAX_LENGTH);
    }
  } while (!Is_Password_Valid(temp_password, PassWord_Length));

  Write_Password(User_Count, temp_password, PassWord_Length);
  User_Count++;
  EEPROM_vWrite(EEPROM_UserCount_Location, User_Count);
}

//=====================================================================================//

/**
 * @brief Validates username during login
 * @details Checks if entered username exists in system
 */
void UserName_Check(void)
{
  CLCD_vClearScreen();
  CLCD_vSendString((u8 *)"Enter Username");
  CLCD_vSetPosition(2, 1);

  u8 CheckLength = 0;
  UserName_Check_Flag = 0;

  while (1)
  {
    Error_State = USART_u8ReceiveData(&KPD_Press);

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
          Clear_Char();
          CheckLength--;
        }
      }
      else if (CheckLength < USERNAME_MAX_LENGTH)
      {
        Check[CheckLength] = KPD_Press;
        CLCD_vSendData(KPD_Press);
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

//=====================================================================================//

/**
 * @brief Validates password during login
 * @details Verifies entered password matches stored password
 */
void PassWord_Check(void)
{
  CLCD_vClearScreen();
  CLCD_vSendString((u8 *)"Enter Password");
  CLCD_vSetPosition(2, 1);

  u8 CheckLength = 0;
  PassWord_Check_Flag = 0;

  while (1)
  {
    Error_State = USART_u8ReceiveData(&KPD_Press);

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
          Clear_Char();
          CheckLength--;
        }
      }
      else if (CheckLength < PASSWORD_MAX_LENGTH)
      {
        Check[CheckLength] = KPD_Press;
        CLCD_vSendData(KPD_Press);
        _delay_ms(200);
        Clear_Char();
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

//=====================================================================================//

/**
 * @brief Handles user sign-in process
 * @details Manages username/password entry and verification,
 *          tracks login attempts
 */
void Sign_In(void)
{
  while (1)
  {
    UserName_Check();
    PassWord_Check();

    if (UserName_Check_Flag == 0 || PassWord_Check_Flag == 0)
    {
      CLCD_vClearScreen();
      CLCD_vSendString((u8 *)"Invalid Login");

      Tries--;
      EEPROM_vWrite(EEPROM_NoTries_Location, Tries);

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
      EEPROM_vWrite(EEPROM_NoTries_Location, NOTPRESSED);
      Tries = Tries_Max;

      // Read and display username
      Read_Username(Current_User, UserName, &UserName_Length);
      CLCD_vClearScreen();
      CLCD_vSendString("Welcome ");
      CLCD_vSetPosition(3, ((20 - UserName_Length) / 2) + 1); // Center username on LCD
      CLCD_vSendString(UserName);
      _delay_ms(1000);
      break;
    }
  }
}

//=====================================================================================//

/**
 * @brief Handles timeout errors
 * @details Displays timeout message and updates system state
 */
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

  EEPROM_vWrite(EEPROM_NoTries_Location, NOTPRESSED);
  Tries = Tries_Max;
}

//=====================================================================================//

/**
 * @brief Clears last entered character
 * @details Helper function for input handling
 */
void Clear_Char(void)
{
  CLCD_vSendCommand(CLCD_SHIFT_CURSOR_LEFT);
  CLCD_vSendData(' ');
  CLCD_vSendCommand(CLCD_SHIFT_CURSOR_LEFT);
}

//=====================================================================================//

/**
 * @brief Resets system to factory settings
 * @details Clears all user data and resets EEPROM
 */
void Factory_Reset(void)
{
  // Clear all user data
  PassWord_Check();
  if (PassWord_Check_Flag == 1)
  {
    CLCD_vClearScreen();
    CLCD_vSendString("Loading ...");
    for (u16 addr = EEPROM_START_ADDRESS; addr <= EEPROM_END_ADDRESS; addr++)
      {
        EEPROM_vWrite(addr, 0xFF);
      }
      User_Count = 0;
      Tries = Tries_Max;
      Log_Event(EVENT_SYSTEM_RESET, 0);
  }
}

//=====================================================================================//

/**
 * @brief Handles user sign-out
 * @details Clears current user state and returns to main menu
 */
void Sign_Out(void)
{
  Current_User = 0xFF;
  Is_Admin = 0;
}
