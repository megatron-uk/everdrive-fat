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
* Tests reading data from a file from the SD card.
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
	init_screen();
	put_string("[Everdrive TextReader Test]", 0, 0);
		
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
	
	/* disable ed */
	ed_end();
}
