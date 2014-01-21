/* ========================================

	Flip endian-ness of various data types ...
	(such as addresses loaded from FAT filesystems -
	Intel x86 uses little-endian)

	Written by John Snowdon (john@target-earth.net), 2014.
	
========================================= */

swap_int16(int16)
char*	int16;
{
	/* Swap the endian-ness of an unsigned 16bit value */
	
	char b;
	b = int16[0];
	int16[0] = int16[1];
	int16[1] = b;
	return 0;
}

swap_int32(int32)
char*	int32;
{
	/* Swap the endian-ness of an 4byte packed unsigned 32bit value */
	
	char	b;
	
	b = int32[0];
	int32[0] = int32[3];
	int32[3] = b;
	
	b = int32[1];
	int32[1] = int32[2];
	int32[2] = b;
	
	return 0;
	
}

