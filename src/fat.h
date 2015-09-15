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
* FAT32 device, filesystem and file function support for
* the Turbo Everdrive flash card.
* 
* All HuC functions (unless stated otherwise) written by John Snowdon (john@target-earth.net), 2014
* 
* Uses sd.h and sd.asm functions developed by Mooz:
* https://gist.github.com/BlockoS
* (Vincent.Cruz@gmail.com, 2012-2014)
*/

/* ================================================================ */

/*
* 
* FAT Filesystem documentation came from:
* http://www.pjrc.com/tech/8051/ide/fat32.html
* 
* Access to a FAT filesystem is as follows 
* ----
* Access MBR for list of partitions
* Access Partition entries for filesystem location on disk
* Access sector 0 of a FAT filesystem for volume information
* Access root directory cluster for folder file entries and their first data cluster address
* Read cluster chains from FAT for subsequent data clusters
* 
* In general:
* MBR -> Partition -> Volume sector -> directory cluster -> Files (-> FAT -> File additional clusters)
*                    
*/

/* ================================================================ */

/* number of text lines in low res mode -
used by some of the test functions/demos */
#define MAX_LINES			27
#define INFO_LINE_START		10

/* FAT defaults */
#define SECTOR_SIZE					512
#define CLUSTER_FAT_ENTRIES_SECT	128
#define CLUSTER_FAT_ENTRY_SIZE		4

/* MBR/FAT/Volume errors */
#define ERR_NO_PART_ENTRY		146
#define ERR_MISS_MBR_CKSUM 		147
#define ERR_MISS_FAT_SIG 		148
#define ERR_MISS_ROOT_CLUSTER	149
#define ERR_NO_RES_SECTORS		150
#define ERR_NO_FAT_COUNT		151
#define ERR_NO_SECT_SIZE_INFO	152

/* File/directory errors */
#define ERR_FILE_NOT_FOUND		153
#define ERR_DIR_NOT_FOUND		154
#define ERR_FS_END				155
#define ERR_END_OF_CHAIN		156
#define ERR_END_OF_DIRECTORY	157
#define ERR_FILENAME_TOO_LONG	158
#define ERR_NO_FREE_FILES		159
#define	ERR_IO_ERROR			199 /* read 'everdrive_error' for actual error code */

/* ================================================================ */

/*
* information about the MBR of a device
* 
* offset of the MBR
* size of each partition entry in bytes
* offsets of the 4 primary partitions
* offsets of the 2 check bytes 
* values of the 2 check bytes 
*/

#define MBR_Part_os			0x0000
#define MBR_Part_sz			16
#define MBR_Part_1_os		0x01BE
#define MBR_Part_2_os		0x01CE
#define MBR_Part_3_os		0x01DE
#define MBR_Part_4_os		0x01EE
#define MBR_CheckByte_1_os	0x01FE
#define MBR_CheckByte_2_os	0x01FF
#define MBR_CheckByte_1		0x55
#define MBR_CheckByte_2		0xAA

/* ================================================================ */

/* information about a partition entry
* 	
* offset of the field
* size of the field in bytes
*/

/* decoding a partition entry */
#define Part_BootFlag_os	0x00
#define Part_BootFlag_sz	0x01
/* For non-LBA devices, the start sector of this filesystem */
#define Part_CHSBegin_os	0x01
#define Part_CHSBegin_sz	0x03
/* The type of the partition - FAT, NTFS, EXT3, UFS etc */
#define Part_TypeCode_os	0x04
#define Part_TypeCode_sz	0x01
/* For non-LBA devices, the end sector of this filesystem */
#define Part_CHSEnd_os		0x05
#define Part_CHSEnd_sz		0x03		
/* for LBA devices, the start sector of the this filesystem */
#define Part_LBABegin_os	0x08
#define Part_LBABegin_sz	0x04
/* not normally used */
#define Part_NumSectors_os	0x0C
#define Part_NumSectors_sz	0x04


/* ================================================================= */

/*
* FAT partition types
* 
* This determines what kind of FAT/file lookups we can do.
* We'll only support FAT32 as we don't want to do both CHS
* and LBA lookups.
*/

#define TypeCode_FAT32		0x0B
#define TypeCode_FAT32_LBA	0x0C
#define TypeCode_FAT16		0x06
#define TypeCode_FAT16_32MB	0x04
#define TypeCode_FAT16_LBA	0x0E

/* ================================================================== */

