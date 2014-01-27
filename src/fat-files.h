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
* fat-files.h
* ======
* A 'stdio-like' library for file handling functions,
* specifically for the Turbo Everdrive FAT library.
* 
* John Snowdon (John.Snowdon@newcastle.ac.uk), 2014
*/
 
#define EOF		"\0"
#define SEEK_SET 	0
#define SEEK_CUR	1
#define SEEK_END 	2
 
/* ===============================
Open/Close file descriptors
=============================== */

fopen(f_path)
char*	f_path;
{
	/* 
		Open a new file and return a file pointer.
		Paths are split by UNIX '/' or DOS '\' style slashes:
		
		File paths are like this:
			"/temp/saves/save.dat"
			"\tmp\saves\save.dat" 
			
		File paths with trailing slashes are assumed to be directories:
			"/games/japan/soldier.pce/" 
			"\games\japan\soldier.pce\" 
			
		We don't use device names:
			"C:\music\motoroad.nsf" 
			"1:\music\motoroad.nsf" 
		
		The first leading slash always refers to the root directory of the current selected filesystem,
		so we can look up its directory entries using "fs_root_dir_cluster".
		
		Input:
			char*, f_path		- Pointer to a null terminated string representing the path to a file.
							Both MS-DOS ("\") and Unix style ("/") directory access is supported - e.g.
								fopen("/homebrew/utils/data/file.txt")
								fopen("\games\save1.dat")
							Access is relative to the root directory of the current open FAT partition. 
							Device names are not supported.
		
		Returns: 
			char, fptr 	- Number of the open file pointer on success.
			0 on failure and sets global var everdrive_error with status code.
	*/
	
	char filename[MAX_FILENAME_SIZE];
	char s_start, s_end;
	char n, fptr;
	

	/* Are any file pointers free? */
	fptr = 0;
	/* fptr 0 is reserved for directory access */
	#ifdef FILEDEBUG
	put_string("Free pointer:", 0, 5);
	#endif
	for (n = 1; n <= NUM_OPEN_FILES; n++) {
		if (file_handles[n] == FPTR_CLOSE_STATUS) {
			fptr = n;
			#ifdef FILEDEBUG
			put_number(fptr, 1, 14, 5);
			#endif
			break;
		}
	}

	/* No free file pointers */
	if (fptr == 0) {
		#ifdef FILEDEBUG
		put_string("None", 14, 5);
		#endif
		everdrive_error = ERR_NO_FREE_FILES;
		return 0;
	}
	/* We have a free file pointer - now try and open the file */

	/* Mark this file pointer as in use */
	file_handles[fptr] = FPTR_OPEN_STATUS;
	
	/* Strip any leading whitespace from the file path */
	while (*f_path == ' '){
		f_path++;
	}
	/* Strip leading directory seperator */
	if ((*f_path == '/') || (*f_path == '\\' )){
		f_path++;
	}
	
	s_start = 0;
	s_end = 1;
	
	/* Load root directory entry so that we can scan for subdirs and files */
	store_directory_entry(0, 0, 1);
	
	#ifdef FILEDEBUG
	put_string("Path:", 0, 6);
	put_string(f_path, 5, 6);
	put_string("Progress:", 0, 7);
	n = 9;
	#endif
	
	for (;;){
		/* Is this character a directory seperator */
		
		if ((f_path[s_end] == 0x2F) || (f_path[s_end] == 0x5C)){
			/* Yes, so the string so far is a sub directory of the current dir */		
			/* find sub dir entry and load it */
			strncpy(filename, f_path + s_start, (s_end - s_start));
			filename[(s_end - s_start)] = '\0';
			if (find_directory_entry(filename, fptr, FILE_TYPE_DIR) == 0){
				/* we then go back around the loop again but searching
				the found sub directory instead... */
				#ifdef FILEDEBUG
				put_string(".", n, 7);
				n++;
				#endif
				/* set start to be end and move both pointers on by +1 */
				s_start = s_end;
				s_start++;
				s_end++;
			} else {
				/* subdir wasn't found in the directory, the path is invalid */
				fclose(fptr);
				#ifdef FILEDEBUG
				put_string("_", n, 7);
				#endif
				everdrive_error = ERR_DIR_NOT_FOUND;	
				return 0;
			}
		} else {
			/* No */
			/* Is the path at this point null terminated? */
			if (f_path[s_end] == 0x00){
				/* Yes, so it's the actual file name */
				strncpy(filename, f_path + s_start, (s_end - s_start));
				filename[(s_end - s_start)] = '\0';
				if (find_directory_entry(filename, fptr, FILE_TYPE_FILE) == 0){
					/* Return this file pointer */
					#ifdef FILEDEBUG
					put_string("o", n, 7);
					n++;
					#endif
					return fptr;
				} else {
					/* file entry wasn't found, the filename is invalid */
					fclose(fptr);
					#ifdef FILEDEBUG
					put_string("x", n, 7);
					#endif
					everdrive_error = ERR_FILE_NOT_FOUND;	
					return 0;
				}
			} else {
				/* No, move the end pointer and go around the loop again ... */
				s_end++;	
			}
		}
		if ((s_end - s_start) > MAX_FILENAME_SIZE){
			#ifdef FILEDEBUG
			put_string("Length!", n, 7);
			#endif
			fclose(fptr);
			everdrive_error = ERR_FILENAME_TOO_LONG;
			return 0;	
		}
	}	
}

