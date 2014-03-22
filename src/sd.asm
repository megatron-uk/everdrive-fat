;;---------------------------------------------------------------------
; Turbo-Everdrive SD card library
;;---------------------------------------------------------------------
SPI_SS         = 0
SPI_FULL_SPEED = 1
SPI_AREAD      = 2

STATE_SPI      = 0
STATE_RY       = 1
STATE_FIFO_WR  = 2
STATE_FIFO_RD  = 3
STATE_FIFO_RST = 4

REG_SPI      = 0
REG_SPI2     = 1 
REG_FIFO     = 3
REG_STATE    = 4
REG_KEY      = 5
REG_CFG      = 6
REG_MAP      = 7
REG_SPI_CFG  = 8
REG_FIRM_VER = 9
REG_KEY2     = 10

SPI_REG_ADDR = $C000    ; SPI registers base address

SPI_TIMEOUT = 1024

;;---------------------------------------------------------------------
; SD card commands
;;---------------------------------------------------------------------
SD_CMD_GO_IDLE_STATE          = $40 ; Init card in spi mode if CS low
SD_CMD_WAKE_UP                = $41 ; Bring card out of idle state
SD_CMD_SEND_IF_COND           = $48 ; Verify SD Memory Card interface operating condition
SD_CMD_SEND_CSD               = $49 ; Read the Card Specific Data (CSD register)
SD_CMD_SEND_CID               = $4A ; Read the card identification information (CID register)
SD_STOP_TRANSMISSION          = $4C ; Forces the card to stop transmission
SD_CMD_SEND_STATUS            = $4D ; Read the card status register
SD_CMD_READ_BLOCK             = $51 ; Read a single data block from the card
SD_CMD_READ_MULTIPLE_BLOCK    = $52 ; Read blocks of data until a STOP_TRANSMISSION
SD_CMD_SET_BLOCK_COUNT        = $57 ; Specify block count for multiple block read or write
SD_CMD_WRITE_BLOCK            = $58 ; Write a single data block to the card
SD_CMD_WRITE_MULTIPLE_BLOCK   = $59 ; Write blocks of data until a STOP_TRANSMISSION
SD_CMD_ERASE_WR_BLK_START     = $60 ; Sets the address of the first block to be erased 
SD_CMD_ERASE_WR_BLK_END       = $61 ; Sets the address of the last block of the continuous range to be erased
SD_CMD_ERASE                  = $66 ; Erase all previously selected blocks
SD_CMD_APP_CMD                = $77 ; Escape for application specific command
SD_CMD_READ_OCR               = $7A ; Read the OCR register of a card 
SD_CMD_SET_WR_BLK_ERASE_COUNT = $97 ; Set the number of write blocks to be pre-erased before writing
SD_CMD_SD_SEND_OP_COMD        = $A9 ; Sends host capacity support information and activates the card's initialization process
SD_CMD_41_TODO                = $69 ;

SD_DATA_START_BLOCK     = $FE ; Start data token for read or write single block
SD_STOP_TRAN_TOKEN      = $FD ; Stop token for write multiple blocks
SD_WRITE_MULTIPLE_TOKEN = $FC ; Start data token for write multiple blocks
SD_DATA_RES_MASK        = $1F ; Mask for data response tokens after a write block operation
SD_DATA_RES_ACCEPTED    = $05 ; Write data accepted token

;;---------------------------------------------------------------------
; SD card type
;;---------------------------------------------------------------------
SD_V2 = 2
SD_HC = 1

;;---------------------------------------------------------------------
; Errors
;;---------------------------------------------------------------------
ERR_NONE             = 0
ERR_TIMEOUT          = 1
ERR_REG_ERROR        = 2
ERR_FILE_TOO_BIG     = 140
ERR_OS_RISK          = 141
ERR_WRONG_OS_SIZE    = 142
ERR_OS_FRAGMENTATION = 143
ERR_OS_BAD_TILE      = 144

FAT_ERR_INIT  = 110
FAT_LFN_ERROR = 115

DISK_ERR_INIT = 50
DISK_ERR_RD1  = 62
DISK_ERR_RD2  = 63

DISK_ERR_WR1 =  64
DISK_ERR_WR2 =  65
DISK_ERR_WR3 =  66
DISK_ERR_WR4 =  67
DISK_ERR_WR5 =  68

