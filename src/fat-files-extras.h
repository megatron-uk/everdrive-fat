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
* fat-files-extras.h
* ======
* Helper functions, internal directory entry methods
* and housekeeping functions necessary for the high-level fat-files.h 
* calls.
*
* John Snowdon (John.Snowdon@newcastle.ac.uk), 2014
*/


/* ====================================

	Low level functions - cluster and directory operations

==================================== */

find_directory_entry(filename, fptr, file_type)
char*	filename;
char	fptr;
char	file_type;
{
	/*
		Searches for a named entry in a directory, the directory is located by the data
		stored at fwa[0].
		Follows clusters if the directory entry spans more than one cluster.
		
		Input:
			char*	filename	- pointer to null terminated string representing the file and path.
			char	fptr		- the file pointer we're conducting the search for.
			const char file_type	- either FILE_TYPE_FILE or FILE_TYPE_DIR.
		
		Output:
			0 on success.
			Non-zero on error or file/directory name not found.
	*/
	
	char next_cluster[4];	/* temporary 32bit calculations */
	char s;			/* loop counter of the number of sectors per cluster */
	char d;			/* loop counter for the number of directory entries per sector */
	char addr[4]; 		/* temporary address buffer */
	
	get_sector_for_cluster(addr, fwa + FILE_Cur_Cluster_os);

	/* Mark the sector buffer as in use by this file pointer now */
	restore_sector_buffer(0);
	
	/* Set the next cluster to initially be the first one we know */
	memcpy(next_cluster, fwa + FILE_Cur_Cluster_os, 4);
		
	/* While there are some clusters left in this chain ... */
	while (int32_is_zero(next_cluster) != 1){
		/* Until we've exhausted all sectors from this cluster ... */
		for (s = 0; s < fs_sectors_per_cluster; s++){
			/* Read 512 bytes of the sector into the buffer */
			everdrive_error = disk_read_single_sector(int32_to_int16_lsb(addr), int32_to_int16_msb(addr), sector_buffer);
			if (everdrive_error != ERR_NONE){
				return ERR_IO_ERROR;
			}
			/* loop through each 32byte record of this sector (16 records per sector) to see if we find a directory entry that matches */
			for (d = 0; d < 16; d++){
				/* Check the type of the directory entry */ 
				if (is_end_of_dir(sector_buffer + (d * FILE_DIR_sz))){
					/* end of directory */
					return ERR_END_OF_DIRECTORY;
					
				} else if (is_empty_dir_entry(sector_buffer + (d * FILE_DIR_sz))){
					/* unused directory entry */
					
				} else if (is_lfn_dir_entry(sector_buffer + (d * FILE_DIR_sz))){
					/* longfilename directory entry */
					
				} else {
					/* normal directory entry - could be file or subdir */
					
					/* is it a sub dir - check bit 3 of the attrib byte */
					if (is_sub_dir(sector_buffer + (d * FILE_DIR_sz))){
						/* sub dir */
						
						/* are we actually looking for a subdir at this point */
						if (file_type == FILE_TYPE_DIR){
							if (short_filename_match(sector_buffer + (d * FILE_DIR_sz) + DIR_Name_os, filename, 12 + d) == 1){
								/* we found the sub directory!
								store the directory entry, so the next search will start
								from that folder/cluster instead of root */
								store_directory_entry(sector_buffer + (d * FILE_DIR_sz), 0, 0);
								return 0;
							}
						}
					} else {
						/* file */
						
						/* are we actually looking for a file at this point */
						if (file_type == FILE_TYPE_FILE){
							if (short_filename_match(sector_buffer + (d * FILE_DIR_sz) + DIR_Name_os, filename, 12 + d) == 1){
								/* we found the file!
								store its directory entry under the correct file pointer number */
								store_directory_entry(sector_buffer + (d * FILE_DIR_sz), fptr, 0);
								return 0;
							}
						}
					}
				}
			}
		}
		/* lookup and set next cluster */
		if (get_next_cluster(fwa + FILE_Cur_Cluster_os, 1) != 0){
			/* If there isn't a next cluster, return file not found and end of chain */
			return ERR_END_OF_CHAIN;
		}
	}
	return ERR_FILE_NOT_FOUND;
}

get_next_sector(dir_entry, set)
char*	dir_entry;
char	set;
{
	/* 
		Increment sector address if there are any remaining sectors in this filesystem 
	
		Input:
			char*	dir_entry	- pointer to directory entry structure.
			char	set		- if true, updates directory entry current sector fields.
			
		Output:
			0 on success and detection of an available next sector within the current cluster.
			Non-zero on no next sector within the current cluster.
	*/
	
	/* TODO: Check max sector count of partition */
	
	return 0;
}

