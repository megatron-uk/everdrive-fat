everdrive-fat
=============

FAT filesystem driver and stdio-like interface for the Turbo Everdrive flash card (http://krikzz.com/) for the NEC PC-Engine and TurboGrafx 16 video game consoles.

The FAT library is implemented in C (using the Small-C subset as provided by the HuC compiler), and leverages the low level Everdrive SD driver written by https://github.com/BlockoS. My work would not have been possible without that driver.

Overview
========

The main FAT library consists of the following C library files:

src/
* fat-dev.h - Implements low level and partition detection routines.
* fat-vol.h - Implements FAT volume sector information retrieval for the current selected partition.
* fat-files.h - Implements fopen() and fclose() - planned implementation for fread(), fseek() etc.
* fat-misc.h - Helper and test functions, will not be needed in production use of the fat library.
* fat.h - Macro importing all the fat library files, several global variables and constants.

In addition, the following files include helper functions that, while not directly related to FAT/Filesystem support, are required:

* math32.h - Several 32bit math functions - add/multiply etc, needed for 32bit sector addressing.
* math32-extras.h - Some simple logical operators/tests for 32bit numbers.
* endian.h - Functions to flip 16 and 32bit data structures from little to big-endian as needed for FAT devices.
* print.h - Functions to print 32bit values as text/hex. Needed for some of the example code, but not in games, for example.

You can find example implementations of the everdrive-fat library under:

examples/
* 01_detect - Detailed SD card detection, partition identifier and FAT filesystem info, demonstrates the fat-dev.h, fat-vol.h functions.
* 02_fileopen - Example code for testing the functionality of fopen() and fread() as included in fat-files.h.
* 03_benchmark - Example code for testing the speed of reading sectors from the SD card.
* 04_textreader - NOT YET IMPLEMENTED.

To include the driver in your game/utility, rename the 'src' directory to 'fat' and drop it in your source code tree. Simply include "fat/fat.h" in your main code. Take a look at the examples for useage details.

Documentation is available for all functions within the relevant header files, further documentation will appear in:

docs/


Current Status
==============

Code has been tested on both physical and emulated systems: a Turbo Express with Everdrive (along with SD and SDHC cards) as well as Mednafen (for general correctness and debugging).

I currently use HuC/PCEAS v3.21. More recent versions of HuC have introduced struct{} support, which would have made the code a lot more compact, it may be an enhancement I look at in the future. At this moment in time the code will NOT compile on the version of HuC with struct support.

The fat library can currently do the following:

* SD Card / Turbo Everdrive - Can detect the Turbo Everdrive flash card and the inserted SD card (and type)
* DOS Master Boot Record - Can autodetect the first available FAT partition and extract start sector information, setting it as the current partition for a given session. Can also choose partition 1-4 manually, setting it as current for a session.
* FAT Volume Record - Can read FAT volume sector information, reading sector/cluster sizes, FAT table starting addresses and data cluster start, resulting in the starting address of the root directory cluster for the filesystem.
* Directories - Directory traversal to find named files/folders is implemented and working via the fopen() call.
* Files - Initial file (read) support is implemented in fread(). For now only the first 512 bytes of a file can be read as the file pointer and get-next-sector logic in not implemented. I am actively working on the logic for following the cluster chain and thus the following sectors.


Feedback
========

Email me, or ue the issue tracker as I don't use the PCEngineFX forums (http://www.pcenginefx.com/forums/index.php?topic=15996.0) as much as I used to.
