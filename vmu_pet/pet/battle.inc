;; Moemon VMU pet: battles, only when the player starts one (docs/VMU_PET_DECISIONS.md): WILD on the VMU (MENU >
;; BATTLE) and VMU to VMU (LINK > BATTLE). Nothing is ever lost: a loss only means no prize.
;;   The two face each other, 3 HP each, taking turns. An attack hits by the attacker's power against the defender's
;;   (power = level + today's STRENGTH * 2 + IVs / 16 + 3 for no care mistakes) and its type against the other's
;;   (gen_edition.py type_chart): super effective hits for 2, not very effective hits less often.
;;   WILD: a MOeMON of the edition near its level; on its own turns the player stops a marker in ## with A for a sure
;;   hit. Prizes: SPARKS (the foe's level x 4) and, for the first 3 wins of a day, 2 EXP each.
;;   LINK (the VM2 calls the VMU on top: link.inc): both swap a card (edition, MOeMON, level, power, a seed) and play
;;   out the same battle from the shared seed, each seeing its own MOeMON on the left; the VM2 attacks first.
;; RAM while it runs (free then: the digits, the clock's scratch, the shop's): below.

B_MYHP   = $36
B_FOEHP  = $37
B_MYPOW  = $38
B_FOEPOW = $39
B_FOESP  = $3a
B_FOELV  = $3b
B_FLAGS  = $3c              ; bit 0 a LINK battle (no marker), bit 1 the foe attacks first
B_MYEFF  = $2d              ; my type against the foe: 0-3 (type_chart's codes, both its types counted)
B_FOEEFF = $2e
B_ANIM   = $2f              ; bit 0 me hidden, bit 1 the foe hidden, bit 2 me up, bit 3 the foe up
B_SURE   = $30              ; the player's marker landed: a sure hit
B_SEED   = $78              ; 2 bytes: the battle's own random numbers (shared on a LINK)
B_T      = $7a

B_HP_MAX        = 3             ; HP each: a short battle (two big hits end it)
B_CARD_SYNC = $62           ; 'b': a battle card starts
B_CARD_LEN  = 6

;; ---- MENU > BATTLE: a wild one
battle_wild:
  callf need_mon
  bz .go
  ret
.go:
  ld p3                     ; the A that chose BATTLE comes up first
  bn acc, BTN_A, .go
  ; the foe: any MOeMON of the edition, its level within 2 of ours (1-50), its power its level + 2-9
  callf rand
  ld O_RNG
  st B_SEED
  callf rand
  ld O_RNG
  st B_SEED + 1
  callf b_rand
.pick:
  callf b_rand
  and #$7f
  st tmp0
  callf b_rand
  and #$1f
  add tmp0
  sub #ED_COUNT
  bn psw, 7, .pick          ; 0-158: try again past the edition's last
  add #ED_COUNT
  st B_FOESP
.lvpick:
  callf b_rand
  and #$07
  sub #5
  bn psw, 7, .lvpick        ; 5-7: again
  add #5                    ; 0-4
  add P_LEVEL
  sub #2                    ; its level - 2 .. + 2
  bp psw, 7, .lv1
  bz .lv1
  st B_FOELV
  sub #MAX_LEVEL + 1
  bp psw, 7, .lvset
  mov #MAX_LEVEL, B_FOELV
  br .lvset
.lv1:
  mov #1, B_FOELV
.lvset:
  callf b_rand
  and #$07
  add #2
  add B_FOELV
  st B_FOEPOW
  callf b_my_power
  st B_MYPOW
  mov #0, B_FLAGS
  ; "A WILD / PIDGEY / APPEARED!"
  callf lcd_clear
  mov #<s_b_wild, trl
  mov #>s_b_wild, trh
  mov #0, c
  mov #0, b
  callf lcd_text
  callf b_foe_name
  mov #0, c
  mov #1, b
  callf lcd_text
  mov #<s_b_appeared, trl
  mov #>s_b_appeared, trh
  mov #0, c
  mov #2, b
  callf lcd_text
  mov #<s_lv, trl
  mov #>s_lv, trh
  mov #0, c
  mov #3, b
  callf lcd_text
  ld B_FOELV
  callf print2
  callf pause_long
  callf pause_long
  callf battle_run
  jmpf battle_end

;; b_my_power: acc = our power: level + STRENGTH * 2 + the IVs' sum / 16 + 3 with no care mistakes
b_my_power:
  mov #0, tmp0              ; the IVs' sum, 16 bits
  mov #0, tmp1
  mov #P_IVS, idx
.iv:
  ld idx
  st 0
  ld @R0
  add tmp0
  st tmp0
  ld tmp1
  addc #0
  st tmp1
  inc idx
  ld idx
  bne #P_IVS + 6, .iv
  ld tmp0                   ; / 16
  ror
  ror
  ror
  ror
  and #$0f
  st tmp0
  ld tmp1
  rol
  rol
  rol
  rol
  and #$f0
  or tmp0
  add P_LEVEL
  add P_STRENGTH
  add P_STRENGTH
  st tmp0
  ld P_MISTAKES
  bnz .some
  ld tmp0
  add #3
  ret
.some:
  ld tmp0
  ret

;; b_foe_name: trh:trl = the foe's name
b_foe_name:
  ld B_FOESP
  rol
  st tmp0
  mov #<ed_name, trl
  mov #>ed_name, trh
  ld tmp0
  ldc
  st tmp1
  ld tmp0
  inc acc
  ldc
  st trh
  ld tmp1
  st trl
  ret

;; b_rand: the battle's own random numbers (the pet's LFSR on B_SEED): acc = the low byte
b_rand:
  ld B_SEED
  or B_SEED + 1
  bnz .ok
  mov #$5a, B_SEED
.ok:
  mov #8, B_T
.step:
  clr1 psw, 7
  ld B_SEED + 1
  rorc
  st B_SEED + 1
  ld B_SEED
  rorc
  st B_SEED
  bn psw, 7, .n
  ld B_SEED + 1
  xor #$b4
  st B_SEED + 1
.n:
  dec B_T
  ld B_T
  bnz .step
  ld B_SEED
  ret

;; b_eff: tmp0 = an attacking type, tmp1 / tmp2 = the defender's types -> acc = 0-3 (type_chart's codes, both types)
b_eff:
  ld tmp1
  callf b_chart
  st tmp3
  bz .r                     ; no effect at all
  ld tmp2
  be tmp1, .one             ; a single type
  callf b_chart
  bz .r
  ; both: (a - 2) + (b - 2): below 0 not very, 0 normal, above super
  add tmp3
  sub #4
  bz .normal
  bp acc, 7, .notvery
  mov #3, acc
  ret
.notvery:
  mov #1, acc
  ret
.normal:
  mov #2, acc
  ret
.one:
  ld tmp3
.r:
  ret

;; b_chart: tmp0 = the attacker's type, acc = the defender's -> acc = type_chart[tmp0][acc]
b_chart:
  push acc
  mov #<type_chart, trl
  mov #>type_chart, trh
  ld tmp0
  st c
  mov #0, acc
  mov #19, b
  mul
  st b                      ; the high byte
  ld c
  add trl
  st trl
  ld b
  addc trh
  st trh
  pop acc
  ldc
  ret

;; b_types: acc = a MOeMON -> tmp1 / tmp2 its types (tmp0 kept)
b_types:
  st tmp3
  mov #<ed_type1, trl
  mov #>ed_type1, trh
  ldc
  st tmp1
  mov #<ed_type2, trl
  mov #>ed_type2, trh
  ld tmp3
  ldc
  st tmp2
  ret

;; battle_run: the battle itself (B_FOESP, B_FOELV, B_FOEPOW, B_MYPOW, B_FLAGS, B_SEED set) -> acc = 1 won, 0 lost
battle_run:
  ; the types' effects, both ways
  ld B_FOESP
  callf b_types
  ld P_SPECIES
  mov #<ed_type1, trl
  mov #>ed_type1, trh
  ldc
  st tmp0
  callf b_eff
  st B_MYEFF
  ld P_SPECIES
  callf b_types
  ld B_FOESP
  mov #<ed_type1, trl
  mov #>ed_type1, trh
  ldc
  st tmp0
  callf b_eff
  st B_FOEEFF
  mov #B_HP_MAX, B_MYHP
  mov #B_HP_MAX, B_FOEHP
  mov #0, B_ANIM
  callf lcd_clear
  callf b_draw
  callf b_hp
  callf pause_long
.turn:
  bp B_FLAGS, 1, .foefirst
  callf b_my_attack
  ld B_FOEHP
  bz .won
  callf b_foe_attack
  ld B_MYHP
  bz .lost
  br .turn
.foefirst:
  callf b_foe_attack
  ld B_MYHP
  bz .lost
  callf b_my_attack
  ld B_FOEHP
  bz .won
  br .turn
.won:
  mov #<s_b_win, trl
  mov #>s_b_win, trh
  callf b_word
  callf pause_long
  callf pause_long
  mov #1, acc
  ret
.lost:
  mov #<s_b_lost, trl
  mov #>s_b_lost, trh
  callf b_word
  callf pause_long
  callf pause_long
  mov #0, acc
  ret

;; b_my_attack: on a WILD battle the marker first (A in ##: a sure hit); then the attack
b_my_attack:
  mov #0, B_SURE
  bp B_FLAGS, 0, .roll
  callf b_marker
.roll:
  ; the chance: ours against theirs, our type against them
  ld B_MYPOW
  st tmp0
  ld B_FOEPOW
  st tmp1
  ld B_MYEFF
  st tmp2
  callf b_chance
  st tmp3
  callf b_rand              ; always one number a turn (a LINK's two units stay together)
  sub tmp3
  bp psw, 7, .hit
  ld B_SURE
  bnz .hit
  ; missed: it lunges, the foe hops aside
  mov #%0100, B_ANIM
  callf b_step
  mov #%1000, B_ANIM
  callf b_step
  mov #0, B_ANIM
  callf b_draw
  mov #<s_b_miss, trl
  mov #>s_b_miss, trh
  callf b_word
  callf pause_long
  ret
.hit:
  mov #%0100, B_ANIM
  callf b_step
  ld B_MYEFF
  callf b_damage
  st tmp3
  ld B_FOEHP
  sub tmp3
  bn psw, 7, .left
  mov #0, acc
.left:
  st B_FOEHP
  mov #%0010, tmp2          ; the foe flashes
  callf b_flash
  callf b_hp
  ld B_MYEFF
  callf b_hit_word
  callf pause_long
  ret

b_foe_attack:
  ld B_FOEPOW
  st tmp0
  ld B_MYPOW
  st tmp1
  ld B_FOEEFF
  st tmp2
  callf b_chance
  st tmp3
  callf b_rand
  sub tmp3
  bp psw, 7, .hit
  mov #%1000, B_ANIM
  callf b_step
  mov #%0100, B_ANIM
  callf b_step
  mov #0, B_ANIM
  callf b_draw
  mov #<s_b_miss, trl
  mov #>s_b_miss, trh
  callf b_word
  callf pause_long
  ret
.hit:
  mov #%1000, B_ANIM
  callf b_step
  ld B_FOEEFF
  callf b_damage
  st tmp3
  ld B_MYHP
  sub tmp3
  bn psw, 7, .left
  mov #0, acc
.left:
  st B_MYHP
  mov #%0001, tmp2
  callf b_flash
  callf b_hp
  ld B_FOEEFF
  callf b_hit_word
  callf pause_long
  ret

;; b_damage: acc = the type code -> the damage (2 super effective, else 1)
b_damage:
  be #3, .two
  mov #1, acc
  ret
.two:
  mov #2, acc
  ret

;; b_hit_word: acc = the type code: "BIG!" (super effective), "HIT!"
b_hit_word:
  be #3, .big
  mov #<s_b_hit, trl
  mov #>s_b_hit, trh
  jmpf b_word
.big:
  mov #<s_b_big, trl
  mov #>s_b_big, trh
  jmpf b_word

;; b_chance: tmp0 = the attacker's power, tmp1 = the defender's, tmp2 = the type code -> acc = the chance of a hit,
;; out of 256: 140 + 6 for each point ahead (to 230), - 6 for each behind (to 44); not very effective -40, no
;; effect -70 (20 at least)
b_chance:
  ld tmp0
  sub tmp1
  bp psw, 7, .behind
  sub #16
  bp psw, 7, .aok
  mov #15, acc
  br .ahead
.aok:
  add #16
.ahead:
  st tmp3
  rol
  add tmp3                  ; * 3
  rol                       ; * 6
  add #140
  br .type
.behind:
  ld tmp1
  sub tmp0
  sub #17
  bp psw, 7, .bok
  mov #16, acc
  br .back
.bok:
  add #17
.back:
  st tmp3
  rol
  add tmp3
  rol
  st tmp3
  mov #140, acc
  sub tmp3
.type:
  st tmp3
  ld tmp2
  be #1, .notvery
  bnz .done
  ld tmp3                   ; no effect
  sub #70
  br .floor
.notvery:
  ld tmp3
  sub #40
.floor:
  bp psw, 7, .min
  sub #20
  bp psw, 7, .min
  add #20
  ret
.min:
  mov #20, acc
  ret
.done:
  ld tmp3
  ret

;; b_marker: the player's turn on a WILD battle: a marker runs along [....##....] on the bottom line; A while it's on
;; ## (columns 5-6) is a sure hit (B_SURE). Two runs at most, then it goes on without.
b_marker:
.up:
  ld p3                     ; an A still held from before doesn't count
  bn acc, BTN_A, .up
  mov #4, b
  callf lcd_clear_line
  mov #<s_track, trl
  mov #>s_track, trh
  mov #0, c
  mov #4, b
  callf lcd_text
  mov #1, tmp0              ; the column (1-10)
  mov #1, tmp1              ; the way it goes
  mov #36, tmp2             ; steps left
.sweep:
  ld tmp0
  st c
  mov #4, b
  mov #94, acc              ; '^'
  callf lcd_char
  callf train_wait
  bnz .press
  ld tmp0
  st c
  mov #4, b
  mov #46, acc              ; '.'
  ld tmp0
  sub #5
  bp psw, 7, .dot
  sub #2
  bn psw, 7, .dot
  mov #35, acc              ; '#'
  br .put
.dot:
  mov #46, acc
.put:
  callf lcd_char
  dec tmp2
  ld tmp2
  bz .none
  ld tmp0
  add tmp1
  st tmp0
  be #10, .turn
  be #1, .turn
  br .sweep
.turn:
  ld tmp1
  xor #$fe
  st tmp1
  br .sweep
.press:
  ld tmp0
  sub #5
  bp psw, 7, .none
  sub #2
  bn psw, 7, .none
  mov #1, B_SURE
.none:
  callf b_hp
  ret

;; b_step: the battle drawn with B_ANIM's lunges, held a moment, then drawn standing
b_step:
  callf b_draw
  mov #14, acc
  callf delay
  ld B_ANIM
  and #%0011
  st B_ANIM
  callf b_draw
  mov #6, acc
  jmpf delay

;; b_flash: tmp2 = which one (B_ANIM's hide bits) blinks three times
b_flash:
  mov #3, tmp1
.f:
  ld tmp2
  st B_ANIM
  callf b_draw
  mov #7, acc
  callf delay
  mov #0, B_ANIM
  callf b_draw
  mov #7, acc
  callf delay
  dec tmp1
  ld tmp1
  bnz .f
  ret

;; b_draw: rows 0-23: ours on the left facing right, the foe on the right facing left (B_ANIM: hidden, up)
b_draw:
  push tmp0
  push tmp1
  mov #0, acc
  mov #24, b
  callf lcd_clear_rows
  bp B_ANIM, 0, .nome
  ld P_SPECIES
  callf b_sprite
  bn B_ANIM, 2, .meflat
  sub #4
  bn psw, 7, .meflat
  mov #0, acc
.meflat:
  mov #0, c
  callf lcd_blit24m
.nome:
  bp B_ANIM, 1, .nofoe
  ld B_FOESP
  callf b_sprite
  bn B_ANIM, 3, .foeflat
  sub #4
  bn psw, 7, .foeflat
  mov #0, acc
.foeflat:
  mov #3, c
  callf lcd_blit24
.nofoe:
  pop tmp1
  pop tmp0
  ret

;; b_sprite: acc = a MOeMON -> trh:trl its sprite, acc = its top row standing on row 23
b_sprite:
  rol
  st tmp0
  mov #<ed_sprite, trl
  mov #>ed_sprite, trh
  ld tmp0
  ldc
  st tmp1
  ld tmp0
  inc acc
  ldc
  st trh
  ld tmp1
  st trl
  mov #0, acc
  ldc
  st tmp1
  mov #24, acc
  sub tmp1
  ret

;; b_hp: the bottom line: our HP on the left, the foe's on the right (a # each)
b_hp:
  push b
  push c
  mov #4, b
  callf lcd_clear_line
  mov #0, c
  ld B_MYHP
  st tmp3
.me:
  ld tmp3
  bz .foe
  mov #35, acc
  callf lcd_char
  inc c
  dec tmp3
  br .me
.foe:
  ld B_FOEHP
  st tmp3
  mov #11, c
.f:
  ld tmp3
  bz .r
  mov #35, acc
  callf lcd_char
  dec c
  dec tmp3
  br .f
.r:
  pop c
  pop b
  ret

;; b_word: trh:trl = a word of 4 letters, between the HP (columns 4-7; the HP take 0-2 and 9-11)
b_word:
  push b
  push c
  mov #4, c
  mov #4, b
  callf lcd_text
  pop c
  pop b
  ret

;; battle_end: acc = 1 won / 0 lost: the prizes, back to the pet, happy or not
battle_end:
  st tmp0
  ld P_BATTLES
  be #255, .b
  inc P_BATTLES
.b:
  mov #SCR_MAIN, screen
  callf lcd_clear
  ld tmp0
  bz .lost
  ld P_WINALL
  be #255, .w
  inc P_WINALL
.w:
  ld B_FOELV                ; SPARKS: the foe's level x 4
  rol
  rol
  and #$fc
  callf sparks_add8
  ld P_WINDAY               ; EXP for the first 3 wins of a day
  sub #3
  bn psw, 7, .noexp
  inc P_WINDAY
  mov #2, acc
  callf gain_exp
.noexp:
  callf save_write
  callf react_happy
  mov #<s_b_won, trl
  mov #>s_b_won, trh
  jmpf message
.lost:
  callf save_write
  callf react_sad
  mov #<s_b_lostmsg, trl
  mov #>s_b_lostmsg, trh
  jmpf message

;; ---- LINK > BATTLE (the VM2 calls; the VMU answers from its main loop: link.inc): they swap cards, then both play
;; the battle from the two seeds; the VM2 (the caller) attacks first.
battle_link:
  callf need_mon
  bz .mon
  ret
.mon:
  callf sio_connected
  bz .cable
  mov #<s_nocable, trl
  mov #>s_nocable, trh
  jmpf link_status
.cable:
  callf b_build_card
  mov #<s_linking, trl
  mov #>s_linking, trh
  callf link_status
  callf sio_init
  mov #10, tmp3             ; the VMU answers between its naps: the card's call is tried again
.call:
  push tmp3
  callf b_card_send
  pop tmp3
  bz .sent
  dec tmp3
  ld tmp3
  bnz .call
  jmpf .fail
.sent:
  ld ticks
  st tmp3
.wait2:
  callf sio_poll
  bz .got2
  ld ticks
  sub tmp3
  sub #8
  bp psw, 7, .wait2
  jmpf .fail
.got2:
  ld b
  bne #B_CARD_SYNC, .wait2
  callf b_card_recv
  bnz .fail
  mov #0, B_FLAGS
  ld FMBUFF + $44           ; the seed: the caller's (mine) low, the other's high
  st B_SEED
  ld FMBUFF + 4
  st B_SEED + 1
  jmpf battle_linked
.fail:
  jmpf link_fail

;; battle_answer (the VMU; the VM2's card sync is in b): its card, ours back, then the battle (they attack first)
battle_answer:
  ld P_STAGE
  be #2, .mon
  ret                       ; an Egg doesn't battle: no answer (the VM2 says NO LINK)
.mon:
  callf b_card_recv
  bnz .fail
  callf b_build_card
  callf link_pause
  callf b_card_send
  bnz .fail
  mov #%10, B_FLAGS
  ld FMBUFF + 4             ; the seed: the caller's (theirs) low, mine high
  st B_SEED
  ld FMBUFF + $44
  st B_SEED + 1
  jmpf battle_linked
.fail:
  mov #<s_link_failed, trl
  mov #>s_link_failed, trh
  jmpf message

;; b_build_card: our card at FMBUFF + $40: the edition, MOeMON, level, power, a seed, 0
b_build_card:
  callf b_my_power
  st B_MYPOW
  callf rand
  ld O_RNG
  bnz .seedok
  inc acc
.seedok:
  st B_T
  mov #FMBUFF + $40, 0
  mov #1, acc
  st @R0
  inc 0
  ld P_SPECIES
  st @R0
  inc 0
  ld P_LEVEL
  st @R0
  inc 0
  ld B_MYPOW
  st @R0
  inc 0
  ld B_T
  st @R0
  inc 0
  mov #0, acc
  st @R0
  ret

;; battle_linked: the cards swapped: theirs in FMBUFF; the battle
battle_linked:
  callf sio_end
  bp P_UNIT, 0, .vm2
  clr1 P_UNIT, 1            ; a VMU: the port is set up again by slave_poll
.vm2:
  ld FMBUFF                 ; the same edition?
  bne #1, .fail
  ld FMBUFF + 1
  st B_FOESP
  ld FMBUFF + 2
  st B_FOELV
  ld FMBUFF + 3
  st B_FOEPOW
  set1 B_FLAGS, 0
  mov #SCR_MAIN, screen
  callf lcd_clear
  mov #<s_b_vs, trl
  mov #>s_b_vs, trh
  mov #0, c
  mov #0, b
  callf lcd_text
  callf b_foe_name
  mov #0, c
  mov #1, b
  callf lcd_text
  mov #<s_lv, trl
  mov #>s_lv, trh
  mov #0, c
  mov #2, b
  callf lcd_text
  ld B_FOELV
  callf print2
  callf pause_long
  callf pause_long
  callf battle_run
  jmpf battle_end
.fail:
  mov #<s_link_failed, trl
  mov #>s_link_failed, trh
  jmpf message

;; b_card_send: my card (FMBUFF + $40, B_CARD_LEN bytes): the sync, the bytes, their sum, each echoed -> acc 0 ok
b_card_send:
  callf sio_flush
  mov #B_CARD_SYNC, acc
  callf sio_xfer
  bnz .r
  mov #0, chk
  mov #FMBUFF + $40, idx
.s:
  ld idx
  st 0
  ld @R0
  push acc
  add chk
  st chk
  pop acc
  callf sio_xfer
  bnz .r
  inc idx
  ld idx
  bne #FMBUFF + $40 + B_CARD_LEN, .s
  ld chk
  callf sio_xfer
.r:
  ret

;; b_card_recv: the sync has come (in b): echo it, then the card into FMBUFF (each byte echoed) -> acc 0 ok
b_card_recv:
  ld b
  callf sio_send
  mov #0, chk
  mov #FMBUFF, idx
.g:
  callf sio_recv
  bnz .bad
  ld b
  st tmp2
  callf sio_send
  ld idx
  st 0
  ld tmp2
  st @R0
  add chk
  st chk
  inc idx
  ld idx
  bne #FMBUFF + B_CARD_LEN, .g
  callf sio_recv
  bnz .bad
  ld b
  st tmp2
  callf sio_send
  callf sio_sent
  ld tmp2
  sub chk
  ret
.bad:
  mov #1, acc
  ret

s_b_wild:
  .string 7 "A WILD"
s_b_appeared:
  .string 10 "APPEARED!"
s_b_vs:
  .string 3 "VS"
s_b_win:
  .string 5 "WIN!"
s_b_lost:
  .string 5 "LOST"
s_b_miss:
  .string 5 "MISS"
s_b_hit:
  .string 5 "HIT!"
s_b_big:
  .string 5 "BIG!"
s_b_won:
  .string 9 "YOU WON!"
s_b_lostmsg:
  .string 11 "IT LOST..."
