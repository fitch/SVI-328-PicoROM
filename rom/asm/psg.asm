;******************************************************************************
; Spectravideo SVI-328 MegaROM
; (c) 2025 Markus Rautopuro
; 
; Use z88dk to compile, type 'make', see 'makefile' for details.
;

PSG_ADDRESS_LATCH       equ 0x88
PSG_DATA_WRITE          equ 0x8c
PSG_DATA_READ           equ 0x90
PSG_REGISTER_R7         equ 0x7
PSG_REGISTER_R15        equ 0xf

VDP_DATA_WRITE          equ 0x80
VDP_ADDRESS_REGISTER    equ 0x81
VDP_DATA_READ           equ 0x84
VDP_RESET_STATUS        equ 0x85

; Function to switch BASIC BIOS ROM

PSG_show_BIOS_ROM:
    di
    ld a, PSG_REGISTER_R15      ; Port B is controlled via register 15
    out (PSG_ADDRESS_LATCH), a
    in a, (PSG_DATA_READ)
    or %00000011                ; Disable CART, low = active (the lower bank cartridge ROM), also disable BK21 if enabled
    out (PSG_DATA_WRITE), a     ; Execute switching the RAM to higher bank
    ret

; Function to switch BASIC BIOS ROM

PSG_show_cartridge_ROM:
    di
    ld a, PSG_REGISTER_R15      ; Port B is controlled via register 15
    out (PSG_ADDRESS_LATCH), a
    in a, (PSG_DATA_READ)
    or %00000010                ; Disable BK21 if enabled
    and %11111110               ; Enable CART, low = active (the lower bank cartridge ROM)
    out (PSG_DATA_WRITE), a     ; Execute switching the RAM to higher bank
    ret

; Function to disable lower bank cartridge ROM and use lower bank RAM instead

PSG_disable_CART_enable_BK21:
    di
    ld a, PSG_REGISTER_R15      ; Port B is controlled via register 15
    out (PSG_ADDRESS_LATCH), a
    in a, (PSG_DATA_READ)
    or %00000001                ; Disable CART, low = active
    and %11111101               ; Enable BK21 (the lower bank RAM)
    out (PSG_DATA_WRITE), a     ; Execute switching the RAM to higher bank
    ret