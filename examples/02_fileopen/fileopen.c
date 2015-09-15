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
* Implements a basic test using Turbo Everdrive FAT library.
*  
* Tests opening a named file from the SD card.
* John Snowdon (john@target-earth.net), 2014
*/

#include "huc.h"
#include "fat/fat.h"

init_screen(){
	/* setup fonts/screen */
	set_color_rgb(1, 7, 7, 7);
	set_font_color(1, 0);
	set_font_pal(0);
	load_default_font();
}

init_fat(){
	/* enable everdrive card */
	clearFATBuffers();
	ed_begin();
	put_string("Initialise", 0, 1);
	everdrive_error = disk_init();
	put_number(everdrive_error, 3, 0, 2);
	everdrive_error = getMBR(0);
	put_number(everdrive_error, 3, 4, 2);
	everdrive_error = getFATVol();
	put_number(everdrive_error, 3, 8, 2);
	everdrive_error = getFATFS();
	put_number(everdrive_error, 3, 12, 2);
	/* NOTE: You should normally check that each call to the above 4 functions
	has returned correctly before proceeding! */
}

prompt_to_continue(row)
char	row;
{
	char j;
	put_string("Press [D] to continue         ", 0, row);
	for (;;) {
		vsync();
		j = joytrg(0);
		if (j & JOY_DOWN) {
			put_string("                               ", 0, row);
			break;
		}
	}
	
}

show_fptr(fptr)
char	fptr;
{
	/*
		Print all the current details known about an
		open file pointer.
	*/
	
	char r;
	put_string("[Everdrive FAT File Info]", 0, 0);
	
	for (r=1; r<=MAX_LINES; r++){
		put_string("                                ", 0, r);
	}
	
	
	put_string("Filename   :", 0, 1);
	put_string(fwa + (fptr * FILE_WORK_SIZE) + FILE_DIR_os + DIR_Name_os, 13, 1);
	
	put_string("Cluster    : 0x", 0, 2);
	put_hex_count(fwa + (fptr * FILE_WORK_SIZE) + FILE_Cur_Cluster_os, 4, 15, 2);
	
	put_string("Sector     : 0x", 0, 3);
	put_hex_count(fwa + (fptr * FILE_WORK_SIZE) + FILE_Cur_Sector_LBA_os, 4, 15, 3);
	
	put_string("Size       : 0x", 0, 4);
	put_hex_count(fwa + (fptr * FILE_WORK_SIZE) + FILE_DIR_os + DIR_FileSize_os, 4, 15, 4);
	
	put_string("Attrib     : 0x", 0, 5);
	put_hex(*fwa + (fptr * FILE_WORK_SIZE) + FILE_DIR_os + DIR_Attr_os, 2, 15, 5);
	
	prompt_to_continue(MAX_LINES);
}

show_file(fptr)
char	fptr;
{
	/*
		Read an open file.
	*/
	
	char c;
	char row, column;
	char r;
	put_string("[Everdrive FAT File Read]", 0, 0);
	
	for (r=1; r<=MAX_LINES; r++){
		put_string("                                ", 0, r);
	}
	
	
	fread(fptr, SECTOR_SIZE);
	c = 0;
	put_string(sector_buffer, 0, 1);
	prompt_to_continue(MAX_LINES);
}


main() {
	char	fh;
	char	fname[64];
	fh = 0;
	strcpy(fname, "/text/dracula.txt");
	
	put_string("[Everdrive FAT File Test]", 0, 0);

	init_screen();	
	init_fat();	
	
	put_string("Filename:", 0, 3);
	put_string(fname, 10, 3);
	put_string("Open file: ", 0, 4);
	fh = fopen(fname);
	if (fh != 0){
		put_string("opened ", 11, 4);
		put_number(fh, 3, 18, 4);
		
		/* prompt to continue */
		prompt_to_continue(MAX_LINES);
		
		/* display file handle details */
		show_fptr(fh);
		
		/* display first n bytes of file */
		show_file(fh);
		
		fclose(fh);
	} else {
		put_string("error", 11, 4);	
		put_number(everdrive_error, 3, 17, 4);
	}
	/* disable ed */
	ed_end();
}
