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
* fat-dev.h
* ======
* Functions for detecting and selecting the current
* active FAT master boot record and partition entry
* for SD card attached to the Turbo Everdrive flash drive.
* 
* John Snowdon (john@target-earth.net), 2014
*/

getMBR(detect_part) 
char detect_part;
{
	/* 
		Get the partition entry of the first fat16/32 filesystem on disk
		The Everdrive must already have been initialised via ed_begin() 
		and disk_init() otherwise this call will fail. 
	
		detect_part 	- a 1 byte char to tell us which partition number to detect, or 0 to scan all 4 primary partitions
	*/
	
	/* Returns ERR_NONE on success otherwise returns appropriate error code */
	
	int	addr_low, addr_hi;
	int 	byte_address;
	char 	i, i_max, i_min;
	char	ptype;
	char	first_part, first_part_type;
	
	first_part = 0;
	first_part_type = 0;
	addr_low = 0x0000;
	addr_hi = 0x0000;
	
	/* 
		Clear the global memory structures so that we having 
		nothing old hanging around in the sector, part or fs data structures.
	*/
	clearFATBuffers();
	
	#ifdef FATDEBUG
	put_string("disk_read", 1, (INFO_LINE_START + 2));
	put_hex(addr_hi, 4, 12, INFO_LINE_START + 2);
	put_hex(addr_low, 4, 17, INFO_LINE_START + 2);
	#endif
	everdrive_error = disk_read_single_sector(addr_low, addr_hi, sector_buffer);
	if (everdrive_error != ERR_NONE) {
		/* disk read error */
		#ifdef FATDEBUG
		put_string("Error", 26, (INFO_LINE_START + 2));
		put_number(everdrive_error, 3, 22, (INFO_LINE_START + 2));
		#endif
		/*return everdrive_error;*/
	} else {
		#ifdef FATDEBUG
		put_string("Ok", 26, (INFO_LINE_START + 2));
		put_number(everdrive_error, 3, 22, (INFO_LINE_START + 2));
		#endif
	}
	
	#ifdef FATDEBUG
	put_string("isValidMBR", 1, (INFO_LINE_START + 3));
	#endif
	everdrive_error = isValidMBR(sector_buffer);
	if (everdrive_error != ERR_NONE) {
		/* missing MBR checksum error */
		#ifdef FATDEBUG
		put_string("Error", 26, (INFO_LINE_START + 3));
		put_number(everdrive_error, 3, 22, (INFO_LINE_START + 3));
		#endif
		/*return everdrive_error;*/
	} else {
		#ifdef FATDEBUG
		put_string("Ok", 26, (INFO_LINE_START + 3));
		put_number(everdrive_error, 3, 22, (INFO_LINE_START + 3));
		#endif
	}
	
	/* for each of the 4 possible partitions */
	if (detect_part == 0) {
		i_max = 4;
		i_min = 0;
	} else {
		i_max = detect_part;
		i_min = detect_part - 1;
	}
	for (i = i_min; i < i_max ; i++) { 
		/* calculate the byte address of the current partition entry 
		in the sector buffer */ 
		byte_address = MBR_Part_1_os + (i  * MBR_Part_sz);
		#ifdef FATDEBUG
		put_string("part@",  1, i + INFO_LINE_START + 4);
		put_hex(byte_address, 3, 6, i + INFO_LINE_START + 4);
		/*put_hex_count(sector_buffer + byte_address, 16, 0, i + INFO_LINE_START + 11);*/
		#endif
		/* is it a FAT partition? */
		ptype = isValidFAT(sector_buffer[(byte_address + Part_TypeCode_os)]);
		
		if (ptype != 0x00) {
			/* write partition type and partition entry */
			#ifdef FATDEBUG
			put_hex(ptype, 2, 23, i + INFO_LINE_START + 4);
			put_string("FAT",  12, i + INFO_LINE_START + 4);
			put_string("Ok",  26, i + INFO_LINE_START + 4);
			#endif
			if (detect_part != 0) {
				memcpy(part_type, ptype, 1);
				memcpy(part_entry, sector_buffer[byte_address], MBR_Part_sz);
			} else {
				/* 
					only exit if we're deliberately scanning a single partition
					otherwise record this as the first fat partition and scan for
					any others
				*/
				if (first_part == 0) {
					first_part = i + 1;
					first_part_type = ptype;
				}
			}
		} else {
			#ifdef FATDEBUG
			put_hex(ptype, 2, 23, i + INFO_LINE_START + 4);
			put_string("Not FAT",  12, i + INFO_LINE_START + 4);
			put_string("Error",  26, i + INFO_LINE_START + 4);
			#endif
		}
	}
	/* Write details of the first FAT partition encountered */
	if (first_part != 0) {
		byte_address = MBR_Part_1_os + ((first_part - 1)  * MBR_Part_sz);
		part_number = first_part;
		part_type = first_part_type;
		memcpy(part_entry, sector_buffer + byte_address, MBR_Part_sz);
		getPartBegin(part_lba_begin, part_entry);
		if (disk_get_cardtype() == 2) {
			/* LBA addressing */
			lba_addressing = 1;
		} else {
			lba_addressing = 0;	
		}
		#ifdef FATDEBUG
		put_string("byte_addr", 1, INFO_LINE_START + 8);
		put_hex(byte_address, 3, 12, INFO_LINE_START + 8);		
		put_string("part_type", 1, INFO_LINE_START + 9);
		put_hex(part_type, 2, 12, INFO_LINE_START + 9);
		put_string("Ok",  26, INFO_LINE_START + 9);
		put_string("Entire 16byte partition entry", 1, MAX_LINES - 3);
		put_hex_count(sector_buffer + byte_address, 16, 0, MAX_LINES - 2);
		#endif
		return ERR_NONE;	
	} else {
		#ifdef FATDEBUG
		put_string("byte_addr", 1, INFO_LINE_START + 8);
		put_string("N/A", 12, INFO_LINE_START + 8);
		put_string("Error",  26, INFO_LINE_START + 8);
		put_string("part_entry", 1, INFO_LINE_START + 9);
		put_string("N/A", 12, INFO_LINE_START + 9);
		put_string("Error",  26, INFO_LINE_START + 9);
		put_string("part_type", 1, INFO_LINE_START + 10);
		put_string("N/A", 12, INFO_LINE_START + 10);
		put_string("Error",  26, INFO_LINE_START + 10);
		#endif
		return ERR_NO_PART_ENTRY;
	}
}

