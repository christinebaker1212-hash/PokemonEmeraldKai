;; Moemon VMU pet: the file icon. The colour icon in the program's own VMS header ($260 the palette, $280-$47f the
;; 32x32 picture, 4 bits a pixel) is what the Dreamcast's file menu, the VM2's GUI, the VMU manager and Kai show.
;;   an Egg      its design in its own colours (gen_edition.py icon_egg_*, ed_egg, ed_egg_rgb; tools/eggs.py)
;;   a MOeMON    its PMD portrait from the PICTURES file when this VMU has it (gen_icons.py: a data file found by its
;;               "MOEPICS" mark, wherever the BIOS put its blocks), else its LCD sprite in its Egg's outline colour
;; The header's reserved bytes $25e / $25f keep the stage and the MOeMON the icon shows: icon_check rewrites the five
;; header pages only when they differ (an Egg laid, hatched, evolved, received).
;; Needs: the pet's RAM names, save_write's way of writing a page (fm_wrt_ex), rev_bits.

ICV_O    = $36               ; the three masks' bytes being drawn (the digits' bytes: free while this runs)
ICV_I    = $37
ICV_P    = $38
ICV_J    = $39               ; the mask byte (0-3) / the picture byte (0-7)
ICV_Y    = $3a               ; the icon row (0-31)
ICV_OUT  = $3b               ; where the next byte goes in the page buffer
ICV_PIX  = $3c
;; lcd_t0 / lcd_t1 the pattern or sprite, lcd_t3 mirrored, lcd_t4 the kind (0 an Egg, 1 a sprite, 2 a picture),
;; lcd_t2 / lcd_t5 scratch; tmp1 the picture's block; tmp2 / tmp3 / idx / chk scratch

;; icon_check: the icon rewritten if it doesn't show this stage and MOeMON
icon_check:
  push acc
  push trl
  push trh
  mov #$5e, trl
  mov #$02, trh
  mov #0, acc
  ldc
  bne P_STAGE, .update
  mov #1, acc
  ldc
  bne P_SPECIES, .update
  pop trh
  pop trl
  pop acc
  ret
.update:
  pop trh
  pop trl
  pop acc
  jmpf icon_update

icon_update:
  push acc
  push b
  push c
  push trl
  push trh
  push tmp0
  push tmp1
  push tmp2
  push tmp3
  push idx
  push chk
  push ocr
  mov #OSC_RCFW, ocr        ; RC 1/6: twice as quick (still the RC clock: Sega's VCCR rule holds)
  mov #0, lcd_t4
  mov #0, lcd_t3
  ld P_STAGE
  bne #2, .egg
  callf pics_find
  bz .sprite
  st tmp0                   ; the PICTURES file's first block
  mov #2, lcd_t4
  br .page0
.sprite:
  mov #1, lcd_t4
  ld P_SPECIES
  rol
  st tmp2
  mov #<ed_sprite, trl
  mov #>ed_sprite, trh
  ld tmp2
  ldc
  st lcd_t0
  ld tmp2
  inc acc
  ldc
  st lcd_t1
  br .page0
.egg:
  ld P_SPECIES
  mov #<ed_egg, trl
  mov #>ed_egg, trh
  ldc
  st tmp2
  bn tmp2, 4, .plain
  mov #1, lcd_t3
.plain:
  ld tmp2
  and #$0f
  rol
  st tmp2
  mov #<icon_egg_pats, trl
  mov #>icon_egg_pats, trh
  ld tmp2
  ldc
  st lcd_t0
  ld tmp2
  inc acc
  ldc
  st lcd_t1

.page0:
  ; page $200: the header as it is up to $25d, the marks, the palette (4 colours, the rest 0)
  mov #$00, trl
  mov #$02, trh
  mov #0, idx
.hdr:
  ld idx
  ldc
  push acc
  ld idx
  add #FMBUFF
  st 0
  pop acc
  st @R0
  inc idx
  ld idx
  bne #$5e, .hdr
  mov #FMBUFF + $5e, 0
  ld P_STAGE
  st @R0
  inc 0
  ld P_SPECIES
  st @R0
  inc 0
.zero:
  mov #0, acc
  st @R0
  inc 0
  ld 0
  bnz .zero
  ld lcd_t4
  be #2, .picpal
  ; an Egg or a sprite: 0 clear, then the Egg's outline, shell and pattern (ed_egg_rgb: 6 bytes each)
  mov #<ed_egg_rgb, trl
  mov #>ed_egg_rgb, trh
  ld P_SPECIES
  st c
  mov #0, acc
  mov #6, b
  mul
  st tmp3
  ld c
  add trl
  st trl
  ld tmp3
  addc trh
  st trh
  mov #0, idx
.pal:
  ld idx
  ldc
  push acc
  ld idx
  add #FMBUFF + $62
  st 0
  pop acc
  st @R0
  inc idx
  ld idx
  bne #6, .pal
  br .write0
