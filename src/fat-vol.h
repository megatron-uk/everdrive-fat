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
* fat-vol.h
* =========
* Functions for reading FAT filesystem geometry 
* data from the volume sector of the currently
* selected FAT partition.
* 
* John Snowdon (john@target-earth.net), 2014
* 
*/
 
getFATVol()  
{
	/* read the volume entry of a fat32 filesystem, 
	given an existing partition entry structure */ 
	
	/*
		ed_buffer
		part_entry
		volume_entry 
	*/
	
	/* return code from the asm functions */
	/*char	addr_low[2], addr_hi[2];*/
	int	addr_low, addr_hi;
	char 	everdrive_error;
	
	/* 
		call the asm functions to open the disk and set the read address
		the asm function takes two 16bit values - low and high bits of the 32bit sector address
	*/
	
	addr_low = int32_to_int16_lsb(part_lba_begin);
	addr_hi = int32_to_int16_msb(part_lba_begin);
	
	#ifdef FATDEBUG
	if (lba_addressing) {
		put_string("LBA", 26, INFO_LINE_START + 1);
	} else {
		put_string("CHS", 26, INFO_LINE_START + 1);
	}
	put_string("Part Start", 1, INFO_LINE_START + 1);
	put_hex_count(part_lba_begin, 4, 12, INFO_LINE_START + 1);
	#endif	
	
	#ifdef FATDEBUG
	put_string("disk_read", 1, INFO_LINE_START + 2);
	put_hex(addr_hi, 4, 12, INFO_LINE_START + 2);
	put_hex(addr_low, 4, 17, INFO_LINE_START + 2);
	#endif
	everdrive_error = disk_read_single_sector(addr_low, addr_hi, sector_buffer);
	if (everdrive_error != ERR_NONE) { 
		/* disk read error */
		#ifdef FATDEBUG
		put_string("Error", 26, INFO_LINE_START + 2);
		put_number(everdrive_error, 3, 22, INFO_LINE_START + 2);
		#endif
		return everdrive_error;
	} else {
		/* 
			ed_buffer should now contain sector 0 - the volume sector
		*/
		#ifdef FATDEBUG
		put_string("Ok", 26, INFO_LINE_START + 2);
		put_number(everdrive_error, 3, 22, INFO_LINE_START + 2);
		put_string("First 32bytes of volume sector", 1, MAX_LINES - 4);
		put_hex_count(sector_buffer, 16, 0, MAX_LINES -3);
		put_hex_count(sector_buffer + 16, 16, 0, MAX_LINES -2);
		#endif
		return ERR_NONE;
	}
}

getFSFATCount(sector_buffer)
char*	sector_buffer;
{
	/* 
		Reads the number of FAT tables for this filesystem
	*/
	
	fs_num_fats = sector_buffer[FAT_NumFATs_os];
	return ERR_NONE;
}
	
getFSReservedSectors()
{

	/* 
		Retrieves the number of reserved sectors before the start of the actual filesystem.
		Read it in reverse as FAT stores it in little-endian.
	*/
	
	char	i;	
	fs_reserved_sectors = 0;
	for (i=FAT_RsvdSecCnt_sz; i>0; i--) {
		fs_reserved_sectors = (fs_reserved_sectors << 8) + sector_buffer[(FAT_RsvdSecCnt_os + i - 1)];
	}
	return ERR_NONE;
}

getFSSectorsPerFAT(sector_buffer, fs_sectors_per_fat)
char*	sector_buffer;
char*	fs_sectors_per_fat;
{

	/* 
		Get the number of sectors reserved to each FAT table in this volume.
		fs_volume_entry			- An existing 16byte record from the call to getFATVolume().
		fs_sectors_per_fat			- The detected number of sectors allocated to each FAT table (4byte packed 32bit value).
	*/
	
	memcpy(fs_sectors_per_fat, sector_buffer + FAT_FATSz32_os, FAT_FATSz32_sz);
	swap_int32(fs_sectors_per_fat);
	return ERR_NONE;
	
}

getFSFATBegin()
{
	/*
		Extract the first sector of the FAT table from the volume sector of a partition.
		
		fs_fat_lba_begin		- the calculated 4byte packed 32bit value
		fs_volume_entry		- an existing 16 byte record from a call to getFATVolume().
		part_entry			- an existing 16 byte record of this partition from the call to getMBRPart().
	*/
	
	char	tmp1[4];
	
	zero_int32(tmp1);
	int16_to_int32(tmp1, fs_reserved_sectors);
	add_int32(fs_fat_lba_begin, part_lba_begin, tmp1);
	
	return ERR_NONE;

}

