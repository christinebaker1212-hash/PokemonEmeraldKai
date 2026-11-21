;; Moemon VMU pet: LINK, the VM2's (the user, 2026-11-21: "the VM2 will act as the main program... one program, with
;; two functions. The Link function will only work if it's a VM2. Otherwise, it acts as normal."). One program: on a
;; VM2 (vm2_check: the mark Kai or tools/vm2mark.py leaves in its SAVE page) MENU > LINK calls the VMU on top; on a VMU
;; there's no LINK, and the pet answers the VM2's calls by itself while the two are joined (slave_poll, from the main
;; loop). Every byte goes both ways as in sio.inc (each echoed). The calls:
;;   SWAP   ('S') the two MOeMON change places: the VMU sends its own (the VM2 keeps it in its SAVE page's tray), then
;;          the VM2 sends its own ('G'); the VMU takes it and says ACK, and only then the VM2 takes the tray's. A cable
;;          pulled at any point loses nothing (at worst one MOeMON is on both: Kai takes each only once).
;;   UPDATE ('U') this program to the VMU (link_update here, pet/update.inc there)
;;   BATTLE ('b') battle.inc's cards, then the same battle on both

;; vm2_check: P_UNIT bit 0 = this is a VM2: the SAVE page's $78-$7b say "VM2M" and its $7c-$7f are this unit's own
;; root block's $33-$36 (when it was formatted: a copy of the program on another VMU doesn't match)
vm2_check:
  mov #0, P_UNIT
  mov #0, idx
.mark:
  mov #<save_page, trl
  mov #>save_page, trh
  ld idx
  add #$78
  ldc
  st tmp0
  mov #<vm2_magic, trl
  mov #>vm2_magic, trh
  ld idx
  ldc
  bne tmp0, .no
  inc idx
  ld idx
  bne #4, .mark
  mov #0, idx
.root:
  mov #<save_page, trl
  mov #>save_page, trh
  ld idx
  add #$7c
  ldc
  st tmp0
  mov #1, fpr               ; block 255: bank 1 $fe00
  mov #$fe, trh
  ld idx
  add #$33
  st trl
  ldf
  mov #0, fpr
  bne tmp0, .no
  inc idx
  ld idx
  bne #4, .root
  set1 P_UNIT, 0
.no:
  mov #0, fpr
  ret

vm2_magic:
  .text 4 "VM2M"

;; slave_poll (a VMU, from the main loop): while a VMU-like unit is joined on top, the port listens; a call is answered
slave_poll:
  bp P_UNIT, 0, .r          ; the VM2 calls, it doesn't answer
  callf sio_connected
  bz .on
  bn P_UNIT, 1, .r
  clr1 P_UNIT, 1
  callf sio_end
.r:
  ret
.on:
  bp P_UNIT, 1, .poll
  set1 P_UNIT, 1
  callf sio_init
.poll:
  callf sio_poll
  bnz .r
  ld b
  be #$55, .update          ; 'U'
  be #$53, .swap            ; 'S'
  be #B_CARD_SYNC, .battle
  ret
.update:
  callf save_write          ; what's on the VMU is kept (the SAVE page is never sent)
  jmpf update_receive
.swap:
  jmpf swap_answer
.battle:
  jmpf battle_answer

;; link_call: acc = a call; sent until the VMU echoes it (it answers between its naps: about 10 tries) -> acc 0 ok
link_call:
  st tmp3
  mov #10, chk
.try:
  ld tmp3
  callf sio_xfer
  bz .r
  dec chk
  ld chk
  bnz .try
  mov #1, acc
.r:
  ret

;; link_start: the cable there? "WORD" on the LINK screen, the port on, the call answered -> acc 0, else said why
link_start:
  push acc
  callf sio_connected
  bz .cable
  pop acc
  mov #<s_nocable, trl
  mov #>s_nocable, trh
  callf link_status
  mov #1, acc
  ret
.cable:
  mov #<s_linking, trl
  mov #>s_linking, trh
  callf link_status
  callf sio_init
  callf sio_flush
  pop acc
  callf link_call
  bz .r
  callf link_fail
  mov #1, acc
.r:
  ret

link_fail:
  callf sio_end
  mov #<s_link_failed, trl
  mov #>s_link_failed, trh
  jmpf link_status

;; ---- SWAP

;; link_swap (the VM2)
link_swap:
  mov #$53, acc
  callf link_start
  bz .go
  ret
