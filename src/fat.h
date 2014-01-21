/* ====================================
 	FAT32 device, filesystem and file function support for
 	the Turbo Everdrive flash card.
 
 	HuC functions below written by John Snowdon (john@target-earth.net), 2014
 	
 	Uses sd.h and sd.asm functions developed by Mooz:
 		https://gist.github.com/BlockoS
 		(Vincent.Cruz@gmail.com, 2012-2014)
 
==================================== */

/* 
	Define FALSE_PART if we're testing, don't have a valid partition or volume
	sector and want to test the partition and fat volume geometry code.
	
	Sample partition record and volume record code is set in fat-misc.h
*/

/* ==================================== 

	FAT Filesystem documentation came from:
	http://www.pjrc.com/tech/8051/ide/fat32.html

	Access to a FAT filesystem is as follows 
	----
	Access MBR for list of partitions
	Access Partition entries for filesystem location on disk
	Access sector 0 of a FAT filesystem for volume information
	Access FAT table for file/dir locations

	MBR -> Partition -> Volume sector -> FAT -> Files
*/

/* number of text lines in low res mode */
#define MAX_LINES		27
#define INFO_LINE_START		10	
#define SECTOR_SIZE		512
#define ERR_NO_PART_ENTRY	146
#define ERR_MISS_MBR_CKSUM 	147
#define ERR_MISS_FAT_SIG 	148
#define ERR_MISS_ROOT_CLUSTER	149
#define ERR_NO_RES_SECTORS	150
#define ERR_NO_FAT_COUNT	151
#define ERR_NO_SECT_SIZE_INFO	152

/* ==================================== 
	information about the MBR of a device
	
	offset of the MBR
	size of each partition entry in bytes
	offsets of the 4 primary partitions
	offsets of the 2 check bytes 
	values of the 2 check bytes 
*/

#define MBR_Part_os		0x0000
#define MBR_Part_sz		16
#define MBR_Part_1_os		0x01BE
#define MBR_Part_2_os		0x01CE
#define MBR_Part_3_os		0x01DE
#define MBR_Part_4_os		0x01EE
#define MBR_CheckByte_1_os	0x01FE
#define MBR_CheckByte_2_os	0x01FF
#define MBR_CheckByte_1		0x55
#define MBR_CheckByte_2		0xAA

/* ==================================== 
	information about a partition entry
	
	offset of the field
	size of the field in bytes
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


/* ==================================== 
	 FAT partition types
	
	This determines what kind of FAT/file lookups we can do.
	We'll only support FAT32 as we don't want to do both CHS
	and LBA lookups.
*/

#define TypeCode_FAT32		0x0B
#define TypeCode_FAT32_LBA	0x0C
#define TypeCode_FAT16		0x06
#define TypeCode_FAT16_32MB	0x04
#define TypeCode_FAT16_LBA	0x0E

/* ==================================== 
	 information about sector 0 of a FAT12/16/32 filesystem 
	
	The offset from the start of the partition entry
	The size in bytes of the field
 */

/* Bytes Per Sector	
16 Bits	Always 512 Bytes */
#define FAT_BytsPerSec_os	0x0B
#define FAT_BytsPerSec_sz	2
/* Sectors Per Cluster 
8 Bits	1,2,4,8,16,32,64,128 */
#define FAT_SecPerClus_os	0x0D
#define FAT_SecPerClus_sz	1
/* Number of Reserved Sectors	
16 Bits	Usually 0x20 */
#define FAT_RsvdSecCnt_os	0x0E
#define FAT_RsvdSecCnt_sz	2
/* Number of FATs	
8 Bits	Always 2 */
#define FAT_NumFATs_os		0x10
#define FAT_NumFATs_sz		1
/* Sectors Per FAT	
32 Bits	Depends on disk size */
#define FAT_FATSz32_os		0x24
#define FAT_FATSz32_sz		4
/* Root Directory First Cluster	
32 Bits	Usually 0x00000002 */
#define FAT_RootClus_os		0x2C
#define FAT_RootClus_sz		4
/* Signature
16 Bits	Always 0xAA55 */
#define FAT_Sig_os			0x1FE
#define FAT_Sig_sz			2
#define FAT_Sig_byte_1		0xAA
#define FAT_Sig_byte_2		0x55
#define FAT_Sig				0xAA55

/*=========================================
	Global variable available from all fat functions and within asm.
	
	They also have their own "space" in RAM (bss). 
	So it's better to have big arrays as global variables.
	These global variables are used by virtually all fat.h functions
	with some also in use by sd.h / sd.asm functions too.
*/

/* read buffer */
char 	sector_buffer[512];
char	everdrive_error;
char	lba_addressing;

/* partition entry for the selected/detected partition */
char	part_entry[16];
char 	part_type;
char 	part_number;
char	part_lba_begin[4];

/* the volume entry for the selected partition */
char	fs_fat_lba_begin[4];
char	fs_fat_sig[2];
char	fs_cluster_lba_begin[4];
char	fs_num_fats;
int	fs_reserved_sectors;
int	fs_sector_size;
char	fs_sectors_per_cluster;
char	fs_sectors_per_fat[4];
char	fs_root_dir_cluster[4];

/* Total global work size == 544 bytes */

/* ===================================== */

/* low level Everdrive SD interface functions - by MooZ */
#include "sd.h"

/* emulated '32bit' integer math functions - sector calculation etc */
#include "math32.h"

/* little-endian to big-endian conversion functions */
#include "endian.h"

/* MBR and partition detection routines */
#include "fat-dev.h"

/* FAT volume data retrieval routines */
#include "fat-vol.h"

/* stdio-like FAT filesytem functions */
#include "fat-files.h"

/* Misc helpers and test routines */
#include "fat-misc.h"
