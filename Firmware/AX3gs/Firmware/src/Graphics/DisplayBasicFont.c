/* 
 * Copyright (c) 2009-2012, Newcastle University, UK.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met: 
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE. 
 *
 * Written to allow legacy/fast font support on page orientated displays
 * KL 19-05-2011
 */
 
// Includes
#include <string.h>
#include "DisplayBasicFont.h"
#include "GraphicsConfig.h"  // Holds the driver header file


// Vars
const unsigned char TABLE1[],TABLE2[]; // See below

static unsigned char display_last_page,display_last_column;
void Display_gotoxy(unsigned char x_col, unsigned char y_row)	// goto xy coordinates
{
	y_row&=0x7; 						// y pos to page, 0-7
	display_last_page = y_row; 					// Store - it cant be read
	display_last_column = x_col;				// Store - it cant be read
	DisplayWriteCommand((x_col>>4)|0x10); 	// high nibble with bit4 set
	DisplayWriteCommand(x_col&0x0f);		// set lower column address
	DisplayWriteCommand(0xb0|y_row);// set GDRAM page address 0..7 (for 8 rows)
}

void Display_putc_basic(unsigned char c,unsigned char size)			// Put character on screen
{
// extract 7x5 ascii from tables + 1 line space
unsigned char char_col_data[6], char_table_pos, i; 

// Step one, retrieve char symbol
if (c<0x20)return; 	// out of ascii table
if (c>0x7f)return;	// out of ascii table	
if (c<0x50)		// use TABLE1
{
	char_table_pos=((c-0x20)*5); // Position in table
	char_col_data[0]=TABLE1[char_table_pos];
	char_col_data[1]=TABLE1[char_table_pos+1];
	char_col_data[2]=TABLE1[char_table_pos+2];
	char_col_data[3]=TABLE1[char_table_pos+3];
	char_col_data[4]=TABLE1[char_table_pos+4];
	char_col_data[5]=0; // char spacing
}				
else //if (c>0x4f)// use TABLE2				
{
	char_table_pos=((c-0x50)*5); // Position in table
	char_col_data[0]=TABLE2[char_table_pos];
	char_col_data[1]=TABLE2[char_table_pos+1];
	char_col_data[2]=TABLE2[char_table_pos+2];
	char_col_data[3]=TABLE2[char_table_pos+3];
	char_col_data[4]=TABLE2[char_table_pos+4];
	char_col_data[5]=0; // char spacing
}

// Print out character with correct size
if (size == 8) // 56x40 pixels - written on all 8 lines
{
	// Stretch the char symbol to 8 bytes
	unsigned char stretched,mask,j;
	mask = 0b1;
	for (j=0;j<8;j++)
	{
		for(i=0;i<6;i++) // 8x6 bytes per line
		{
			stretched = 0;
			if(char_col_data[i]&mask)stretched = 0xff;
			DisplayWriteData(stretched);
			DisplayWriteData(stretched);
			DisplayWriteData(stretched);
			DisplayWriteData(stretched);
			DisplayWriteData(stretched);
			DisplayWriteData(stretched);
			DisplayWriteData(stretched);
			DisplayWriteData(stretched);
		}
		mask<<=1; // 1 line / pix
		Display_gotoxy(display_last_column,(display_last_page+1));
	}
	Display_gotoxy(display_last_column+48,(display_last_page-8));
}
// Print out character with correct size
else if (size == 4) // 28x20 pixels - written in 4 lines
{
	// Stretch the char symbol to 4 bytes
	unsigned char stretched,mask,j;
	mask = 0b1;
	for (j=0;j<4;j++)
	{
		for(i=0;i<6;i++) // 8x6 bytes per line
		{
			stretched = 0;
			if(char_col_data[i]&mask)stretched |= 0x07;
			if(char_col_data[i]&(mask<<1))stretched |= 0x70;
			DisplayWriteData(stretched);
			DisplayWriteData(stretched);
			DisplayWriteData(stretched);
			DisplayWriteData(0);
		}
		mask<<=2;  // 1 line / 2 pix
		Display_gotoxy(display_last_column,(display_last_page+1));
	}
	Display_gotoxy(display_last_column+24,(display_last_page-4));
}
// Print out character with correct size
else if (size == 2) // 14x10 pixels - written in 2 lines
{
	// Stretch the char symbol to 2 bytes
	unsigned char stretched,mask,j;
	mask = 0b1;
	for (j=0;j<2;j++)
	{
		for(i=0;i<6;i++) // 2x6 bytes per line
		{
			stretched = 0;
			if(char_col_data[i]&mask)stretched |= 0x03;
			if(char_col_data[i]&(mask<<1))stretched |= 0x0c;
			if(char_col_data[i]&(mask<<2))stretched |= 0x30;
			if(char_col_data[i]&(mask<<3))stretched |= 0xc0;
			DisplayWriteData(stretched);
			DisplayWriteData(stretched);
		}
		mask<<=4; // 1 line / 4pix
		Display_gotoxy(display_last_column,(display_last_page+1));
	}
	Display_gotoxy(display_last_column+12,(display_last_page-2));
}
// Smallest font
else //if(size == 0) // 7x5 pixels - Default
{
	DisplayWriteData(char_col_data[0]);
	DisplayWriteData(char_col_data[1]);
	DisplayWriteData(char_col_data[2]);
	DisplayWriteData(char_col_data[3]);
	DisplayWriteData(char_col_data[4]);
	DisplayWriteData(char_col_data[5]);
}

return;
}

