/*
 *<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<    SECURITY_interface.h    >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 *  Author : Abdallah Abdelmoemen Shehawey
 *  Layer  : APP_Layer
 *
 */
#ifndef SECURITY_INTERFACE_H_
#define SECURITY_INTERFACE_H_

#include "Security_config.h"

/* EEPROM Memory Map */
#define EEPROM_START_ADDRESS       0x00
#define EEPROM_END_ADDRESS         0x3FF // 1024 bytes total

/* System Status Locations */
#define EEPROM_SYSTEM_STATUS       0x10 // 1 byte for system status flags
#define EEPROM_NoTries_Location    0x12
#define EEPROM_UserCount_Location  0x13
#define EEPROM_ADMIN_FLAG_Location 0x14
#define EEPROM_CHECKSUM_Location   0x15 // 1 byte for data integrity check
#define EEPROM_BACKUP_START        0x200     // Start of backup section

/* System Status Flags */
#define SYSTEM_INITIALIZED         0x01
#define SYSTEM_LOCKED              0x02
#define SYSTEM_MAINTENANCE         0x04
#define SYSTEM_BACKUP_VALID        0x08

/* Security Levels */
#define SECURITY_LEVEL_LOW         0
#define SECURITY_LEVEL_MEDIUM      1
#define SECURITY_LEVEL_HIGH        2

/* Password Complexity Requirements */
#define PASSWORD_MIN_LENGTH        8
#define PASSWORD_MAX_LENGTH        20
#define USERNAME_MIN_LENGTH        5
#define USERNAME_MAX_LENGTH        20

#define PASS_NEED_UPPER            1
#define PASS_NEED_LOWER            1
#define PASS_NEED_NUMBER           1
#define PASS_NEED_SPECIAL          1

/* User Types and Permissions */
#define USER_TYPE_GUEST            0
#define USER_TYPE_NORMAL           1
#define USER_TYPE_ADMIN            2
#define USER_TYPE_SUPER            3

/* Event Types */
#define EVENT_LOGIN_SUCCESS        0x01
#define EVENT_LOGIN_FAIL           0x02
#define EVENT_PASS_CHANGE          0x03
#define EVENT_USER_CHANGE          0x04
#define EVENT_USER_DELETE          0x05
#define EVENT_USER_CREATE          0x06
#define EVENT_SYSTEM_RESET         0x07
#define EVENT_BACKUP_CREATE        0x08
#define EVENT_BACKUP_RESTORE       0x09
#define EVENT_SYSTEM_LOCK          0x0A
#define EVENT_SYSTEM_UNLOCK        0x0B

/* Error Codes */
#define ERROR_NONE                 0x00
#define ERROR_INVALID_USER         0x01
#define ERROR_INVALID_PASS         0x02
#define ERROR_USER_EXISTS          0x03
#define ERROR_SYSTEM_FULL          0x04
#define ERROR_TIMEOUT              0x05
#define ERROR_SYSTEM_LOCKED        0x06
#define ERROR_NO_PERMISSION        0x07
#define ERROR_CHECKSUM             0x08


typedef enum // it should be before functions prototypes
{
  false,
  true
} bool;

/* Function Prototypes */

/* System Management */
void EEPROM_vInit(void                        );

/* User Management */
void UserName_Set(void                        );
void PassWord_Set(void                        );
void Change_Username(void                     );
void Change_Password(void                     );
bool Delete_User(void                           );
void Delete_User_By_Admin(u8 user_index       );

/* Authentication */
void Sign_In(void                             );
void Sign_Out(void                            );
bool Is_Password_Valid(u8 *password, u8 length  );
bool Is_Username_Exists(u8 *username, u8 length );
void UserName_Check(void                      );
void PassWord_Check(void                      );

/* System Protection */
void Error_TimeOut(void                       );

/* Backup and Recovery */
void Factory_Reset(void                       );

/* Event Logging */
void Log_Event(u8 event_type, u8 user_index   );
void Clear_Char(void                          );

/* Menu Functions */
void User_Menu(void                           );
void Admin_Menu(void                          );

/* External Variables */
extern volatile u8 Error_State;
extern volatile u8 KPD_Press;
extern volatile u8 UserName[20];
extern volatile u8 UserName_Length;
extern volatile u8 PassWord_Length;
extern volatile u8 Tries;
extern volatile u8 Check[21];
extern volatile u8 UserName_Check_Flag;
extern volatile u8 PassWord_Check_Flag;
extern volatile u8 Current_User;
extern volatile u8 User_Count;
extern volatile u8 Is_Admin;

#endif /* SECURITY_INTERFACE_H_ */