.go:
  ; 1: the VMU's MOeMON into the tray (the SAVE page's $4c on: what's in flash after it, the mark, is kept)
  callf save_keep_tail
  callf sio_recv
  bnzo .fail
  ld b
  st tmp0
  callf sio_send
  ld tmp0
  bneo #PET_BYTES, .fail
  mov #0, tmp2
  mov #FMBUFF + $4c, idx
.get:
  callf sio_recv
  bnzo .fail
  ld b
  st tmp0
  callf sio_send
  ld idx
  st 0
  ld tmp0
  st @R0
  add tmp2
  st tmp2
  inc idx
  ld idx
  bne #FMBUFF + $4c + PET_BYTES, .get
  callf sio_recv
  bnzo .fail
  ld b
  st tmp0
  callf sio_send
  callf sio_sent
  ld tmp0
  bneo tmp2, .bad
  ld FMBUFF + $4c           ; a MOeMON's bytes ('P' first)
  bne #$50, .bad
  callf save_write_buf      ; kept in flash before anything else
  mov #SIO_ACK, acc
  callf sio_send
  callf sio_sent
  ; 2: ours to the VMU: 'G', the length, the bytes, their sum, then its ACK
  callf link_pause
  mov #$47, acc
  callf sio_xfer
  bnz .unsure
  mov #PET_BYTES, acc
  callf sio_xfer
  bnz .unsure
  mov #0, tmp2
  mov #P_MAGIC, idx
.give:
  ld idx
  st 0
  ld @R0
  push acc
  add tmp2
  st tmp2
  pop acc
  callf sio_xfer
  bnz .unsure
  inc idx
  ld idx
  bne #P_MAGIC + PET_BYTES, .give
  ld tmp2
  callf sio_xfer
  bnz .unsure
  callf sio_recv
  bnz .unsure
  ld b
  bne #SIO_ACK, .kept
  callf sio_end
  ; the VMU has ours: the tray's is ours now
  mov #0, idx
.take:
  mov #<save_page, trl
  mov #>save_page, trh
  ld idx
  add #$4c
  ldc
  push acc
  ld idx
  add #P_MAGIC
  st 0
  pop acc
  st @R0
  inc idx
  ld idx
  bne #PET_BYTES, .take
  callf save_write
  mov #<s_swapped, trl
  mov #>s_swapped, trh
  jmpf message
.bad:
  mov #SIO_NAK, acc
  callf sio_send
  callf sio_sent
.fail:
  jmpf link_fail
.kept:
  ; the VMU said no: nothing changed on either
  callf sio_end
  mov #<s_link_failed, trl
  mov #>s_link_failed, trh
  jmpf link_status
.unsure:
  ; it may or may not have taken ours: ours stays here too (one MOeMON on both at worst; Kai takes each once)
  callf sio_end
  mov #<s_check_vmu, trl
  mov #>s_check_vmu, trh
  jmpf link_status