void Display_clear_line(unsigned char line)
{
	unsigned char i =128;
	DisplayWriteCommand(0x10); 		// set column = 0
	DisplayWriteCommand(0x00);		// set lower column address
	DisplayWriteCommand(0xb0|(line&0x7));// set GDRAM page address 0..7 (for 8 rows)
	while(i>0)
	{
		DisplayWriteData(0); // clear display
		i--;	
	}
}

// Print line on screen
void DisplayPrintLine(char* string, unsigned char line, unsigned char size)
{
	unsigned int length = strlen(string);
	static unsigned char current_line = 0;
	static unsigned char previous_size = 0;
	unsigned char i;

	// Set start line
	if (line == PREVIOUS_LINE) 			{current_line -= previous_size;current_line&=0x7;}
	else if (line == NEXT_LINE) 		{current_line += previous_size;current_line&=0x7;} // TODO: Scroll funtionality
	else 								{current_line = line&0x7;}	

	// Keep track of font size
	previous_size = size;

	// Clear line contents
	for (i=0;i<size;i++)
	{
		Display_clear_line(current_line+i);
	}

	// Print characters out
	Display_gotoxy(0,current_line);
	for (;length>0;length--)
	{
		Display_putc_basic(*string++,size);
	}
}

void Display_print_xy(char* string, unsigned char x, unsigned char y, unsigned char size)
{
	unsigned int length = strlen(string);
	Display_gotoxy(x,y);
	for (;length>0;length--)
	{
		Display_putc_basic(*string++,size);
	}
}

