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

#define NOTPRESSED                            0xFF

/* Maximum number of users that can be stored */
// Maximum users that can fit in EEPROM (992 bytes available / 42 bytes per user = 23 users)
#define MAX_USERS                             23


/* Size of each user's data block */
// 42 bytes per user (2 bytes lengths + 20 bytes username + 20 bytes password)

#define USER_BLOCK_SIZE                       0x2A
/* Status locations */
#define EEPROM_NoTries_Location               0x12
#define EEPROM_UserCount_Location             0x13

/* User data block start addresses */
#define EEPROM_USER_START                     0x20

/* Offsets within each user block */
#define USER_NAME_LENGTH_OFFSET               0x00
#define USER_PASS_LENGTH_OFFSET               0x01
#define USER_NAME_START_OFFSET                0x02
#define USER_NAME_MAX_SIZE                    0x14     // 20 bytes for username
#define USER_PASS_START_OFFSET                0x16     // 0x02 + 0x14 = 0x16
#define USER_PASS_MAX_SIZE                    0x14     // 20 bytes for password

/* Minimum lengths */
#define USERNAME_MIN_LENGTH                   5
#define PASSWORD_MIN_LENGTH                   5

/* Maximum lengths */
#define USERNAME_MAX_LENGTH                   20
#define PASSWORD_MAX_LENGTH                   20

/* Number of tries allowed */
#define Tries_Max 3

/* Function prototypes */
void EEPROM_vInit        (void);
void UserName_Set        (void);
void PassWord_Set        (void);
void Sign_In             (void);
void UserName_Check      (void);
void PassWord_Check      (void);
void Error_TimeOut       (void);
void Clear_Char          (void);

#endif /* SECURITY_INTERFACE_H_ */
