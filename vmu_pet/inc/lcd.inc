;; Moemon VMU pet: the 48x32 screen. XRAM: xbnk 0 holds lines 0-15, xbnk 1 lines 16-31; within a bank each pair of
;; lines takes 16 bytes ($180 + pair * 16: the even line's 6 bytes, the odd line's 6, 4 unused). Bit 7 is the left
;; pixel. Writes go through @R2 (address $100 + r2).
;; Needs (RAM bank 1): lcd_t0 .. lcd_t5 (six bytes of scratch), and font_3x5 (gen_font.py).

;; lcd_clear: every pixel off
lcd_clear:
  push acc
  mov #0, xbnk
.bank:
  mov #$80, 2
.byte:
  mov #0, acc
  st @R2
  inc 2
  ld 2
  bne #$fc, .byte
  ld xbnk
  bnz .done
  mov #1, xbnk
  br .bank
.done:
  pop acc
  ret

;; lcd_char: acc = an ASCII character, c = column 0-11, b = text line 0-4 (6 pixels a line)
lcd_char:
  push acc
  push b
  push c
  sub #32
  bp acc, 7, .space         ; below 32: draw a space
  bne #64, .chk
.chk:
  bp psw, 7, .ok            ; < 64 (32-95)
  sub #32                   ; lowercase drawn as uppercase
  br .ok
.space:
  mov #0, acc
.ok:
  st lcd_t0                 ; glyph index
  mov #<font_3x5, trl
  mov #>font_3x5, trh
  mov #5, lcd_t1
.mul5:
  ld trl
  add lcd_t0
  st trl
  ld trh
  addc #0
  st trh
  dec lcd_t1
  ld lcd_t1
  bnz .mul5
  ld b                      ; first pixel line = text line * 6
  rol
  st lcd_t2
  rol
  add lcd_t2
  st lcd_t2
  mov #$0f, lcd_t4          ; mask keeping the other half of the byte
  ld c
  and #1
  bz .even
  mov #$f0, lcd_t4
.even:
  mov #0, lcd_t1            ; glyph row
.row:
  ld lcd_t1
  ldc
  st lcd_t3
  ld c
  and #1
  bz .noshift
  ld lcd_t3
  ror
  ror
  ror
  ror
  and #$0f
  st lcd_t3
.noshift:
  ld lcd_t2
  add lcd_t1
  st lcd_t5                 ; the pixel line
  mov #0, xbnk
  bn lcd_t5, 4, .bank0
  mov #1, xbnk
.bank0:
  ld lcd_t5
  and #$0e
  rol
  rol
  rol
  add #$80
  st 2
  bn lcd_t5, 0, .evenline
  ld 2
  add #6
  st 2
.evenline:
  ld c
  ror
  and #$7f
  add 2
  st 2
  ld @R2
  and lcd_t4
  or lcd_t3
  st @R2
  inc lcd_t1
  ld lcd_t1
  bne #5, .row
  pop c
  pop b
  pop acc
  ret

;; lcd_text: trh:trl = a 0-terminated string in the program, c = column, b = line. Leaves c after the last letter.
lcd_text:
  push acc
  mov #0, lcd_t5
.next:
  ld trl
  push acc
  ld trh
  push acc
  ld lcd_t5
  ldc
  st lcd_t3
  pop acc
  st trh
  pop acc
  st trl
  ld lcd_t3
  bz .end
  push trl
  push trh
  push lcd_t5
  call lcd_char
  pop lcd_t5
  pop trh
  pop trl
  inc c
  inc lcd_t5
  br .next
.end:
  pop acc
  ret

;; lcd_num: acc = 0-255 drawn as 3 digits at c, b. Leaves c after it.
lcd_num:
  push acc
  st lcd_t3
  mov #0, lcd_t0            ; hundreds
.h:
  ld lcd_t3
  sub #100
  bp psw, 7, .hd
  st lcd_t3
  inc lcd_t0
  br .h
.hd:
  mov #0, lcd_t1            ; tens
.t:
  ld lcd_t3
  sub #10
  bp psw, 7, .td
  st lcd_t3
  inc lcd_t1
  br .t
.td:
  push lcd_t1
  push lcd_t3
  ld lcd_t0
  add #48
  call lcd_char
  inc c
  pop lcd_t3
  pop lcd_t1
  push lcd_t3
  ld lcd_t1
  add #48
  call lcd_char
  inc c
  pop lcd_t3
  ld lcd_t3
  add #48
  call lcd_char
  inc c
  pop acc
  ret

;; lcd_clear_line: b = text line, blanks its 12 cells
lcd_clear_line:
  push acc
  push c
  mov #0, c
.cl:
  mov #32, acc
  call lcd_char
  inc c
  ld c
  bne #12, .cl
  pop c
  pop acc
  ret