restore_sector_buffer(fptr)
char	fptr;
{
	/*
		If the sector buffer is not currently owned by this file pointer it
		reverts it to the state time it was used according to the file metadata 
		in the file work area for this pointer.
		
		This should be present at the start of every fread(), fseek(), fget(), fwrite() etc. function call.
		
		Input:
			char	fptr		- The number of the file pointer.
			
		Returns:
			0 on Success and restoration of the sector buffer.
			Non-zero on error.
	*/

	int	addr_lo, addr_hi;
	/* Don't need to do anything, already own the buffer */
	if (sector_buffer_current_fptr == fptr) return 0;
	
	sector_buffer_current_fptr = fptr;
	
	/* If the position in sector buffer value is not equal to the size of a sector then re-read the last sector 
	if ((fwa[((fptr * FILE_WORK_SIZE) + FILE_Cur_PosInBuffer_os)] != SECTOR_SIZE) || (fwa[((fptr * FILE_WORK_SIZE) + FILE_Cur_PosInBuffer_os + 1)] != 0)){
	*/
	if (*fptr_sector_pos(fptr) < (fs_sector_size - 1)){
		addr_lo = int32_to_int16_lsb(fwa + (fptr * FILE_WORK_SIZE) + FILE_Cur_Sector_LBA_os);
		addr_hi = int32_to_int16_msb(fwa + (fptr * FILE_WORK_SIZE) + FILE_Cur_Sector_LBA_os);
		everdrive_error = disk_read_single_sector(addr_lo, addr_hi, sector_buffer);
		if (everdrive_error != ERR_NONE){
			return ERR_IO_ERROR;
		}
	}
	
	return 0;
	
}

get_next_cluster(dir_entry, set)
char*	dir_entry;
char	set;
{
	
	/*
		Given a directory entry, read the FAT to see what its next cluster in the chain is.
		How to find a FAT entry for a cluster 
		A FAT entry is 32bits
		128 entries per sector (assuming sector = 512bytes)
		
		cluster_number == FAT entry number
		
		e.g. cluster_number 255
			255 / 128 = 1..... 
			sector = fs_fat_lba_begin + 1
			read sector 
			
		Input:
			char*	dir_entry	- pointer to directory entry structure.
			char	set		- if true, updates directory entry current cluster and current sector fields.
			
		Returns:
			0 on success and detection of the available next cluster.
			Non-zero on cluster not found or no next cluster.
	*/
	
	char	fat_sector_lba[4];
	char	fat_sector_offset[4];
	char	skip_sectors[4];
	char	end_of_file[4];
	char	skip_records[4];
	char	next_cluster[4];
	
	end_of_file[0] = 0xFF;
	end_of_file[1] = 0xFF;
	end_of_file[2] = 0xFF;
	end_of_file[3] = 0xFF;
	
	restore_sector_buffer(0);
	
	zero_int32(fat_sector_lba);
	zero_int32(fat_sector_offset);
	zero_int32(skip_sectors);
	zero_int32(skip_records);
	
	/* Take a copy of the current cluster number - eg 255 */	
	copy_int32(fat_sector_offset, dir_entry + FILE_Cur_Cluster_os);
	
	/* Divide by 128 to get number of sectors in the FAT before the one that holds our desired cluster chain - eg 1 */
	div_pow_int32(fat_sector_offset, 7);
	
	/* Multiply the number of sectors found by the number of fat entries in one sector - eg 1 x 128 */
	mul_int32_int8(skip_sectors, fat_sector_offset, CLUSTER_FAT_ENTRIES_PER_SECTOR);
	
	/* Subtract that number from the current cluster number - eg 255 - 128 = 127 
	This is how many 32bit records we need to skip in the sector buffer until we get to the one we want */
	
	/* THIS IS NOT IMPLEMENTED!!! */
	sub_int32(skip_records, fat_sector_offset, skip_sectors);
	
	/* Add the offset onto the start sector for the fat to let the hardware know what sector of the disk to read */
	add_int32(fat_sector_lba, fs_fat_lba_begin, fat_sector_offset);
	
	everdrive_error = disk_read_single_sector(int32_to_int16_lsb(fat_sector_lba), int32_to_int16_msb(fat_sector_lba), sector_buffer);
	if (everdrive_error != ERR_NONE){
		return ERR_IO_ERROR;
	} else {
		memcpy(next_cluster, sector_buffer + (skip_records * CLUSTER_FAT_ENTRY_SIZE), CLUSTER_FAT_ENTRY_SIZE);
		/* test if valid next cluster */
		if (gte_int32(next_cluster, end_of_file)){
			/* invalid - current cluster is marked as last */
			return 1;	
		}
		/* if valid and if set then update cluster number */
		if (set == 1){
			/* update dir entry */
			memcpy(dir_entry + FILE_Cur_Cluster_os, sector_buffer + (skip_records * CLUSTER_FAT_ENTRY_SIZE), FILE_Cur_Cluster_sz);
			/* correct endian-ness */
			swap_int32(dir_entry + FILE_Cur_Cluster_os);
			return 0;
		}
		/* if valid return 0 == next cluster found */
		return 0;
	}
}

