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
 
#include "fat/fat-files-extras.h"

#define EOF			"\0"
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
								Device names are NOT supported.
		
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
	for (n = 1; n <= NUM_OPEN_FILES; n++) {
		if (file_handles[n] == FPTR_CLOSE_STATUS) {
			fptr = n;
			break;
		}
	}

	/* No free file pointers */
	if (fptr == 0) {
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
				/* set start to be end and move both pointers on by +1 */
				s_start = s_end;
				s_start++;
				s_end++;
			} else {
				/* subdir wasn't found in the directory, the path is invalid */
				fclose(fptr);
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
					return fptr;
				} else {
					/* file entry wasn't found, the filename is invalid */
					fclose(fptr);
					everdrive_error = ERR_FILE_NOT_FOUND;	
					return 0;
				}
			} else {
				/* No, move the end pointer and go around the loop again ... */
				s_end++;	
			}
		}
		if ((s_end - s_start) > MAX_FILENAME_SIZE){
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
			char, fptr					- Number of an open file pointer to be closed.
		
		Returns: 
			0 on success
			Non-zero error code on failure
			
		Use global variables:
			char	file_handles[n]		- array of open/close file numbers.
			char	fwa[x]				- buffer of open file metadata.
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
		This is probably one of the most complex routines.
		
		Input:
			char, fptr 		- The number of an open file pointer, as returned by fopen().
			char*, f_buf 	- Pointer to memory location.
			int, n_bytes 	- Number of bytes to read from the file into memory.
		
		Returns: 
			Non-zero on success (number of bytes reads)
			0 on failure and sets everdrive_error
	*/
		
	/* 
		did we do a partial read last time?
			yes
				do we still own sector_buffer?
					yes
						copy remaining bytes to f_buf (eg pos 500 - 512)
						if we read n_bytes then 
							return size
						else
							decrement n_bytes
					no
						set ownsership of sector buffer
						read entire sector
						copy bytes from old seek pos to f_buf (eg pos 500 - 512)
						if we read n_bytes then 
							return size
						else
							decrment n_bytes
			no
				skip
				
		if n_bytes > 0
		check if any sectors are left in the current cluster
			yes
				increment sector number
				set ownership of sector buffer
				read sector
				copy new bytes to f_buf (eg pos 0 - 116)
				return size
			no
				check if there are any more clusters
				yes
					set new cluster number
					get new sector number
					set ownership of sector_buffer
					read sector
					copy new bytes to f_buf (eg pos 0 - 116)
					return size
				no
					send EOF
	*/
	
	int xfer_bytes;	
	int rem_bytes;
	int os;
	char rem_bytes_32[4];
	int buffer_os;
	
	os = 0;
	/* restore the sector buffer for our use, if necessary */
	restore_sector_buffer(fptr);
	
	/* only continue if we're not already at the end of the file */
	if (lt_int32(fptr_file_pos(fptr), fptr_file_size(fptr))){
		
		/* Did we do a partial read last time? */ 
		rem_bytes = (fs_sector_size - 1) - fptr_sector_pos(fptr);
		if (rem_bytes > 0){
			/* Yes, so we should read the rem_bytes remaining bytes from this sector first */
			everdrive_error = disk_read_single_sector(int32_to_int16_lsb(fptr_sector_num(fptr)), int32_to_int16_msb(fptr_sector_num(fptr)), sector_buffer);
			if (everdrive_error != ERR_NONE){
				return ERR_IO_ERROR;
			}
			
			/* is the number of bytes requested less than that remaining in this sector? */
			if (rem_bytes < n_bytes){
				xfer_bytes = rem_bytes;
			} else {
				xfer_bytes = n_bytes;
			}
			
			/* copy the requested number of bytes to the users output buffer 
			and increment f_buf*/
			buffer_os = fptr_sector_pos(fptr);
			memcpy(f_buf, sector_buffer + buffer_os, xfer_bytes);
			os = xfer_bytes;
			
			/* increment sector position counter */
			int16_to_int32(rem_bytes_32, rem_bytes);
			add_int32(fptr_sector_pos(fptr), fptr_sector_pos(fptr), xfer_bytes);
			
			/* increment file position counter */
			add_int32(fptr_file_pos(fptr), fptr_file_pos(fptr), xfer_bytes);
			
			/* decrement the number of bytes we need to read now */
			n_bytes = n_bytes - xfer_bytes;
		} 
		
		/* We didn't do a partial read last time, or we've just read the remaining bytes 
		from that sector */
		
		/* have we got any more bytes to read? */
		if (n_bytes > 0){
			/* Yes, so loop until we have nothing left to read */
			while (n_bytes > 0){
				/* any sectors left in current cluster? */
				if (fptr_get_next_sector(fptr) == 0){
					/* Ok, is n_bytes > fs_sector_size */
					if (n_bytes > fs_sector_size){	
						/* yes, so read a sector, copy it and loop again */
						everdrive_error = disk_read_single_sector(int32_to_int16_lsb(fptr_sector_num(fptr)), int32_to_int16_msb(fptr_sector_num(fptr)), sector_buffer);
						if (everdrive_error != ERR_NONE){
							return ERR_IO_ERROR;
						}
						/* copy data to user buffer */
						memcpy(f_buf + os, sector_buffer, fs_sector_size);
						/* increment offset for next loop pass */
						os += fs_sector_size;
						/* decrement number of bytes still needing to be copied */
						n_bytes = n_bytes - fs_sector_size;
						
						/* update sector position counter - i.e. pos 512 of 512 bytes */
						add_int32(fptr_sector_pos(fptr), fptr_sector_pos(fptr), xfer_bytes);
						/* increment file position counter - i.e. pos 128000 of 768000 bytes */
						add_int32(fptr_file_pos(fptr), fptr_file_pos(fptr), xfer_bytes);
			
					} else {
						/* No, read n_bytes only */
						everdrive_error = disk_read_single_sector(int32_to_int16_lsb(fptr_sector_num(fptr)), int32_to_int16_msb(fptr_sector_num(fptr)), sector_buffer);
						if (everdrive_error != ERR_NONE){
							return ERR_IO_ERROR;
						}
						memcpy(f_buf + os, sector_buffer, n_bytes);
						n_bytes = 0;
						
						/* update sector position counter - i.e. pos 512 of 512 bytes */
						add_int32(fptr_sector_pos(fptr), fptr_sector_pos(fptr), xfer_bytes);
						/* increment file position counter - i.e. pos 128000 of 768000 bytes */
						add_int32(fptr_file_pos(fptr), fptr_file_pos(fptr), xfer_bytes);
						
					}
				} else {
					/* Cannot move to next sector - end of file
					TO DO !!!
					*/
				}
				/* TO DO !!!
				everdrive_error = disk_read_single_sector(int32_to_int16_lsb(fptr_sector_num(fptr)), int32_to_int16_msb(fptr_sector_num(fptr)), sector_buffer);
				if (everdrive_error != ERR_NONE){
					return ERR_IO_ERROR;
				}*/	
			}
			return n_bytes;
		} else {
			/* no, already read all requested bytes */
			return n_bytes;	
		}		
	}
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
			char, fptr 		- The number of an open file pounter, as returned by fopen().
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
			char, fptr 	- The number of an open file pointer, as returned by fopen().
		
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
			char, fptr 	- The number of an open file pointer, as returned by fopen().
		
		Returns: 
			0 on success
			Non-zero error code on failure
	*/	 
	
	return 0;
}
