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
* int8_to_int32() 	- Converts an unsigned single byte char to a 4byte 32bit number
* int16_to_int32() 	- Converts an unsigned two byte int to a 4byte 32bit number
* int32_is_zero() 	- Tests if a 4byte 32bit number is zero
* add_int32() 		- Adds two 4byte 32bit numbers - does not handle overflow
* sub_int32() 		- SUbtracts two 4byte 32bit numbers - does not handle overflow OR negatives
* mul_int32() 		- NOT IMPLEMENTED!!!
* mul_int32_int16()	- SLOW!!! Multiplies a 4byte 32bit number by a 16bit integer - does not handle overflow - needs refactoring
* mul_int32_int8()	- Multiplies a 4byte 32bit number by a 8bit integer - does not handle overflow - needs refactoring
* div_pow_int32()	- Divides a 32bit number by a given power of 2. Uses bit shifting.
* zero_int32()		- Initialises a 4byte packed 32bit number (sets each memory location to 0x00)
*/

#include "fat/math32-extras.h"

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

int8_to_int32(int32_result, int8)
char* 	int32_result;
char 	int8;
{
	/* 
		Takes an unsigned 8bit number
		and converts to the lowest byte of a packed 4 byte array
	*/
	
	int32_result[0] = 0x00;
	int32_result[1] = 0x00;
	int32_result[2] = 0x00;
	int32_result[3] = int8;
}

int16_to_int32(int32_result, int16)
char* 	int32_result;
int	int16;
{
	/* 
		Takes an unsigned 16bit number
		and converts to the lower two bytes of a packed 4 byte array
	*/
	
	int32_result[0] = 0x00;
	int32_result[1] = 0x00;
	int32_result[2] = int16 >> 8;
	int32_result[3] = int16 & 0xff;
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
	char	i, pos;
	char 	carry;
		
	/* zero_int32(int32_result); */
		
	carry = 0x00;
	/* loop over each byte of the 4byte array from lsb to msb */
	for (i = 1; i < 5; i++) {
		pos = 4 - i; 
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
		
		TO DO - Really feels like this could be refactored to be
		a short loop. But it does work as-is.
	*/
	
	char borrow;
	char result[4];
	int sum;
	
	/* If the byte 1 of the first integer is smaller than that of the second, then underflow would
	occur _regardless_ */
	if (int32_a[0] < int32_b[0]) {	
		zero_int32(int32_result);
		return 1;
	} 	
	
	/* byte 4 */
	if (int32_a[3] >= int32_b[3]){
		result[3] = int32_a[3] - int32_b[3];
		borrow = 0;
	} else {
		sum = int32_a[3] + 0x0100;
		result[3] = sum - int32_b[3] - borrow;
		borrow = 1;
	}
	
	/* byte 3 */
	if ((int32_a[2] - borrow) >= int32_b[2]){
		result[2] = (int32_a[2] - int32_b[2]) - borrow;
		borrow = 0;
	} else {
		sum = int32_a[2] + 0x0100;
		result[2] = sum - int32_b[2] - borrow;
		borrow = 1;
	}
	
	/* byte 2 */
	if ((int32_a[1] - borrow) >= int32_b[1]){
		result[1] = int32_a[1] - int32_b[1] - borrow;
		borrow = 0;
	} else {
		sum = int32_a[1] + 0x0100;
		result[1] = sum - int32_b[1] - borrow;
		borrow = 1;	
	}
	
	/* byte 1 */
	if ((int32_a[0] - borrow) >= int32_b[0]){
		result[0] = int32_a[0] - int32_b[0] - borrow;
	} else {
		zero_int32(int32_result);
		return 1;
	}
	
	copy_int32(int32_result, result);
	return 0;
}

mul_int32_int16(int32_result, int16)
char*	int32_result;
int		int16;
{
	/* 
		Multiply a 32bit number by a 16bit number and stores the result
		as 4 bytes.
		
		This is VERY SLOW (but works) - multiple add_int32 calls, it
		should be refactored to use bit shifting.
		
		TODO: CONVERT TO SHIFT
		
		Returns 0 on success, 1 on error or overflow.
	*/
	
	char overflow;
	int i;
	char r[4];
	memcpy(r, int32_result, 4);
	zero_int32(int32_result);

	for (i = 0; i < int16; i++) {
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
		Multiply a 32bit number by an 8bit number.
		Result is a 32bit number and returns 0 or 1 to
		indicate whether overflow occurred.
		
		Input:
			char*	int32_result	- Pointer to 32bit location in memory to hold result.
			char*	int32		- Pointer to 32bit number to be multiplied.
			char	int8		- 8 bit number to multiply.
			
		Resturns:
			0 on success / no overflow and sets int32_result.
			1 on overflow occured.
			
	*/
	
	char i;
	char v;
	int old_v;
	
	v = 0;
	for (i = 3; i > 0; --i){
		
		/* multiply and add overflow from last loop*/
		old_v = (int32[i] * int8) + v;
		
		/* store lsb */
		int32_result[i] = old_v & 0xff;
		
		/* store msb as overflow for next loop */
		v = (old_v >> 8) & 0xff;
	}
	int32_result[0] = (int32[0] * int8) + v;
	if (int32[0] > int32_result[0]){
		/* overflow */
		return 1;
	} else {
		return 0;
	}
}

div_pow_int32(int32, power)
char*	int32;
char	power;
{
	/*
		Divide a 32bit number by a power of 2.
		e.g. 65536 / 2^7 = 512
		
		Input:
			char*	int32	- Pointer to 32bit value in memory.
			char	power	- The power of 2 to divide by.
			
		Result:
			Updates the value of int32. No remainder.
	*/
	
	char i;
	char v, old_v;
	
	v = 0;
	for (i = 0; i < 4; i++){
		old_v = (int32[i] << 1);
		int32[i] = (int32[i] >> power) + v;
		v = old_v;
	}
}

zero_int32(int32_result)
char*	int32_result;
{
	/* Zeroes out a 32bit number */
	
	int32_result[0] = 0x00;
	int32_result[1] = 0x00;
	int32_result[2] = 0x00;
	int32_result[3] = 0x00;
}

copy_int32(int32_result, int32)
char*	int32_result;
char*	int32;
{
	/* Copies a 32bit number */
	
	memcpy(int32_result, int32, 4);
}
