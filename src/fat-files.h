/* ===============================

	fat-files.h
	======
	A 'stdio-like' library for file handling functions,
	specifically for the Turbo Everdrive FAT library.

	John Snowdon (John.Snowdon@newcastle.ac.uk), 2014

 =============================== */
 
#define EOF				255 
 
#define FOPEN_MAX		2
#define FILENAME_MAX	255

#define SEEK_SET 		0
#define SEEK_CUR		1
#define SEEK_END 		2

 
/* ===============================
Open/Close file descriptors
=============================== */

fopen(f_path)
char*	f_path;
{
	/* 
		Open a new file and return a file pointer
		
		Input:
			char*, f_path		- Pointer to a memory location representing the path to a file.
							Both MS-DOS ("\") and Unix style ("/") directory access is supported - e.g.
								fopen("/homebrew/utils/data/file.txt")
								fopen("\games\save1.dat")
							Access is relative to the root directory of the current open FAT partition. 
							Device names are not supported.
		
		Returns: 
			char, fptr 	- Number of the open file pointer on success
			0 on failure
	*/
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
	 */
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
