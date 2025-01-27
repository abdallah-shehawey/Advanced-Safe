/*
 *<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<      CLCD_extrachar.h     >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
 *
 *  Author : Abdallah Abdelmoemen Shehawey
 *  Layer  : HAL
 *  SWC    : CLCD
 *
 */

#ifndef CLCD_EXTRACHAR_H_
#define CLCD_EXTRACHAR_H_

/*

 We draw each char on website ===>> https://maxpromer.github.io/LCD-Character-Creator/

 Hint : *this website already neglect the first 3 bits in each byte = 0 (on the left side)
        *take the hex value or add 000 on the left
*/

// عبده عدوي
u8 CLCD_u8ExtraChar[] = {
	0x00, 0x01, 0x01, 0x01, 0x09, 0x1F, 0x08, 0x00
};

#endif /* CLCD_EXTRACHAR_H_ */
