/*
 *<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<    SECURITY_config.h    >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 *  Author : Abdallah Abdelmoemen Shehawey
 *  Layer  : APP_Layer
 *
 */

#ifndef SECURITY_CONFIG_H_
#define SECURITY_CONFIG_H_

/* Minimum lengths - moved from interface.h */
#define USERNAME_MIN_LENGTH              5
#define PASSWORD_MIN_LENGTH              8

#define Tries_Max                        3
#define USERNAME_MAX_LENGTH              20
#define PASSWORD_MAX_LENGTH              20

/* EEPROM Memory Layout */
#define EEPROM_USER_START                0x20
#define USER_BLOCK_SIZE                  0x2A

/* User Data Block Offsets */
#define USER_NAME_LENGTH_OFFSET          0x00
#define USER_NAME_START_OFFSET           0x01
#define USER_PASS_LENGTH_OFFSET          0x15
#define USER_PASS_START_OFFSET           0x16

/* System Constants */
#define NOTPRESSED                       0xFF
#define MAX_USERS                        23

#endif /* SECURITY_CONFIG_H_ */