/*
* information about sector 0 of a FAT12/16/32 filesystem 
*
* The offset from the start of the partition entry
* The size in bytes of the field
*/

#define FAT_BytsPerSec_os	0x0B 	/* Bytes Per Sector - 16 Bits - Always 512 Bytes */
#define FAT_BytsPerSec_sz	2
#define FAT_SecPerClus_os	0x0D 	/* Sectors Per Cluster - 8 Bits - Generally 1,2,4,8,16,32,64,128 */
#define FAT_SecPerClus_sz	1
#define FAT_RsvdSecCnt_os	0x0E 	/* Number of Reserved Sectors - 16 Bits - Usually 0x20 */
#define FAT_RsvdSecCnt_sz	2
#define FAT_NumFATs_os		0x10 	/* Number of FATs - 8 Bits - Always 2 */
#define FAT_NumFATs_sz		1
#define FAT_FATSz32_os		0x24 	/* Sectors Per FAT	- 32 Bits - Depends on disk size */
#define FAT_FATSz32_sz		4
#define FAT_RootClus_os		0x2C 	/* Root Directory First Cluster - 32 Bits - Usually 0x00000002 */
#define FAT_RootClus_sz		4
#define FAT_Sig_os			0x1FE 	/* Signature - 16 Bits - Always 0xAA55 */
#define FAT_Sig_sz			2
#define FAT_Sig_byte_1		0xAA
#define FAT_Sig_byte_2		0x55
#define FAT_Sig				0xAA55

/* ======================================================================= */

/*
* Global variable available from all fat functions and within asm.
* 
* They also have their own "space" in RAM (bss). 
* So it's better to have big arrays as global variables. (MooZ)
* These global variables are used by virtually all fat.h functions
* with some also in use by sd.h / sd.asm functions too.
*/

/* read buffer */
char	sector_buffer_current_fptr;	/* Which open fptr has data in the sector_buffer (as the buffer may need to be flushed when multiple files are open). */
char 	sector_buffer[SECTOR_SIZE];	/* Memory to read each sector in from the Turbo Everdrive SD card. */
char	everdrive_error;			/* Hold error codes from low level everdrive routines. */
char	lba_addressing;				/* Flag to indicate whether LBA addressing (SDHC) or byte addressing (SD) is active. */

/* partition entry for the selected/detected partition */
char	part_entry[16];				/* Holds the partition entry from the master boot record for the current selected partition. */
char 	part_type;					/* The hex code for the partition type of the current selected partition - only FAT32 are allowed */
char 	part_number;				/* The partition number (1-4) of the current selected partition. */
char	part_lba_begin[4];			/* The starting LBA address pf the current selected partition. */

/* the volume entry for the selected partition */
char	fs_fat_lba_begin[4];		/* The starting LBA address of the FAT table for the current filesystem. */
char	fs_fat_sig[2];				/* The detected 0xAA55 byte signature of the current filesystem. */
char	fs_cluster_lba_begin[4];	/* Where the first data cluster is of the current filesystem. */
char	fs_num_fats;				/* The number of FAT tables for the current filesystem (should always be 2). */
int		fs_reserved_sectors;		/* The number of reserved sectors after the FAT tables and before the data clusters. */
int		fs_sector_size;				/* The size of a single sector in this filesystem, in bytes. */
char	fs_sectors_per_cluster;		/* Number of sectors grouped in a single cluster. */
char	fs_sectors_per_fat[4];		/* How many sectors does each FAT table take up. */
char	fs_root_dir_cluster[4];		/* Location of the first cluster of the root directory entry - from here you can scan for sub directories and files. */

/* Total global work size == 545 bytes including the 512 byte sector read buffer */

/* status types for closed/available and  open/used file pointers */
#define FPTR_OPEN_STATUS		0xFF	/* value set in the file pointer array when a fptr is open/in-use */
#define FPTR_CLOSE_STATUS		0x00	/* value set in the file pointer array when a fptr is closed/free */
#define FILE_TYPE_FILE			0x1		/* constant for find_directory_entry when looking for file entry */
#define FILE_TYPE_DIR			0x2		/* constant for find_directory_entry when looking for dir entry */
#define MAX_FILENAME_SIZE		12		/* old DOS 8+3 format (including '.' seperator */

/* ============================================================ */

/* DOS FAT32 Directory entry structure
*
* The definition of the fields held within a 32byte directory
* entry record, obtained when reading the sectors of a directory
* cluster.
*/