is_empty_dir_entry(dir_entry)
char*	dir_entry;
{
	/* Checks for a empty directory entry marker in a given directory entry */
	
	if (dir_entry[DIR_Name_os] == 0xE5) return 1;
	return 0;
}

is_end_of_dir(dir_entry)
char*	dir_entry;
{
	/* Checks for a end-of-dir marker at a given dir entry */
	
	if (dir_entry[DIR_Name_os] == 0x00) return 1;
	return 0;
}

is_lfn_dir_entry(dir_entry)
char*	dir_entry;
{
	/* Checks for a longfilename signature at a dir entry - returns true if all 4 least significant bits must be set */

	if (dir_entry[DIR_Attr_os] & 0x0F) return 1;
	return 0;	
}

is_sub_dir(dir_entry)
char*	dir_entry;
{
	/* checks attrib byte of a directory entry and returns true if its a subdir */
	
	if (dir_entry[DIR_Attr_os] & 0x10) return 1;
	return 0;
}

get_sector_for_cluster(address, cluster_number)
char*	address;
char*	cluster_number;
{
	/*
		Given a cluster number, set the LBA sector address.
	*/	
	
	/* 
		formula to work out LBA address based on cluster number:
		lba_addr = cluster_begin_lba + ((cluster_number - 2) * sectors_per_cluster);		
	*/
	
	char offset_num_clusters[4], offset_num_sectors[4];
	
	if (memcmp(cluster_number, fs_root_dir_cluster, 4) == 0){
		memcpy(address, fs_cluster_lba_begin, 4);
	} else {
		zero_int32(offset_num_sectors);
		copy_int32(offset_num_clusters, cluster_number);
		dec_int32(offset_num_clusters);
		dec_int32(offset_num_clusters);
		mul_int32_int8(offset_num_sectors, offset_num_clusters, fs_sectors_per_cluster);
		put_hex_count(offset_num_clusters, 4, 0, MAX_LINES);
		add_int32(address, fs_cluster_lba_begin, offset_num_sectors);
	}
}

store_directory_entry(base_addr, fptr, use_root)
char*	base_addr;
char	fptr;
char	use_root;
{
	/*
		Update the subdir entry / file metadata for
		a given file0x2590 + ((15e - 2) * 8) pointer from a base address in 
		memory (usually a found directory entry).
		
		Input:
			char*	base_addr	- Location in memory that holds the directory/file metadata for this file pointer.
			char	fptr		- The number of the open filepointer we're updating. Used as an index into the file work area.
			char	root		- Store root directory information (reset fptr #0 to the root directory entry)
						
		Returns:
			N/A.
	*/
	
	char b; /* loop counter */
	
	/* Are we re-loading the root directory? */
	if (use_root == 1){
		/* Yes, blank any existing data in that segment of the file work area */
		for (b = 0; b < FILE_WORK_SIZE; b++){
			fwa[b] = 0x00;
		}
		/* Restore the starting cluster of the root directory */
		memcpy(fwa + FILE_Cur_Cluster_os, fs_root_dir_cluster, 4);
		
	} else {
		/* No, store new (sub) directory or file entry */
		memcpy(fwa + (fptr * FILE_WORK_SIZE), base_addr, FILE_DIR_sz); 
		
		/* Byte swap from little to big-endian: filesize and cluster number */
		swap_int32(fwa + (fptr * FILE_WORK_SIZE) + FILE_DIR_os + DIR_FileSize_os);
		swap_int16(fwa + (fptr * FILE_WORK_SIZE) + FILE_DIR_os + DIR_FstClusHI_os);
		swap_int16(fwa + (fptr * FILE_WORK_SIZE) + FILE_DIR_os + DIR_FstClusLO_os);
		
		/* Set total number of clusters to be 0 */
		/* TODO go off and count cluster chain */
		fwa[((fptr * FILE_WORK_SIZE) + FILE_Total_Clusters_os)] = 0;
		fwa[((fptr * FILE_WORK_SIZE) + FILE_Total_Clusters_os + 1)] = 0;
		
		/* Set current cluster count to be 0 */
		fwa[((fptr * FILE_WORK_SIZE) + FILE_Cur_Cluster_Count_os)] = 0;
		fwa[((fptr * FILE_WORK_SIZE) + FILE_Cur_Cluster_Count_os + 1)] = 0;
		
		/* Set current byte position in file to be 0 */
		zero_int32(fwa + (fptr * FILE_WORK_SIZE) + FILE_Cur_PosInFile_os);
		
		/* Set current byte position in sector buffer to be 0 */
		zero_int32(fwa + (fptr * FILE_WORK_SIZE) + FILE_Cur_PosInBuffer_os);
		
		/* Set current cluster number to be the starting cluster 
		found in the directory entry fields */
		memcpy(fwa + (fptr * FILE_WORK_SIZE) + FILE_Cur_Cluster_os, fwa + (fptr * FILE_WORK_SIZE) + FILE_DIR_os + DIR_FstClusHI_os, 2);
		memcpy(fwa + (fptr * FILE_WORK_SIZE) + FILE_Cur_Cluster_os + 2, fwa + (fptr * FILE_WORK_SIZE) + FILE_DIR_os + DIR_FstClusLO_os, 2);
	
		/* Set current sector number to be the first one in the starting cluster */
		get_sector_for_cluster(fwa + (fptr * FILE_WORK_SIZE) + FILE_Cur_Sector_LBA_os, fwa + (fptr * FILE_WORK_SIZE) + FILE_Cur_Cluster_os);
		
		/* set current sector count (of N sectors per cluster) to be 0 */
		fwa[((fptr * FILE_WORK_SIZE) + FILE_Cur_Sector_Count_os)] = 0;
		fwa[((fptr * FILE_WORK_SIZE) + FILE_Cur_Sector_Count_os + 1)] = 0;
	}
}

