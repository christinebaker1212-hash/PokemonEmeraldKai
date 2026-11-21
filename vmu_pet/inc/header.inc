;; Moemon VMU pet: the start of every GAME file, Sega's GHEAD.ASM in waterbear's syntax (VMU tutorial VMT-88).
;; Sega's "change X" (jump into the BIOS's same address) is `not1 ext, 0` + `jmpf X`; the BIOS returns to the
;; instruction after it (libevmu: fm_wrt_ex 0x105, fm_vrf_ex 0x115, fm_prd_ex 0x125, timer_ex 0x139).
;; The program defines: main, int_base_timer (called every half second, after the BIOS's clock), int_sio1.

  .org 0
  jmpf main
  .org $03
  reti                      ; INT0: the Dreamcast's 5 V (unused: main polls P7)
  .org $0b
  reti                      ; INT1
  .org $13
  reti                      ; INT2
  .org $1b
  jmpf timer_ex             ; INT3: the base timer
  .org $23
  reti                      ; T0
  .org $2b
  reti                      ; T1
  .org $33
  reti                      ; SIO0
  .org $3b
  reti                      ; SIO1
  .org $43
  reti                      ; RFB (maple)
  .org $4b
  clr1 p3int, 1             ; P3: a button (wakes HALT)
  reti

  .org $100
fm_wrt_ex:                  ; write the 128-byte page at RAM bank 1 $80 to flash FMBANK:FMADD_H:FMADD_L
  not1 ext, 0
  jmpf fm_wrt_ex
  ret

  .org $110
fm_vrf_ex:                  ; compare it: acc 0 = the same
  not1 ext, 0
  jmpf fm_vrf_ex
  ret

  .org $120
fm_prd_ex:                  ; read a page into RAM bank 1 $80
  not1 ext, 0
  jmpf fm_prd_ex
  ret

  .org $130
timer_ex:
  push ie
  clr1 ie, 7
  not1 ext, 0
  jmpf timer_ex
  callf int_base_timer      ; the BIOS returns to $139: here (far: the program may put it past 4 KB)
  pop ie
  reti

  .org $1f0
game_end:                   ; back to the VMU's menus (does not return)
  not1 ext, 0
  jmpf game_end
