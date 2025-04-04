;******************************************************************************
; Spectravideo SVI-328 PicoROM
; (c) 2025 Markus Rautopuro
; 
; Use z88dk to compile, type 'make', see 'makefile' for details.
;

PUBLIC _main                    ; Required by z88dk
_main:

MACRO PrintBIOS address
    ld hl, address
    call PrintString
ENDM

ROM_STACK               equ 0xf21c

    org 0x0000
    di
    ld sp, ROM_STACK

    ld hl, LAUNCHER_ROM_START
    ld de, LAUNCHER_RAM_START
    ld bc, LAUNCHER_SIZE
    ldir
    
    jp LAUNCHER_RAM_START

LAUNCHER_ROM_START:
    phase 0xc000
LAUNCHER_RAM_START:
    call PSG_show_BIOS_ROM
    call ShowSplash

ShowWifiSelector:
    call SVI_ROM_ERAFNK         ; Disable function key line in text mode
	call SVI_ROM_INITXT         ; Initialize text mode

    jp WaitForPicoState

.request_and_store_credentials:
    call PSG_show_BIOS_ROM

    PrintBIOS MessageSSID
    ld hl, SSID
    ld b, SSID_MAX_LENGTH
    call InputString

    PrintBIOS MessagePassword
    ld hl, Password
    ld b, PASSWORD_MAX_LENGTH
    call InputString

    PrintBIOS MessageStoring    

    call PSG_show_cartridge_ROM

    call WriteModeTogglePicoROM
    ld a, (PR_WRITE_SSID)       ; Write SSID command
    ld hl, SSID
    ld bc, SSID_MAX_LENGTH
    call WriteToPicoROM
    call WriteModeTogglePicoROM
    ld a, (PR_WRITE_TERMINATE)  ; Terminate write command

    call WriteModeTogglePicoROM
    ld a, (PR_WRITE_PASSWORD)   ; Write PASSWORD command
    ld hl, Password
    ld bc, PASSWORD_MAX_LENGTH
    call WriteToPicoROM
    call WriteModeTogglePicoROM
    ld a, (PR_WRITE_TERMINATE)  ; Terminate write command

    jp WaitForPicoState

.credentials_stored:
    call PSG_show_BIOS_ROM
    PrintBIOS MessageStored

    jp WaitForPicoState

.wait_for_wifi_connect:
    call PSG_show_BIOS_ROM
    PrintBIOS MessageConnecting

    jp WaitForPicoState

.wifi_connected:
    call PSG_show_BIOS_ROM
    PrintBIOS MessageSuccess

    jp WaitForPicoState

.client_connected:
    call PSG_show_BIOS_ROM
    PrintBIOS MessageClientConnected

    jp WaitForPicoState

.rom_ready:
    call PSG_show_BIOS_ROM
    PrintBIOS MessageROMReady
    call PSG_show_cartridge_ROM

    jp 0x0000                   ; Currently supports 32 kB ROMs that start at 0x0000

.wifi_error:
    call PSG_show_BIOS_ROM
    PrintBIOS MessageError    

.infinite_loop:
    jr infinite_loop

WaitForPicoState:
    call PSG_show_cartridge_ROM
.state_loop:
    ld a, (PR_STATE_ADDRESS)
    ld b, a
    ld a, (LastState)
    cp b
    jr z, state_loop

    ld a, b
    ld (LastState), a

    cp PR_STATE_WAITING_CREDENTIALS
    jp z, request_and_store_credentials
    cp PR_STATE_CREDENTIALS_STORED
    jr z, credentials_stored
    cp PR_STATE_WIFI_CONNECTING
    jr z, wait_for_wifi_connect
    cp PR_STATE_WIFI_CONNECTED
    jr z, wifi_connected
    cp PR_STATE_WIFI_ERROR
    jr z, wifi_error
    cp PR_STATE_CLIENT_CONNECTED
    jr z, client_connected
    cp PR_STATE_ROM_READY
    jr z, rom_ready
    jr state_loop