;;---------------------------------------------------------------------
; Ram copy instructions
;;---------------------------------------------------------------------
MEMCPY_SOURCE_ALT_DEST_INC = $f3 
MEMCPY_SOURCE_DEC_DEST_DEC = $c3 
MEMCPY_SOURCE_INC_DEST_ALT = $e3 
MEMCPY_SOURCE_INC_DEST_NOP = $d3 
MEMCPY_SOURCE_INC_DEST_INC = $73 
MEMCPY_RTS                 = $60

;;---------------------------------------------------------------------
; 
;;---------------------------------------------------------------------
SPI_SS_OFF .macro
    lda  SPI_REG_ADDR + REG_SPI_CFG
    ora  #(1 << SPI_SS)
    sta  SPI_REG_ADDR + REG_SPI_CFG
    .endm
    
SPI_SS_ON .macro 
    lda  SPI_REG_ADDR + REG_SPI_CFG
    and  #~(1 << SPI_SS)
    sta  SPI_REG_ADDR + REG_SPI_CFG
    .endm
    
SPI_SPEED_ON .macro
    lda  SPI_REG_ADDR + REG_SPI_CFG
    ora  #(1 << SPI_FULL_SPEED)
    sta  SPI_REG_ADDR + REG_SPI_CFG
    .endm
    
SPI_SPEED_OFF .macro
    lda  SPI_REG_ADDR + REG_SPI_CFG
    and  #~(1 << SPI_FULL_SPEED)
    sta  SPI_REG_ADDR + REG_SPI_CFG
    .endm
    
SPI_AREAD_ON .macro
    lda  SPI_REG_ADDR + REG_SPI_CFG
    ora  #(1 << SPI_AREAD)
    sta  SPI_REG_ADDR + REG_SPI_CFG
    .endm
    
SPI_AREAD_OFF .macro
    lda  SPI_REG_ADDR + REG_SPI_CFG
    and  #~(1 << SPI_AREAD)
    sta  SPI_REG_ADDR + REG_SPI_CFG
    .endm
    
SPI_BUSY_WAIT .macro
    stw    #SPI_TIMEOUT, <_ed_timeout
.spi_busy_wait_\@:
    decw   <_ed_timeout
    ora    <_ed_timeout
    beq    .spi_busy_end_\@          ; If we leave there the clary flag should have been set by sbc
    lda    SPI_REG_ADDR + REG_STATE
    and    #(1 << STATE_SPI)
    beq    .spi_busy_wait_\@
    clc
.spi_busy_end_\@:
    .endm

FIFO_RD_WAIT .macro
.spi_rd_wait_\@:
    lda    SPI_REG_ADDR + REG_STATE
    and    #(1 << STATE_FIFO_RD)
    beq    .spi_rd_wait_\@
    .endm
    
FIFO_WR_WAIT .macro
.spi_wr_wait_\@:
    lda    SPI_REG_ADDR + REG_STATE
    and    #(1 << STATE_FIFO_WR)
    beq    .spi_wr_wait_\@
    .endm

;;---------------------------------------------------------------------
; 
;;---------------------------------------------------------------------
    .zp
_ed_bl        .ds 1 ; MPR6 save
_ed_addr      .ds 4 ; current card address
_ed_buffer    .ds 4 ; 4 bytes buffer
_ed_cardtype  .ds 1 ; card type

_ed_timeout   .ds 2

ed_block_cp_inst  .ds 1
ed_block_cp_src   .ds 2
ed_block_cp_dst   .ds 2
ed_block_cp_size  .ds 2
ed_block_cp_rts   .ds 1

    .code

;;---------------------------------------------------------------------
; name : ed_map
; desc : Save mpr 6 and set it in order to use Everdrive's registers.
;;---------------------------------------------------------------------
ed_map  .macro
    tma    #$6
    sta    <_ed_bl
    cla
    tam    #$6
    .endm
    
;;---------------------------------------------------------------------
; name : ed_unmap
; desc : Restore mpr 6.
;;---------------------------------------------------------------------
ed_unmap .macro
    lda    <_ed_bl
    tam    #$6
    .endm
    
