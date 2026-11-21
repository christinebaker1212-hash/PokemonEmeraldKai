;; Moemon VMU pet, KANTO edition: a first draft (docs/VMU_PET_DECISIONS.md).
;;
;; One MOeMON raised from an Egg. Care is judged by the day (from 4 AM by the VMU's clock), never the minute: a day asks
;; for 4 meals, 3 trainings and no poop left; a missed one is a care mistake, never a loss. Levels come from care (about
;; a week of good care to level 50); evolutions at their levels, the rest by care. SPARKS come from tapping (A on the
;; pet) and helpers that earn while it's closed (up to 12 hours' worth). LINK sends a grown MOeMON to a VM2 (then a new
;; Egg starts) or takes one from a VM2 in place of an Egg.
;;
;; Buttons: A tap the pet (SPARKS) / choose; B the menu / back; UP DOWN move; LEFT RIGHT pages; MODE leave (saves).
;; The screen is only drawn on the RC clock; the crystal only while HALTed (Sega's VCCR rules: inc/sfr.inc).

  .include "../inc/sfr.inc"
  .include "../inc/header.inc"

  .org $200
  .text 16 "MOEMON KANTO"
  .text 32 "MOEMON VMU PET - KANTO EDITION"
  .string 16 "MOEMON KAI"
  .include icon "icon.png" speed=16

;; ---- RAM bank 1 ($00-$0f: the indirect registers; $7c-$ff: the BIOS's flash call area and page buffer)
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
sio_err    = $1a
keys       = $1b            ; held (1 = pressed, P3's bits)
keys_new   = $1c            ; pressed since the last look
keys_prev  = $1d
screen     = $1e            ; SCR_*
cursor     = $1f
scroll     = $20
page       = $21
msg_left   = $22            ; ticks the message stays on the bottom line
anim       = $23
posx       = $24            ; the pet's byte column (1 or 2)
bob        = $25
dirty      = $26            ; the state changed: save soon
tmp0       = $27
tmp1       = $28
tmp2       = $29
tmp3       = $2a
idx        = $2b
chk        = $2c
t_year     = $2d            ; years since 2000
t_month    = $2e
t_day      = $2f
t_hour     = $30
t_min      = $31
dn_l       = $32            ; the care day (days since 2000, a day starting at 4 AM)
dn_h       = $33
units      = $34            ; tens of minutes since 4 AM (0-143)
keys_latch = $35            ; presses seen while drawing (lcd_hook)
digits     = $36            ; 7 bytes: a number's decimal digits ($36-$3c)
earn       = $3d            ; tens of minutes the helpers earned for
last_ticks = $3e
redraw     = $3f            ; the screen needs drawing

;; The MOeMON ($40-$67: what LINK sends; Kai reads it from the save page)
P_MAGIC    = $40            ; 'P'
P_STAGE    = $41            ; 0 nothing, 1 an Egg, 2 a MOeMON
P_SPECIES  = $42            ; the edition's index (gen_edition.py)
P_LEVEL    = $43
P_EXP      = $44            ; 0-7 (8 a level)
P_HUNGER   = $45            ; 0-4 (meals today)
P_STRENGTH = $46            ; 0-4
P_POOP     = $47            ; 0-3
P_FLAGS    = $48            ; bit 0 sick, bit 1 shiny
P_MISTAKES = $49
P_DAYS     = $4a
P_FED      = $4b            ; meals today
P_TRAINED  = $4c            ; trainings today
P_FEDALL   = $4d            ; meals ever (to 255)
P_TRAINALL = $4e            ; trainings ever (to 255)
P_EGGLEFT  = $4f            ; tens of minutes until the Egg hatches
P_PID      = $50            ; 4 bytes: the personality (Kai: nature, Ability, gender)
P_IVS      = $54            ; 6 bytes: 0-31 each
P_IVUPS    = $5a            ; IV raises bought (Kai adds 5 to the lowest IV for each)
P_TID      = $5b            ; 4 bytes: the transfer ID (Kai takes each MOeMON once)
P_EDITION  = $5f            ; 1 = KANTO
P_MASTER   = $60            ; MASTER BALLS bought (they come to Kai with it)
P_ASTRAL   = $61            ; ASTRAL BALLS bought
P_WINDAY   = $62            ; battles won today (the first 3 give EXP)
P_WINALL   = $63            ; battles won ever (to 255)
P_BATTLES  = $64            ; battles ever (to 255)
P_END      = $68
PET_BYTES  = 40
;; The owner ($68-$7b: stays on this VMU)
O_SPARKS   = $68            ; 3 bytes, low first
O_TAP      = $6b            ; TAP UP level: a tap earns 1 + this + O_PRESTIGE
O_HELPER   = $6c            ; helpers: SPARKS each ten minutes
O_PRESTIGE = $6d            ; MOeMON sent on: each adds a SPARK a tap, for good
O_LASTDAY  = $6e            ; 2 bytes: the care day last seen
O_LASTUNIT = $70            ; the ten minutes last seen
O_RNG      = $71            ; 2 bytes
O_TAPS     = $73            ; taps since the last save
O_MAGIC    = $74            ; 'O'

SCR_MAIN   = 0
SCR_MENU   = 1
SCR_STATUS = 2
SCR_SHOP   = 3
SCR_LINK   = 4
SCR_FEED   = 5

EXP_LEVEL  = 8
MAX_LEVEL  = 50

;; ---------------------------------------------------------------- start

main:
  set1 psw, 1
  callf clk_fast
  callf batt_off
  mov #0, p3int             ; the button interrupt only around HALT (level-triggered: a held button floods the CPU)
  mov #0, keys_latch
  mov #0, mirror
  mov #0, overlay
  mov #0, ticks
  mov #0, keys_prev
  mov #$ff, keys_prev       ; buttons still held from the menu don't count
  mov #8, sio_speed
  mov #$fe, sio_speed       ; the link: RC, the fastest (LINK TEST 4: the VM2 garbles the crystal's)
  mov #1, posx
  mov #0, bob
  mov #0, anim
  mov #0, msg_left
  mov #0, dirty
  mov #SCR_MAIN, screen
  callf save_load
  callf time_read
  ld O_MAGIC
  be #$4f, .known           ; 'O': this VMU has run the pet before
  callf owner_new
.known:
  ld P_STAGE
  bnz .has
  callf egg_new
.has:
  callf time_catch_up
  callf save_write
  callf lcd_clear
  mov #1, redraw

loop:
  callf read_keys
  ld keys
  bn acc, BTN_MODE, .stay
  jmpf leave
.stay:
  ld keys_new
  bz .nokey
  callf on_key
.nokey:
  ld ticks
  be last_ticks, .sametick
  st last_ticks
  callf on_tick
  callf icon_check          ; the file icon follows an Egg laid, hatched, evolved or received (icon.inc)
.sametick:
  ld redraw
  bz .halt
  mov #0, redraw
  callf draw_screen
.halt:
  ld keys
  bnz loop                  ; something held: keep looking
  mov #%00000101, p3int     ; a button wakes HALT (the vector clears the request)
  callf clk_slow
  set1 pcon, 0              ; HALT: a button or the half second wakes it
  callf clk_fast
  mov #0, p3int
  jmpf loop

leave:
  callf save_write
  callf batt_on
  jmpf game_end

;; ---------------------------------------------------------------- buttons

read_keys:
  ld p3
  xor #$ff
  or keys_latch             ; and any press seen while drawing
  st keys
  mov #0, keys_latch
  ld keys_prev
  xor #$ff
  and keys
  st keys_new
  ld keys
  st keys_prev
  ld keys_new
  bz .none
  push psw                  ; a button: the BIOS's idle count starts again (it sleeps after ~2 minutes without)
  clr1 psw, 1
  mov #1, acc
  st $33
  pop psw
.none:
  ret

on_key:
  ld screen
  be #SCR_MAIN, .main
  be #SCR_MENU, .menu
  be #SCR_STATUS, .status
  be #SCR_SHOP, .shop
  be #SCR_LINK, .link
  be #SCR_FEED, .feed
  ret
.main:
  jmpf key_main
.menu:
  jmpf key_menu
.status:
  jmpf key_status
.shop:
  jmpf key_shop
.link:
  jmpf key_link
.feed:
  jmpf key_feed

;; lcd_hook: lcd_char calls it before each letter: the buttons sampled, so a short press during a redraw counts
lcd_hook:
  push psw
  push acc
  ld p3
  xor #$ff
  or keys_latch
  st keys_latch
  pop acc
  pop psw
  ret

;; ---------------------------------------------------------------- the half second

on_tick:
  inc anim
  ld msg_left
  bz .nomsg
  dec msg_left
  ld msg_left
  bnz .nomsg
  mov #1, redraw            ; the message's time is up
.nomsg:
  ld anim
  and #1
  bnz .odd
  callf time_catch_up       ; every second: the clock (days, helpers, the Egg)
.odd:
  ld screen
  bne #SCR_MAIN, .done
  ; the pet moves: a hop every tick, a step every 8
  ld bob
  xor #1
  st bob
  ld anim
  and #7
  bnz .nostep
  ld posx
  xor #3                    ; 1 <-> 2
  st posx
  sub #2
  bz .faceright
  mov #0, mirror
  br .nostep
.faceright:
  mov #1, mirror            ; facing the way it went
.nostep:
  callf draw_pet
.done:
  ret

;; ---------------------------------------------------------------- the main screen

key_main:
  ld keys_new
  bp acc, BTN_B, .menu
  bp acc, BTN_A, .tap
  bp acc, BTN_UP, .status
  bp acc, BTN_DOWN, .status
  ret
.menu:
  mov #SCR_MENU, screen
  mov #0, cursor
  mov #0, scroll
  mov #1, redraw
  ret
.status:
  mov #SCR_STATUS, screen
  mov #0, page
  mov #1, redraw
  ret
.tap:
  ; SPARKS: 1 + TAP UP + those sent on
  ld O_TAP
  add O_PRESTIGE
  add #1
  callf sparks_add8
  ld bob
  xor #1
  st bob
  callf draw_pet
  callf draw_bottom
  inc O_TAPS
  ld O_TAPS
  bne #60, .notyet
  mov #0, O_TAPS
  callf save_write          ; now and then, not every tap (flash wears)
.notyet:
  ret

draw_screen:
  callf lcd_clear
  ld screen
  be #SCR_MAIN, .main
  be #SCR_MENU, .menu
  be #SCR_STATUS, .status
  be #SCR_SHOP, .shop
  be #SCR_LINK, .link
  be #SCR_FEED, .feed
  ret
.main:
  callf draw_pet
  jmpf draw_bottom
.menu:
  jmpf draw_menu
.status:
  jmpf draw_status
.shop:
  jmpf draw_shop
.link:
  jmpf draw_link
.feed:
  jmpf draw_feed

;; draw_pet: rows 0-23: the MOeMON (or its Egg) at posx, hopping; poop and sickness at the right
draw_pet:
  mov #0, acc
  mov #24, b
  callf lcd_clear_rows
  ld P_STAGE
  be #2, .mon
  ; the Egg: its design by the MOeMON inside (gen_edition.py ed_egg: bits 0-3 the design, bit 4 mirrored), rocking
  ; by a row and turning with `mirror`
  ld P_SPECIES
  mov #<ed_egg, trl
  mov #>ed_egg, trh
  ldc
  st tmp2
  and #$0f
  rol
  st tmp0
  mov #<egg_pats, trl
  mov #>egg_pats, trh
  ld tmp0
  ldc
  st tmp1
  ld tmp0
  inc acc
  ldc
  st trh
  ld tmp1
  st trl
  ld posx
  st c
  ld tmp2
  ror
  ror
  ror
  ror
  xor mirror
  and #1
  bnz .eggm
  mov #4, acc
  sub bob
  callf lcd_blit24
  br .extras
.eggm:
  mov #4, acc
  sub bob
  callf lcd_blit24m
  br .extras
.mon:
  ld P_SPECIES
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
  ldc                       ; its height: it stands on row 23, a row up when it hops
  st tmp0
  mov #24, acc
  sub tmp0
  sub bob
  bn psw, 7, .top
  mov #0, acc
.top:
  st tmp1                   ; the top row
  ld posx
  st c
  ld mirror
  bnz .mirrored
  ld tmp1
  callf lcd_blit24
  br .extras
.mirrored:
  ld tmp1
  callf lcd_blit24m
.extras:
  ; an icon beside it: a heart or a sweat drop up high, a meal at its mouth
  ld overlay
  bz .noover
  sub #1
  rol
  rol
  rol                       ; 8 bytes each
  st tmp2
  mov #<ic_overlays, trl
  mov #>ic_overlays, trh
  ld trl
  add tmp2
  st trl
  ld trh
  addc #0
  st trh
  ld posx
  add #3
  st c
  mov #2, acc
  ld overlay
  sub #3
  bp psw, 7, .high
  mov #15, acc              ; a meal: by its mouth
  br .ov
.high:
  mov #2, acc
.ov:
  callf lcd_blit8
.noover:
  ; poop: one icon a poop at the right, from the bottom up
  ld P_POOP
  bz .nopoop
  st tmp2
  mov #16, tmp3
.poop:
  mov #<ic_poop, trl
  mov #>ic_poop, trh
  mov #5, c
  ld tmp3
  callf lcd_blit8
  ld tmp3
  sub #8
  st tmp3
  dec tmp2
  ld tmp2
  bnz .poop
.nopoop:
  ld P_FLAGS
  bn acc, 0, .well
  mov #<ic_sick, trl
  mov #>ic_sick, trh
  mov #0, c
  mov #0, acc
  callf lcd_blit8
.well:
  ret


;; draw_bottom: line 4: the message, or "LV12 123456" (an Egg: "EGG 123456")
draw_bottom:
  push b
  push c
  mov #4, b
  callf lcd_clear_line
  ld msg_left
  bz .stat
  ld msg_ptr_l
  st trl
  ld msg_ptr_h
  st trh
  mov #0, c
  callf lcd_text
  br .out
.stat:
  mov #0, c
  ld P_STAGE
  be #2, .lv
  mov #<s_egg, trl
  mov #>s_egg, trh
  callf lcd_text
  br .sp
.lv:
  mov #<s_lv, trl
  mov #>s_lv, trh
  callf lcd_text
  ld P_LEVEL
  callf print2
.sp:
  inc c
  callf print_sparks
.out:
  pop c
  pop b
  ret

msg_ptr_l = $76
msg_ptr_h = $77
mirror    = $75             ; the MOeMON faces the other way (its sprite mirrored)
overlay   = $7b             ; an icon beside it: 0 none, 1 a heart, 2 a sweat drop, 3-5 a meal being eaten

;; message: trh:trl = text for the bottom line, shown about 2 seconds
message:
  ld trl
  st msg_ptr_l
  ld trh
  st msg_ptr_h
  mov #6, msg_left          ; ticks: 3 s on a VMU (the emulator ticks faster)
  mov #SCR_MAIN, screen
  mov #1, redraw
  ret


;; ---------------------------------------------------------------- animations

;; delay: acc = about 10 ms units (at RC 1/12); the buttons keep being sampled
delay:
  st lcd_t3
.o:
  callf lcd_hook
  mov #120, lcd_t4
.i:
  dec lcd_t4
  ld lcd_t4
  bnz .i
  dec lcd_t3
  ld lcd_t3
  bnz .o
  ret

;; react_happy: it jumps twice with a heart beside it (a meal, a vitamin, a clean-up, medicine, good training, hatching)
react_happy:
  ld screen
  bne #SCR_MAIN, .r
  mov #1, overlay
  mov #0, tmp3
.f:
  ld tmp3
  and #1
  bz .down
  mov #4, bob               ; up 4 rows
  br .draw
.down:
  mov #0, bob
.draw:
  push tmp3
  callf draw_pet
  pop tmp3
  mov #12, acc
  callf delay
  inc tmp3
  ld tmp3
  bne #5, .f
  mov #0, overlay
  mov #0, bob
  callf draw_pet
.r:
  ret

;; react_sad: it shakes its head (turning back and forth), a sweat drop beside it (full already, a bad training)
react_sad:
  ld screen
  bne #SCR_MAIN, .r
  mov #2, overlay
  ld mirror
  push acc
  mov #0, tmp3
.f:
  ld tmp3
  and #1
  st mirror
  mov #0, bob
  push tmp3
  callf draw_pet
  pop tmp3
  mov #10, acc
  callf delay
  inc tmp3
  ld tmp3
  bne #6, .f
  pop acc
  st mirror
  mov #0, overlay
  callf draw_pet
.r:
  ret


;; eat_anim: a meal by its mouth, eaten in three bites (it bobs as it chews), then it's happy
eat_anim:
  mov #SCR_MAIN, screen
  callf lcd_clear
  callf draw_bottom
  mov #3, overlay
.bite:
  ld overlay
  and #1
  st bob
  callf draw_pet
  mov #25, acc
  callf delay
  inc overlay
  ld overlay
  bne #6, .bite
  mov #0, overlay
  mov #0, bob
  jmpf react_happy

;; hatch_anim: the Egg wobbles (turning back and forth), then the MOeMON is there and happy
hatch_anim:
  ld screen
  bne #SCR_MAIN, .r
  mov #0, tmp3
.w:
  ld tmp3
  and #1
  st bob
  st mirror                 ; turning as it rocks
  push tmp3
  callf draw_pet
  pop tmp3
  mov #8, acc
  callf delay
  inc tmp3
  ld tmp3
  bne #8, .w
  mov #0, bob
  mov #0, mirror
.r:
  ret

;; ---------------------------------------------------------------- the menu

MENU_ITEMS = 9

key_menu:
  ld keys_new
  bp acc, BTN_B, .back
  bp acc, BTN_UP, .up
  bp acc, BTN_DOWN, .down
  bp acc, BTN_A, .pick
  ret
.back:
  mov #SCR_MAIN, screen
  mov #1, redraw
  ret
.up:
  ld cursor
  bz .r
  dec cursor
  br .scroll
.down:
  ld cursor
  be #MENU_ITEMS - 1, .r
  inc cursor
.scroll:
  ; four lines show: keep the cursor in them
  ld cursor
  sub scroll
  bp psw, 7, .above
  sub #4
  bp psw, 7, .drawit
  ld cursor
  sub #3
  st scroll
  br .drawit
.above:
  ld cursor
  st scroll
.drawit:
  mov #1, redraw
.r:
  ret
.pick:
  ld cursor
  be #0, .feed
  be #1, .train
  be #2, .battle
  be #3, .clean
  be #4, .meds
  be #5, .status
  be #6, .shop
  be #7, .link
  jmpf about
.feed:
  jmpf act_feed_menu
.train:
  jmpf act_train
.battle:
  jmpf battle_wild
.clean:
  jmpf act_clean
.meds:
  jmpf act_meds
.status:
  mov #SCR_STATUS, screen
  mov #0, page
  mov #1, redraw
  ret
.shop:
  mov #SCR_SHOP, screen
  mov #0, cursor
  mov #0, scroll
  mov #1, redraw
  ret
.link:
  mov #SCR_LINK, screen
  mov #0, cursor
  mov #1, redraw
  ret

draw_menu:
  mov #<s_menu, trl
  mov #>s_menu, trh
  mov #0, c
  mov #0, b
  callf lcd_text
  mov #0, tmp0              ; the line
.line:
  ld scroll
  add tmp0
  st tmp1                   ; the item
  sub #MENU_ITEMS
  bn psw, 7, .done
  ld tmp1
  rol
  st tmp2
  mov #<menu_texts, trl
  mov #>menu_texts, trh
  ld tmp2
  ldc
  st tmp3
  ld tmp2
  inc acc
  ldc
  st trh
  ld tmp3
  st trl
  ld tmp0
  add #1
  st b
  mov #1, c
  callf lcd_text
  ld tmp1
  bne cursor, .nocur
  mov #0, c
  mov #62, acc              ; '>'
  callf lcd_char
.nocur:
  inc tmp0
  ld tmp0
  bne #4, .line
.done:
  ret

menu_texts:
  .word s_m_feed, s_m_train, s_m_battle, s_m_clean, s_m_meds, s_m_status, s_m_shop, s_m_link, s_m_about

;; ---------------------------------------------------------------- care

;; only a MOeMON (not an Egg) is cared for
need_mon:
  ld P_STAGE
  be #2, .yes
  mov #<s_egg_cant, trl
  mov #>s_egg_cant, trh
  callf message
  mov #1, acc
  ret
.yes:
  mov #0, acc
  ret

act_feed_menu:
  callf need_mon
  bnz .r
  mov #SCR_FEED, screen
  mov #0, cursor
  mov #1, redraw
.r:
  ret

draw_feed:
  mov #<s_feed, trl
  mov #>s_feed, trh
  mov #0, c
  mov #0, b
  callf lcd_text
  mov #<s_meat, trl
  mov #>s_meat, trh
  mov #1, c
  mov #1, b
  callf lcd_text
  mov #<s_vitamin, trl
  mov #>s_vitamin, trh
  mov #1, c
  mov #2, b
  callf lcd_text
  mov #<s_hunger_h, trl
  mov #>s_hunger_h, trh
  mov #0, c
  mov #4, b
  callf lcd_text
  ld P_HUNGER
  callf print_hearts
  ld cursor
  add #1
  st b
  mov #0, c
  mov #62, acc
  callf lcd_char
  ret

key_feed:
  ld keys_new
  bp acc, BTN_B, .back
  bp acc, BTN_UP, .flip
  bp acc, BTN_DOWN, .flip
  bp acc, BTN_A, .eat
  ret
.back:
  mov #SCR_MENU, screen
  mov #1, redraw
  ret
.flip:
  ld cursor
  xor #1
  st cursor
  mov #1, redraw
  ret
.eat:
  ld cursor
  bnz .vit
  ; a meal: hunger +1 (4 a day), 2 EXP; every second meal a poop
  ld P_HUNGER
  be #4, .full
  inc P_HUNGER
  inc P_FED
  ld P_FEDALL
  be #255, .fa
  inc P_FEDALL
.fa:
  ld P_FED
  and #1
  bnz .nopoop
  ld P_POOP
  be #3, .nopoop
  inc P_POOP
.nopoop:
  mov #2, acc
  callf gain_exp
  callf save_write
  callf eat_anim
  mov #<s_yum, trl
  mov #>s_yum, trh
  jmpf message
.vit:
  ld P_STRENGTH
  be #4, .full
  inc P_STRENGTH
  mov #2, acc
  callf gain_exp
  callf save_write
  mov #SCR_MAIN, screen
  callf lcd_clear
  callf react_happy
  mov #<s_strong, trl
  mov #>s_strong, trh
  jmpf message
.full:
  mov #SCR_MAIN, screen
  callf lcd_clear
  callf react_sad
  mov #<s_full, trl
  mov #>s_full, trh
  jmpf message

act_clean:
  callf need_mon
  bnz .r
  ld P_POOP
  bz .already
  mov #0, P_POOP
  mov #2, acc
  callf gain_exp
  callf save_write
  mov #SCR_MAIN, screen
  callf lcd_clear
  callf react_happy
  mov #<s_cleaned, trl
  mov #>s_cleaned, trh
  jmpf message
.already:
  mov #<s_clean_already, trl
  mov #>s_clean_already, trh
  jmpf message
.r:
  ret

act_meds:
  callf need_mon
  bnz .r
  ld P_FLAGS
  bn acc, 0, .well
  clr1 P_FLAGS, 0
  callf save_write
  mov #SCR_MAIN, screen
  callf lcd_clear
  callf react_happy
  mov #<s_cured, trl
  mov #>s_cured, trh
  jmpf message
.well:
  mov #<s_not_sick, trl
  mov #>s_not_sick, trh
  jmpf message
.r:
  ret

;; act_train: three rounds of a cursor sweeping a track: A while it's in the middle. 2 EXP, 2 more a hit; two hits
;; or more: strength +1.
act_train:
  callf need_mon
  bz .go
  ret
.go:
  ld p3                     ; the A that chose TRAIN comes up first (or it would count as round 1's press)
  bn acc, BTN_A, .go
  mov #0, tmp2              ; hits
  mov #0, tmp3              ; the round
.round:
  callf lcd_clear
  mov #<s_train, trl
  mov #>s_train, trh
  mov #0, c
  mov #0, b
  callf lcd_text
  ld tmp3
  add #49
  callf lcd_char
  mov #<s_track, trl
  mov #>s_track, trh
  mov #0, c
  mov #2, b
  callf lcd_text
  mov #<s_press_a, trl
  mov #>s_press_a, trh
  mov #0, c
  mov #4, b
  callf lcd_text
  mov #0, tmp0              ; the cursor's column (1-10)
  mov #1, tmp1              ; its direction (1 right, $ff left)
  mov #1, tmp0
.sweep:
  ; draw it
  ld tmp0
  st c
  mov #3, b
  mov #94, acc              ; '^'
  callf lcd_char
  callf train_wait          ; about 90 ms, watching A
  bnz .press
  ld tmp0
  st c
  mov #3, b
  mov #32, acc
  callf lcd_char
  ld tmp0
  add tmp1
  st tmp0
  be #10, .turn
  be #1, .turn
  br .sweep
.turn:
  ld tmp1
  xor #$fe                  ; 1 <-> $ff
  st tmp1
  br .sweep
.press:
  ld tmp0
  sub #5
  bp psw, 7, .miss          ; < 5
  sub #2
  bn psw, 7, .miss          ; >= 7
  inc tmp2
  mov #<s_hit, trl
  mov #>s_hit, trh
  br .result
.miss:
  mov #<s_miss, trl
  mov #>s_miss, trh
.result:
  mov #4, b
  callf lcd_clear_line
  mov #0, c
  callf lcd_text
  callf pause_long
  inc tmp3
  ld tmp3
  bneo #3, .round
  ; the training counts for today
  inc P_TRAINED
  ld P_TRAINALL
  be #255, .ta
  inc P_TRAINALL
.ta:
  ld tmp2
  sub #2
  bp psw, 7, .weak
  ld P_STRENGTH
  be #4, .weak
  inc P_STRENGTH
.weak:
  ld tmp2
  push acc
  rol
  add #2
  callf gain_exp
  callf save_write
  mov #SCR_MAIN, screen
  callf lcd_clear
  pop acc
  sub #2
  bp psw, 7, .badday
  callf react_happy         ; two hits or three
  br .tsaid
.badday:
  callf react_sad
.tsaid:
  mov #<s_trained, trl
  mov #>s_trained, trh
  jmpf message

;; train_wait: about 90 ms; acc = 1 if A was pressed meanwhile
train_wait:
  mov #40, idx
.w:
  ld p3
  bn acc, BTN_A, .a
  mov #30, chk
.inner:
  dec chk
  ld chk
  bnz .inner
  dec idx
  ld idx
  bnz .w
  mov #0, acc
  ret
.a:
  ; wait for A to come up, so one press is one press
.up:
  ld p3
  bn acc, BTN_A, .up
  mov #1, acc
  ret

pause_long:                 ; about 0.7 s
  mov #64, idx
.o:
  mov #0, chk
.i:
  dec chk
  ld chk
  bnz .i
  dec idx
  ld idx
  bnz .o
  ret

;; gain_exp: acc = EXP; levels at EXP_LEVEL each, to MAX_LEVEL; evolutions on the way
gain_exp:
  add P_EXP
  st P_EXP
.lvl:
  ld P_LEVEL
  be #MAX_LEVEL, .cap
  ld P_EXP
  sub #EXP_LEVEL
  bp psw, 7, .done
  st P_EXP
  inc P_LEVEL
  callf evolve_check
  br .lvl
.cap:
  mov #0, P_EXP
.done:
  mov #1, dirty
  ret

;; evolve_check: at its level, it becomes what it becomes (EEVEE by care: no mistakes VAPOREON, trained more than fed
;; FLAREON, else JOLTEON)
evolve_check:
  ld P_SPECIES
  mov #<ed_evo_to, trl
  mov #>ed_evo_to, trh
  ldc
  be #$ff, .none
  st tmp0
  ld P_SPECIES
  mov #<ed_evo_lv, trl
  mov #>ed_evo_lv, trh
  ldc
  st tmp1
  ld P_LEVEL
  sub tmp1
  bp psw, 7, .none
  ld P_SPECIES
  mov #<ed_evo_kind, trl
  mov #>ed_evo_kind, trh
  ldc
  bne #2, .become
  ld P_MISTAKES
  bnz .notvap
  mov #133, tmp0            ; VAPOREON (national 134)
  br .become
.notvap:
  ld P_FEDALL
  sub P_TRAINALL
  bn psw, 7, .become        ; fed >= trained: JOLTEON (the table's)
  mov #135, tmp0            ; FLAREON (national 136)
.become:
  ld tmp0
  st P_SPECIES
  mov #<s_evolved, trl
  mov #>s_evolved, trh
  callf message
.none:
  ret

;; ---------------------------------------------------------------- the day, the clock, the helpers

;; time_read: the VMU's clock (RAM bank 0, binary: year $17-$18, month $19, day $1a, hour $1b, minute $1c) -> t_*,
;; dn (the care day: days since 2000, starting at 4 AM), units (tens of minutes since 4 AM)
time_read:
  push psw
  clr1 psw, 1
  ld $18
  push acc
  ld $19
  push acc
  ld $1a
  push acc
  ld $1b
  push acc
  ld $1c
  set1 psw, 1
  st t_min
  pop acc
  st t_hour
  pop acc
  st t_day
  pop acc
  st t_month
  pop acc
  sub #208                  ; 2000 = $07d0: the low byte minus $d0 (years 2000-2255 assumed)
  st t_year
  pop psw
  ; days = year * 365 + (year + 3) / 4 + the days before this month (+1 after February in a leap year) + day - 1
  mov #0, acc
  ld t_year
  st c
  mov #0, acc
  mov #109, b
  mul                       ; acc:c = year * 109
  st dn_h
  ld c
  st dn_l
  ld dn_h
  add t_year                ; + year * 256 = year * 365
  st dn_h
  ld t_year
  add #3
  ror
  ror
  and #$3f
  callf dn_add8
  ld t_month
  sub #1
  rol
  st tmp0
  mov #<cum_days, trl
  mov #>cum_days, trh
  ld tmp0
  ldc
  callf dn_add8
  ld tmp0
  inc acc
  ldc
  add dn_h
  st dn_h
  ld t_year
  and #3
  bnz .noleap
  ld t_month
  sub #3
  bp psw, 7, .noleap
  mov #1, acc
  callf dn_add8
.noleap:
  ld t_day
  sub #1
  callf dn_add8
  ; a day starts at 4 AM: before that it's still yesterday
  ld t_hour
  sub #4
  bn psw, 7, .after4
  ld dn_l
  sub #1
  st dn_l
  ld dn_h
  subc #0
  st dn_h
  ld t_hour
  add #20
  br .hr
.after4:
.hr:
  ; units = (hours since 4 AM) * 6 + minutes / 10
  st tmp0                   ; hours since 4 AM
  mov #0, acc
  ld tmp0
  st c
  mov #0, acc
  mov #6, b
  mul
  ld c
  st units
  mov #0, acc
  ld t_min
  st c
  mov #0, acc
  mov #10, b
  div
  ld c
  add units
  st units
  ret

dn_add8:
  add dn_l
  st dn_l
  ld dn_h
  addc #0
  st dn_h
  ret

;; time_catch_up: whatever happened since last time: days ended (care judged), the helpers' SPARKS (up to 12 hours),
;; the Egg's time
time_catch_up:
  callf time_read
  ; days passed = dn - O_LASTDAY
  ld dn_l
  sub O_LASTDAY
  st tmp0
  ld dn_h
  subc O_LASTDAY + 1
  st tmp1
  bpo psw, 7, .back          ; the clock went back: nothing happened (never punished)
  ld tmp1
  bnz .many
  ld tmp0
  bz .sameday
  sub #8
  bp psw, 7, .few
.many:
  mov #7, tmp0              ; a week judged at most
.few:
  ; each day: judged, then a new one
  ld tmp0
  st tmp2
.day:
  callf day_end
  dec tmp2
  ld tmp2
  bnz .day
  ; the helpers: one day passed = from last time to the day's end plus today so far; more = 12 hours
  ld tmp0
  bne #1, .full
  mov #144, acc
  sub O_LASTUNIT
  add units
  br .earned
.full:
  mov #72, acc
  br .earned
.sameday:
  ld units
  sub O_LASTUNIT
  bp psw, 7, .back
.earned:
  st earn
  sub #73
  bp psw, 7, .capok
  mov #72, earn
.capok:
  ld earn
  bz .store
  ; SPARKS += helpers * earn
  mov #0, acc
  ld earn
  st c
  mov #0, acc
  ld O_HELPER
  st b
  mov #0, acc
  mul
  push acc
  ld c
  callf sparks_add8
  pop acc
  add O_SPARKS + 1
  st O_SPARKS + 1
  ld O_SPARKS + 2
  addc #0
  st O_SPARKS + 2
  ; the Egg's time
  ld P_STAGE
  bne #1, .stored
  ld P_EGGLEFT
  sub earn
  bn psw, 7, .left
  mov #0, acc
.left:
  st P_EGGLEFT
  bnz .stored
  callf hatch
.stored:
  mov #1, dirty
  mov #1, redraw
.store:
.back:
  ld dn_l
  st O_LASTDAY
  ld dn_h
  st O_LASTDAY + 1
  ld units
  st O_LASTUNIT
  ld dirty
  bz .r
  mov #0, dirty
  callf save_write
.r:
  ret

;; day_end: a day's care judged: under 4 meals or 3 trainings, a mistake each; poop left makes it sick (sick at the
;; day's end: another mistake); all done: 10 EXP. Then a new day.
day_end:
  ld P_STAGE
  bne #2, .new
  ld P_FED
  sub #4
  bn psw, 7, .fedok
  inc P_MISTAKES
.fedok:
  ld P_TRAINED
  sub #3
  bn psw, 7, .trainok
  inc P_MISTAKES
.trainok:
  ld P_FLAGS
  bn acc, 0, .wassick
  inc P_MISTAKES
.wassick:
  ld P_POOP
  bz .nopoop
  set1 P_FLAGS, 0
.nopoop:
  ld P_FED
  sub #4
  bp psw, 7, .new
  ld P_TRAINED
  sub #3
  bp psw, 7, .new
  ld P_POOP
  bnz .new
  mov #10, acc
  callf gain_exp
.new:
  mov #0, P_HUNGER
  mov #0, P_STRENGTH
  mov #0, P_FED
  mov #0, P_TRAINED
  mov #0, P_WINDAY
  ld P_DAYS
  be #255, .r
  inc P_DAYS
.r:
  ret

;; ---------------------------------------------------------------- Eggs, hatching, sending on

;; egg_new: a new Egg: any hatchable MOeMON of the edition, equally likely; its personality, IVs, a 1/4096 shiny, a
;; transfer ID; it hatches in 30 minutes
egg_new:
  ; the RNG gets a stir from the clock
  ld t_min
  xor O_RNG
  st O_RNG
  ld units
  xor O_RNG + 1
  st O_RNG + 1
  mov #P_MAGIC, 0           ; clear the MOeMON
.clr:
  mov #0, acc
  st @R0
  inc 0
  ld 0
  bne #P_END, .clr
  mov #$50, P_MAGIC         ; 'P'
  mov #1, P_STAGE
  mov #1, P_EDITION
  mov #1, P_LEVEL
  mov #3, P_EGGLEFT
  callf rand
  mov #0, acc               ; species = rand % ED_HATCH_COUNT
  ld O_RNG + 1
  st c
  ld O_RNG
  mov #ED_HATCH_COUNT, b
  div
  ld b
  mov #<ed_hatch, trl
  mov #>ed_hatch, trh
  ldc
  st P_SPECIES
  mov #P_PID, idx           ; personality and transfer ID: random
.pid:
  callf rand
  ld idx
  st 0
  ld O_RNG
  st @R0
  inc idx
  ld idx
  bne #P_PID + 4, .pid
  mov #P_TID, idx
.tid:
  callf rand
  ld idx
  st 0
  ld O_RNG + 1
  st @R0
  inc idx
  ld idx
  bne #P_TID + 4, .tid
  mov #P_IVS, idx
.iv:
  callf rand
  ld idx
  st 0
  ld O_RNG
  and #31
  st @R0
  inc idx
  ld idx
  bne #P_IVS + 6, .iv
  callf rand
  ld O_RNG + 1
  and #$0f
  bnz .plain
  ld O_RNG
  bnz .plain
  set1 P_FLAGS, 1           ; shiny (1 in 4096)
.plain:
  mov #1, dirty
  ret

hatch:
  callf hatch_anim
  mov #2, P_STAGE
  mov #1, P_LEVEL
  mov #0, P_EXP
  mov #<s_hatched, trl
  mov #>s_hatched, trh
  callf message
  callf draw_pet
  jmpf react_happy

owner_new:
  mov #O_SPARKS, 0
.clr:
  mov #0, acc
  st @R0
  inc 0
  ld 0
  bne #$76, .clr
  mov #$4f, O_MAGIC
  mov #$a5, O_RNG
  mov #$3c, O_RNG + 1
  ld dn_l
  st O_LASTDAY
  ld dn_h
  st O_LASTDAY + 1
  ld units
  st O_LASTUNIT
  ret

;; rand: O_RNG, a 16-bit Galois LFSR (taps $b400), stepped 8 times
rand:
  ld O_RNG
  or O_RNG + 1
  bnz .ok
  mov #$a5, O_RNG           ; never all zeros
.ok:
  mov #8, tmp3
.step:
  clr1 psw, 7
  ld O_RNG + 1
  rorc
  st O_RNG + 1
  ld O_RNG
  rorc
  st O_RNG
  bn psw, 7, .n
  ld O_RNG + 1
  xor #$b4
  st O_RNG + 1
.n:
  dec tmp3
  ld tmp3
  bnz .step
  ret

;; ---------------------------------------------------------------- SPARKS

sparks_add8:
  add O_SPARKS
  st O_SPARKS
  ld O_SPARKS + 1
  addc #0
  st O_SPARKS + 1
  ld O_SPARKS + 2
  addc #0
  st O_SPARKS + 2
  bn psw, 7, .ok
  mov #$ff, O_SPARKS        ; no more than 16 million
  mov #$ff, O_SPARKS + 1
  mov #$ff, O_SPARKS + 2
.ok:
  ret


;; sparks_take: trh:trl = a price (3 bytes, low first): acc = 0 and paid, or acc = 1 (not enough)
sparks_take:
  mov #0, acc
  ldc
  st tmp0
  mov #1, acc
  ldc
  st tmp1
  mov #2, acc
  ldc
  st tmp2
  ld O_SPARKS
  sub tmp0
  st chk
  ld O_SPARKS + 1
  subc tmp1
  st idx
  ld O_SPARKS + 2
  subc tmp2
  bp psw, 7, .short
  st O_SPARKS + 2
  ld idx
  st O_SPARKS + 1
  ld chk
  st O_SPARKS
  mov #0, acc
  ret
.short:
  mov #1, acc
  ret

;; print_sparks: the SPARKS at c, b, without leading zeros (to 999999)
print_sparks:
  ld O_SPARKS
  st tmp0
  ld O_SPARKS + 1
  st tmp1
  ld O_SPARKS + 2
  st tmp2
  ; digits from 100000 down by subtraction
  mov #0, idx               ; which power
  mov #0, chk               ; a digit printed yet
.pow:
  ld idx
  mov #<pow10, trl
  mov #>pow10, trh
  st tmp3
  rol
  add tmp3                  ; 3 bytes each
  st tmp3
  ld tmp3                   ; the power's 3 bytes first (an add between the subtractions would lose the borrow)
  ldc
  st lcd_t0
  ld tmp3
  inc acc
  ldc
  st lcd_t1
  ld tmp3
  add #2
  ldc
  st lcd_t2
  mov #48, digits           ; the digit, as a character
.sub:
  ld tmp0
  sub lcd_t0
  st lcd_t4
  ld tmp1
  subc lcd_t1
  st lcd_t5
  ld tmp2
  subc lcd_t2
  bp psw, 7, .digit
  st tmp2
  ld lcd_t5
  st tmp1
  ld lcd_t4
  st tmp0
  inc digits
  ld digits
  bne #58, .sub
  mov #57, digits           ; over 999999: 9s
.digit:
  ld digits
  be #48, .zero
  mov #1, chk
.zero:
  ld chk
  bnz .put
  ld idx
  be #5, .put               ; the last digit always
  br .next
.put:
  ld digits
  callf lcd_char
  inc c
.next:
  inc idx
  ld idx
  bne #6, .pow
  mov #83, acc              ; 'S'
  callf lcd_char
  inc c
  ret

pow10:
  .byte $a0, $86, $01       ; 100000
  .byte $10, $27, $00       ; 10000
  .byte $e8, $03, $00       ; 1000
  .byte $64, $00, $00       ; 100
  .byte $0a, $00, $00       ; 10
  .byte $01, $00, $00       ; 1

;; print2: acc = 0-99 as two digits at c (one if under 10)
print2:
  st tmp0
  mov #0, tmp1
.t:
  ld tmp0
  sub #10
  bp psw, 7, .u
  st tmp0
  inc tmp1
  br .t
.u:
  ld tmp1
  bz .one
  add #48
  callf lcd_char
  inc c
.one:
  ld tmp0
  add #48
  callf lcd_char
  inc c
  ret

;; print_hearts: acc = 0-4 hearts as "****" (filled) and "----"
print_hearts:
  st tmp0
  mov #0, tmp1
.h:
  ld tmp1
  sub tmp0
  mov #42, acc              ; '*'
  bp psw, 7, .put
  mov #45, acc              ; '-'
.put:
  callf lcd_char
  inc c
  inc tmp1
  ld tmp1
  bne #4, .h
  ret

;; ---------------------------------------------------------------- the status pages

STATUS_PAGES = 4

key_status:
  ld keys_new
  bp acc, BTN_B, .back
  bp acc, BTN_A, .back
  bp acc, BTN_RIGHT, .next
  bp acc, BTN_DOWN, .next
  bp acc, BTN_LEFT, .prev
  bp acc, BTN_UP, .prev
  ret
.back:
  mov #SCR_MAIN, screen
  mov #1, redraw
  ret
.next:
  inc page
  ld page
  bne #STATUS_PAGES, .d
  mov #0, page
  br .d
.prev:
  ld page
  bnz .p
  mov #STATUS_PAGES, page
.p:
  dec page
.d:
  mov #1, redraw
  ret

draw_status:
  ld page
  be #0, .p0
  be #1, .p1
  be #2, .p2
  jmpf status_sparks
.p0:
  jmpf status_name
.p1:
  jmpf status_care
.p2:
  jmpf status_today

;; page 0: its name, level, EXP, age, mistakes (an Egg: when it hatches)
status_name:
  ld P_STAGE
  be #2, .mon
  mov #<s_egg_title, trl
  mov #>s_egg_title, trh
  mov #0, c
  mov #0, b
  callf lcd_text
  mov #<s_hatches, trl
  mov #>s_hatches, trh
  mov #0, c
  mov #2, b
  callf lcd_text
  mov #0, c
  mov #3, b
  ld P_EGGLEFT
  callf print2
  mov #48, acc
  callf lcd_char
  inc c
  inc c
  mov #<s_min, trl
  mov #>s_min, trh
  callf lcd_text
  ret
.mon:
  callf species_name
  mov #0, c
  mov #0, b
  callf lcd_text
  ld P_FLAGS
  bn acc, 1, .plain
  mov #11, c
  mov #42, acc              ; '*': shiny
  callf lcd_char
.plain:
  mov #<s_lv, trl
  mov #>s_lv, trh
  mov #0, c
  mov #1, b
  callf lcd_text
  ld P_LEVEL
  callf print2
  inc c
  mov #<s_exp, trl
  mov #>s_exp, trh
  callf lcd_text
  ld P_EXP
  add #48
  callf lcd_char
  inc c
  mov #47, acc
  callf lcd_char
  inc c
  mov #56, acc
  callf lcd_char
  mov #<s_age, trl
  mov #>s_age, trh
  mov #0, c
  mov #2, b
  callf lcd_text
  ld P_DAYS
  callf print2
  mov #<s_mistakes, trl
  mov #>s_mistakes, trh
  mov #0, c
  mov #3, b
  callf lcd_text
  ld P_MISTAKES
  callf print2
  ret

;; species_name: trh:trl = the MOeMON's name
species_name:
  ld P_SPECIES
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

status_care:
  mov #<s_hunger, trl
  mov #>s_hunger, trh
  mov #0, c
  mov #0, b
  callf lcd_text
  mov #0, c
  mov #1, b
  ld P_HUNGER
  callf print_hearts
  mov #<s_strength, trl
  mov #>s_strength, trh
  mov #0, c
  mov #2, b
  callf lcd_text
  mov #0, c
  mov #3, b
  ld P_STRENGTH
  callf print_hearts
  mov #<s_healthy, trl
  mov #>s_healthy, trh
  ld P_FLAGS
  bn acc, 0, .h
  mov #<s_sick, trl
  mov #>s_sick, trh
.h:
  mov #0, c
  mov #4, b
  callf lcd_text
  ret

status_today:
  mov #<s_today, trl
  mov #>s_today, trh
  mov #0, c
  mov #0, b
  callf lcd_text
  mov #<s_fed, trl
  mov #>s_fed, trh
  mov #0, c
  mov #1, b
  callf lcd_text
  ld P_FED
  add #48
  callf lcd_char
  inc c
  mov #<s_of4, trl
  mov #>s_of4, trh
  callf lcd_text
  mov #<s_trainedn, trl
  mov #>s_trainedn, trh
  mov #0, c
  mov #2, b
  callf lcd_text
  ld P_TRAINED
  callf print2
  mov #<s_of3, trl
  mov #>s_of3, trh
  callf lcd_text
  mov #<s_poopn, trl
  mov #>s_poopn, trh
  mov #0, c
  mov #3, b
  callf lcd_text
  ld P_POOP
  add #48
  callf lcd_char
  ret

status_sparks:
  mov #<s_sparks_t, trl
  mov #>s_sparks_t, trh
  mov #0, c
  mov #0, b
  callf lcd_text
  mov #0, c
  mov #1, b
  callf print_sparks
  mov #<s_taplv, trl
  mov #>s_taplv, trh
  mov #0, c
  mov #2, b
  callf lcd_text
  ld O_TAP
  add O_PRESTIGE
  add #1
  callf print2
  mov #<s_helpers, trl
  mov #>s_helpers, trh
  mov #0, c
  mov #3, b
  callf lcd_text
  ld O_HELPER
  callf print2
  ret

;; ---------------------------------------------------------------- the shop

SHOP_ITEMS = 7

key_shop:
  ld keys_new
  bp acc, BTN_B, .back
  bp acc, BTN_UP, .up
  bp acc, BTN_DOWN, .down
  bp acc, BTN_A, .buy
  ret
.back:
  mov #SCR_MENU, screen
  mov #6, cursor
  mov #3, scroll
  mov #1, redraw
  ret
.up:
  ld cursor
  bz .r
  dec cursor
  br .sc
.down:
  ld cursor
  be #SHOP_ITEMS - 1, .r
  inc cursor
.sc:
  ld cursor
  sub scroll
  bp psw, 7, .above
  sub #3
  bp psw, 7, .dr
  ld cursor
  sub #2
  st scroll
  br .dr
.above:
  ld cursor
  st scroll
.dr:
  mov #1, redraw
.r:
  ret
.buy:
  jmpf shop_buy

;; the price of item cursor into trh:trl (3 bytes): TAP UP and HELPER by their level
shop_price:
  ld cursor
  bz .tap
  be #1, .helper
  sub #2
  st tmp0
  rol
  add tmp0
  st tmp0
  mov #<prices, trl
  mov #>prices, trh
  br .at
.tap:
  ld O_TAP
  br .lv
.helper:
  ld O_HELPER
.lv:
  st tmp0
  rol
  add tmp0
  st tmp0
  mov #<level_prices, trl
  mov #>level_prices, trh
.at:
  ld trl
  add tmp0
  st trl
  ld trh
  addc #0
  st trh
  ret

shop_buy:
  ; can it be bought at all?
  ld cursor
  be #0, .tapok
  be #1, .helpok
  be #2, .candy
  be #3, .ivok
  be #4, .shinyok
  br .ballok
.tapok:
  ld O_TAP
  beo #9, .maxed
  br .pay
.helpok:
  ld O_HELPER
  beo #9, .maxed
  br .pay
.candy:
  ld P_STAGE
  bneo #2, .notyet
  ld P_LEVEL
  be #MAX_LEVEL, .maxed
  br .pay
.ivok:
  ld P_IVUPS
  be #6, .maxed
  br .pay
.shinyok:
  ld P_FLAGS
  bp acc, 1, .maxed
  br .pay
.ballok:
  ld cursor
  be #5, .master
  ld P_ASTRAL
  be #9, .maxed
  br .pay
.master:
  ld P_MASTER
  be #9, .maxed
.pay:
  callf shop_price
  callf sparks_take
  bnz .short
  ld cursor
  be #0, .gtap
  be #1, .ghelp
  be #2, .gcandy
  be #3, .giv
  be #4, .gshiny
  be #5, .gmaster
  inc P_ASTRAL
  br .bought
.gtap:
  inc O_TAP
  br .bought
.ghelp:
  inc O_HELPER
  br .bought
.gcandy:
  ld P_EXP
  mov #EXP_LEVEL, acc
  callf gain_exp
  br .bought
.giv:
  inc P_IVUPS
  br .bought
.gshiny:
  set1 P_FLAGS, 1
  br .bought
.gmaster:
  inc P_MASTER
.bought:
  callf save_write
  mov #<s_bought, trl
  mov #>s_bought, trh
  jmpf message
.short:
  mov #<s_short, trl
  mov #>s_short, trh
  jmpf message
.maxed:
  mov #<s_maxed, trl
  mov #>s_maxed, trh
  jmpf message
.notyet:
  mov #<s_egg_cant, trl
  mov #>s_egg_cant, trh
  jmpf message

draw_shop:
  mov #0, c
  mov #0, b
  callf print_sparks
  mov #0, tmp2              ; the line
.line:
  ld scroll
  add tmp2
  st tmp3
  sub #SHOP_ITEMS
  bn psw, 7, .done
  ld tmp3
  rol
  st tmp0
  mov #<shop_texts, trl
  mov #>shop_texts, trh
  ld tmp0
  ldc
  st tmp1
  ld tmp0
  inc acc
  ldc
  st trh
  ld tmp1
  st trl
  ld tmp2
  add #1
  st b
  mov #1, c
  callf lcd_text
  ld tmp3
  bne cursor, .nocur
  mov #0, c
  mov #62, acc
  callf lcd_char
.nocur:
  inc tmp2
  ld tmp2
  bne #3, .line
.done:
  ; the price of the one at the cursor
  mov #<s_price, trl
  mov #>s_price, trh
  mov #0, c
  mov #4, b
  callf lcd_text
  callf shop_price
  mov #0, acc
  ldc
  st O_TMP_A
  mov #1, acc
  ldc
  st O_TMP_B
  mov #2, acc
  ldc
  st O_TMP_C
  ; print it with the SPARKS printer: swap the price in for a moment
  ld O_SPARKS
  push acc
  ld O_SPARKS + 1
  push acc
  ld O_SPARKS + 2
  push acc
  ld O_TMP_A
  st O_SPARKS
  ld O_TMP_B
  st O_SPARKS + 1
  ld O_TMP_C
  st O_SPARKS + 2
  callf print_sparks
  pop acc
  st O_SPARKS + 2
  pop acc
  st O_SPARKS + 1
  pop acc
  st O_SPARKS
  ret

O_TMP_A = $78
O_TMP_B = $79
O_TMP_C = $7a

shop_texts:
  .word s_i_tap, s_i_helper, s_i_candy, s_i_iv, s_i_shiny, s_i_master, s_i_astral

;; TAP UP and HELPER: by their level (0-8)
level_prices:
  .byte 50, 0, 0
  .byte 150, 0, 0
  .byte $c2, $01, 0         ; 450
  .byte $e8, $03, 0         ; 1000
  .byte $d0, $07, 0         ; 2000
  .byte $88, $13, 0         ; 5000
  .byte $10, $27, 0         ; 10000
  .byte $20, $4e, 0         ; 20000
  .byte $50, $c3, 0         ; 50000
  .byte $50, $c3, 0
;; RARE CANDY, IV UP, SHINY, MASTER BALL, ASTRAL BALL
prices:
  .byte $2c, $01, 0         ; 300
  .byte $f4, $01, 0         ; 500
  .byte $88, $13, 0         ; 5000
  .byte $10, $27, 0         ; 10000
  .byte $30, $75, 0         ; 30000

;; ---------------------------------------------------------------- LINK (to and from a VM2)

key_link:
  ld keys_new
  bp acc, BTN_B, .back
  bp acc, BTN_UP, .flip
  bp acc, BTN_DOWN, .flip
  bp acc, BTN_A, .go
  ret
.back:
  mov #SCR_MENU, screen
  mov #7, cursor
  mov #4, scroll
  mov #1, redraw
  ret
.flip:
  ld keys_new
  bp acc, BTN_UP, .up
  ld cursor
  be #2, .r
  inc cursor
  br .moved
.up:
  ld cursor
  bz .r
  dec cursor
.moved:
  mov #1, redraw
.r:
  ret
.go:
  ld cursor
  be #1, .recv
  be #2, .battle
  jmpf link_send
.recv:
  jmpf link_receive
.battle:
  jmpf battle_link

draw_link:
  mov #<s_link, trl
  mov #>s_link, trh
  mov #0, c
  mov #0, b
  callf lcd_text
  mov #<s_send, trl
  mov #>s_send, trh
  mov #1, c
  mov #1, b
  callf lcd_text
  mov #<s_receive, trl
  mov #>s_receive, trh
  mov #1, c
  mov #2, b
  callf lcd_text
  mov #<s_m_battle, trl
  mov #>s_m_battle, trh
  mov #1, c
  mov #3, b
  callf lcd_text
  mov #<s_link_hint, trl
  mov #>s_link_hint, trh
  mov #0, c
  mov #4, b
  callf lcd_text
  ld cursor
  add #1
  st b
  mov #0, c
  mov #62, acc
  callf lcd_char
  ret

;; link_status: trh:trl = a line for the LINK screen's bottom line (in place of TOP TO TOP)
link_status:
  push trl
  push trh
  mov #4, b
  callf lcd_clear_line
  pop trh
  pop trl
  mov #0, c
  mov #4, b
  callf lcd_text
  ret

;; link_send: a grown MOeMON (level 50) to the other unit: SYNC, length, its 40 bytes, checksum, each echoed, then
;; the other side's ACK; then a new Egg starts (helpers and TAP UP start over; one more sent on: a SPARK a tap more)
link_send:
  ld P_STAGE
  bne #2, .notready
  ld P_LEVEL
  be #MAX_LEVEL, .ready
.notready:
  mov #<s_not_grown, trl
  mov #>s_not_grown, trh
  jmpf link_status
.ready:
  callf sio_connected
  bz .cable
  mov #<s_nocable, trl
  mov #>s_nocable, trh
  jmpf link_status
.cable:
  mov #<s_sending, trl
  mov #>s_sending, trh
  callf link_status
  callf sio_init
  callf sio_flush
  mov #0, chk
  mov #P_MAGIC, idx
.sum:
  ld idx
  st 0
  ld @R0
  add chk
  st chk
  inc idx
  ld idx
  bne #P_MAGIC + PET_BYTES, .sum
  mov #SIO_SYNC, acc
  callf sio_xfer
  bnz .fail
  mov #PET_BYTES, acc
  callf sio_xfer
  bnz .fail
  mov #P_MAGIC, idx
.send:
  ld idx
  st 0
  ld @R0
  callf sio_xfer
  bnz .fail
  inc idx
  ld idx
  bne #P_MAGIC + PET_BYTES, .send
  ld chk
  callf sio_xfer
  bnz .fail
  callf sio_recv
  bz .ans
  callf sio_recv
  bnz .fail
.ans:
  ld b
  bne #SIO_ACK, .refused
  callf sio_end
  ; it's gone: a new Egg; per pet, the helpers and TAP UP start over; it leaves a SPARK a tap for good
  inc O_PRESTIGE
  mov #0, O_TAP
  mov #0, O_HELPER
  callf egg_new
  callf save_write
  mov #<s_sent, trl
  mov #>s_sent, trh
  jmpf message
.refused:
  callf sio_end
  mov #<s_refused, trl
  mov #>s_refused, trh
  jmpf link_status
.fail:
  callf sio_end
  mov #<s_link_failed, trl
  mov #>s_link_failed, trh
  jmpf link_status

;; link_receive: wait (up to ~10 s, B stops) for a MOeMON from the other unit; taken only in place of an Egg
link_receive:
  callf sio_connected
  bz .cable
  mov #<s_nocable, trl
  mov #>s_nocable, trh
  jmpf link_status
.cable:
  mov #<s_waiting, trl
  mov #>s_waiting, trh
  callf link_status
  callf sio_init
  ld ticks
  st tmp3
.wait:
  callf sio_poll
  bz .got
  ld p3
  bn acc, BTN_B, .stop
  ld ticks
  sub tmp3
  sub #20
  bp psw, 7, .wait
.stop:
  callf sio_end
  mov #<s_link_failed, trl
  mov #>s_link_failed, trh
  jmpf link_status
.got:
  ld b
  bne #SIO_SYNC, .wait
  ld b
  callf sio_send            ; echo
  callf sio_recv
  bnzo .lost
  ld b
  st tmp2
  callf sio_send
  ld tmp2
  bneo #PET_BYTES, .lost
  mov #0, chk
  mov #0, idx
.get:
  callf sio_recv
  bnzo .lost
  ld b
  st tmp2
  callf sio_send
  ld idx
  add #FMBUFF               ; collected in the page buffer first
  st 0
  ld tmp2
  st @R0
  add chk
  st chk
  inc idx
  ld idx
  bne #PET_BYTES, .get
  callf sio_recv
  bnz .lost
  ld b
  st tmp2
  callf sio_send
  callf sio_sent
  callf link_pause
  ld tmp2
  sub chk
  bnz .bad
  ld FMBUFF                 ; a MOeMON's bytes ('P' first), and room for it (only an Egg is replaced)
  bne #$50, .bad
  ld P_STAGE
  be #2, .full
  mov #0, idx
.take:
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
  bne #PET_BYTES, .take
  mov #SIO_ACK, acc
  callf sio_send
  callf sio_sent
  callf sio_end
  callf save_write
  mov #<s_received, trl
  mov #>s_received, trh
  jmpf message
.full:
  mov #SIO_NAK, acc
  callf sio_send
  callf sio_sent
  callf sio_end
  mov #<s_has_mon, trl
  mov #>s_has_mon, trh
  jmpf link_status
.bad:
  mov #SIO_NAK, acc
  callf sio_send
  callf sio_sent
.lost:
  callf sio_end
  mov #<s_link_failed, trl
  mov #>s_link_failed, trh
  jmpf link_status

link_pause:
  mov #200, idx
.l:
  dec idx
  ld idx
  bnz .l
  ret

about:
  mov #<s_about, trl
  mov #>s_about, trh
  jmpf message

;; ---------------------------------------------------------------- saving (the program's own SAVE page)

;; save_load: the state from the SAVE page (the magic first: a fresh program has none)
save_load:
  mov #<save_page, trl
  mov #>save_page, trh
  mov #0, idx
.chk:
  ld idx
  ldc
  st tmp0
  ld idx
  push trl
  push trh
  mov #<save_magic, trl
  mov #>save_magic, trh
  ldc
  pop trh
  pop trl
  bne tmp0, .fresh
  inc idx
  ld idx
  bne #16, .chk
  mov #0, idx
.copy:
  ld idx
  add #16
  ldc
  push acc
  ld idx
  add #P_MAGIC
  st 0
  pop acc
  st @R0
  inc idx
  ld idx
  bne #$7c - P_MAGIC, .copy
  ret
.fresh:
  mov #P_MAGIC, 0
.clr:
  mov #0, acc
  st @R0
  inc 0
  ld 0
  bne #$7c, .clr
  ret

;; save_write: the magic and $40-$7b into the SAVE page, through the BIOS (interrupts off, RC 1/6). Keeps trh:trl
;; (a message's text is often waiting in them).
save_write:
  push acc
  push b
  push c
  push trl
  push trh
  mov #0, idx
.magic:
  mov #<save_magic, trl
  mov #>save_magic, trh
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
  bne #16, .magic
  mov #0, idx
.copy:
  ld idx
  add #P_MAGIC
  st 0
  ld @R0
  push acc
  ld idx
  add #FMBUFF + 16
  st 0
  pop acc
  st @R0
  inc idx
  ld idx
  bne #$7c - P_MAGIC, .copy
  mov #FMBUFF + 16 + $7c - P_MAGIC, 0
.zero:
  mov #0, acc
  st @R0
  inc 0
  ld 0
  bnz .zero
  mov #0, FMBANK
  mov #>save_page, FMADD_H
  mov #<save_page, FMADD_L
  push ocr
  push ie
  clr1 ie, 7
  mov #OSC_RCFW, ocr
  callf fm_wrt_ex
  pop ie
  pop ocr
  pop trh
  pop trl
  pop c
  pop b
  pop acc
  ret

save_magic:
  .text 12 "MOEMONVMUPET"
  .byte 1, 1, 0, 0          ; the format (1), the edition (1 = KANTO)
save_magic_end:

;; ---------------------------------------------------------------- the clocks, the battery

;; clk_fast: RC (drawing, the link); clk_slow: the crystal (HALT). VCCR written whole (write-only).
clk_fast:
  mov #VCCR_RC, vccr
  mov #OSC_RC, ocr
  ret

clk_slow:
  mov #VCCR_XT, vccr
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

int_base_timer:
  push psw
  set1 psw, 1
  inc ticks
  pop psw
  clr1 btcr, 1
  ret

  .include "../inc/sio.inc"
  .include "../inc/lcd.inc"

;; ---------------------------------------------------------------- words

s_lv:
  .string 3 "LV"
s_egg:
  .string 4 "EGG"
s_menu:
  .string 5 "MENU"
s_m_feed:
  .string 5 "FEED"
s_m_train:
  .string 6 "TRAIN"
s_m_clean:
  .string 6 "CLEAN"
s_m_meds:
  .string 9 "MEDICINE"
s_m_status:
  .string 7 "STATUS"
s_m_shop:
  .string 5 "SHOP"
s_m_link:
  .string 5 "LINK"
s_m_battle:
  .string 7 "BATTLE"
s_m_about:
  .string 6 "ABOUT"
s_feed:
  .string 5 "FEED"
s_meat:
  .string 5 "MEAL"
s_vitamin:
  .string 8 "VITAMIN"
s_hunger_h:
  .string 4 "HP "
s_yum:
  .string 5 "YUM!"
s_strong:
  .string 8 "STRONG!"
s_full:
  .string 6 "FULL!"
s_cleaned:
  .string 7 "CLEAN!"
s_clean_already:
  .string 8 "IT'S OK"
s_cured:
  .string 7 "CURED!"
s_not_sick:
  .string 9 "NOT SICK"
s_egg_cant:
  .string 9 "NOT YET!"
s_train:
  .string 10 "TRAINING "
s_track:
  .string 13 "[....##....]"
s_press_a:
  .string 8 "A IN ##"
s_hit:
  .string 5 "HIT!"
s_miss:
  .string 5 "MISS"
s_trained:
  .string 9 "TRAINED!"
s_evolved:
  .string 9 "EVOLVED!"
s_hatched:
  .string 9 "HATCHED!"
s_egg_title:
  .string 7 "AN EGG"
s_hatches:
  .string 11 "HATCHES IN"
s_min:
  .string 4 "MIN"
s_exp:
  .string 5 "EXP "
s_age:
  .string 6 "DAYS "
s_mistakes:
  .string 10 "MISTAKES "
s_hunger:
  .string 7 "HUNGER"
s_strength:
  .string 9 "STRENGTH"
s_healthy:
  .string 8 "HEALTHY"
s_sick:
  .string 6 "SICK!"
s_today:
  .string 6 "TODAY"
s_fed:
  .string 7 "MEALS "
s_of4:
  .string 3 "/4"
s_trainedn:
  .string 7 "TRAIN "
s_of3:
  .string 3 "/3"
s_poopn:
  .string 6 "POOP "
s_sparks_t:
  .string 7 "SPARKS"
s_taplv:
  .string 7 "A TAP "
s_helpers:
  .string 9 "HELPERS "
s_price:
  .string 7 "PRICE "
s_i_tap:
  .string 7 "TAP UP"
s_i_helper:
  .string 7 "HELPER"
s_i_candy:
  .string 11 "RARE CANDY"
s_i_iv:
  .string 6 "IV UP"
s_i_shiny:
  .string 6 "SHINY"
s_i_master:
  .string 7 "MASTER"
s_i_astral:
  .string 7 "ASTRAL"
s_bought:
  .string 8 "BOUGHT!"
s_short:
  .string 11 "NOT ENOUGH"
s_maxed:
  .string 8 "NO MORE"
s_link:
  .string 9 "LINK VM2"
s_send:
  .string 5 "SEND"
s_receive:
  .string 8 "RECEIVE"
s_link_hint:
  .string 11 "TOP TO TOP"
s_not_grown:
  .string 9 "NOT LV50"
s_nocable:
  .string 9 "NO CABLE"
s_sending:
  .string 11 "SENDING..."
s_waiting:
  .string 11 "WAITING..."
s_sent:
  .string 9 "SENT ON!"
s_received:
  .string 9 "WELCOME!"
s_refused:
  .string 10 "NOT TAKEN"
s_has_mon:
  .string 8 "HAS ONE"
s_link_failed:
  .string 8 "NO LINK"
s_about:
  .string 13 "MOEMON KANTO"

cum_days:
  .word 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334

ic_poop:
  .byte $10, $28, $54, $38, $44, $ba, $82, $7c
ic_sick:
  .byte $3c, $7e, $5a, $7e, $3c, $24, $18, $00
ic_overlays:                ; 1 a heart, 2 a sweat drop, 3-5 a meal: whole, bitten, crumbs
  .byte $00, $6c, $fe, $fe, $7c, $38, $10, $00
  .byte $10, $10, $38, $38, $7c, $7c, $38, $00
  .byte $00, $3c, $7e, $ff, $ff, $7e, $3c, $00
  .byte $00, $3c, $7e, $f0, $e0, $70, $3c, $00
  .byte $00, $00, $00, $50, $20, $50, $00, $00

  .include "font.inc"
  .include "../inc/rev_bits.inc"
  .include "kanto.inc"
  .include "icon.inc"
  .include "battle.inc"

  ;; The SAVE page: 128 bytes on a 128-byte boundary (the magic, then $40-$7b)
  .cnop 0, $80
save_page:
  .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
  .byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

  .cnop 0, $200
