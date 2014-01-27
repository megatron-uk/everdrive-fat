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
* Detects inserted card type,
* Reads first detected FAT partition,
* Displays FAT filesystem geometry.
*  
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

prompt(current)
char current;
{
	char j;
	for (;;) {
		vsync();
		j = joytrg(0);
		if (j & JOY_DOWN) {
			if ((current + 1) < 4){
				info_area();
				return (current + 1);
			}
		}
		if (j & JOY_UP) {
			if ((current - 1) >= 0){
				put_string("                               ", 1, current + 1);
				info_area();
				return (current - 1);
			}
		}
	}
}

info_area()
{
	char i;
	
	put_string("[DEBUG]", 0, INFO_LINE_START);
	for (i=1 ; i<17 ; i++) {
		put_string("                                ", 0, INFO_LINE_START + i);
	}
	put_string("[D/U] Next/Previous", 0, MAX_LINES);
	
}

do_init()
{
	char sd_type;
	/* initialise disk interface */
	put_string("disk_init", 1, 1);
	sd_type = 0;
	everdrive_error = 0;
	everdrive_error = disk_init();
	if (everdrive_error != ERR_NONE) {
		put_string("No Card", 12, 1);
		put_string("Error", 26, 1);
		put_number(everdrive_error, 3, 22, 1);
	} else {
		sd_type = disk_get_cardtype();
		switch(sd_type){
			case 3:
				put_string("SD v2", 12, 1);
				break;
			case 2:
				put_string("SD HC", 12, 1);
				break;
			default:
				put_string("SD/MMC ???", 12, 1);
				break;
		}
		put_hex(sd_type, 2, 23, 1);
		put_string("OK", 26, 1);
	}
}

do_mbr()
{
	/* read and print partition info - part_entry, part_type, part_number, part_lba_begin */
	put_string("getMBR", 1, 2);
	everdrive_error = 0;
	everdrive_error = getMBR(0);
	if (everdrive_error != ERR_NONE) {
		put_string("No MBR", 12, 2);
		put_string("Error", 26, 2);
		put_number(everdrive_error, 3, 22, 2);
	} else {
		put_string("Found P", 12, 2);
		put_number(part_number, 1, 19, 2);
		put_string("OK", 26, 2);
		put_hex(part_type, 2, 23, 2);
	}
}

do_fatvol()
{
	/* read volume sector of the detected FAT partition - fs_volume_entry*/
	put_string("getFATVol", 1, 3);
	everdrive_error = 0;
	everdrive_error = getFATVol();
	if (everdrive_error != ERR_NONE) {
		put_string("No Vol", 12, 3);
		put_string("Error", 26, 3);
		put_number(everdrive_error, 3, 22, 3);
	} else {
		put_string("Found", 12, 3);
		put_string("OK", 26, 3);
		put_number(everdrive_error, 3, 22, 3);	
	}
}

do_fs()
{
	/* Get information about the filesystem itself using the read fs_volume_entry and part_entry record */
	put_string("getFATFS", 1, 4);
	everdrive_error = 0;
	everdrive_error = getFATFS();
	if (everdrive_error != ERR_NONE) {
		put_string("No FS", 12, 4);
		put_string("Error", 26, 4);
		put_number(everdrive_error, 5, 20, 4);
	} else {
		put_string("Found", 12, 4);
		put_string("OK", 26, 4);
		put_number(everdrive_error, 5, 20, 4);	
	}
}	

main() {
	char	sel;
	
	init_screen();
	put_string("[Everdrive FAT Test]", 0, 0);
	info_area();
	prompt();
	
	sel = 0;
	
	/* enable everdrive card */
	clearFATBuffers();
	ed_begin();
	for (;;){
	
		if (sel == 0) {
			do_init();
			sel = prompt(0);
		}
		
		if (sel == 1) {
			do_mbr();
			sel = prompt(1);
		}
		
		if (sel == 2) {
			do_fatvol();
			sel = prompt(2);
		}
		
		if (sel == 3) {
			do_fs();
			sel = prompt(3);
		}		
	}

	/* disable ed */
	ed_end();
	put_string("End of test                ", 1, 26);
}