// in ram - character tables
const unsigned char TABLE1[240]=				
{0x00,0x00,0x00,0x00,0x00,	// 20 space	 	
0x00,0x00,0x5f,0x00,0x00,	// 21 !
0x00,0x07,0x00,0x07,0x00,	// 22 "
0x14,0x7f,0x14,0x7f,0x14,	// 23 #
0x24,0x2a,0x7f,0x2a,0x12,	// 24 $
0x23,0x13,0x08,0x64,0x62,	// 25 %
0x36,0x49,0x55,0x22,0x50,	// 26 &
0x00,0x05,0x03,0x00,0x00,	// 27 '
0x00,0x1c,0x22,0x41,0x00,	// 28 (
0x00,0x41,0x22,0x1c,0x00,	// 29 )
0x14,0x08,0x3e,0x08,0x14,	// 2a *
0x08,0x08,0x3e,0x08,0x08,	// 2b +
0x00,0x50,0x30,0x00,0x00,	// 2c ,
0x08,0x08,0x08,0x08,0x08,	// 2d -
0x00,0x60,0x60,0x00,0x00,	// 2e .
0x20,0x10,0x08,0x04,0x02,	// 2f /
0x3e,0x51,0x49,0x45,0x3e,	// 30 0
0x00,0x42,0x7f,0x40,0x00,	// 31 1
0x42,0x61,0x51,0x49,0x46,	// 32 2
0x21,0x41,0x45,0x4b,0x31,	// 33 3
0x18,0x14,0x12,0x7f,0x10,	// 34 4
0x27,0x45,0x45,0x45,0x39,	// 35 5
0x3c,0x4a,0x49,0x49,0x30,	// 36 6
0x01,0x71,0x09,0x05,0x03,	// 37 7
0x36,0x49,0x49,0x49,0x36,	// 38 8
0x06,0x49,0x49,0x29,0x1e,	// 39 9
0x00,0x36,0x36,0x00,0x00,	// 3a :
0x00,0x56,0x36,0x00,0x00,	// 3b ;
0x08,0x14,0x22,0x41,0x00,	// 3c <
0x14,0x14,0x14,0x14,0x14,	// 3d =
0x00,0x41,0x22,0x14,0x08,	// 3e >
0x02,0x01,0x51,0x09,0x06,	// 3f ?
0x32,0x49,0x79,0x41,0x3e,	// 40 @
0x7e,0x11,0x11,0x11,0x7e,	// 41 A
0x7f,0x49,0x49,0x49,0x36,	// 42 B
0x3e,0x41,0x41,0x41,0x22,	// 43 C
0x7f,0x41,0x41,0x22,0x1c,	// 44 D
0x7f,0x49,0x49,0x49,0x41,	// 45 E
0x7f,0x09,0x09,0x09,0x01,	// 46 F
0x3e,0x41,0x49,0x49,0x7a,	// 47 G
0x7f,0x08,0x08,0x08,0x7f,	// 48 H
0x00,0x41,0x7f,0x41,0x00,	// 49 I
0x20,0x40,0x41,0x3f,0x01,	// 4a J
0x7f,0x08,0x14,0x22,0x41,	// 4b K
0x7f,0x40,0x40,0x40,0x40,	// 4c L
0x7f,0x02,0x0c,0x02,0x7f,	// 4d M
0x7f,0x04,0x08,0x10,0x7f,	// 4e N
0x3e,0x41,0x41,0x41,0x3e};	
const unsigned char TABLE2[240]=				
{0x7f,0x09,0x09,0x09,0x06,	// 50 P
0x3e,0x41,0x51,0x21,0x5e,	// 51 Q
0x7f,0x09,0x19,0x29,0x46,	// 52 R
0x46,0x49,0x49,0x49,0x31,	// 53 S
0x01,0x01,0x7f,0x01,0x01,	// 54 T
0x3f,0x40,0x40,0x40,0x3f,	// 55 U
0x1f,0x20,0x40,0x20,0x1f,	// 56 V
0x3f,0x40,0x38,0x40,0x3f,	// 57 W
0x63,0x14,0x08,0x14,0x63,	// 58 X
0x07,0x08,0x70,0x08,0x07,	// 59 Y
0x61,0x51,0x49,0x45,0x43,	// 5a Z
0x00,0x7f,0x41,0x41,0x00,	// 5b [
0x02,0x04,0x08,0x10,0x20,	// 5c
0x00,0x41,0x41,0x7f,0x00,	// 5d
0x04,0x02,0x01,0x02,0x04,	// 5e
0x40,0x40,0x40,0x40,0x40,	// 5f
0x00,0x01,0x02,0x04,0x00,	// 60
0x20,0x54,0x54,0x54,0x78,	// 61 a
0x7f,0x48,0x44,0x44,0x38,	// 62 b
0x38,0x44,0x44,0x44,0x20,	// 63 c
0x38,0x44,0x44,0x48,0x7f,	// 64 d
0x38,0x54,0x54,0x54,0x18,	// 65 e
0x08,0x7e,0x09,0x01,0x02,	// 66 f
0x0c,0x52,0x52,0x52,0x3e,	// 67 g
0x7f,0x08,0x04,0x04,0x78,	// 68 h
0x00,0x44,0x7d,0x40,0x00,	// 69 i
0x20,0x40,0x44,0x3d,0x00,	// 6a j 
0x7f,0x10,0x28,0x44,0x00,	// 6b k
0x00,0x41,0x7f,0x40,0x00,	// 6c l
0x7c,0x04,0x18,0x04,0x78,	// 6d m
0x7c,0x08,0x04,0x04,0x78,	// 6e n
0x38,0x44,0x44,0x44,0x38,	// 6f o
0x7c,0x14,0x14,0x14,0x08,	// 70 p
0x08,0x14,0x14,0x18,0x7c,	// 71 q
0x7c,0x08,0x04,0x04,0x08,	// 72 r
0x48,0x54,0x54,0x54,0x20,	// 73 s
0x04,0x3f,0x44,0x40,0x20,	// 74 t
0x3c,0x40,0x40,0x20,0x7c,	// 75 u
0x1c,0x20,0x40,0x20,0x1c,	// 76 v
0x3c,0x40,0x30,0x40,0x3c,	// 77 w
0x44,0x28,0x10,0x28,0x44,	// 78 x
0x0c,0x50,0x50,0x50,0x3c,	// 79 y
0x44,0x64,0x54,0x4c,0x44,	// 7a z
0x08,0x08,0x2a,0x08,0x08,	// 7b
0x00,0x00,0x7f,0x00,0x00,	// 7c
0x00,0x41,0x36,0x08,0x00,	// 7d
0x10,0x08,0x08,0x10,0x08,	// 7e
0x78,0x46,0x41,0x46,0x78};	
