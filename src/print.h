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

#ifdef PRINTFUNCS
put_hex_count(buffer, count, col, row)
char*	buffer;
char	count;
char	col;
char	row;
{
	/* 
		print an arbitrary chunk of memory pointed to by a buffer
		display it as hex at a given text coord on the screen.
		
		Input:
			char*, buffer	- Pointer to location in memory.
			char, count	- Number of bytes to display from that memory location.
			char, col	- Start column (text mode).
			char, row	- Start tow (text mode).
	*/
	
	char hex_byte;
	for (hex_byte=0; hex_byte<count; hex_byte++) {
		put_hex(buffer[hex_byte], 2, col + (2 * hex_byte), row);
	}
	return 0;
}
 
put_string_count(buffer, count, col, row)
char*	buffer;
char	count;
char	col;
char	row;
{
	/* 
		print an arbitrary chunk of memory pointed to by a buffer
		display it as a string at a given text coord on the screen.
		
		Input:
			char*, buffer	- Pointer to location in memory.
			char, count	- Number of bytes to display from that memory location.
			char, col	- Start column (text mode).
			char, row	- Start tow (text mode).
	*/
	
	char c_byte;
	for (c_byte=0; c_byte<count; c_byte++) {
		put_char(buffer[c_byte], col + c_byte, row);
	}
	return 0;
}
#endif