;;---------------------------------------------------------------------
; name : ed_begin
; desc : Enable everdrive.
; in   :
; out  :
;;---------------------------------------------------------------------    
ed_begin:
    ; Setup size and rts instruction for memory copy code.
    stw    #512, <ed_block_cp_size
    lda    #MEMCPY_RTS
    sta    <ed_block_cp_rts
    
    lda    #$A5
    sta    SPI_REG_ADDR + REG_KEY
    lda    #$56
    sta    SPI_REG_ADDR + REG_KEY2
    stz    SPI_REG_ADDR + REG_CFG
    stz    SPI_REG_ADDR + REG_SPI_CFG
    rts

;;---------------------------------------------------------------------
; name : ed_end
; desc : Disable everdrive.
; in   :
; out  :
;;---------------------------------------------------------------------
ed_end:
    stz    SPI_REG_ADDR + REG_KEY
    rts

;;---------------------------------------------------------------------
; name : spi_recv
; desc : Read a byte from spi register.
; in   :
; out  : A value of spi register.
;;--------------------------------------------------------------------- 
spi_recv:
    lda    #$ff
; Warning! do not add code here or you will break spi_recv!    
;;---------------------------------------------------------------------
; name : spi_send
; desc : Write a byte to spi register.
; in   : A byte to write
; out  : X value of spi register.
;;--------------------------------------------------------------------- 
spi_send:
    sta SPI_REG_ADDR + REG_SPI
    SPI_BUSY_WAIT
    lda SPI_REG_ADDR + REG_SPI

    rts

;;---------------------------------------------------------------------
; name : spi_wait_ready
; desc : Wait until spi register equals $ff
; in   : 
; out  : A spi register value
;        C flag is set if timeout occured
;;--------------------------------------------------------------------- 
spi_wait_ready:
    stw    #SPI_TIMEOUT, <_ed_timeout
.loop:
    jsr    spi_recv
    cmp    #$ff
    bne    .end

    decw   <_ed_timeout
    ora    <_ed_timeout
    bne    .loop
.timeout:
    rts
.end:
    clc
    rts
    
;;---------------------------------------------------------------------
; name : mmc_cmd
; desc : Write a byte to spi register.
; in   : A command
;        X crc
;        _ed_buffer 4 bytes data
; out  : X command result
;;--------------------------------------------------------------------- 
mmc_cmd:
    phx
    pha
    
    SPI_BUSY_WAIT
    bcc  .no_timeout
        ; Wait timeout occured!
        pla    ; Restore stack.
        pla
        clx    ; (todo)
        rts
.no_timeout:

    SPI_SS_OFF
    SPI_SS_ON

    jsr    spi_recv
    jsr    spi_recv
    
    ; Send command
    pla
    jsr    spi_send
    ; Send arguments
    lda    <_ed_buffer+3
    jsr    spi_send
    lda    <_ed_buffer+2
    jsr    spi_send
    lda    <_ed_buffer+1
    jsr    spi_send
    lda    <_ed_buffer
    jsr    spi_send
    ; Send crc
    pla
    jsr    spi_send
    
    ; Wait for response
    jsr    spi_recv
    jsr    spi_recv
    cmp    #$ff
    bne    .l1
    
    clx
    cly
.l0:
        jsr    spi_recv
        cmp    #$ff
        bne    .l1
 
        inx
        bne    .l0
        iny
        cpy    #high(2048)
        bne    .l0
.l1:
    tax
    SPI_SS_OFF
    rts

;;---------------------------------------------------------------------
; name : disk_init
; desc : Initialize disk.
; in   : 
; out  : X error code
;;---------------------------------------------------------------------
disk_init:
    stz    <_ed_cardtype
    
    lda    #$ff
    sta    <_ed_addr
    sta    <_ed_addr+1
    sta    <_ed_addr+2
    sta    <_ed_addr+3
    
    SPI_SS_OFF
    SPI_SPEED_OFF
    
    ldx    #32
.l0:
    jsr    spi_recv
    dex
    bne    .l0

    ; Send reset command to turn SD card in SPI mode.
    stwz   <_ed_buffer
    stwz   <_ed_buffer+2
    lda    #SD_CMD_GO_IDLE_STATE
    ldx    #$95
    jsr    mmc_cmd
    cpx    #$01
    beq    .l1
    
    lda    #SD_CMD_GO_IDLE_STATE
    ldx    #$95
    jsr    mmc_cmd
    cpx    #$01
    beq    .l1
        ldx    #(DISK_ERR_INIT)
        rts
