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
* Speed test of SD card.
* John Snowdon (john@target-earth.net), 2014
*/

#include "huc.h"
#include "fat/fat.h"

init_screen(){
	set_color_rgb(1, 7, 7, 7);
	set_font_color(1, 0);
	set_font_pal(0);
	load_default_font();
}

main() {	
	
	char	error;
	int		c;
	char	fh;
	char	fname[64];
	char	t1[4], t2[4];
	char	buf[512];
	
	char 	num[4], num2[4];
	
	char	i_res[4], i1[4], i2[4];
	
	fh = 0;
	strcpy(fname, "/text/dracula.txt");
	
	init_screen();
	put_string("[Everdrive SD Speed Test]", 0, 0);
		
	/* enable everdrive card */
	clearFATBuffers();
	ed_begin();
	put_string("Initialise:", 0, 1);
	everdrive_error = disk_init();
	put_number(everdrive_error, 3, 12, 1);
	everdrive_error = getMBR(0);
	put_number(everdrive_error, 3, 16, 1);
	everdrive_error = getFATVol();
	put_number(everdrive_error, 3, 20, 1);
	everdrive_error = getFATFS();
	put_number(everdrive_error, 3, 24, 1);
	
	put_string("Filename:", 0, 2);
	put_string(fname, 10, 2);
	
	put_string("Opening:", 0, 3);
	fh = fopen(fname);
	if (fh != 0){
		put_string("OK! fptr", 9, 3);
		put_number(fname, 17, 3);
		
		/* speed test for single sector */
		put_string("Reading 512b :", 0, 4);
		get_timer(t1, 1);
		error = fread(fh, SECTOR_SIZE);
		get_timer(t2, 0);
		put_timer(t2, 0, 0, 5);
		put_hex(sector_buffer[511], 2, 15, 4);
		
		/* speed test of multiple reads */
		put_string("Reading 64kb :", 0, 7);
		get_timer(t1, 1);
		for (c=0; c<128; c++){
			error = fread(fh, SECTOR_SIZE);
		}
		get_timer(t2, 0);
		put_timer(t2, 0, 0, 8);
			
		put_string("Reading 128kb :", 0, 10);
		get_timer(t1, 1);
		for (c=0; c<256; c++){
			error = fread(fh, SECTOR_SIZE);
		}
		get_timer(t2, 0);
		put_timer(t2, 0, 0, 11);
		put_hex(sector_buffer[511], 2, 15, 10);
		
		put_string("Reading 256kb :", 0, 13);
		get_timer(t1, 1);
		for (c=0; c<512; c++){
			error = fread(fh, SECTOR_SIZE);
		}
		get_timer(t2, 0);
		put_timer(t2, 0, 0, 14);
		put_hex(sector_buffer[511], 2, 15, 13);
		
		put_string("Reading 512kb :", 0, 16);
		get_timer(t1, 1);
		for (c=0; c<1024; c++){
			error = fread(fh, SECTOR_SIZE);
		}
		get_timer(t2, 0);
		put_timer(t2, 0, 0, 17);
		put_hex(sector_buffer[511], 2, 15, 16);
		
		fclose(fh);
	} else {
		
		put_string("Error", 9, 3);	
		put_number(everdrive_error, 3, 15, 3);
		
		put_string("Timer test :", 0, 4);
		get_timer(t1, 1);
		for (c=0; c<100; c++){
			fh = 512 / 4;
		}
		get_timer(t2, 0);
		put_timer(t1, 0, 0, 5);
		put_timer(t2, 0, 0, 6);
	}
	
	/* disable ed */
	ed_end();
}

get_timer(time, now)
char*	time;
char	now;
{
	/* Get timer */

	char v;
	if (now == 1) clock_reset();	
	v = clock_tt();
	time[3] = v;
	v = clock_ss();
	time[2] = v;
	v = clock_mm();
	time[1] = v;
	v = clock_hh();
	time[0] = v;
}

put_timer(time, raw, col, row)
char*	time;
char 	raw;
char	col;
char	row;
{
	if (raw == 1){
		put_hex_count(time, 4, col, row);
	} else {
		put_string("   h    m    s    t", col, row);
		put_number(time[0], 3, col, row);
		put_number(time[1], 3, col + 5, row);
		put_number(time[2], 3, col + 10, row);
		put_number(time[3], 3, col + 15, row);
	}
}