fclose(fptr)
char	fptr;
{
	 /* 
		Close an open file pointer 
	 
		Input:
			char, fptr			- Number of an open file pointer to be closed.
		
		Returns: 
			0 on success
			Non-zero error code on failure
			
		Use global variables:
			char		file_handles[n]		- array of open/close file numbers.
			char		fwa[x]		- buffer of open file metadata.
			const	FILE_WORK_SIZE		- how many bytes of buffer memory are used by a single open file.
			const	FPTR_CLOSE_STATUS	- constant to set the fptr flag to.
	 */
	 
	 int	n;
	 
	 /* Erase buffer memory used by this file */
	 for (n = ((fptr - 1)* FILE_WORK_SIZE); n< (fptr * FILE_WORK_SIZE); n++) {
	 	 fwa[n] = 0x00;
	 }
	 
	 /* Mark fptr as free */
	 file_handles[fptr - 1] = FPTR_CLOSE_STATUS;
	 
	 return ERR_NONE;
}

/* ===============================
Read/Write multiple bytes
=============================== */

fread(fptr, f_buf, n_bytes)
char	fptr;
char*	f_buf;
int	n_bytes;
{
	/* 
		Read a number of bytes from an open file pointer to memory.
		Increments file pointer position by the number of bytes read.
		
		Input:
			char, fptr 		- The number of an open file pointer, as returned by fopen().
			char*, f_buf 	- Pointer to memory location.
			int, n_bytes 	- Number of bytes to read from the file into memory.
		
		Returns: 
			0 on success
			Non-zero error code on failure
	*/
}

fwrite(fptr, f_buf, n_bytes)
char	fptr;
char*	f_buf;
int	n_bytes;
{
	/* 
		Write a number of bytes from memory to an open file pointer.
		Increments file pointer position by the number of bytes written.
	
		Input:
			char, fptr 		- The number of an open file pointer, as returned by fopen().
			char*, f_buf 	- Pointer to memory location to read from.
			int, n_bytes 	- Number of bytes to write to the file from memory.
		
		Returns: 
			0 on success
			Non-zero error code on failure
	*/
}

/* ===============================
Read/Write single bytes
=============================== */

fgetc(fptr)
char	fptr;
{
	/* 
		Read a single byte from an open file pointer to memory and increments the
		file position pointer by 1.
		
		Input:
			char, fptr		- The number of an open file pointer, as returned by fopen().
		
		Returns: 
			on success returns char data.
			on error - non-zero error code.
	*/
}

fputc(fptr, f_char)
char	fptr;
char	f_char;
{
	/* 
		Write a single byte to an open file pointer from a memory location and increments the
		file position pointer by 1.
		
		Input:
			char, fptr 		- The number of an open file pounter, as returned by fopen().
			char, f_char	- 8bit data to be written to file.
		
		Returns: 
			on success returns f_char. 
			on error - non-zero error code.
	*/ 		 
}

/* ===============================
File position functions.
=============================== */

fseek(fptr, fpos, seek_mode)
char	fptr;
char*	fpos;
char	seek_mode;
{
	/* 
		Seek to a position in a given file
		
		Input:
			char, fptr 			- The number of an open file pounter, as returned by fopen().
			char*, fpos		- A 4byte memory location emulating a 32bit integer representing the offset into the file to move the file pointer.
			char, seek_mode	- One of SEEK_SET, SEEK_CUR, SEEK_END indicating 
								SEEK_SET: seek from beginning of file
								SEEK_CUR: seek from current position
								SEEK_END: seek from end of file
			
		Returns: 
			0 on success
			Non-zero error code on failure
	*/
 
}

