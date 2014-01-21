/************************************************
 * Turbo-Everdrive SD card library
 ************************************************/
 
/* ASM routines */
#asm
SD_BANK = 2

    .bank SD_BANK
    .org  $6000
    .include "sd.asm"
    
    ; [todo] move it to fat.h?
    ;.include "fat.asm"
    
    ; "Restore" bank
    .bank DATA_BANK    
    .code
    
sd_call .macro
    ; Map Everdrive routines bank
    tam   #$3
    pha
    lda   #SD_BANK
    tam   #$3
    ; Map Everdrive register bank
    ed_map
    ; Call the routine
    jsr    \1
    ; Restore the MPR used by the Everdrive registers
    ed_unmap
    ; Restore the MPR used by the Everdrive routines
    pla
    tam   #$3
    .endm
#endasm

/* SD card type */
#define SD_V2 2
#define SD_HC 1

/* Errors */
#define ERR_NONE             0
#define ERR_FILE_TOO_BIG     140
#define ERR_OS_RISK          141
#define ERR_WRONG_OS_SIZE    142
#define ERR_OS_FRAGMENTATION 143
#define ERR_OS_BAD_TILE      144

#define FAT_ERR_INIT  110
#define FAT_LFN_ERROR 115

#define DISK_ERR_INIT 50
#define DISK_ERR_RD1  62
#define DISK_ERR_RD2  63

#define DISK_ERR_WR1 64
#define DISK_ERR_WR2 65
#define DISK_ERR_WR3 66
#define DISK_ERR_WR4 67
#define DISK_ERR_WR5 68

/**
 * Enable everdrive.
 **/ 
ed_begin()
{
#asm
    sd_call    ed_begin
#endasm
}

/**
 * Disable everdrive.
 **/
ed_end()
{
#asm
    sd_call    ed_end
#endasm
}

/**
 * Initialize disk.
 * \return error code
 **/
disk_init()
{
#asm
    sd_call  disk_init
#endasm
} 

/**
 * Start multiple blocks read sequence.
 * For a standard SD card, the address is a standard byte address.
 * For a SD HC, it's the sector address.
 * Note that the address is a 32 bits word.
 * \param [in] addr_lo Least significant word.
 * \param [in] addr_hi Most significant word.
 * \return DISK_ERR_RD2 if an error occured, 0 for success
 **/
disk_start_read_sequence(addr_lo, addr_hi)
int addr_lo;
int addr_hi;
{
#asm
    lda    [__sp]
    sta    <_ed_addr+2
    ldy    #1
    lda    [__sp],Y
    sta    <_ed_addr+3
    iny
    lda    [__sp],Y
    sta    <_ed_addr
    iny
    lda    [__sp],Y
    sta    <_ed_addr+1
    iny
    sd_call  disk_start_read_sequence
#endasm
}
   
/**
 * Stop multiple blocks read sequence.
 **/
disk_stop_read_sequence()
{
#asm
    sd_call  disk_stop_read_sequence
#endasm
}
 
/**
 * Read sector.
 * \param [in] dest Destination pointer.
 * \return DISK_ERR_RD1 if an error occured, 0 for success
 **/
disk_read_sector(dest)
int *dest;
{
#asm
    stx    <ed_block_cp_dst
    sta    <ed_block_cp_dst+1
    sd_call  disk_read_sector
#endasm
}

/**
 * Read a single sector.
 * For a standard SD card, the address is a standard byte address.
 * For a SD HC, it's the sector address.
 * Note that the address is a 32 bits word.
 * \param [in] addr_lo Least significant word.
 * \param [in] addr_hi Most significant word.
 * \param [in] dest Destination pointer.
 * \return 
 *    DISK_ERR_RD1 Read failed
 *    DISK_ERR_RD2 Open failed
 **/
disk_read_single_sector (addr_lo, addr_hi, dest)
int addr_lo;
int addr_hi;
int *dest;
{
#asm
    lda    [__stack]
    sta     <ed_block_cp_dst  
    ldy    #1
    lda    [__stack], Y
    sta    <ed_block_cp_dst+1
    iny
    lda    [__stack], Y
    sta    <_ed_addr+2
    iny
    lda    [__stack], Y
    sta    <_ed_addr+3
    iny
    lda    [__stack], Y
    sta    <_ed_addr
    iny
    lda    [__stack], Y
    sta    <_ed_addr+1
    iny
    sd_call  disk_read_single_sector
#endasm
}

/**
 * Start multiple blocks write sequence.
 * For a standard SD card, the address is a standard byte address.
 * For a SD HC, it's the sector address.
 * Note that the address is a 32 bits word.
 * \param [in] addr_lo Least significant word.
 * \param [in] addr_hi Most significant word.
 * \return [todo] if an error occured, 0 for success
 **/
disk_start_write_sequence(addr_lo, addr_hi)
int addr_lo;
int addr_hi;
{
#asm
    lda    [__sp]
    sta    <_ed_addr+2
    ldy    #1
    lda    [__sp],Y
    sta    <_ed_addr+3
    iny
    lda    [__sp],Y
    sta    <_ed_addr
    iny
    lda    [__sp],Y
    sta    <_ed_addr+1
    iny
    sd_call  disk_start_write_sequence
#endasm
}
   
/**
 * Stop multiple blocks write sequence.
 **/
disk_stop_write_sequence()
{
#asm
    sd_call  disk_stop_write_sequence
#endasm
}
 
/**
 * Write sector.
 * \param [in] src Data source pointer.
 * \return [todo]
 **/
disk_write_sector(dest)
int *dest;
{
#asm
    stx    <ed_block_cp_dst
    sta    <ed_block_cp_dst+1
    sd_call  disk_write_sector
#endasm
}

/**
 * Write a single sector.
 * For a standard SD card, the address is a standard byte address.
 * For a SD HC, it's the sector address.
 * Note that the address is a 32 bits word.
 * \param [in] addr_lo Least significant word.
 * \param [in] addr_hi Most significant word.
 * \param [in] source Data source pointer.
 * \return [todo]
 **/
disk_write_single_sector(addr_lo, addr_hi, src)
int addr_lo;
int addr_hi;
int *src;
{
#asm
    lda    [__sp]
    sta    <_ed_addr+2
    ldy    #1
    lda    [__sp],Y
    sta    <_ed_addr+3
    iny
    lda    [__sp],Y
    sta    <_ed_addr
    iny
    lda    [__sp],Y
    sta    <_ed_addr+1
    iny
    lda    [__sp],Y
    sta    <ed_block_cp_src
    iny
    lda    [__sp],Y
    sta    <ed_block_cp_src+1
    iny
    sd_call  disk_write_single_sector
#endasm
}

/**
 * Retrieve card type
 * \return SD card type (either SD_V2 or SD_HC)
 **/
disk_get_cardtype()
{
#asm
    ldx  <_ed_cardtype
    cla
#endasm
}
