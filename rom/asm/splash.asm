;******************************************************************************
; Spectravideo SVI-328 PicoROM
; (c) 2025 Markus Rautopuro
; 
; Use z88dk to compile, type 'make', see 'makefile' for details.
;

ShowSplash:
    ld a, 1                     ; Screen 1 / Black color
    ld (SVI_RAM_SCREEN), a
    ld (SVI_RAM_BORCLR), a
    ld (SVI_RAM_BACK_COLOR), a
    ld a, 3                     ; Green
    ld (SVI_RAM_FRONT_COLOR), a
    ld (SVI_RAM_ATRBYT), a

    call SVI_ROM_INIGRP
    call SVI_ROM_INIGRP         ; FIXME: INIGRP has to be called twice for it to work

    ld de, 0x1800 + 320         ; Hide name table entries below the logo before printing (starting from pattern 320 = 2nd slot, 0x40)
    ld a, e
    out (VDP_ADDRESS_REGISTER), a
    ld a, d
    or a, 0x40                  ; Write to VRAM
    out (VDP_ADDRESS_REGISTER), a

    ld a, 0                     
    ld b, 768 - 320 - 256       ; Loop from 320 to 767 = 448 times
    ld d, 1 + 1                 ; 448 requires 1 x 256 and 1 x 192
.loop_hide
    out (VDP_DATA_WRITE), a     ; Zero out
    djnz loop_hide
    dec d
    jp nz, loop_hide

    ld a, 72
    ld (SVI_RAM_CSRX), a
    ld a, 90
    ld (SVI_RAM_CSRY), a
    PrintBIOS MessageROM        ; Draw the texts on screen

    ld a, 86
    ld (SVI_RAM_CSRX), a
    ld a, 180
    ld (SVI_RAM_CSRY), a
    PrintBIOS MessageCopyright

    ld bc, 40                    
    ld de, 82 - 42              ; Move a little bit up (42) from original location
    call SVI_ROM_PRLOGO_SVI     ; Print Spectravideo in red

    ld bc, 56
    ld de, 100 - 42
    ld h, 72
    call SVI_ROM_PRLOGO_LINE    ; First logo line (without loading ATRBYT)

    ld a, 2
    ld (SVI_RAM_ATRBYT), a

    ld bc, 58
    ld de, 104 - 42
    ld h, 70
    call SVI_ROM_PRLOGO_LINE    ; Second line

    ld a, 12
    ld (SVI_RAM_ATRBYT), a 

    ld bc, 60
    ld de, 108 - 42
    ld h, 68
    call SVI_ROM_PRLOGO_LINE    ; Third line

    ld de, 0x1800 + 320         ; Restore name table entries to show texts
    ld a, e
    out (VDP_ADDRESS_REGISTER), a
    ld a, d
    or a, 0x40                  ; Write to VRAM
    out (VDP_ADDRESS_REGISTER), a

    ld a, 0x40
    ld b, 768 - 320 - 256       ; Loop from 320 to 767 = 448 times
    ld d, 1 + 1                 ; 448 requires 1 x 256 and 1 x 192
.loop_show
    out (VDP_DATA_WRITE), a     ; Restore name table entry
    inc a
    djnz loop_show
    dec d
    jp nz, loop_show


    call SVI_ROM_PRLOGO_WAIT    ; Execute boot logo waiting procedure
    ret

MessageROM:
    db "SVI-328 PicoROM 1.0", 0

MessageCopyright:
    db "(c) 2025 MAG-4", 0