fgetpos(fptr)
char	fptr;
{
	/* 
		Return the current position in the file that fptr is pointing to.
	
		Input:
			char, fptr 	- The number of an open file pounter, as returned by fopen().
		
		Returns: 
			Returns a pointer [char*] to the 32bit file position counter for this fptr object. 
	*/	 
}

frewind(fptr)
char	fptr;
{
	/* 
		Return the file pointer back to the beginning of the file.
		
		Input:
			char, fptr 	- The number of an open file pounter, as returned by fopen().
		
		Returns: 
			0 on success
			Non-zero error code on failure
	*/	 
}


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
	
	/* debug stats */
	#ifdef FILEDEBUG
	int c, cs;
	c = 0;
	cs = 0;
	#endif
	
	#ifdef FILEDEBUG
	/* What is the current 'working' directory - is it the root dir cluster */
	if (memcmp(fwa + FILE_Cur_Cluster_os, fs_root_dir_cluster, 4) == 0){
		/* Yes root cluster */
		put_string("searching root directory", 0, 9);
	} else {
		/* No, some other cluster - we're in a sub dir */
		put_string("searching sub-directory", 0, 9);
	}
	#endif
	get_sector_for_cluster(addr, fwa + FILE_Cur_Cluster_os);

	/* Mark the sector buffer as in use by this file pointer now */
	sector_buffer_current_fptr = 0;
	
	/* Set the next cluster to initially be the first one we know */
	memcpy(next_cluster, fwa + FILE_Cur_Cluster_os, 4);
	
	#ifdef FILEDEBUG
	if (file_type == FILE_TYPE_DIR){
		put_string("subdir search :            ", 0, 8);
		put_string(filename, 15, 8);
	} else {
		put_string("file search   :            ", 0, 8);
		put_string(filename, 15, 8);	
	}
	#endif
	
	/* While there are some clusters left in this chain ... */
	#ifdef FILEDEBUG
	put_string("cluster:        h/        h LBA", 0, 10);
	put_string("sector :    /   ", 0, 11);
	put_number(fs_sectors_per_cluster, 3, 13, 11);
	#endif
	while (int32_is_zero(next_cluster) != 1){
		#ifdef FILEDEBUG
		put_hex_count(next_cluster, 4, 8, 10);
		put_hex_count(addr, 4, 18, 10);
		c++;
		#endif
		/* Until we've exhausted all sectors from this cluster ... */
		for (s = 0; s < fs_sectors_per_cluster; s++){
			#ifdef FILEDEBUG
			put_number(s, 3, 8, 11);
			cs++;
			#endif
			/* Read 512 bytes of the sector into the buffer */
			everdrive_error = disk_read_single_sector(int32_to_int16_lsb(addr), int32_to_int16_msb(addr), sector_buffer);
			if (everdrive_error != ERR_NONE){
				#ifdef FILEDEBUG
				put_string("Error reading data", 0, MAX_LINES - 2);
				#endif
				return ERR_IO_ERROR;
			}
			/* loop through each 32byte record of this sector (16 records per sector) to see if we find a directory entry that matches */
			for (d = 0; d < 16; d++){
				#ifdef FILEDEBUG
				put_string("                           ", 0, 12 + d);
				#endif
				/* Check the type of the directory entry */ 
				if (is_end_of_dir(sector_buffer + (d * FILE_DIR_sz))){
					
					/* end of directory */
					#ifdef FILEDEBUG
					put_string("<   > END", 0, 12 + d);
					#endif
					return ERR_END_OF_DIRECTORY;
					
				} else if (is_empty_dir_entry(sector_buffer + (d * FILE_DIR_sz))){
					/* unused directory entry */
					#ifdef FILEDEBUG
					put_string("<   > Empty", 0, 12 + d);
					#endif
					
				} else if (is_lfn_dir_entry(sector_buffer + (d * FILE_DIR_sz))){
					/* longfilename directory entry */
					#ifdef FILEDEBUG
					put_string("<   > LFN", 0, 12 + d);
					#endif
					
				} else {
					/* normal directory entry - could be file or subdir */
					
					/* is it a sub dir - check bit 3 of the attrib byte */
					if (is_sub_dir(sector_buffer + (d * FILE_DIR_sz))){
						/* sub dir */
						#ifdef FILEDEBUG
						put_string("<DIR> ", 0, 12 + d);
						put_string_count(sector_buffer + (d * FILE_DIR_sz), 11, 6, 12 + d);
						#endif
						
						/* are we actually looking for a subdir at this point */
						if (file_type == FILE_TYPE_DIR){
							if (short_filename_match(sector_buffer + (d * FILE_DIR_sz) + DIR_Name_os, filename, 12 + d) == 1){
								#ifdef FILEDEBUG
								put_string("!", 30, 12 + d);
								#endif
								/* we found the sub directory!
								store the directory entry, so the next search will start
								from that folder/cluster instead of root */
								store_directory_entry(sector_buffer + (d * FILE_DIR_sz), 0, 0);
								return 0;
							} else {
								#ifdef FILEDEBUG
								put_string(".", 30, 12 + d);
								#endif
							}
						}
					} else {
						/* file */
						#ifdef FILEDEBUG
						put_string("<FIL> ", 0, 12 + d);
						put_string_count(sector_buffer + (d * FILE_DIR_sz), 11, 6, 12 + d);
						#endif
						
						/* are we actually looking for a file at this point */
						if (file_type == FILE_TYPE_FILE){
							if (short_filename_match(sector_buffer + (d * FILE_DIR_sz) + DIR_Name_os, filename, 12 + d) == 1){
								#ifdef FILEDEBUG
								put_string("!", 30, 12 + d);
								#endif
								/* we found the file!
								store its directory entry under the correct file pointer number */
								store_directory_entry(sector_buffer + (d * FILE_DIR_sz), fptr, 0);
								return 0;
							} else {
								#ifdef FILEDEBUG
								put_string(".", 30, 12 + d);
								#endif
							}
						}
					}
				}
			}
			
			/* Increment sector position */
			if (get_next_sector(addr) != 0){
				#ifdef FILEDEBUG
				put_string("No more sectors in FS", 0, MAX_LINES - 2);
				#endif
				return ERR_FS_END;	
			}
		}
		/* lookup next cluster */
		if (get_next_cluster(next_cluster, fwa + FILE_Cur_Cluster_os) != 0){
			zero_int32(next_cluster);
		} else {
			/* update address for disk read from the next cluster for this directory */	
			mul_int32_int8(addr, next_cluster, fs_sectors_per_cluster);
		}
	}
	#ifdef FILEDEBUG
	put_string("Nothing found", 0, MAX_LINES - 2);
	#endif
	return ERR_FILE_NOT_FOUND;
}

