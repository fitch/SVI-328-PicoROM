;******************************************************************************
; Spectravideo SVI-328 MegaROM
; (c) 2025 Markus Rautopuro
; 
; Use z88dk to compile, type 'make', see 'makefile' for details.
;

SVI_ROM_CHGET           equ 0x403d
SVI_ROM_CHPUT           equ 0x394d
SVI_ROM_INITXT          equ 0x3541
SVI_ROM_INIGRP          equ 0x3610
SVI_ROM_ERAFNK          equ 0x3b86
SVI_ROM_PRLOGO_SVI      equ 0x47e5
SVI_ROM_PRLOGO_LINE     equ 0x4835
SVI_ROM_PRLOGO_WAIT     equ 0x47ce
SVI_ROM_CLS             equ 0x3778

SVI_RAM_CURSOR_STATUS	equ 0xfa05
SVI_RAM_CSRX            equ 0xfa04
SVI_RAM_CSRY            equ 0xfa03
SVI_RAM_CSR             equ 0xfa03
SVI_RAM_SCREEN          equ 0xfe3a
SVI_RAM_FRONT_COLOR     equ 0xfa0a
SVI_RAM_BACK_COLOR      equ 0xfa0b
SVI_RAM_BORCLR          equ 0xfa0c
SVI_RAM_ATRBYT          equ 0xfa13