.l1:
   
    stw    #$01aa, <_ed_buffer
    stwz   <_ed_buffer+2 
    lda    #SD_CMD_SEND_IF_COND
    ldx    #$87
    jsr    mmc_cmd
    phx
    
    jsr    spi_recv
    jsr    spi_recv
    jsr    spi_recv
    jsr    spi_recv
    jsr    spi_recv

    pla
    cmp    #$ff
    bne    .l3
        ldx    #(DISK_ERR_INIT + 1)
        rts
.l3
    cpx    #$05
    beq    .l4
        smb1    <_ed_cardtype
.l4

    bbs1   <_ed_cardtype, .v2_card
        jmp   .std_card
.v2_card:
        lda    #low(16384)
        sta    <_cl
        lda    #high(16384)
        sta    <_ch
.l5:
            lda    #$ff
            sta    <_ed_buffer
            sta    <_ed_buffer+1
            sta    <_ed_buffer+2
            sta    <_ed_buffer+3
            lda    #SD_CMD_APP_CMD
            ldx    #$95
            jsr    mmc_cmd  
            cpx    #$ff
            bne    .l6
                ldx    #(DISK_ERR_INIT + 2)
                rts
.l6:
            cpx    #$01
            bne    .l8
            
            stwz   <_ed_buffer
            stw    #$4030, <_ed_buffer+2
            lda    #SD_CMD_41_TODO
            ldx    #$95
            jsr    mmc_cmd  
            cpx    #$ff
            bne    .l7
                ldx    #(DISK_ERR_INIT + 3)
                rts
.l7:
            cpx    #$00
            beq    .l9
.l8:
            sec
            lda    <_cl
            sbc    #$01
            sta    <_cl
            lda    <_ch
            sbc    #$00
            sta    <_ch
            bcs    .l5
            lda    #(DISK_ERR_INIT + 4)
            rts
.l9:
        stwz   <_ed_buffer
        stwz   <_ed_buffer+2
        lda    #SD_CMD_READ_OCR
        ldx    #$95
        jsr    mmc_cmd
        cpx    #$ff
        bne    .l10
            ldx    #(DISK_ERR_INIT + 5)
            rts
.l10:
        SPI_SS_ON
        jsr    spi_recv
        pha
        
        jsr    spi_recv
        jsr    spi_recv
        jsr    spi_recv
            
        pla
        and    #$40
        bne    .l11
            smb0    <_ed_cardtype
.l11:
        SPI_SPEED_ON
        clx
        rts

.std_card:
        stwz   <_ed_buffer
        stwz   <_ed_buffer+2
        lda    #SD_CMD_APP_CMD
        ldx    #$95
        jsr    mmc_cmd   
        cpx    #$ff
        bne    .l12
            ldx    #(DISK_ERR_INIT + 6)
            rts
.l12:
        lda    #SD_CMD_41_TODO
        ldx    #$95
        jsr    mmc_cmd
        cpx    #$ff
        bne    .l13
            ldx    #(DISK_ERR_INIT + 7)
            rts
.l13:
        stw    #16384, <_cx
.l14:
            cpx   #$01
            bcs   .l17
                stwz   <_ed_buffer
                stwz   <_ed_buffer+2
                lda    #SD_CMD_APP_CMD
                ldx    #$95
                jsr    mmc_cmd
                cpx    #$ff
                bne    .l15
                    ldx    #(DISK_ERR_INIT + 8)
                    rts
.l15:
                cpx    #$01
                bne    .l18

                lda    #SD_CMD_41_TODO
                ldx    #$95
                jsr    mmc_cmd
                cpx    #$ff
                bne    .l16
                    ldx    #(DISK_ERR_INIT + 9)
                    rts
.l16:
                cpx    #$00
                beq    .l19
.l17:
                stwz   <_ed_buffer
                stwz   <_ed_buffer+2
                lda    #SD_CMD_WAKE_UP
                ldx    #$95
                jsr    mmc_cmd
                cpx    #$00
                beq    .l19
.l18:
        sbcw   #$0001, <_cx
        bcs    .l14
        lda    #(DISK_ERR_INIT + 10)
        rts