;; swap_answer (the VMU; the VM2's 'S' is in b)
swap_answer:
  ld b
  callf sio_send            ; the call's echo
  ; 1: ours to the VM2 (nothing changes here unless 2 comes whole)
  mov #PET_BYTES, acc
  callf sio_xfer
  bnzo .fail
  mov #0, tmp2
  mov #P_MAGIC, idx
.give:
  ld idx
  st 0
  ld @R0
  push acc
  add tmp2
  st tmp2
  pop acc
  callf sio_xfer
  bnzo .fail
  inc idx
  ld idx
  bne #P_MAGIC + PET_BYTES, .give
  ld tmp2
  callf sio_xfer
  bnzo .fail
  callf sio_recv
  bnzo .fail
  ld b
  bneo #SIO_ACK, .fail
  ; 2: the VM2's: 'G', the length, the bytes into the page buffer, the sum
  callf sio_recv
  bnzo .fail
  ld b
  bneo #$47, .fail
  callf sio_send
  callf sio_recv
  bnzo .fail
  ld b
  st tmp0
  callf sio_send
  ld tmp0
  bneo #PET_BYTES, .fail
  mov #0, tmp2
  mov #FMBUFF, idx
.get:
  callf sio_recv
  bnzo .fail
  ld b
  st tmp0
  callf sio_send
  ld idx
  st 0
  ld tmp0
  st @R0
  add tmp2
  st tmp2
  inc idx
  ld idx
  bne #FMBUFF + PET_BYTES, .get
  callf sio_recv
  bnzo .fail
  ld b
  st tmp0
  callf sio_send
  callf sio_sent
  ld tmp0
  bne tmp2, .nak
  ld FMBUFF
  bne #$50, .nak
  ; taken. What left was grown? Then, as for any MOeMON sent on: the helpers and TAP UP start over, a SPARK a tap more
  ld P_STAGE
  bne #2, .nograd
  ld P_LEVEL
  bne #MAX_LEVEL, .nograd
  inc O_PRESTIGE
  mov #0, O_TAP
  mov #0, O_HELPER
.nograd:
  mov #0, idx
.copy:
  ld idx
  add #FMBUFF
  st 0
  ld @R0
  push acc
  ld idx
  add #P_MAGIC
  st 0
  pop acc
  st @R0
  inc idx
  ld idx
  bne #PET_BYTES, .copy
  ld P_STAGE
  bnz .has
  callf egg_new             ; the VM2 had none: a new Egg here
.has:
  callf save_write
  mov #SIO_ACK, acc
  callf sio_send
  callf sio_sent
  mov #0, msg_left
  callf lcd_clear
  mov #<s_swapped, trl
  mov #>s_swapped, trh
  jmpf message
.nak:
  mov #SIO_NAK, acc
  callf sio_send
  callf sio_sent
.fail:
  mov #<s_link_failed, trl
  mov #>s_link_failed, trh
  jmpf message

;; ---- UPDATE (the VM2): this program's pages 1 .. UPD_PAGES - 1, then page 0, to the VMU's receiver
link_update:
  mov #$55, acc
  callf link_start
  bz .go
  ret
.go:
  mov #<s_updating, trl
  mov #>s_updating, trh
  callf link_status
  mov #1, tmp0              ; the page (16 bits)
  mov #0, tmp1
.page:
  mov #5, tmp3              ; tries
.try:
  push tmp3
  callf lu_page
  pop tmp3
  bz .ok
  dec tmp3
  ld tmp3
  bnz .try
  jmpf link_fail
.ok:
  ld tmp0
  or tmp1
  bz .end                   ; page 0 was the last
  ; a # on line 3 every 32 pages
  ld tmp0
  and #$1f
  bnz .nobar
  ld tmp1
  rol
  rol
  rol
  and #$08
  st c
  ld tmp0
  ror
  ror
  ror
  ror
  ror
  and #$07
  or c
  st c
  mov #3, b
  mov #35, acc
  callf lcd_char
.nobar:
  inc tmp0
  ld tmp0
  bnz .wrap
  inc tmp1
.wrap:
  ld tmp1
  bne #>UPD_PAGES, .page
  ld tmp0
  bne #<UPD_PAGES, .page
  mov #0, tmp0              ; then page 0
  mov #0, tmp1
  br .page
.end:
  mov #$45, acc             ; 'E'
  callf sio_xfer
  bnz .failed
  callf sio_recv
  bnz .failed
  ld b
  bne #SIO_ACK, .failed
  callf sio_end
  mov #<s_updated, trl
  mov #>s_updated, trh
  jmpf message
.failed:
  jmpf link_fail

;; lu_page: page tmp1:tmp0 sent: 'P', its number, its 128 bytes (from our own flash), their sum -> acc 0 on ACK
lu_page:
  mov #$50, acc
  callf sio_xfer
  bnz .bad
  ld tmp0
  callf sio_xfer
  bnz .bad
  ld tmp1
  callf sio_xfer
  bnz .bad
  ld tmp0
  add tmp1
  st tmp2
  ld tmp0
  ror
  and #$7f
  st trh
  ld tmp1
  ror
  and #$80
  or trh
  st trh
  mov #0, trl
  bn tmp0, 0, .even
  mov #$80, trl
.even:
  mov #0, idx
.b:
  ld idx
  ldc
  push acc
  add tmp2
  st tmp2
  pop acc
  callf sio_xfer
  bnz .bad
  inc idx
  ld idx
  bne #$80, .b
  ld tmp2
  callf sio_xfer
  bnz .bad
  callf sio_recv
  bnz .bad
  ld b
  bne #SIO_ACK, .bad
  mov #0, acc
  ret
.bad:
  mov #1, acc
  ret

s_linking:
  .string 8 "CALLING"
s_swap:
  .string 5 "SWAP"
s_update:
  .string 7 "UPDATE"
s_swapped:
  .string 9 "SWAPPED!"
s_check_vmu:
  .string 10 "CHECK VMU"
s_updating:
  .string 9 "UPDATING"
s_updated:
  .string 9 "UPDATED!"