/* directory entry structure */
#define DIR_Name_os			0x00	/* In a directory entry, DIR_Name is at byte 0 */
#define DIR_Name_sz			11		/* Old DOS 8+3 naming convention. eg MYSAVES__.TXT */
#define DIR_Name_Ext_os		0x08	/* Just the extension of the file. eg TXT */	
#define DIR_Name_Ext_sz		3
#define DIR_Attr_os			0x0B	/* Attrib byte - file status flags. */
#define DIR_Attr_sz			1
#define DIR_FstClusHI_os 	0x14	/* High 16bits of the starting data cluster for this file. */
#define DIR_FstClusHI_sz 	2
#define DIR_FstClusLO_os 	0x1A	/* Low 16bits of the starting data cluster for this file. */
#define DIR_FstClusLO_sz 	2
#define DIR_FileSize_os 	0x1C
#define DIR_FileSize_sz 	4		/* Size of the file in bytes. */

/* ============================================================= */

/* FAT entry structure
*
* The structure of an entry in the FAT table 
* The FAT is just a big list of 32bit integers, stored as 128
* of these integers per sector (assuming 512byte sectors).
*/

/* FAT entry structure */
#define FAT_Next

/* ============================================================= */

/* Metadata we hold open files
*
* As we don't have struct support, we allocate a region
* of memory for each open file and index into it to set/get
* info relating to the file.
*/

/* metadata held for each open file */
#define FILE_DIR_os	 				0x00	/* 32 bytes to hold the directory entry of the file (which includes start cluster), as described above. */
#define FILE_DIR_sz					32
#define FILE_Total_Clusters_os	 	0x20 	/* 2 bytes to hold the total number of clusters the file occupies (eg 10). */
#define FILE_Total_Clusters_sz		2
#define FILE_Cur_Cluster_Count_os 	0x22 	/* 2 bytes to hold the count of the current cluster we're reading (eg 2 of 10). */
#define FILE_Cur_Cluster_Count_sz	2
#define FILE_Cur_Cluster_os 		0x24 	/* 4 bytes to hold the actual number of the cluster we're reading (eg 0x00000003). */
#define FILE_Cur_Cluster_sz			4
#define FILE_Cur_Sector_Count_os 	0x28	/* 2 bytes to hold the count of the current sector in this cluster (eg 4 of 64). */
#define FILE_Cur_Sector_Count_sz 	2
#define FILE_Cur_Sector_LBA_os 		0x2A 	/* 4 bytes to hold the actual LBA of the sector we're reading (eg 0x00004040). */
#define FILE_Cur_Sector_LBA_sz		4
#define FILE_Cur_PosInFile_os 		0x2E	/* 4 bytes to hold the current position in the overall file (eg 127860 of 10245560 bytes) */
#define FILE_Cur_PosInFile_sz		4
#define FILE_Cur_PosInBuffer_os		0x32	/* 2 bytes to hold the current position in the current read buffer (eg 64 of 512 bytes) */
#define FILE_Cur_PosInBuffer_sz		2

/* ============================================================ */

#define FILE_WORK_SIZE			52	/* 50 bytes total work ram required per file */
#define NUM_OPEN_FILES			2 	/* Set the number of simultaneous open files here and multiply the FILE_WORK_SIZE figure */
									/* to get the total bytes required for the global fwa. A minimum */
									/* of 2 open files are required - 1 is reserved for directory traversal leaving 1 for user files. */
										
char	file_handles[NUM_OPEN_FILES];		/* stores flags to indicate which files are open - file '0' is reserved for directory access. */
char	fwa[104];							/* metadata for all possible open files -
											calculated as FILE_WORK_SIZE x NUM_OPEN_FILES
											maximum allowed size is 32768 bytes */

/* =========================================================== */

/* low level Everdrive SD interface functions - by MooZ */
#include "fat/sd.h"

/* emulated '32bit' integer math functions - sector calculation etc */
#include "fat/math32.h"

/* little-endian to big-endian conversion functions */
#include "fat/endian.h"

/* MBR and partition detection routines */
#include "fat/fat-dev.h"

/* FAT volume data retrieval routines */
#include "fat/fat-vol.h"

/* stdio-like FAT filesytem functions */
#include "fat/fat-files.h"

/* Misc helpers */
#include "fat/fat-misc.h"

/* Additional string printing functions -
DISABLE this include if you are not using any
of the debugging code! */
#include "fat/print.h"