getFSClusterBegin()
{
	/*
		Extract the first sector of the first data cluster	the volume sector of a partition.
		
		volume_entry			- an existing 16 byte record from a call to getFATVolume().
		cluster_lba_begin		- a 4 byte packed 32bit value used to store the the starting LBA sector of the first data cluster for this partition.
		partition_lba_begin	- a 4 byte packed 32bit value used to store the the starting LBA sector of the this partition.
		num_fats				- the number of FAT tables for this volume, obtained from getFATCount(). [1byte char]
		sectors_per_fat		- number of sectors allocated for each FAT table, obtained from getFATSectors() [4byte packed 32bit number].
		reserved_sectors		- number of reserved sectors after the end of the FAT table and before data cluster begin, obtained from getFATReservedSectors() [2byte integer].
	*/
	
	char 	error;
	char 	tmp[4];
	
	/* 
		cluster_lba_begin = Partition_LBA_Begin + Number_of_Reserved_Sectors + (num_fats * sectors_per_fat);
	*/
	
	error = 0;
	zero_int32(fs_cluster_lba_begin);
	zero_int32(tmp);
	
	/* Number of sectors allocated to the FAT tables */
	error = mul_int32_int8(tmp, fs_sectors_per_fat, fs_num_fats);
	if (error == 1) {
		/* overflow */
		return error;
	}
	
	/* This sector is the start of the data (root directory) after all the sectors calculated above */
	error = add_int32(fs_cluster_lba_begin, fs_fat_lba_begin, tmp);
	if (error == 1) {
		/* overflow */
		return error;
	}
	return ERR_NONE;
	
}

getFSSectorSize()
{
	/*
		Retrieve the size (a 16bit integer) of a single filesystem sector.
		Read it in reverse as FAT stores it in little-endian format.
	*/
	
	char	i;	
	fs_sector_size = 0;
	for (i=FAT_BytsPerSec_sz; i>0; i--) {
		fs_sector_size = (fs_sector_size << 8) + sector_buffer[(FAT_BytsPerSec_os + i - 1)];
	}
	if (fs_sector_size == 0) {
		return ERR_NO_SECT_SIZE_INFO;
	} 
	return ERR_NONE;
}

getFSSectorClusterSize()
{
	/*
		Retrieve the number of sectors (a 8bit integer) to a filesystem cluster for this filesystem.	
	*/
	
	fs_sectors_per_cluster = sector_buffer[FAT_SecPerClus_os];
	if (int32_is_zero(fs_sectors_per_cluster)) {
		return ERR_NO_SECT_SIZE_INFO;		
	}
	return ERR_NONE;
}

getFATSignature(sector_buffer)
char*	sector_buffer;
{
	/*
		Read the filesystem signature from the end of the volume sector.
		This is just another verification that we're reading a FAT filesystem.
	*/	
	
	memcpy(fs_fat_sig, sector_buffer + FAT_Sig_os, FAT_Sig_sz);
	if ((fs_fat_sig[1] == FAT_Sig_byte_1) & (fs_fat_sig[0] ==  FAT_Sig_byte_2)) {
		return ERR_NONE;
	} else {
		return ERR_MISS_FAT_SIG;
	}
}

getFSRootCluster(sector_buffer)
char*	sector_buffer;
{
	/*
		Read the cluster number of the root directory.
	*/
	
	memcpy(fs_root_dir_cluster, sector_buffer + FAT_RootClus_os, FAT_RootClus_sz);
	swap_int32(fs_root_dir_cluster);
	if (int32_is_zero(fs_root_dir_cluster)) {
		return ERR_MISS_ROOT_CLUSTER;		
	}
	return ERR_NONE;
}