.l19:
        SPI_SPEED_ON
        clx
        rts

;;---------------------------------------------------------------------
; name : spi_send_to_ram
; desc : Copy 512 bytes from sd to ram
; in   : <ed_block_cp_dst destination
; out  : X : ERR_TIMEOUT    Failed to read SPI register after numerous try.
;            ERR_REG_ERROR  The SPI register value is incorrect.
;            ERR_NONE (0)   The buffer was successfully copied.
;;---------------------------------------------------------------------
spi_send_to_ram:
    jsr    spi_wait_ready
    bcc    .check_token
.timeout:
    ldx    #ERR_TIMEOUT
    rts
.check_token: 
    cmp    #SD_DATA_START_BLOCK
    beq    .begin_transfer
        ldx    #DISK_ERR_RD1
        rts
        
.begin_transfer:
    ; Setup RAM copy instruction
    lda    #MEMCPY_SOURCE_ALT_DEST_INC
    sta    <ed_block_cp_inst
    stw    #SPI_REG_ADDR, <ed_block_cp_src

    SPI_AREAD_ON

    lda    SPI_REG_ADDR
    jsr    ed_block_cp_inst
    lda    SPI_REG_ADDR

    SPI_AREAD_OFF
    
    clx    ; ERR_NONE
    rts

;;---------------------------------------------------------------------
; name : disk_update_address
; desc : Update address if needed
; in   : <_ed_addr 32 bytes address (byte address on standard SD)
;                                   (sector address on SD HC)
;        <_ed_cardtype SD card type
; out  : <_ed_cardtype updated address
;;---------------------------------------------------------------------
disk_update_address:
    bbr0   <_ed_cardtype, .l0
        ; Standard SD cards take a byte address.
        ; Whereas SDHC cards take a sector (hence the mul by 512).
        lda    <_ed_addr+2
        sta    <_ed_addr+3
        lda    <_ed_addr+1
        sta    <_ed_addr+2
        lda    <_ed_addr
        sta    <_ed_addr+1
        stz    <_ed_addr
        asl    <_ed_addr+1
        rol    <_ed_addr+2
        rol    <_ed_addr+3
.l0:
    rts
   
;;---------------------------------------------------------------------
; name : disk_start_read_sequence
; desc : Start multiple blocks read sequence.
; in   : <_ed_addr 32 bytes address (byte address on standard SD)
;                                   (sector address on SD HC)
; out  : X DISK_ERR_RD2 if an error occured, 0 for success
;;---------------------------------------------------------------------
disk_start_read_sequence:
    jsr    disk_update_address
    ; [todo] add state?
    
    stw    <_ed_addr+2, <_ed_buffer+2
    stw    <_ed_addr,   <_ed_buffer
    lda    #SD_CMD_READ_MULTIPLE_BLOCK
    ldx    #$01     ; dummy CRC
    jsr    mmc_cmd
    cpx    #$00
    beq    .l1
        ldx    #DISK_ERR_RD2
        rts
.l1:    
    SPI_SS_ON;
    clx
    rts

;;---------------------------------------------------------------------
; name : disk_stop_read_sequence
; desc : Stop multiple blocks read sequence.
; in   : 
; out  :
;;---------------------------------------------------------------------
disk_stop_read_sequence:
    stwz   <_ed_buffer
    stwz   <_ed_buffer+2
    
    ; [todo] wait
    lda    #SD_STOP_TRANSMISSION
    ldx    #$87     ; Dummy CRC
    jsr    mmc_cmd

    rts

;;---------------------------------------------------------------------
; name : disk_read_sector
; desc : Read sector.
; in   : <ed_block_cp_dst destination 
; out  : X DISK_ERR_RD1 if an error occured, 0 for success
;;---------------------------------------------------------------------
disk_read_sector:
    jsr    spi_send_to_ram
    cpx    #$00
    beq    .l0
        ldx   #DISK_ERR_RD1
        rts
.l0:
    
    inc    <_ed_addr
    bne    .l1
    inc    <_ed_addr+1
    bne    .l1
    inc    <_ed_addr+2
    bne    .l1
    inc    <_ed_addr+3
    bne    .l1
.l1:
    clx
.end:
    rts
    
