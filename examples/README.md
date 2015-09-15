examples
========
 
Contents

* settings.ini - Defines the path to HuC compiler and the PCEAS assembler. Only needed if you're on a unix-type system and you don't have either in your path.

* README.md - This file.

01_detect
=========
A simple PC-Engine utility that attempts to detect an SD card through a Turbo-Everdrive cartridge. If the SD card is detected, then it tries to detect basic card, partition and filesystem parameters. If these routines don't work, then none of the later example tools will either!

Tests the following libraries and functions:

fat/sd.h
* disk_init() - Initialise the IO ports on the Turbo Everdrive card, ready for sending commands.
* disk_get_cardtype() - Detect the type of the inserted SD/SDHC card.

fat/fat-dev.h
* getMBR() - Read the master boot record and detect/set a DOS/FAT partition. If no parameters are given, returns and sets (in global data structures used by all other fat functions) details of the first FAT partition detected.

fat/fat-vol.h
* getFATVol() - Reads the volume entry of a FAT partition table. This should be sector 0, the starting sector for any FAT filesystem which then allows interrogation of the basic FAT filesystem.
* getFATFS() - Reads initial FAT filesystem details; starting sector, number of reserved sectors etc, necessary for any further FAT directory or file operations.


02_fileopen
===========
A PC-Engine tool that attempts to open a file path (hard coded in fileopen.c for now) using the FAT directory table. If successful, prints basic information about the file using the directory table data.

In addition to all of the basic functions from 01_detect, this also tests the following libraries and function calls:

fat/fat-files.h
* fopen() - Search for a directory entry, matches user-entered filename (hardcoded in fileopen.c for now) and returns a file handle (if any are free!) on success. The number of available file handles (defaults to 2 - one for directory operations, one for user file access) is set in fat.h - each additional file handle requires a further set of duplicate global data structures; approx 50 bytes per file handle.
* fread() - Uses an open file handle to read bytes from a file on the filesystem (currently limited to first sector/512 bytes).
* fclose() - Closes an open file handle and restores all global data structures related to it.


03_benchmark
============
A PC-Engine tool that opens a named file and runs a basic benchmarking test - it reads a number of sectors from the file and records the time taken.

In addition to all the previous function calls in 02_fileopen, also uses some HuC timer functions to estimate the read speed of the Turbo-Everdrive and SD card hardware. This is currently quite a 'braindead' test, as it simply reads the same 512byte sector from a named file each time. The implementation of the 'get next sector' logic should enable this test to be altered to time the reading of an entire file.

04_textreader
=============
A more advanced PC-Engine tool that opens a named text file from the SD card and allows the user to page through it, both forward and back.

NOT CURRENTLY IMPLEMENTED - Will require the 'get next sector' logic in fat/fat-files-extras.h.