getFATFS()
{
	/* 
		Read filesystem parameters from an already read partition entry and volume sector 
		part_entry			- Already read 16 byte partition record from the call to getPartBegin()
		fs_volume_entry		- Already read 16 byte filesystem sector 0 record from the call to getFATVolume()
		
		Set the following global variables which are needed to read directories and files from a fat filesystem:
		
		fs_fat_lba_begin
		fs_cluster_lba_begin
		fs_num_fats
		fs_reserved_sectors
		fs_sectors_per_fat
	*/

	char	error;
	
	#ifdef FATDEBUG
	if (lba_addressing) {
		put_string("LBA", 26, INFO_LINE_START + 1);
	} else {
		put_string("CHS", 26, INFO_LINE_START + 1);
	}
	put_string("Part Start", 1, INFO_LINE_START + 1);
	put_string("h", 20, INFO_LINE_START + 1);
	put_hex_count(part_lba_begin, 4, 12, INFO_LINE_START + 1);
	#endif	
	
	/* Get the bytes per sector of this FAT filesystem */
	error = 0;
	error = getFSSectorSize();
	#ifdef FATDEBUG
	put_string("Sect Size", 1, INFO_LINE_START + 2);
	#endif
	if (error != ERR_NONE){
		#ifdef FATDEBUG
		put_number(error, 3, 22, INFO_LINE_START + 2);
		put_number(fs_sector_size, 5, 15, INFO_LINE_START + 2);
		put_string("Error", 26, INFO_LINE_START + 2);
		#endif
		return error;
	} else {
		#ifdef FATDEBUG
		put_number(error, 3, 22, INFO_LINE_START + 2);
		put_number(fs_sector_size, 5, 15, INFO_LINE_START + 2);
		put_string("OK", 26, INFO_LINE_START + 2);
		#endif
	}
	
	/* Get the number of sectors per cluster of this FAT filesystem */
	error = 0;
	error = getFSSectorClusterSize();
	#ifdef FATDEBUG
	put_string("Sects/Clus", 1, INFO_LINE_START + 3);
	#endif
	if (error != ERR_NONE){
		#ifdef FATDEBUG
		put_number(error, 3, 22, INFO_LINE_START + 3);
		put_number(fs_sectors_per_cluster, 3, 17, INFO_LINE_START + 3);
		put_string("Error", 26, INFO_LINE_START + 3);
		#endif
		return error;
	} else {
		#ifdef FATDEBUG
		put_number(error, 3, 22, INFO_LINE_START + 3);
		put_number(fs_sectors_per_cluster, 3, 17, INFO_LINE_START + 3);
		put_string("OK", 26, INFO_LINE_START + 3);
		#endif
	}
	
	/* Get the number of sectors for each FAT for this partition */
	error = 0;
	error = getFSSectorsPerFAT(sector_buffer, fs_sectors_per_fat);
	#ifdef FATDEBUG
	put_string("Sects/FAT", 1, INFO_LINE_START + 4);
	put_string("h", 20, INFO_LINE_START + 4);
	#endif
	if (error != ERR_NONE){
		#ifdef FATDEBUG
		put_number(error, 3, 22, INFO_LINE_START + 4);
		put_hex_count(fs_sectors_per_fat, 4, 12, INFO_LINE_START + 4);
		put_string("Error", 26, INFO_LINE_START + 4);
		#endif
		return error;
	} else {
		#ifdef FATDEBUG
		put_number(error, 3, 22, INFO_LINE_START + 4);
		put_hex_count(fs_sectors_per_fat, 4, 12, INFO_LINE_START + 4);
		put_string("OK", 26, INFO_LINE_START + 4);
		#endif
	}
	
	
	/* Get the number of reserved sectors after the FAT tables */
	error = 0;
	error = getFSReservedSectors();
	#ifdef FATDEBUG
	put_string("Res Sects", 1, INFO_LINE_START + 5);
	#endif
	if (error != ERR_NONE){
		#ifdef FATDEBUG
		put_number(error, 3, 22, INFO_LINE_START + 5);
		put_number(fs_reserved_sectors, 5, 15, INFO_LINE_START + 5);
		put_string("Error", 26, INFO_LINE_START + 5);
		#endif
		return error;
	} else {
		#ifdef FATDEBUG
		put_number(error, 3, 22, INFO_LINE_START + 5);
		put_number(fs_reserved_sectors, 5, 15, INFO_LINE_START + 5);
		put_string("OK", 26, INFO_LINE_START + 5);
		#endif
	}
	
	/* Get the number of FAT table for this volume (should ALWAYS be == 0x02) */
	error = 0;
	error = getFSFATCount(sector_buffer);
	#ifdef FATDEBUG
	put_string("FAT Count", 1, INFO_LINE_START + 6);
	#endif
	if (error != ERR_NONE){
		#ifdef FATDEBUG
		put_number(error, 3, 22, INFO_LINE_START + 6);
		put_number(fs_num_fats, 1, 19, INFO_LINE_START + 6);
		put_string("Error", 26, INFO_LINE_START + 6);
		#endif
		return error;
	} else {
		#ifdef FATDEBUG
		put_number(error, 3, 22, INFO_LINE_START + 6);
		put_number(fs_num_fats, 1, 19, INFO_LINE_START + 6);
		put_string("OK", 26, INFO_LINE_START + 6);
		#endif
	}
	
	/* Get the start sector of the FAT table */
	error = 0;
	error = getFSFATBegin();
	#ifdef FATDEBUG
	put_string("FS Start", 1, INFO_LINE_START + 7);
	put_string("h", 20, INFO_LINE_START + 7);
	#endif
	if (error != ERR_NONE){
		#ifdef FATDEBUG
		put_number(error, 3, 22, INFO_LINE_START + 7);
		put_hex_count(fs_fat_lba_begin, 4, 12, INFO_LINE_START + 7);
		put_string("Error", 26, INFO_LINE_START + 7);
		#endif
		return error;
	} else {
		#ifdef FATDEBUG
		put_number(error, 3, 22, INFO_LINE_START + 7);
		put_hex_count(fs_fat_lba_begin, 4, 12, INFO_LINE_START + 7);
		put_string("OK", 26, INFO_LINE_START + 7);
		#endif
	}
	
	/* Get the start sector of the first data cluster for this partition */
	error = 0;
	error = getFSClusterBegin();
	#ifdef FATDEBUG
	put_string("Clus Start", 1, INFO_LINE_START + 8);
	put_string("h", 20, INFO_LINE_START + 8);
	#endif
	if (error != ERR_NONE){
		#ifdef FATDEBUG
		put_number(error, 3, 22, INFO_LINE_START + 8);
		put_hex_count(fs_cluster_lba_begin, 4, 12, INFO_LINE_START + 8);
		put_string("Error", 26, INFO_LINE_START + 8);
		#endif
		return error;
	} else {
		#ifdef FATDEBUG
		put_number(error, 3, 22, INFO_LINE_START + 8);
		put_hex_count(fs_cluster_lba_begin, 4, 12, INFO_LINE_START + 8);
		put_string("OK", 26, INFO_LINE_START + 8);
		#endif
	}
	
	/* Set the cluster number of the root directory */
	error = 0;
	error = getFSRootCluster(sector_buffer);
	#ifdef FATDEBUG
	put_string("Root Clus", 1, (INFO_LINE_START + 9));
	put_string("h", 20, INFO_LINE_START + 9);
	#endif
	if (error != ERR_NONE){
		#ifdef FATDEBUG
		put_number(error, 3, 22, (INFO_LINE_START + 9));
		put_hex_count(fs_root_dir_cluster, 4, 12, (INFO_LINE_START + 9));
		put_string("Error", 26, (INFO_LINE_START + 9));
		#endif
		return error;
	} else {
		#ifdef FATDEBUG
		put_number(error, 3, 22, (INFO_LINE_START + 9));
		put_hex_count(fs_root_dir_cluster, 4, 12, (INFO_LINE_START + 9));
		put_string("OK", 26, (INFO_LINE_START + 9));
		#endif
	}
	
	/* Finally check if the FAT32 filesystem signature is present */
	error = 0;
	error = getFATSignature(sector_buffer);
	#ifdef FATDEBUG
	put_string("FAT Sig", 1, (INFO_LINE_START + 10));
	#endif
	if (error != ERR_NONE){
		#ifdef FATDEBUG
		put_number(error, 3, 22, (INFO_LINE_START + 10));
		put_hex_count(fs_fat_sig, 2, 16, (INFO_LINE_START + 10));
		put_string("Error", 26, (INFO_LINE_START + 10));
		#endif
		return error;
	} else {
		#ifdef FATDEBUG
		put_number(error, 3, 22, (INFO_LINE_START + 10));
		put_hex_count(fs_fat_sig, 2, 16, (INFO_LINE_START + 10));
		put_string("OK", 26, (INFO_LINE_START + 10));
		#endif
	}
	return ERR_NONE;	
}