MessageSSID:
    db "Let's connect you to Wi-Fi first.", 13, 10, 13, 10
    db "SSID:", 13, 10
    db 0

MessagePassword:
    db 13, 10, 13, 10, "Password:", 13, 10
    db 0

MessageStoring:
    db 13, 10, 13, 10, "Storing credentials...", 13, 10
    db 0

MessageStored:
    db "Credentials stored.", 13, 10
    db 0

MessageConnecting:
    db "Connecting to Wi-Fi...", 13, 10
    db 0

MessageSuccess:
    db "Connected, waiting for a client...", 13, 10
    db 0

MessageClientConnected:
    db "Client connected, waiting for a ROM...", 13, 10
    db 0

MessageROMReady:
    db "ROM ready, booting...", 13, 10
    db 0

MessageError:
    db "Error connecting, reboot and try again.", 13, 10
    db 0

LastState:
    db PR_STATE_UNKNOWN

SSID_MAX_LENGTH         equ 32
PASSWORD_MAX_LENGTH     equ 32

SSID:
    ds SSID_MAX_LENGTH    
Password:
    ds PASSWORD_MAX_LENGTH    

PR_WRITE_SSID           equ 0x7f00
PR_WRITE_PASSWORD       equ 0x7f01
PR_WRITE_TERMINATE      equ 0x7f02

PR_STATE_ADDRESS        equ 0x7fff
PR_STATE_WAITING_CREDENTIALS equ 100
PR_STATE_CREDENTIALS_STORED equ 101
PR_STATE_WIFI_CONNECTING equ 103
PR_STATE_WIFI_CONNECTED equ 104
PR_STATE_WIFI_ERROR     equ 105
PR_STATE_CLIENT_CONNECTED equ 106
PR_STATE_RECEIVING_ROM  equ 107
PR_STATE_ROM_READY      equ 108
PR_STATE_ERROR          equ 254
PR_STATE_UNKNOWN        equ 255

PR_SIGNATURE_1          equ 0x7dea
PR_SIGNATURE_2          equ 0x7d0b
PR_SIGNATURE_3          equ 0x7eef

    include "asm/svi_bios.inc"

; Function to toggle PicoROM write mode

WriteModeTogglePicoROM:
    ld bc, PR_SIGNATURE_1
    ld de, PR_SIGNATURE_2
    ld hl, PR_SIGNATURE_3
    ld a, (bc)                  ; Read sequence to toggle write mode
    ld a, (de)
    ld a, (hl)
    ret

; Function to write to PicoROM
;   HL = buffer
;   BC = buffer length

WriteToPicoROM:
    ld d, 0x7f
.write_loop:
    ld e, (hl)
    ld a, (de)
    cpi
    jp pe, write_loop
    ret

; Function to get a string from the user
;   HL = buffer
;   B = buffer length

InputString:
    ld c, l
.input_loop:
    call SVI_ROM_CHGET
    cp 13
    jp z, end_input
    cp 8
    jp z, handle_backspace
    call SVI_ROM_CHPUT

    ld (hl), a
    inc hl
    djnz input_loop
    jr end_input

.handle_backspace:
    ld a, l
    cp c
    jp z, InputString
    inc b
    dec hl
    ld (hl), 0
    ld a, 8
    call SVI_ROM_CHPUT
    ld a, 32
    call SVI_ROM_CHPUT
    ld a, 8
    call SVI_ROM_CHPUT
    jp input_loop

.end_input:
    di
    ld (hl), 0
    ret

; Function to print a zero-terminated string using SVI_ROM_CHPUT

PrintString:
    ld a, (hl)			
    or a
    ret z
    inc hl
    call SVI_ROM_CHPUT
    jr PrintString

    include "asm/splash.asm"
    include "asm/psg.asm"

    dephase
LAUNCHER_SIZE           equ $ - LAUNCHER_ROM_START 

    defs 32768-$, 0xff	        ; Pad to 32 kB