fptr_get_next_sector(fptr)
char	fptr;
{
	/* Update the sector LBA address for an open file, return an error if there
	are no further sectors. 
	
		Input:
			char fptr	- An existing open file pointer
			
		Output:
			0 on success
			Non-zero on error or no further sectors
	
	*/
	
	/* Check if any further sectors in the current cluster */
	if (fptr_cluster_sector_pos(fptr) < fs_sectors_per_cluster){
		/* Yes, just update to next sector */
		/* Update cluster sector pos - i.e. sector 12 of 16 -> 13 of 16 */
		fwa + ((fptr * FILE_WORK_SIZE) + FILE_Cur_Cluster_Count_os);
		/* Update sector LBA address - i.e. 00003078 -> 00003079 */
		inc_int32(fwa + ((fptr * FILE_WORK_SIZE) + FILE_Cur_Cluster_os));
		/* Set sector buffer pos to zero */
		memcpy(fwa + ((fptr * FILE_WORK_SIZE) + FILE_Cur_PosInBuffer_os), 0x0000, 2);
		return 0;
	} else {
		/* No, but are there any more clusters? */
	
		/* Yes, update to that cluster */
			/* Update cluster sector pos - i.e. sector 12 of 16 -> 13 of 16 */
			mem(fwa + ((fptr * FILE_WORK_SIZE) + FILE_Cur_Cluster_Count_os));
			/* Update sector LBA address - i.e. 00003078 -> 00003079 */
			inc_int32(fwa + ((fptr * FILE_WORK_SIZE) + FILE_Cur_Cluster_os));
			/* Set sector buffer pos to zero */
			memcpy(fwa + ((fptr * FILE_WORK_SIZE) + FILE_Cur_PosInBuffer_os), 0x0000, 2);
		/* Update to the first sector of that cluster */
		
		/* No, this must be end of file */
	}
}

fptr_cluster_num(fptr)
char 	fptr;
{
	/* return current cluster number for the file 
	
		Input:
			char fptr	- An existing open file pointer
	
		Output:
			char*		- pointer to 32bit value representing the current cluster number
	*/
	
	return fwa + (fptr * FILE_WORK_SIZE) + FILE_Cur_Cluster_os;
}

fptr_cluster_sector_pos(fptr)
char	fptr;
{
	/* return current sector number within the current cluster for the file 
		e.g. sector 12 of 16 for this cluster.
	
		Input:
			char fptr	- An existing open file pointer
	
		Output:
			char		- the current sector we're reading of this clusterr
	*/
	
	return fwa + (fptr * FILE_WORK_SIZE) + FILE_Cur_Cluster_Count_os;
}

