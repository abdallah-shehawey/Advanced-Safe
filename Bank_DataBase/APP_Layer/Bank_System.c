/*
 * Bank System Application
 * Created on: Jun 26, 2024
 * Author: Abdallah Abdelmoemen Shehawey
 * Description: Main application file for the Advanced Safe system with user management and security features
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

/* External Variables Declaration */
extern volatile u8 Error_State;  // Stores the current error state of operations
extern volatile u8 User_Count;   // Tracks the total number of registered users
extern volatile u8 Is_Admin;     // Flag indicating if current user is admin
extern volatile u8 Current_User; // Index of the currently logged in user

/* Constants */
#define INPUT_TIMEOUT_MS 30000 // 30 seconds timeout for user input
#define DISPLAY_DELAY_MS 2000  // 2 seconds display time for messages

/* Function Prototypes */

/**
 * @brief Displays welcome screens with project and developer information
 * @details Shows a welcome message followed by developer credits
 */
void Display_Welcome(void);

/**
 * @brief Initializes and displays status of system components
 * @details Initializes LCD, USART, and EEPROM while showing progress
 */
void Display_Init_Status(void);

/**
 * @brief Waits for user input with timeout functionality
 * @param input Pointer to store received input
 * @param timeout_ms Maximum wait time in milliseconds
 * @return 1 if input received, 0 if timeout occurred
 */
u8 Wait_For_Input(u8 *input, u16 timeout_ms);

/**
 * @brief Displays error message on LCD
 * @param message Error message to display
 */
void Display_Error(const u8 *message);

/**
 * @brief Displays main menu with system status
 * @details Shows sign-in/register options and system status including user count and remaining tries
 */
void Display_Menu(void);

/**
 * @brief Main program entry point
 * @details Program flow:
 *         1. Initialize hardware components
 *         2. Display welcome and initialization screens
 *         3. Enter main menu loop:
 *            - Handle sign in
 *            - Handle new user registration
 *            - Monitor system status
 * @return 0 on normal termination
 */
int main(void)
{
  // Initialize hardware components
  USART_vInit();
  EEPROM_vInit();
  CLCD_vInit();

  // Show welcome and initialization screens
  Display_Welcome();
  Display_Init_Status();

  u8 choice;

  // Main program loop
  while (1)
  {
    Display_Menu();

    // Wait for user input with timeout
    if (!Wait_For_Input(&choice, INPUT_TIMEOUT_MS))
    {
      Display_Error((u8 *)"Input Timeout!");
      continue;
    }

    // Handle user choice
    if (choice == '1')
    {
      Sign_In();
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
      // Check system capacity
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

      // Register new user
      UserName_Set();
      PassWord_Set();
      Log_Event(EVENT_USER_CREATE, User_Count - 1);

      // Show remaining capacity
      CLCD_vClearScreen();
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

void Display_Welcome(void)
{
  CLCD_vClearScreen();
  CLCD_vSendString((u8 *)"     Welcome to");
  CLCD_vSetPosition(2, 1);
  CLCD_vSendString((u8 *)"   Advanced Safe");
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

  // Initialize and verify LCD
  CLCD_vSendString((u8 *)"LCD...");
  CLCD_vSendString((u8 *)"OK");
  _delay_ms(500);

  // Initialize and verify USART
  CLCD_vSetPosition(2, 1);
  CLCD_vSendString((u8 *)"USART...");
  USART_vInit();
  CLCD_vSendString((u8 *)"OK");
  _delay_ms(500);

  // Initialize and verify EEPROM
  CLCD_vSetPosition(2, 1);
  CLCD_vSendString((u8 *)"EEPROM...");
  EEPROM_vInit();
  CLCD_vSendString((u8 *)"OK");
  _delay_ms(500);
}

/**
 * @brief Implementation of input wait with timeout
 * @details Polls for USART input until timeout occurs
 * @param input Pointer to store received character
 * @param timeout_ms Maximum time to wait in milliseconds
 * @return 1 if input received, 0 if timeout occurred
 */
u8 Wait_For_Input(u8 *input, u16 timeout_ms)
{
  u16 elapsed = 0;
  while (elapsed < timeout_ms)
  {
    Error_State = USART_u8ReceiveData(input);
    if (Error_State == OK)
    {
      return 1; // Input received successfully
    }
    _delay_ms(1);
    elapsed++;
  }
  return 0; // Timeout occurred
}

/**
 * @brief Implementation of error message display
 * @details Clears screen, shows error header and message
 * @param message Error message to display
 */
void Display_Error(const u8 *message)
{
  CLCD_vClearScreen();
  CLCD_vSendString((u8 *)"Error:");
  CLCD_vSetPosition(2, 1);
  CLCD_vSendString((u8 *)message);
  _delay_ms(DISPLAY_DELAY_MS);
}

/**
 * @brief Implementation of main menu display
 * @details Shows:
 *         - Sign in option
 *         - New user registration option
 *         - Current user count and maximum capacity
 *         - System status and remaining tries
 */
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
