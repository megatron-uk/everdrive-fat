/* 		
* This file is part of everdrive-fat.

* everdrive-fat is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
* 
* Foobar is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
* 
* You should have received a copy of the GNU Lesser General Public License
* along with everdrive-fat.  If not, see <http://www.gnu.org/licenses/>.
* 
*/

/*
* math32.h
* ========
* 
* Basic 32bit math functions, as well as helper functions to cast 
* single byte 'char' and double byte 'int' numbers into 4 byte packed
* arrays representing 32bit numbers.
* 
* This is not speed tested. Consider it SLOW.
* 
* Written by John Snowdon (john@target-earth.net), 2014.
* 
* int32_to_int16_lsb	- returns the 16 least significant bits of a 32bit number
* int32_to_int16_msb	- returns the 16 most significant bits of a 32bit number
* char_to_int32() 	- Converts an unsigned single byte char to a 4byte 32bit number
* int_to_int32() 	- Converts an unsigned two byte int to a 4byte 32bit number
* int32_is_zero() 	- Tests if a 4byte 32bit number is zero
* add_int32() 		- Adds two 4byte 32bit numbers - does not handle overflow
* sub_int32() 		- Subtracts two 4byte 32bit numbers
* mul_int32() 		- NOT IMPLEMENTED!!! Multiplies two 4byte 32bit numbers - does not handle overflow
* mul_int32_int16()	- SLOW!!! Multiplies a 4byte 32bit number by a 16bit integer - does not handle overflow - needs refactoring
* mul_int32_int8()	- SLOW!!! Multiplies a 4byte 32bit number by a 8bit integer - does not handle overflow - needs refactoring
* zero_int32()		- Initialises a 4byte packed 32bit number (sets each memory location to 0x00)
*/

#include "math32-extras.h"

int32_to_int16_lsb(int32)
char*	int32;
{
	/* returns the 16 least significant bits of a 32bit value as
	an integer */
	return (int32[2] << 8) + int32[3];
}

int32_to_int16_msb(int32)
char*	int32;
{
	/* returns the 16 most significant bits of a 32bit value as
	an integer */
	return (int32[0] << 8) + int32[1];
}

char_to_int32(int32_result, int8)
char* 	int32_result;
char 	int8;
{
	/* 
		Takes an unsigned 8bit number
		and converts to a packed 4 byte array
	*/
	
	int32_result[0] = 0x00;
	int32_result[1] = 0x00;
	int32_result[2] = 0x00;
	int32_result[3] = int8;
	return 0;
}

int_to_int32(int32_result, int16)
char* 	int32_result;
int	int16;
{
	/* 
		Takes a pointer to an unsigned 16bit number
		and converts to a packed 4 byte array
		
		TODO: CONVERT TO SHIFT
	*/
	
	int32_result[0] = 0x00;
	int32_result[1] = 0x00;
	int32_result[2] = int16 >> 8;
	int32_result[3] = int16 & 0xff;
	
	return 0;
}


int32_is_zero(int32)
char* 	int32;
{
	/* 
		Is a packed 4 byte array == 0
		returns 1 if true, otherwise 0
	*/
	
	if (int32[0] == 0x00) if (int32[1] == 0x00) if (int32[2] == 0x00) if (int32[3] == 0x00) return 1;
	
	return 0;
	
}

mul_int32(int32_result, int32_a, int32_b)
char* 	int32_result;
char* 	int32_a;
char* 	int32_b;
{
	/* 
		Takes two 32bit values, stored as 4 bytes each - 
		multiplies and stores the result.
		Clearly a 32bit output cannot hold the product
		of two 32bit integers, but we just assume that we
		won't ever be using two values that would bring this about.
		More likely to be 2 x 16bit values, or a 24bit and 8bit etc.
		We don't handle overflow.
		
		Returns 0 on success, 1 on error or overflow.
	*/
	
	return 0;

}

add_int32(int32_result, int32_a, int32_b)
char* 	int32_result;
char* 	int32_a;
char* 	int32_b;
{
	/* 
		adds and stores the result.
		Returns 0 on success, 1 on error or overflow.
	*/

	int 	sum;
	char	add_i, pos;
	char 	carry;
		
	/* zero_int32(int32_result); */
		
	carry = 0x00;
	/* loop over each byte of the 4byte array from lsb to msb */
	for (add_i = 1; add_i < 5; add_i++) {
		pos = 4 - add_i; 
		/* sum the two 1 byte numbers as a 2 byte int */
		sum = int32_a[pos] + int32_b[pos] + carry;
		/* would integer overflow occur with this sum? */
		if (sum > 0x00ff) {
			/* store the most significant byte for next loop */
			carry = (sum >> 8) & 0x00ff;
		} else {
			/* no carry needed */
			carry = 0x00;	
		}
		/* store the least significant byte */
		int32_result[pos] = (sum & 0x00ff);
	}
	
	/* Has overflow occured (ie number > 32bit) */
	if (carry != 0x00) {
		return 1;
	} else {
		return 0;
	}
	
}

sub_int32(int32_result, int32_a, int32_b)
char* 	int32_result;
char* 	int32_a;
char* 	int32_b;
{
	/* 
		Takes two 32bit values, stored as 4 bytes each - 
		subtracts and stores the result.
		
		Returns 0 on success, 1 on error or overflow.
	*/
	
	return 0;
	
}

mul_int32_int16(int32_result, int16)
char*	int32_result;
int	int16;
{
	/* 
		Multiply a 32bit number by a 16bit number -
		This is VERY SLOW (but works) - multiple add_int32 calls, it
		should be refactored to use bit shifting.
		
		TODO: CONVERT TO SHIFT
	*/
	
	char overflow;
	int mul_i;
	char r[4];
	memcpy(r, int32_result, 4);
	zero_int32(int32_result);

	for (mul_i = 0; mul_i < int16; mul_i++) {
		overflow = add_int32(int32_result, int32_result, r);
		if (overflow != 0){
			return overflow;
		}
	}
	return 0;
}

mul_int32_int8(int32_result, int32, int8)
char*	int32_result;
char*	int32;
char	int8;
{
	/* 
		Multiply a 32bit number by an 8bit number - 
		This is VERY SLOW (but works) - multiple add_int32 calls, it
		should be refactored to use bit shifting.
		
		TODO: CONVERT TO SHIFT
	*/
	
	char overflow;
	char mul_i;
	char r[4];
	memcpy(r, int32, 4);
	zero_int32(int32_result);

	for (mul_i = 0; mul_i < int8; mul_i++) {
		overflow = add_int32(int32_result, int32_result, r);
		if (overflow != 0) {
			return 1;
		}
	}
	return 0;
	
}

zero_int32(int32_result)
char*	int32_result;
{
	/* Zeroes out a 32bit number */
	
	int32_result[0] = 0x00;
	int32_result[1] = 0x00;
	int32_result[2] = 0x00;
	int32_result[3] = 0x00;
	return 0;
}

copy_int32(int32_result, int32)
char*	int32_result;
char*	int32;
{
	/* Copies a 32bit number */
	
	memcpy(int32_result, int32, 4);
	return 0;
}

isBitSet(c, n)
char	c;
char	n;
{
	/* 
		Not a 32bit math operation, but simple bit test -
		is a bit as pos n set in char c.
	*/
	char mask[];
	mask[0] = 128;
	mask[1] = 64;
	mask[2] = 32;
	mask[3] = 16;
	mask[4] = 8;
	mask[5] = 4;
	mask[6] = 2;
	mask[7] = 1;
	
	return ((c & mask[n]) != 0);
}
