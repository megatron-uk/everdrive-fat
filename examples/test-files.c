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
#include "fat.h"

init_screen(){
	set_color_rgb(1, 7, 7, 7);
	set_font_color(1, 0);
	set_font_pal(0);
	load_default_font();
}

main() {
	char	fh;
	char	fname[64];
	fh = 0;
	strcpy(fname, "/homebrew/ed.pce");
	
	init_screen();
	put_string("[Everdrive FAT File Test]", 0, 0);
		
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
	
	put_string("Filename:", 0, 3);
	put_string(fname, 10, 3);
	put_string("Open file: ", 0, 4);
	fh = fopen(fname);
	if (fh != 0){
		put_string("opened ", 11, 4);
		put_number(fh, 3, 18, 4);
	} else {
		put_string("error", 11, 4);	
		put_number(everdrive_error, 3, 17, 4);
	}
	/* disable ed */
	ed_end();
}