.picpal:
  ; the picture's palette: file block 2 + species / 64, at (species & 63) * 8
  ld P_SPECIES
  ror
  ror
  ror
  ror
  ror
  ror
  and #$03
  add #2
  callf pics_block
  callf blk_at
  ld P_SPECIES
  and #$1f
  rol
  rol
  rol
  st trl                    ; (species & 31) * 8
  bn P_SPECIES, 5, .lowhalf
  inc trh                   ; (species & 63) >= 32: the block's second 256 bytes
.lowhalf:
  mov #FMBUFF + $60, 0
.pp:
  ldf
  st @R0
  inc trl
  inc 0
  ld 0
  bne #FMBUFF + $68, .pp
  mov #0, fpr
  ; the picture itself: file block 5 + species / 2
  ld P_SPECIES
  ror
  and #$7f
  add #5
  callf pics_block
  st tmp1
.write0:
  mov #$02, FMADD_H
  mov #$00, FMADD_L
  callf icon_page

  ; the picture: four pages of 8 rows (16 bytes a row)
  mov #0, ICV_Y
.page:
  mov #FMBUFF, ICV_OUT
.row:
  callf ic_row
  inc ICV_Y
  ld ICV_Y
  and #7
  bnz .row
  ; this page's address: m = row / 8 (1-4) -> $280, $300, $380, $400
  ld ICV_Y
  ror
  ror
  ror
  and #$1f
  st tmp2
  mov #$00, FMADD_L
  bn tmp2, 0, .evenpage
  mov #$80, FMADD_L
.evenpage:
  ld tmp2
  ror
  and #$7f
  add #2
  st FMADD_H
  callf icon_page
  ld ICV_Y
  bne #32, .page
  pop ocr
  pop chk
  pop idx
  pop tmp3
  pop tmp2
  pop tmp1
  pop tmp0
  pop trh
  pop trl
  pop c
  pop b
  pop acc
  ret

;; icon_page: the page buffer to flash at FMADD_H:FMADD_L (bank 0), as save_write does it
icon_page:
  mov #0, FMBANK
  push ocr
  push ie
  clr1 ie, 7
  mov #OSC_RCFW, ocr
  callf fm_wrt_ex
  pop ie
  pop ocr
  ret

;; ic_row: icon row ICV_Y's 16 bytes into the page buffer at ICV_OUT
ic_row:
  ld lcd_t4
  be #2, .pic
  mov #0, ICV_J
.mj:
  callf ic_fetch
  mov #4, b
.two:
  callf ic_pix
  rol
  rol
  rol
  rol
  st ICV_PIX
  callf ic_pix
  or ICV_PIX
  callf ic_put
  dec b
  ld b
  bnz .two
  inc ICV_J
  ld ICV_J
  bne #4, .mj
  ret
.pic:
  ; 8 bytes at the picture's block + (species & 1) * 256 + row * 8, 2 bits a pixel -> 4
  ld tmp1
  callf blk_at
  bn P_SPECIES, 0, .even
  inc trh
.even:
  ld ICV_Y
  rol
  rol
  rol
  and #$f8
  st trl
  mov #0, ICV_J
.pb:
  ldf
  st ICV_O
  ror
  ror
  and #$30
  st ICV_PIX
  ld ICV_O
  ror
  ror
  ror
  ror
  and #$03
  or ICV_PIX
  callf ic_put
  ld ICV_O
  rol
  rol
  and #$30
  st ICV_PIX
  ld ICV_O
  and #$03
  or ICV_PIX
  callf ic_put
  inc trl
  inc ICV_J
  ld ICV_J
  bne #8, .pb
  mov #0, fpr
  ret

;; ic_put: acc into the page buffer at ICV_OUT
ic_put:
  push acc
  ld ICV_OUT
  st 0
  pop acc
  st @R0
  inc ICV_OUT
  ret

;; ic_pix: the next pixel's colour from the masks' top bits (outline 1, pattern 3, inside 2, else 0); shifts them
ic_pix:
  mov #0, chk
  bn ICV_I, 7, .a
  mov #2, chk
.a:
  bn ICV_P, 7, .b
  mov #3, chk
.b:
  bn ICV_O, 7, .c
  mov #1, chk
.c:
  ld ICV_O
  rol
  st ICV_O
  ld ICV_I
  rol
  st ICV_I
  ld ICV_P
  rol
  st ICV_P
  ld chk
  ret

;; ic_fetch: mask byte ICV_J of icon row ICV_Y into ICV_O / ICV_I / ICV_P
ic_fetch:
  ld lcd_t4
  bnz .sprite
  ; the Egg: icon rows 2-29 (28 rows of 4 bytes); mirrored: the bytes the other way round, each one's bits too
  ld ICV_Y
  sub #2
  bp psw, 7, .none
  sub #28
  bn psw, 7, .none
  ld ICV_Y
  sub #2
  rol
  rol
  st lcd_t2
  ld lcd_t3
  bz .fwd
  ld ICV_J
  xor #3
  br .off
.fwd:
  ld ICV_J
