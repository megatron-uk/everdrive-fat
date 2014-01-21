/* ===============================

	fat-misc.h
	======
	Miscellaneous helper functions for FAT code
	using the Turbo Everdrive flash card.

	John Snowdon (John.Snowdon@newcastle.ac.uk), 2014

 =============================== */

 clearFATBuffers()
{
	/* 
		zero out all global memory each time we call
		getMBR() as that routine sets initial global values.
	*/

	int 	i;
	
	/* sector buffer */
	for (i = 0; i < SECTOR_SIZE ; i++) {
		sector_buffer[i] = 0x00;
	}
	
	/* part_entry */
	for (i = 0; i < 16 ; i++) {
		part_entry[i] = 0x00;
	}
	
	/* sector addresses */
	for (i = 0; i < 4 ; i++) {
		part_lba_begin[i] = 0x00;
		fs_fat_lba_begin[i] = 0x00;
		fs_cluster_lba_begin[i] = 0x00;
		fs_sectors_per_fat[i] = 0x00;
	}
	
	fs_sector_size = 0;
	fs_sectors_per_cluster = 0;
	lba_addressing = 0;
	return 0;
}

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
			char, col		- Start column (text mode).
			char, row	- Start tow (text mode).
	*/
	
	char hex_byte;
	for (hex_byte=0; hex_byte<count; hex_byte++) {
		put_hex(buffer[hex_byte], 2, col + (2 * hex_byte), row);
	}
	return 0;
}
 
#ifdef FALSE_PART
false_part()
{
	/* TEST PARTITION RECORD */
	part_entry[0] = 0x00;
	part_entry[1] = 0x20;
	part_entry[2] = 0x21;
	part_entry[3] = 0x00;
	part_entry[4] = 0x0b;
	part_entry[5] = 0x6d;
	part_entry[6] = 0x3d;
	part_entry[7] = 0xf0;
	part_entry[8] = 0x00;
	part_entry[9] = 0x08;
	part_entry[10] = 0x00;
	part_entry[11] = 0x00;
	part_entry[12] = 0x00;
	part_entry[13] = 0xe8;
	part_entry[14] = 0x3a;
	part_entry[15] = 0x00;
	
	part_type = 0x0C;
	
	part_number = 1;
	
	getPartBegin(part_lba_begin, part_entry);
	lba_addressing = 1;
	
	return 0;
}

false_vol()
{
	
	/* TEST VOLUME SECTOR */
	int i;
	for (i = 0; i < SECTOR_SIZE ; i++) {
		sector_buffer[i] = 0x00;
	}
	
	/* memory loc / memory value / byte offset */
	sector_buffer[0] = 0xEB; /* 0x00 */
	sector_buffer[1] = 0x00;
	sector_buffer[2] = 0x90;
	sector_buffer[3] = 0x20;
	sector_buffer[4] = 0x20;
	sector_buffer[5] = 0x20;
	sector_buffer[6] = 0x20;
	sector_buffer[7] = 0x20;
	sector_buffer[8] = 0x20;
	sector_buffer[9] = 0x20;
	sector_buffer[10] = 0x20; /* 0x0A */
	sector_buffer[11] = 0x00;
	sector_buffer[12] = 0x02;
	sector_buffer[13] = 0x40;
	sector_buffer[14] = 0x34;
	sector_buffer[15] = 0x11; /* 0x0F */
	sector_buffer[16] = 0x02;
	sector_buffer[17] = 0x00;
	sector_buffer[18] = 0x00;
	sector_buffer[19] = 0x00;
	sector_buffer[20] = 0x00;
	sector_buffer[21] = 0xF8;
	sector_buffer[22] = 0x00;
	sector_buffer[23] = 0x00;
	sector_buffer[24] = 0x3F;
	sector_buffer[25] = 0x00;
	sector_buffer[26] = 0xFF;
	sector_buffer[27] = 0x00;
	sector_buffer[28] = 0x00;
	sector_buffer[29] = 0x20;
	sector_buffer[30] = 0x00;
	sector_buffer[31] = 0x00;
	sector_buffer[32] = 0x00;
	sector_buffer[33] = 0xC0; /* 0x20 */
	sector_buffer[34] = 0xEC;
	sector_buffer[35] = 0x00;
	sector_buffer[36] = 0x66;
	sector_buffer[37] = 0x07;
	sector_buffer[38] = 0x00;
}
#endif