;;---------------------------------------------------------------------
; name : disk_read_single_sector
; desc : Read a single sector.
; in   : <_ed_addr 32 bytes address (byte address on standard SD)
;                                   (sector address on SD HC)
;        <ed_block_cp_dst destination 
; out  : X DISK_ERR_RD1 Read failed
;          DISK_ERR_RD2 Open failed
;;---------------------------------------------------------------------
disk_read_single_sector:    
    jsr    disk_update_address
    
    stw    <_ed_addr+2, <_ed_buffer+2
    stw    <_ed_addr,   <_ed_buffer
    lda    #SD_CMD_READ_BLOCK
    ldx    #$87
    jsr    mmc_cmd
    cpx    #ERR_NONE
    beq    .l1
        SPI_SS_OFF
        ldx    #DISK_ERR_RD2
        rts
.l1:    
    SPI_SS_ON
    jsr    spi_send_to_ram
    cpx    #ERR_NONE
    bne    .err
.ok:
        SPI_SS_OFF
        clx
        rts
.err:
        SPI_SS_OFF
        ldx   #DISK_ERR_RD1
        rts

;;---------------------------------------------------------------------
; name : spi_write_to_card
; desc : Write 512 bytes from ram to sd
; in   : <ed_block_cp_src [todo]
; out  : X : ERR_TIMEOUT    Failed to read SPI register after numerous try.
;            ERR_REG_ERROR  The SPI register value is incorrect.
;            ERR_NONE (0)   The buffer was successfully copied.
;;---------------------------------------------------------------------
spi_write_to_card:   
    ; We don't use memory copy here.
    
    ; Send token
    lda    #SD_DATA_START_BLOCK
    jsr    spi_send
    
    ; Copy first 256 bytes
    cly
.l0:
    lda    [ed_block_cp_src], Y
    jsr    spi_send
    iny
    bne    .l0

    inc    <ed_block_cp_src+1
    
    ; Copy last 256 bytes
    cly
.l1:
    lda    [ed_block_cp_src], Y
    jsr    spi_send
    iny
    bne    .l1
   
    ; Send dummy CRC (ffff)
    lda    #$ff
    jsr    spi_send
    
    lda    #$ff
    jsr    spi_send
    
    ; Read response
    jsr    spi_recv
    and    #SD_DATA_RES_MASK
    cmp    #SD_DATA_RES_ACCEPTED
    bne    .nok
    
    ; Wait for the end of transfer
    jsr    spi_wait_ready
    
    ldx    #ERR_NONE
    rts

.nok:
    ldx    #ERR_REG_ERROR
    rts
    
;;---------------------------------------------------------------------
; [todo] test it!
; name : disk_write_single_sector
; desc : Write a single sector.
; in   : <_ed_addr 32 bytes address (byte address on standard SD)
;                                   (sector address on SD HC)
;        <ed_block_cp_src destination 
; out  : X [todo]
;;---------------------------------------------------------------------
disk_write_single_sector:    
    jsr    disk_update_address
    
    stw    <_ed_addr+2, <_ed_buffer+2
    stw    <_ed_addr,   <_ed_buffer
    lda    #SD_CMD_WRITE_BLOCK
    ldx    #$87     ; dummy CRC
    jsr    mmc_cmd
    cpx    #ERR_NONE
    beq    .l1
        ldx    #DISK_ERR_RD2 ; [todo]
        rts
.l1:

    SPI_SS_ON
    
    jsr    spi_write_to_card
    
    SPI_SS_OFF
     
    rts
        
;;---------------------------------------------------------------------
; name : disk_start_write_sequence
; desc : 
; in   : (todo) block
;        (todo) pre erase count
;        (todo) data pointer
; out  : (todo)
;;---------------------------------------------------------------------
disk_start_write_sequence:
    ; (todo)
    rts

;;---------------------------------------------------------------------
; name : disk_stop_write_sequence
; desc : 
; in   :
; out  : (todo)
;;---------------------------------------------------------------------
disk_stop_write_sequence:
    ; (todo)
    rts

;;---------------------------------------------------------------------
; name : disk_write_sector
; desc : (todo)
; in   : <ed_block_cp_src
; out  : (todo)
;;---------------------------------------------------------------------
disk_write_sector:
    ; (todo)
    rts
        
    
