/* =============================================

	Basic 32bit math functions, as well as helper functions to cast 
	single byte 'char' and double byte 'int' numbers into 4 byte packed
	arrays representing 32bit numbers.
	
	This is not speed tested. Consider it SLOW.
	
	Written by John Snowdon (john@target-earth.net), 2014.
	
	---
	
	int32_to_int16_lsb	- returns the 16 least significant bits of a 32bit number
	int32_to_int16_msb	- returns the 16 most significant bits of a 32bit number
	char_to_int32() 	- Converts an unsigned single byte char to a 4byte 32bit number
	int_to_int32() 		- Converts an unsigned two byte int to a 4byte 32bit number
	int32_is_zero() 	- Tests if a 4byte 32bit number is zero
	add_int32() 		- Adds two 4byte 32bit numbers - does not handle overflow
	sub_int32() 		- Subtracts two 4byte 32bit numbers
	mul_int32() 		- NOT IMPLEMENTED!!! Multiplies two 4byte 32bit numbers - does not handle overflow
	mul_int32_int16()	- SLOW!!! Multiplies a 4byte 32bit number by a 16bit integer - does not handle overflow - needs refactoring
	mul_int32_int8()	- SLOW!!! Multiplies a 4byte 32bit number by a 8bit integer - does not handle overflow - needs refactoring
	zero_int32()		- Initialises a 4byte packed 32bit number (sets each memory location to 0x00)

	---
	
	Helper methods from: http://web.media.mit.edu/~stefanm/yano/picc_Math32.html
	
	dec_int32() 		- Decrement a 32bit value by 1
	lt_int32() 		- Is a < b
	lte_int32() 		- Is a <= b
	ge_int32() 		- Is a > b
	gte_int32() 		- Is a >= b
	
=============================================== */

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

	for (mul_i = 0x0000; mul_i < int16; mul_i++) {
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

	for (mul_i = 0x00; mul_i < int8; mul_i++) {
		overflow = add_int32(int32_result, int32, r);
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

/* ====================================== 

	These additional helpers below come from:
	http://web.media.mit.edu/~stefanm/yano/picc_Math32.html

	////////////////////////////////////////////////////////////////////////////
	//                                                                        
	// This code is based on code by Vadim Gerasimov (vadim@media.mit.edu),   
	// and extended skillfully by Mark Newman (newman@media.mit.edu) for the  
	// Robotic F.A.C.E. project                                               
	//                                                                        
	// Copyright (C) 2001-2004 MIT Media Laboratory                           
	//                                                                        
	////////////////////////////////////////////////////////////////////////////
	
*/

dec_int32(int32_result)
char*	int32_result;
{
	/* Decrement (in-place) a 32bit number */
	if (int32_result[0]--==0) if(int32_result[1]--==0) if (int32_result[2]--==0) --int32_result[3];
}

inc_int32(int32_result)
char*	int32_result;
{
	/* Increment (in-place) a 32bit number */
	if (int32_result[3]++==0) if(int32_result[2]++==0) if (int32_result[1]++==0) ++int32_result[0];
}

lt_int32(int32_a, int32_b)
char*	int32_a;
char*	int32_b;
{
	/* Boolean - Less Than (a < b) */
	int i;
	for (i = 3; i >= 0; i--) {
		if (int32_a[i] > int32_b[i])
			return 0;
		if (int32_a[i] < int32_b[i])
			return 1;
	}
	return 0;
}

lte_int32(int32_a, int32_b)
char*	int32_a;
char*	int32_b;
{
	/* Boolean - Less Than or Equal To (a <= b) */
	int i;
	for (i = 3; i >= 0; i--) {
		if (int32_a[i] < int32_b[i]) {
			return 1;
		}
		if (int32_a[i] > int32_b[i]) {
			return 0;
		}
	}
	return 1;
}

gt_int32(int32_a, int32_b)
char*	int32_a;
char*	int32_b;
{
	/* Boolean - Greater Than (a > b) */
	int i;
	for (i = 3; i >= 0; i--) {
		if (int32_a[i] < int32_b[i]) {
			return 0;
		}
		if (int32_a[i] > int32_b[i]) {
			return 1;
		}
	}
	return 0;
}

gte_int32(int32_a, int32_b)
char*	int32_a;
char*	int32_b;
{
	/* Boolean - Greater Than or Equal To (a >= b) */
	int i;
	for (i = 3; i >= 0; i--) {
		if (int32_a[i] > int32_b[i]) {
			return 1;
		}
		if (int32_a[i] < int32_b[i]) {
			return 0;
		}
	}
	return 1;
}

shift_int32(int32_result)
char*	int32_result;
{
	/* Bitshifts a 32bit number */
	
	char i, in, out;
	in = 0;
	for (i = 0; i < 4; i++) {
		out = int32_result[i] >> 7;
		int32_result[i] = in + (int32_result[i] << 1);
		in = out;
	}
}
