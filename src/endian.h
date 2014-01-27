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
* endian.h
* ========
* Flip endian-ness of various data types ...
* (such as addresses loaded from FAT filesystems -
* Intel x86 uses little-endian, MOS6502 is big-endian)
* 
* Written by John Snowdon (john@target-earth.net), 2014.
*/

swap_int16(int16)
char*	int16;
{
	/* Swap the endian-ness of an unsigned 16bit value */
	
	char b;
	b = int16[0];
	int16[0] = int16[1];
	int16[1] = b;
	return 0;
}

swap_int32(int32)
char*	int32;
{
	/* Swap the endian-ness of an 4byte packed unsigned 32bit value */
	
	char	b;
	
	b = int32[0];
	int32[0] = int32[3];
	int32[3] = b;
	
	b = int32[1];
	int32[1] = int32[2];
	int32[2] = b;
	
	return 0;
	
}

