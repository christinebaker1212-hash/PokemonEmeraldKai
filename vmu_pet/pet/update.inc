;; Moemon VMU pet: the UPDATE receiver, at the end of the file ($BC00-$BFFF), which an update never rewrites (the
;; user, 2026-11-21: "the VMU a slave program to the VM2, the main transfer program"). The VM2's pet (LINK > UPDATE,
;; link_update) sends its program page by page; this writes each into the VMU's own file and checks it.
;;   Self-contained: it calls nothing of the program (being rewritten), only the BIOS's flash write through
;;   fm_wrt_ex ($100, the same in every version), with interrupts off the whole time.
;;   Page 0 (the reset vector) is pointed here first and is the last page sent: a cut cable or a flat battery leaves a
;;   VMU that starts straight into this receiver, waiting for the VM2 to try again (MODE goes back to the VMU's menus).
;;   The pages: 0 to UPD_PAGES - 1 ($0000-$BB7F: the program and its version page). The SAVE page ($BB80) and this
;;   receiver are never sent.
;; From the VM2, every byte echoed back (as all the pet's links):
;;   'U'                                     the call
;;   'P', n low, n high, 128 bytes, sum      a page (sum: of n's two bytes and the 128); answered ACK or NAK
;;   'E'                                     the end (after page 0): answered ACK; the VMU starts the new version

UPD_PAGES  = $177           ; 375 pages: $0000-$BB7F
UPD_ADDR   = $bc00

UR_NL      = $10            ; RAM bank 1 (the program's RAM: it's being replaced anyway)
UR_NH      = $11
UR_SUM     = $12
UR_I       = $13
UR_T0      = $14
UR_T1      = $15
UR_COL     = $16            ; the bar: how far it is (pixels, 0-39)
UR_CNT     = $17            ; pages since the bar last grew

  .org $bc00
;; update_receive: from the pet's main loop, when the VM2's 'U' has come (in b; not yet echoed). Never returns.
update_receive:
  clr1 ie, 7
  set1 psw, 1
  mov #VCCR_RC, vccr
  mov #OSC_RC, ocr
  ld b
  push acc
  callf ur_screen
  pop acc
  callf ur_send             ; the call's echo
  callf ur_mark             ; page 0 starts here now, until the update is whole
  jmpf ur_pages

;; update_resume: the program started while an update was unfinished (page 0 points here): wait for the VM2
update_resume:
  clr1 ie, 7
  set1 psw, 1
  mov #VCCR_RC, vccr
  mov #OSC_RC, ocr
  mov #0, p3int
  callf ur_sio_init
  callf ur_screen
ur_wait:
  ld p3
  bn acc, BTN_MODE, .menus
  callf ur_poll
  bnz ur_wait
  ld b
  bne #$55, ur_wait         ; 'U'
  callf ur_send
  jmpf ur_pages
.menus:
  mov #0, scon0             ; the port as the BIOS wants it, then the VMU's menus (the next start comes back here)
  mov #0, scon1
  mov #$bf, p1fcr
  mov #$a4, p1ddr
  jmpf game_end

ur_pages:
  mov #0, UR_COL
  mov #0, UR_CNT
.next:
  callf ur_recv
  bnzo .fail
  ld b
  be #$50, .page            ; 'P'
  beo #$45, .end             ; 'E'
  br .next
.page:
  callf ur_send
  callf ur_recv
  bnzo .fail
  ld b
  st UR_NL
  st UR_SUM
  callf ur_send
  callf ur_recv
  bnzo .fail
  ld b
  st UR_NH
  add UR_SUM
  st UR_SUM
  ld b
  callf ur_send
  mov #FMBUFF, 0            ; the 128 bytes straight into the BIOS's page buffer
.byte:
  callf ur_recv
  bnzo .fail
  ld b
  st @R0
  add UR_SUM
  st UR_SUM
  ld b
  callf ur_send
  inc 0
  ld 0
  bnz .byte
  callf ur_recv             ; the sum
  bnzo .fail
  ld b
  callf ur_send
  ld b
  bne UR_SUM, .nak
  ; a page of the program? (n < UPD_PAGES: never the SAVE page or this receiver)
  ld UR_NH
  sub #>UPD_PAGES
  bn psw, 7, .hi
  br .inrange
.hi:
  bnz .nak
  ld UR_NL
  sub #<UPD_PAGES
  bn psw, 7, .nak
.inrange:
  ; its address: n * 128
  ld UR_NL
  ror
  and #$7f
  st UR_T0
  ld UR_NH
  ror
  and #$80
  or UR_T0
  st FMADD_H
  mov #0, FMADD_L
  bn UR_NL, 0, .even
  mov #$80, FMADD_L
.even:
  mov #0, FMBANK
  mov #OSC_RCFW, ocr
  callf fm_wrt_ex
  mov #OSC_RC, ocr
  ; read back
  ld FMADD_H
  st trh
  ld FMADD_L
  st trl
  mov #0, UR_I
.chk:
  ld UR_I
  ldc
  st UR_T0
  ld UR_I
  add #FMBUFF
  st 0
  ld @R0
  bne UR_T0, .nak
  inc UR_I
  ld UR_I
  bne #$80, .chk
  mov #$06, acc             ; ACK
  callf ur_send
  callf ur_bar
  jmpf .next
.nak:
  mov #$15, acc             ; NAK: the VM2 sends it again
  callf ur_send
  jmpf .next
.end:
  callf ur_send
  mov #$06, acc
  callf ur_send
  callf ur_sent
  mov #0, scon0
  mov #0, scon1
  mov #$bf, p1fcr
  mov #$a4, p1ddr
  jmpf $0000                ; the new version, from its start
.fail:
  ; nothing for a while: say so, and wait for the VM2 to call again
  mov #<ur_w_again, trl
  mov #>ur_w_again, trh
  callf ur_word
  jmpf ur_wait

;; ur_mark: page 0 with its first instruction a jump here (update_resume), so an unfinished update comes back
ur_mark:
  mov #0, trl
  mov #0, trh
  mov #0, UR_I
.copy:
  ld UR_I
  ldc
  push acc
  ld UR_I
  add #FMBUFF
  st 0
  pop acc
  st @R0
  inc UR_I
  ld UR_I
  bne #$80, .copy
  mov #<ur_resume_jump, trl
  mov #>ur_resume_jump, trh
  mov #0, UR_I
.jump:
  ld UR_I
  ldc
  push acc
  ld UR_I
  add #FMBUFF
  st 0
  pop acc
  st @R0
  inc UR_I
  ld UR_I
  bne #3, .jump
  mov #0, FMBANK
  mov #0, FMADD_H
  mov #0, FMADD_L
  mov #OSC_RCFW, ocr
  callf fm_wrt_ex
  mov #OSC_RC, ocr
  ret

ur_resume_jump:
  jmpf update_resume

;; ---- the serial port (the program's sio.inc, on its own: no clock interrupt here, so waits are counted)

ur_sio_init:
  mov #0, scon0
  mov #0, scon1
  mov #$fe, sbr
  clr1 p1, 0
  clr1 p1, 2
  clr1 p1, 3
  mov #%00000101, p1fcr
  mov #%00000101, p1ddr
  mov #0, sbuf0
  mov #0, sbuf1
  set1 scon1, 3
  ret

ur_send:
  push acc
.wait:
  ld scon0
  bp acc, 3, .wait
  pop acc
  st sbuf0
  set1 scon0, 3
  ret

ur_sent:
  push acc
.wait:
  ld scon0
  bp acc, 3, .wait
  pop acc
  ret

;; ur_poll: acc = 0 and b = the byte when one has come, else acc = $ff
ur_poll:
  ld scon1
  bp acc, 1, .got
  bp acc, 3, .none
  set1 scon1, 3
.none:
  mov #$ff, acc
  ret
.got:
  ld sbuf1
  st b
  clr1 scon1, 1
  clr1 scon1, 6
  set1 scon1, 3
  mov #0, acc
  ret

;; ur_recv: wait a few seconds for a byte: acc = 0 and b = it, or acc = $ff
ur_recv:
  mov #0, UR_T0
  mov #120, UR_T1
.loop:
  callf ur_poll
  bz .done
  dec UR_T0
  ld UR_T0
  bnz .loop
  dec UR_T1
  ld UR_T1
  bnz .loop
  mov #$ff, acc
.done:
  ret

;; ---- the screen: UPDATE, and a bar of 40 pixels (rows 14-17) that fills as the pages come

ur_screen:
  mov #0, xbnk
.bank:
  mov #$80, 2
.b:
  ld 2
  and #$0f
  sub #$0c
  bp psw, 7, .real
  ld 2
  add #4
  st 2
  br .nx
.real:
  mov #0, acc
  st @R2
  inc 2
.nx:
  ld 2
  bnz .b
  ld xbnk
  bnz .words
  mov #1, xbnk
  br .bank
.words:
  mov #<ur_w_update, trl
  mov #>ur_w_update, trh
  callf ur_word
  ; the bar's frame: rows 13 and 18 across columns 4-43, its ends at columns 3 and 44
  mov #13, acc
  callf ur_row
  callf ur_frame_line
  mov #18, acc
  callf ur_row
  callf ur_frame_line
  mov #14, UR_I
.ends:
  ld UR_I
  callf ur_row
  ld @R2
  or #$10                   ; column 3
  st @R2
  ld 2
  add #5
  st 2
  ld @R2
  or #$08                   ; column 44
  st @R2
  inc UR_I
  ld UR_I
  bne #18, .ends
  ret

ur_frame_line:
  mov #$0f, acc             ; columns 4-7
  st @R2
  inc 2
  mov #$ff, acc
  st @R2
  inc 2
  st @R2
  inc 2
  st @R2
  inc 2
  st @R2
  inc 2
  mov #$f0, acc             ; columns 40-43
  st @R2
  ret

;; ur_word: trh:trl = 5 rows of 6 bytes: drawn on rows 4-8
ur_word:
  mov #0, UR_I
.r:
  ld UR_I
  add #4
  callf ur_row
  mov #0, UR_T0
.c:
  ld UR_I
  st UR_T1
  rol
  add UR_T1
  rol                       ; the row * 6
  add UR_T0
  ldc
  st @R2
  inc 2
  inc UR_T0
  ld UR_T0
  bne #6, .c
  inc UR_I
  ld UR_I
  bne #5, .r
  ret

;; ur_row: acc = a pixel row 0-31 -> xbnk and r2 at the row's first byte
ur_row:
  st UR_T1
  mov #0, xbnk
  bn UR_T1, 4, .b0
  mov #1, xbnk
.b0:
  ld UR_T1
  and #$0e
  rol
  rol
  rol
  add #$80
  bn UR_T1, 0, .even
  add #6
.even:
  st 2
  ret

;; ur_bar: one more page: every 9 pages the bar grows a pixel (375 pages fill its 40)
ur_bar:
  inc UR_CNT
  ld UR_CNT
  bne #9, .r
  mov #0, UR_CNT
  ld UR_COL
  be #40, .r
  ; pixel column 4 + UR_COL on rows 14-17
  add #4
  st UR_T0
  mov #14, UR_I
.row:
  ld UR_I
  callf ur_row
  ld UR_T0
  ror
  ror
  ror
  and #$1f
  add 2
  st 2
  ld UR_T0
  and #7
  st UR_T1
  mov #$80, acc
.sh:
  dec UR_T1
  bp UR_T1, 7, .put
  ror
  br .sh
.put:
  or @R2
  st @R2
  inc UR_I
  ld UR_I
  bne #18, .row
  inc UR_COL
.r:
  ret

ur_w_update:
  .byte $ac, $c4, $ee, $00, $00, $00
  .byte $aa, $aa, $48, $00, $00, $00
  .byte $ac, $ae, $4c, $00, $00, $00
  .byte $a8, $aa, $48, $00, $00, $00
  .byte $e8, $ca, $4e, $00, $00, $00
ur_w_again:
  .byte $46, $4e, $cc, $00, $00, $00
  .byte $a8, $a4, $a2, $00, $00, $00
  .byte $ea, $e4, $a4, $00, $00, $00
  .byte $aa, $a4, $a0, $00, $00, $00
  .byte $a6, $ae, $a4, $00, $00, $00
update_end:
