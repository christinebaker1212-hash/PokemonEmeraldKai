;; Moemon VMU pet: the 48x32 screen. XRAM: xbnk 0 holds lines 0-15, xbnk 1 lines 16-31; within a bank each pair of
;; lines takes 16 bytes ($180 + pair * 16: the even line's 6 bytes, the odd line's 6, 4 unused). Bit 7 is the left
;; pixel. Writes go through @R2 (address $100 + r2).
;; Needs (RAM bank 1): lcd_t0 .. lcd_t5 (six bytes of scratch), and font_3x5 (gen_font.py).

;; lcd_clear: every pixel off. Only the bytes that exist: in each 16 the last 4 (low digit C-F) aren't there (Sega's
;; simulator manual, "XRAM> ..."): writing them on a real VMU breaks the screen up.
lcd_clear:
  push acc
  mov #0, xbnk
.bank:
  mov #$80, 2
.byte:
  ld 2
  and #$0f
  sub #$0c
  bp psw, 7, .real          ; low digit 0-B
  ld 2
  add #4                    ; skip C-F
  st 2
  br .next
.real:
  mov #0, acc
  st @R2
  inc 2
.next:
  ld 2
  bne #$00, .byte           ; to $FF (+1 wraps to 0)
  ld xbnk
  bnz .done
  mov #1, xbnk
  br .bank
.done:
  pop acc
  ret

;; lcd_char: acc = an ASCII character, c = column 0-11, b = text line 0-4 (6 pixels a line). Past column 11:
;; nothing (it would land in the next line's bytes). Calls lcd_hook first (the program's: e.g. the buttons sampled while
;; drawing, so a short press during a redraw isn't missed).
lcd_char:
  callf lcd_hook
  push acc
  ld c
  sub #12
  bp psw, 7, .incol
  pop acc
  ret
.incol:
  pop acc
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
  callf lcd_char
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
  callf lcd_char
  inc c
  pop lcd_t3
  pop lcd_t1
  push lcd_t3
  ld lcd_t1
  add #48
  callf lcd_char
  inc c
  pop lcd_t3
  ld lcd_t3
  add #48
  callf lcd_char
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
  callf lcd_char
  inc c
  ld c
  bne #12, .cl
  pop c
  pop acc
  ret

;; lcd_hex: acc = a byte drawn as 2 hex digits at c, b. Leaves c after it.
lcd_hex:
  push acc
  push acc
  ror
  ror
  ror
  ror
  call .digit
  pop acc
  call .digit
  pop acc
  ret
.digit:
  and #$0f
  push acc
  sub #10
  pop acc
  bp psw, 7, .num
  add #55                   ; 'A' - 10
  br .put
.num:
  add #48
.put:
  callf lcd_char
  inc c
  ret

;; lcd_row: acc = a pixel row 0-31 -> xbnk set, r2 = the XRAM address (low byte) of the row's first byte
lcd_row:
  st lcd_t5
  mov #0, xbnk
  bn lcd_t5, 4, .b0
  mov #1, xbnk
.b0:
  ld lcd_t5
  and #$0e
  rol
  rol
  rol
  add #$80
  bn lcd_t5, 0, .even
  add #6
.even:
  st 2
  ret

;; lcd_clear_rows: acc = the first pixel row, b = how many: those rows blank
lcd_clear_rows:
  push acc
  push b
  st lcd_t4
.row:
  ld lcd_t4
  callf lcd_row
  mov #6, lcd_t3
.byte:
  mov #0, acc
  st @R2
  inc 2
  dec lcd_t3
  ld lcd_t3
  bnz .byte
  inc lcd_t4
  dec b
  ld b
  bnz .row
  pop b
  pop acc
  ret

;; lcd_blit24: trh:trl = a sprite (a row count, then 3 bytes a row), acc = its top pixel row, c = the byte column
;; (0-3) its 24 pixels start at. The sprite's bytes replace what's there.
lcd_blit24:
  push b
  st lcd_t2                 ; the row
  mov #0, acc
  ldc
  st lcd_t1                 ; rows left
  mov #1, lcd_t0            ; the data index
.row:
  ld lcd_t1
  bz .done
  ld lcd_t2
  callf lcd_row
  ld 2
  add c
  st 2
  mov #3, b
.byte:
  ld lcd_t0
  ldc
  st @R2
  inc 2
  inc lcd_t0
  dec b
  ld b
  bnz .byte
  inc lcd_t2
  dec lcd_t1
  br .row
.done:
  pop b
  ret

;; lcd_blit8: trh:trl = 8 bytes (an 8x8 icon), acc = its top pixel row, c = the byte column (0-5)
lcd_blit8:
  push b
  st lcd_t2
  mov #0, lcd_t0
.row:
  ld lcd_t2
  callf lcd_row
  ld 2
  add c
  st 2
  ld lcd_t0
  ldc
  st @R2
  inc lcd_t2
  inc lcd_t0
  ld lcd_t0
  bne #8, .row
  pop b
  ret

;; lcd_text_at: like lcd_text but trh:trl, c, b saved (handy for one-line calls)

;; lcd_blit24m: like lcd_blit24, mirrored (the sprite faces the other way): each row's 3 bytes in reverse order, each
;; byte's bits reversed (rev_bits: a 256-byte table the program includes)
lcd_blit24m:
  push b
  st lcd_t2
  mov #0, acc
  ldc
  st lcd_t1
  mov #1, lcd_t0
.row:
  ld lcd_t1
  bz .done
  ld lcd_t2
  callf lcd_row
  ld 2
  add c
  add #2                    ; from the right byte leftwards
  st 2
  mov #3, b
.byte:
  ld lcd_t0
  ldc
  push trl
  push trh
  st lcd_t3
  mov #<rev_bits, trl
  mov #>rev_bits, trh
  ld lcd_t3
  ldc
  pop trh
  pop trl
  st @R2
  dec 2
  inc lcd_t0
  dec b
  ld b
  bnz .byte
  inc lcd_t2
  dec lcd_t1
  br .row
.done:
  pop b
  ret
