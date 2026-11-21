;; Moemon VMU pet: LINK TEST. Proves that two VMU programs can talk over the top connector (a VMU and a VM2), and
;; where a program running on the VM2 saves (its internal memory or the SD card's image).
;;
;; Run it on both units, plug them together. A on one sends a 32-byte packet; the other checks it, saves it in its own
;; file (the SAVE page below) and answers ACK. SENT / GOT count successes and are saved too, so the SD card can be
;; checked afterwards (vmu_pet/tools/find_linktest.py). UP / DOWN pick the speed (both units should match):
;; 1 = Sega's sample rate (slow, about 15 s), 2-4 faster. MODE leaves.

  .include "../inc/sfr.inc"
  .include "../inc/header.inc"

  ;; The VMS header (block 1)
  .org $200
  .text 16 "LINK TEST"
  .text 32 "MOEMON VMU LINK TEST"
  .string 16 "MOEMON KAI"
  .include icon "icon.png" speed=16

;; RAM bank 1 ($00-$0f are the indirect registers)
lcd_t0     = $10
lcd_t1     = $11
lcd_t2     = $12
lcd_t3     = $13
lcd_t4     = $14
lcd_t5     = $15
ticks      = $16
sio_t0     = $17
sio_t1     = $18
sio_speed  = $19
connected  = $1a
a_held     = $1b
ud_held    = $1c
sent_count = $1d
recv_count = $1e
boot_count = $1f
speed_idx  = $20
idx        = $21
chk        = $22
tmp        = $23
buf        = $30            ; 32 bytes
PAYLEN     = 32

main:
  set1 psw, 1               ; RAM bank 1
  call clk_fast
  call batt_off
  mov #0, ticks
  mov #0, connected
  mov #1, a_held            ; ignore an A still held from the menu
  mov #1, ud_held
  mov #0, speed_idx
  call speed_set
  call save_load
  inc boot_count
  call lcd_clear
  mov #<s_title, trl
  mov #>s_title, trh
  mov #0, c
  mov #0, b
  call lcd_text
  call draw_counts
  mov #<s_nocable, trl
  mov #>s_nocable, trh
  call status
  call draw_speed

loop:
  ld p3
  bn acc, BTN_MODE, finish
  call sio_connected
  bnz .nocable
  ld connected
  bnz .con
  call sio_init
  mov #1, connected
  mov #<s_cable, trl
  mov #>s_cable, trh
  call status
.con:
  call sio_poll
  bnz .noin
  ld b
  bne #SIO_SYNC, loop
  call recv_packet
  jmp loop
.noin:
  call keys
  jmp loop
.nocable:
  ld connected
  bz .idle
  call sio_end
  mov #0, connected
  mov #<s_nocable, trl
  mov #>s_nocable, trh
  call status
.idle:
  call keys
  call clk_slow
  set1 pcon, 0              ; HALT until the next half second
  call clk_fast
  jmp loop

finish:
  call sio_end
  call batt_on
  jmp game_end

;; keys: A sends (when connected), UP / DOWN the speed
keys:
  ld p3
  bp acc, BTN_A, .a_up
  ld a_held
  bnz .ud
  mov #1, a_held
  ld connected
  bz .ud
  call send_packet
  br .ud
.a_up:
  mov #0, a_held
.ud:
  ld p3
  and #%00000011
  be #%00000011, .ud_up
  ld ud_held
  bnz .done
  mov #1, ud_held
  ld p3
  bn acc, BTN_UP, .up
  dec speed_idx
  br .clamp
.up:
  inc speed_idx
.clamp:
  ld speed_idx
  and #3
  st speed_idx
  call speed_set
  call draw_speed
  ld connected
  bz .done
  ld sio_speed
  st sbr
  ret
.ud_up:
  mov #0, ud_held
.done:
  ret

speed_set:
  mov #<speeds, trl
  mov #>speeds, trh
  ld speed_idx
  ldc
  st sio_speed
  ret

speeds:
  .byte $dd, $f0, $fa, $fe

;; send_packet: SYNC, length, 32 bytes, checksum, each echoed; then the other side's ACK
send_packet:
  mov #<s_sending, trl
  mov #>s_sending, trh
  call status
  call clk_slow
  call sio_flush
  mov #0, idx               ; the packet: boot count, sent count, then a pattern
  mov #0, chk
.fill:
  ld idx
  add #buf
  st 0
  ld idx
  be #0, .b0
  be #1, .b1
  ld idx
  rol
  add idx
  add boot_count            ; idx * 3 + boot
  br .put
.b0:
  ld boot_count
  br .put
.b1:
  ld sent_count
.put:
  st @R0
  add chk
  st chk
  inc idx
  ld idx
  bne #PAYLEN, .fill
  mov #SIO_SYNC, acc
  call sio_xfer
  bnz .noans
  mov #PAYLEN, acc
  call sio_xfer
  bnz .bad
  mov #0, idx
.send:
  ld idx
  add #buf
  st 0
  ld @R0
  call sio_xfer
  bnz .bad
  inc idx
  ld idx
  bne #PAYLEN, .send
  ld chk
  call sio_xfer
  bnz .bad
  call sio_recv             ; the answer, after the other side saves (up to 3 s)
  bz .ans
  call sio_recv
  bnz .noack
.ans:
  call clk_fast
  ld b
  bne #SIO_ACK, .nak
  inc sent_count
  call save_write
  call draw_counts
  mov #<s_sentok, trl
  mov #>s_sentok, trh
  jmp status
.noans:
  call clk_fast
  mov #<s_noans, trl
  mov #>s_noans, trh
  jmp status
.bad:
  call clk_fast
  mov #<s_echo, trl
  mov #>s_echo, trh
  jmp status
.noack:
  call clk_fast
  mov #<s_noack, trl
  mov #>s_noack, trh
  jmp status
.nak:
  mov #<s_nak, trl
  mov #>s_nak, trh
  jmp status

;; recv_packet: the SYNC has arrived (in b)
recv_packet:
  call clk_slow
  ld b
  call sio_send             ; echo
  call sio_recv
  bnz .lost
  ld b
  st tmp
  call sio_send
  ld tmp
  bne #PAYLEN, .lost
  mov #0, idx
  mov #0, chk
.get:
  call sio_recv
  bnz .lost
  ld b
  st tmp
  call sio_send
  ld idx
  add #buf
  st 0
  ld tmp
  st @R0
  add chk
  st chk
  inc idx
  ld idx
  bne #PAYLEN, .get
  call sio_recv
  bnz .lost
  ld b
  st tmp
  call sio_send
  call sio_sent
  call pause
  ld tmp
  sub chk
  bnz .bad
  inc recv_count
  call save_write
  mov #SIO_ACK, acc
  call sio_send
  call sio_sent
  call clk_fast
  call draw_counts
  mov #<s_gotok, trl
  mov #>s_gotok, trh
  jmp status
.bad:
  mov #SIO_NAK, acc
  call sio_send
  call sio_sent
  call clk_fast
  mov #<s_baddata, trl
  mov #>s_baddata, trh
  jmp status
.lost:
  call clk_fast
  mov #<s_lost, trl
  mov #>s_lost, trh
  jmp status

;; pause: about 60 ms at the crystal clock (the other side gets ready to listen)
pause:
  push acc
  mov #40, acc
.l:
  dec acc
  ld acc
  bnz .l
  pop acc
  ret

;; save_load: the counts from this file's SAVE page (the program reads its own flash)
save_load:
  mov #<save_page, trl
  mov #>save_page, trh
  mov #16, acc
  ldc
  st sent_count
  mov #17, acc
  ldc
  st recv_count
  mov #18, acc
  ldc
  st boot_count
  ret

;; save_write: the SAVE page = the magic, the counts, the last packet; written by the BIOS (interrupts off, RC 1/6)
save_write:
  push acc
  push c
  mov #<save_page, trl
  mov #>save_page, trh
  mov #0, c
.magic:
  ld c
  ldc
  push acc
  ld c
  add #FMBUFF
  st 0
  pop acc
  st @R0
  inc c
  ld c
  bne #16, .magic
  ld sent_count
  st FMBUFF + 16
  ld recv_count
  st FMBUFF + 17
  ld boot_count
  st FMBUFF + 18
  mov #FMBUFF + 19, 0
.zero:
  mov #0, acc
  st @R0
  inc 0
  ld 0
  bnz .zero                 ; to the page's end ($ff + 1 = 0)
  mov #0, c
.copy:
  ld c
  add #buf
  st 0
  ld @R0
  push acc
  ld c
  add #FMBUFF + 32
  st 0
  pop acc
  st @R0
  inc c
  ld c
  bne #PAYLEN, .copy        ; the last packet at +32
  mov #0, FMBANK
  mov #>save_page, FMADD_H
  mov #<save_page, FMADD_L
  push ocr
  push ie
  clr1 ie, 7
  mov #OSC_RCFW, ocr
  call fm_wrt_ex
  pop ie
  pop ocr
  pop c
  pop acc
  ret

;; clk_fast: the internal RC clock (1/12) for drawing; clk_slow: the 32 kHz crystal for idling and the serial link
clk_fast:
  mov #OSC_RC, ocr
  ret

clk_slow:
  mov #OSC_XT, ocr
  ret

batt_off:
  push psw
  clr1 psw, 1
  mov #$ff, acc
  st LOW_BATT_CHK
  pop psw
  ret

batt_on:
  push psw
  clr1 psw, 1
  mov #0, acc
  st LOW_BATT_CHK
  pop psw
  ret

;; status: trh:trl = a string for line 3
status:
  push b
  push c
  push trl
  push trh
  mov #3, b
  call lcd_clear_line
  pop trh
  pop trl
  mov #0, c
  call lcd_text
  pop c
  pop b
  ret

draw_counts:
  mov #<s_sent, trl
  mov #>s_sent, trh
  mov #0, c
  mov #1, b
  call lcd_text
  ld sent_count
  call lcd_num
  mov #<s_got, trl
  mov #>s_got, trh
  mov #0, c
  mov #2, b
  call lcd_text
  ld recv_count
  call lcd_num
  ret

draw_speed:
  mov #4, b
  call lcd_clear_line
  mov #<s_speed, trl
  mov #>s_speed, trh
  mov #0, c
  call lcd_text
  ld speed_idx
  add #49
  call lcd_char
  ret

int_base_timer:
  push psw
  set1 psw, 1
  inc ticks
  pop psw
  clr1 btcr, 1
  ret

s_title:
  .string 12 "LINK TEST 1"
s_sent:
  .string 6 "SENT "
s_got:
  .string 6 "GOT  "
s_nocable:
  .string 9 "NO CABLE"
s_cable:
  .string 9 "CABLE OK"
s_sending:
  .string 11 "SENDING..."
s_recv:
  .string 10 "RECEIVING"
s_sentok:
  .string 9 "SENT OK!"
s_gotok:
  .string 11 "GOT IT OK!"
s_noans:
  .string 10 "NO ANSWER"
s_echo:
  .string 11 "ECHO WRONG"
s_noack:
  .string 7 "NO ACK"
s_nak:
  .string 13 "THEY GOT BAD"
s_baddata:
  .string 9 "BAD DATA"
s_lost:
  .string 8 "LOST IT"
s_speed:
  .string 7 "SPEED "

  .include "../inc/sio.inc"
  .include "../inc/lcd.inc"
  .include "font.inc"

  ;; The SAVE page: 128 bytes on a 128-byte boundary in this file, written by save_write.
  .cnop 0, $80
save_page:
  .byte "MOEMONLINKTEST01"
  .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

  .cnop 0, $200