isValidFAT(part_type_byte)
char* part_type_byte;
{
	/* 
		check a partition table entry to see if the filesystem type is FAT 
	
		Takes the byte located at the partition type offset of a partition record -
		returns the detected FAT16/32 partition type (should be the same as input), otherwise
		returns a blank partition code 0x00 
	*/
	
	switch(part_type_byte)
	{
		case TypeCode_FAT32:
			return TypeCode_FAT32;
			break;
		
		case TypeCode_FAT32_LBA:
			return TypeCode_FAT32_LBA;
			break;
			
		default:
			return 0x00;
			break;
	}
}

isValidMBR(ed_buffer)
char* ed_buffer;
{
	/* 
		Check the MBR contains the two trailing checksum bytes

		Takes an already retrieved 512 byte sector buffer of sector 0
		returns ERR_NONE on success, or error code on missing bytes 
	*/
	
	if ((ed_buffer[MBR_CheckByte_1_os] == MBR_CheckByte_1) && (ed_buffer[MBR_CheckByte_2_os] == MBR_CheckByte_2)) {
		return ERR_NONE;
	}
	return ERR_MISS_MBR_CKSUM;
}

getPartBegin(part_lba_begin, part_entry)
char* 	part_lba_begin;
char* 	part_entry;
{
	
	/*
		Returns the beginning LBA sector of the partition.
		part_lba_begin		- The retrieved 4byte packed 32bit sector address of the partition
		part_entry			- An existing 16byte record from the call to getMBRPart().
		
		This is only needed if re-reading a partition entry, as getMBRPart reads and sets this value.
	*/
	
	/* copy and flip the LBA starting sector address */
	memcpy(part_lba_begin, part_entry + Part_LBABegin_os, Part_LBABegin_sz);
	swap_int32(part_lba_begin);
	return ERR_NONE;
	
}