fptr_sector_num(fptr)
char	fptr;
{
	/* return current sector number for the file 
	
		Input:
			char fptr	- An existing open file pointer
	
		Output:
			char*		- pointer to 32bit value representing the current sector number
	*/
	
	return fwa + (fptr * FILE_WORK_SIZE) + FILE_Cur_Sector_LBA_os;
}

fptr_sector_pos(fptr)
char	fptr;
{
	/* return byte position within the current sector 
	
		Input:
			char fptr	- An existing open file pointer
	
		Output:
			int		- byte pos within the current sector
	*/
	
	return fwa + (fptr * FILE_WORK_SIZE) + FILE_Cur_PosInBuffer_os;
}

fptr_file_name(fptr)
char	fptr;
{
	/* return name of the file 
	
		Input:
			char fptr	- An existing open file pointer
	
		Output:
			char*		- pointer to 8+3 character array containing the filename
	*/
	
	return fwa + (fptr * FILE_WORK_SIZE) + FILE_DIR_os + DIR_Name_os;
}

fptr_file_size(fptr)
char	fptr;
{
	/* return file size for the file 
	
		Input:
			char fptr	- An existing open file pointer
	
		Output:
			char*		- pointer to 32bit value representing the file size
	*/
	
	return fwa + (fptr * FILE_WORK_SIZE) + FILE_DIR_os + DIR_FileSize_os;
}

fptr_file_pos(fptr)
char	fptr;
{
	/* return current byte position within the file 
	
		Input:
			char fptr	- An existing open file pointer
	
		Output:
			char*		- pointer to 32bit value representing the current file position
	*/
	
	return fwa + (fptr * FILE_WORK_SIZE) + FILE_Cur_PosInFile_os;
}

short_filename_match(fat_name, match_name, l)
char*	fat_name;
char*	match_name;
char l;
{
	/*
		Attempt to see if two null-terminated strings in memory, of length str_sz
		are the same, accounting for trailing spaces in either str1 or str2.
		
		This is designed for short (8+3) DOS filenames, where each string
		can be 0-8 characters, with a suffix of 0-3.
		
		A filename read from disk can have trailing spaces before the suffix. e.g.
		FILE    TXT
		... is the same as:
		FILE.TXT
		... entered by a user
		
		Input:
			char*	fat_name	- pointer to filename string as retrieved from filesystem (embedded spaces, no '.' seperator).
			char*	match_name	- pointer to a null terminated string we're looking to match.
			
		Returns:
			1 on match.
			0 on not matched.
	*/
	char	ci;	/* index into 11 char filename */
	char	cnt;	/* next position to insert a char into space-removed filename */
	char	a,b;	/* case converted chars of filename */
	char	shortname[MAX_FILENAME_SIZE]; /* space-removed filename from filesystem */
		
	/* Truncate the fat filename to collapse spaces after filenmame 
	and before extension, after extensions and add null terminator */
	cnt = 0;
	for (ci = 1; ci <= MAX_FILENAME_SIZE; ci++){
		shortname[(ci - 1)] = 0x00;
	}
	for (ci = 1; ci <= MAX_FILENAME_SIZE; ci++){
		/* is it a space? */
		if (fat_name[(ci - 1)] == ' '){
			/* yes - are we at less than pos 9 */
			if (ci < 9){
				/* yes - is the char as pos 9 a space */
				if (fat_name[8] == ' '){
					/* yes - this is the end of the string */
					shortname[cnt] = '\0';
					break;
				} else {
					/* no - store a '.' and move to pos 9 */
					shortname[cnt] = '.';
					cnt++;
					ci = 8;
				}
			} else {
				/* no - end of string */
				shortname[cnt] = '\0';
				break;
			}
		} else {
			/* no - store char */
			shortname[cnt] = fat_name[(ci - 1)];
			cnt++;
			/* if this is pos 11 add the null terminator and exit */
			if (ci == 12){
				shortname[cnt] = '\0';
				break;
			}
		}
	}
	
	for (ci = 0; ci <= MAX_FILENAME_SIZE; ci++){
		/* Have we reached the null byte of the match string? */
		if (match_name[ci] != '\0'){
			/* no */			
			if ((shortname[ci] > 96) && (shortname[ci] < 123)){
				a = shortname[ci] - 'a' + 'A';
			} else {
				a = shortname[ci];
			}
			
			if ((match_name[ci] > 96) && (match_name[ci] < 123)){
				b = match_name[ci] - 'a' + 'A';
			} else {
				b = match_name[ci];
			}	
			
			if (a != b) {
				return 0;
			}
		} else {
			/* yes */
			return 1;	
		}
	}
	return 1;
}