.off:
  add lcd_t2
  st lcd_t5
  mov #<icon_egg_ol, trl
  mov #>icon_egg_ol, trh
  ld lcd_t5
  ldc
  callf ic_mir
  st ICV_O
  mov #<icon_egg_in, trl
  mov #>icon_egg_in, trh
  ld lcd_t5
  ldc
  callf ic_mir
  st ICV_I
  ld lcd_t0
  st trl
  ld lcd_t1
  st trh
  ld lcd_t5
  ldc
  callf ic_mir
  st ICV_P
  ret
.none:
  mov #0, ICV_O
  mov #0, ICV_I
  mov #0, ICV_P
  ret
.sprite:
  ; the LCD sprite (h rows standing on icon row 27), 4 pixels in: icon columns 4-27, the outline colour
  mov #0, ICV_I
  mov #0, ICV_P
  ld lcd_t0
  st trl
  ld lcd_t1
  st trh
  mov #0, acc
  ldc
  st lcd_t2                 ; h
  mov #28, acc
  sub lcd_t2
  st lcd_t5                 ; its top row
  ld ICV_Y
  sub lcd_t5
  bp psw, 7, .none
  st lcd_t5                 ; its row
  sub lcd_t2
  bn psw, 7, .none
  ld lcd_t5
  rol
  add lcd_t5
  inc acc
  st lcd_t5                 ; 1 + row * 3
  mov #0, ICV_O
  ld ICV_J
  bz .right
  dec acc
  add lcd_t5
  ldc
  rol
  rol
  rol
  rol
  and #$f0
  st ICV_O
.right:
  ld ICV_J
  be #3, .done
  add lcd_t5
  ldc
  ror
  ror
  ror
  ror
  and #$0f
  or ICV_O
  st ICV_O
.done:
  ret

;; ic_mir: acc's bits reversed when the Egg is mirrored (lcd_t3)
ic_mir:
  push acc
  ld lcd_t3
  bz .no
  pop acc
  push trl
  push trh
  mov #<rev_bits, trl
  mov #>rev_bits, trh
  ldc
  pop trh
  pop trl
  ret
.no:
  pop acc
  ret

;; ---- the VMU's file system, read with LDF (blocks 128-255 are flash bank 1)

;; blk_at: acc = a block -> fpr, trh, trl at its first byte (acc kept)
blk_at:
  push acc
  mov #0, fpr
  bn acc, 7, .low
  mov #1, fpr
.low:
  rol
  and #$fe
  st trh
  mov #0, trl
  pop acc
  ret

;; fat_next: acc = a block -> acc = the next block of its file, b = the high byte ($ff: it was the last)
fat_next:
  st lcd_t2
  mov #1, fpr               ; the FAT: block 254 = bank 1 $fc00, 2 bytes a block
  rol
  and #$fe
  st trl
  mov #$fc, trh
  bn lcd_t2, 7, .low
  mov #$fd, trh
.low:
  inc trl
  ldf
  st b
  dec trl
  ldf
  push acc
  mov #0, fpr
  pop acc
  ret

;; pics_block: acc = a block of the PICTURES file (0 its first) -> acc = where it is (tmp0 = its first block)
pics_block:
  st tmp2
  ld tmp0
.step:
  push acc
  ld tmp2
  bz .here
  dec tmp2
  pop acc
  callf fat_next
  br .step
.here:
  pop acc
  ret

;; pics_find: the PICTURES file of this edition -> acc = its first block, or 0 (none: block 0 is always a GAME's).
;; The directory: blocks 253 down to 241, 16 entries of 32 bytes each (type $33 a data file, the first block at +2).
pics_find:
  mov #253, tmp2
.blk:
  mov #0, tmp3
.ent:
  ld tmp2
  callf blk_at
  bn tmp3, 3, .first8
  inc trh
.first8:
  ld tmp3
  and #$07
  rol
  rol
  rol
  rol
  rol
  st trl
  ldf
  bne #$33, .next
  inc trl
  inc trl
  ldf
  st idx
  callf pics_check
  bnz .found
.next:
  inc tmp3
  ld tmp3
  bne #16, .ent
  dec tmp2
  ld tmp2
  bne #240, .blk
  mov #0, fpr
  mov #0, acc
  ret
.found:
  mov #0, fpr
  ld idx
  ret

;; pics_check: is the data file starting at block idx this edition's PICTURES? ("MOEPICS", 1, the edition at its
;; $280: its second block's $80) -> acc 1 / 0
pics_check:
  ld idx
  callf fat_next
  push acc
  ld b
  be #$ff, .last
  pop acc
  callf blk_at
  mov #$80, trl
  mov #0, chk
.cmp:
  ldf
  st ICV_PIX
  push trl
  push trh
  mov #<pics_magic, trl
  mov #>pics_magic, trh
  ld chk
  ldc
  pop trh
  pop trl
  bne ICV_PIX, .no
  inc trl
  inc chk
  ld chk
  bne #9, .cmp
  mov #0, fpr
  mov #1, acc
  ret
.last:
  pop acc
.no:
  mov #0, fpr
  mov #0, acc
  ret

pics_magic:
  .text 7 "MOEPICS"
  .byte 1, 1                ; the format, the edition (1 = KANTO)