get_next_sector(sector_address)
char*	sector_address;
{
	/* 
		Increment sector address if there are any remaining sectors in this filesystem 
	
		Input:
			char*	sector_address	- pointer to 32bit number representing a LBA sector address;
			
		Output:
			0 on success and sets *sector_address.
			Non-zero on reaching sector limit.
	*/
	
	/* TODO: Check max sector count of partition */
	
	inc_int32(sector_address);
	
	return 0;
}

get_next_cluster(next_cluster, cluster)
char*	next_cluster;
char*	cluster;
{
	/*
		Given a cluster number, read the FAT to see what its next cluster in the chain is.
		
		Input:
			char*	next_cluster	- pointer to 32bit number to set on success.
			char*	cluster		- pointer to 32bit number representing current cluster.
			
		Returns:
			0 on success and sets *next_cluster.
			Non-zero on cluster not found or no next cluster and sets *next_cluster = cluster.
	*/
	
	return 1;
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
	
	char* offset_num_clusters[4], offset_num_sectors[4];
	
	if (memcmp(cluster_number, fs_root_dir_cluster, 4) == 0){
		memcpy(address, fs_cluster_lba_begin, 4);
	} else {
		zero_int32(offset_num_sectors);
		copy_int32(offset_num_clusters, cluster_number);
		dec_int32(offset_num_clusters);
		dec_int32(offset_num_clusters);
		mul_int32_int8(offset_num_sectors, offset_num_clusters, fs_sectors_per_cluster);
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
		a given file pointer from a base address in 
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
		memcpy(fwa + (fptr * FILE_DIR_os), base_addr, FILE_DIR_sz); 
		
		/* Byte swap from little to big-endian: filesize and cluster number */
		swap_int32(fwa + (fptr * FILE_DIR_os) + DIR_FileSize_os);
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
	}
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
