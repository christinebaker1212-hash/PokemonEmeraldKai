;-| Button Remapping |-----------------------------------------------------
; This section lets you remap the player's buttons (to easily change the
; button configuration). The format is:
;   old_button = new_button
; If new_button is left blank, the button cannot be pressed.
[Remap]
x = x
y = y
z = z
a = a
b = b
c = c
s = s

;-| Default Values |-------------------------------------------------------
[Defaults]
; Default value for the "time" parameter of a Command. Minimum 1.
command.time = 15

; Default value for the "buffer.time" parameter of a Command. Minimum 1,
; maximum 30.
command.buffer.time = 1

;-| Super Motions |--------------------------------------------------------
; Super Izuna Drop
[Command]
name = "sid"
command = ~50$DB, $F, $B, $U, a
time = 50
[Command]
name = "sid"
command = ~50$DB, $F, $B, $U, b
time = 50
[Command]
name = "sid"
command = ~50$DB, $F, $B, $U, c
time = 50
[Command]
name = "sid"
command = ~50$DB, $F, $B, $U, ~a
time = 50
[Command]
name = "sid"
command = ~50$DB, $F, $B, $U, ~b
time = 50
[Command]
name = "sid"
command = ~50$DB, $F, $B, $U, ~c
time = 50
[Command]
name = "sid_s"
command = ~50$DB, $F, $B, $U, s
time = 50
[Command]
name = "sid_s"
command = ~50$DB, $F, $B, $U, ~s
time = 50
[Command]
name = "sid_s"
command = ~50$DB, $F, $B, $U, a+b
time = 50
[Command]
name = "sid_s"
command = ~50$DB, $F, $B, $U, b+c
time = 50
[Command]
name = "sid_s"
command = ~50$DB, $F, $B, $U, a+c
time = 50
[Command]
name = "sid_s"
command = ~50$DB, $F, $B, $U, a+b+c
time = 50

; Super Claw
[Command]
name = "superclaw"
command = ~50$B, $F, $B, $F, x
time = 50
[Command]
name = "superclaw"
command = ~50$B, $F, $B, $F, y
time = 50
[Command]
name = "superclaw"
command = ~50$B, $F, $B, $F, z
time = 50
[Command]
name = "superclaw"
command = ~50$B, $F, $B, $F, ~x
time = 50
[Command]
name = "superclaw"
command = ~50$B, $F, $B, $F, ~y
time = 50
[Command]
name = "superclaw"
command = ~50$B, $F, $B, $F, ~z
time = 50

; Super Kick Flip
[Command]
name = "skflip"
command = ~50$B, $F, $B, $F, a
time = 50
[Command]
name = "skflip"
command = ~50$B, $F, $B, $F, b
time = 50
[Command]
name = "skflip"
command = ~50$B, $F, $B, $F, c
time = 50
[Command]
name = "skflip"
command = ~50$B, $F, $B, $F, ~a
time = 50
[Command]
name = "skflip_"
command = ~50$B, $F, $B, $F, ~b
time = 50
[Command]
name = "skflip"
command = ~50$B, $F, $B, $F, ~c
time = 50

;-| Special Motions |------------------------------------------------------
; Backflip
[Command]
name = "backflip_p"
command = x+y
[Command]
name = "backflip_p"
command = y+z
[Command]
name = "backflip_p"
command = x+z
[Command]
name = "backflip_p"
command = x+y+z
[Command]
name = "backflip_k"
command = a+b
[Command]
name = "backflip_k"
command = b+c
[Command]
name = "backflip_k"
command = a+c
[Command]
name = "backflip_k"
command = a+b+c

; Rolling Claw
[Command]
name = "roll_x"
command = ~50$B, F, x
[Command]
name = "roll_y"
command = ~50$B, F, y
[Command]
name = "roll_z"
command = ~50$B, F, z
[Command]
name = "roll_x"
command = ~50$B, F, ~x
[Command]
name = "roll_y"
command = ~50$B, F, ~y
[Command]
name = "roll_z"
command = ~50$B, F, ~z
[Command]
name = "roll_s"
command = ~50$B, F, s
[Command]
name = "roll_s"
command = ~50$B, F, ~s

; Sky High Claw
[Command]
name = "climb_x"
command = ~50$D, $U, x
[Command]
name = "climb_y"
command = ~50$D, $U, y
[Command]
name = "climb_z"
command = ~50$D, $U, z
[Command]
name = "climb_x"
command = ~50$D, $U, ~x
[Command]
name = "climb_y"
command = ~50$D, $U, ~y
[Command]
name = "climb_z"
command = ~50$D, $U, ~z

; Wall Cling Thrust
[Command]
name = "climb_a"
command = ~50$D, $U, a
[Command]
name = "climb_b"
command = ~50$D, $U, b
[Command]
name = "climb_c"
command = ~50$D, $U, c
[Command]
name = "climb_a"
command = ~50$D, $U, ~a
[Command]
name = "climb_b"
command = ~50$D, $U, ~b
[Command]
name = "climb_c"
command = ~50$D, $U, ~c
[Command]
name = "climb_s"
command = ~50$D, $U, s
[Command]
name = "climb_s"
command = ~50$D, $U, ~s
[Command]
name = "climb_s"
command = ~50$D, $U, a+b
[Command]
name = "climb_s"
command = ~50$D, $U, b+c
[Command]
name = "climb_s"
command = ~50$D, $U, a+c
[Command]
name = "climb_s"
command = ~50$D, $U, a+b+c

; Kick Flip
[Command]
name = "kflip_a"
command = ~50$DB, F, a
[Command]
name = "kflip_b"
command = ~50$DB, F, b
[Command]
name = "kflip_c"
command = ~50$DB, F, c
[Command]
name = "kflip_a"
command = ~50$DB, F, ~a
[Command]
name = "kflip_b"
command = ~50$DB, F, ~b
[Command]
name = "kflip_c"
command = ~50$DB, F, ~c

;-| Double Tap |-----------------------------------------------------------
[Command]
name = "FF"     ;Required (do not remove)
command = F, F
time = 10

[Command]
name = "BB"     ;Required (do not remove)
command = B, B
time = 10

;-| 2/3 Button Combination |-----------------------------------------------
[Command]
name = "recovery" ;Required (do not remove)
command = x+y
time = 1

[Command]
name = "recovery"
command = y+z
time = 1

[Command]
name = "recovery"
command = x+z
time = 1

[Command]
name = "recovery"
command = a+b
time = 1

[Command]
name = "recovery"
command = b+c
time = 1

[Command]
name = "recovery"
command = a+c
time = 1

;-| Dir + Button |---------------------------------------------------------
[Command]
name = "back_x"
command = /$B,x
time = 1

[Command]
name = "back_y"
command = /$B,y
time = 1

[Command]
name = "back_z"
command = /$B,z
time = 1

[Command]
name = "down_x"
command = /$D,x
time = 1

[Command]
name = "down_y"
command = /$D,y
time = 1

[Command]
name = "down_z"
command = /$D,z
time = 1

[Command]
name = "fwd_x"
command = /$F,x
time = 1

[Command]
name = "fwd_y"
command = /$F,y
time = 1

[Command]
name = "fwd_z"
command = /$F,z
time = 1

[Command]
name = "up_x"
command = /$U,x
time = 1

[Command]
name = "up_y"
command = /$U,y
time = 1

[Command]
name = "up_z"
command = /$U,z
time = 1

[Command]
name = "back_a"
command = /$B,a
time = 1

[Command]
name = "back_b"
command = /$B,b
time = 1

[Command]
name = "back_c"
command = /$B,c
time = 1

[Command]
name = "down_a"
command = /$D,a
time = 1

[Command]
name = "down_b"
command = /$D,b
time = 1

[Command]
name = "down_c"
command = /$D,c
time = 1

[Command]
name = "fwd_a"
command = /$F,a
time = 1

[Command]
name = "fwd_b"
command = /$F,b
time = 1

[Command]
name = "fwd_c"
command = /$F,c
time = 1

[Command]
name = "up_a"
command = /$U,a
time = 1

[Command]
name = "up_b"
command = /$U,b
time = 1

[Command]
name = "up_c"
command = /$U,c
time = 1

;-| Single Button |---------------------------------------------------------
[Command]
name = "a"
command = a
time = 1

[Command]
name = "b"
command = b
time = 1

[Command]
name = "c"
command = c
time = 1

[Command]
name = "x"
command = x
time = 1

[Command]
name = "y"
command = y
time = 1

[Command]
name = "z"
command = z
time = 1

[Command]
name = "start"
command = s
time = 1

;-| Single Dir |------------------------------------------------------------
[Command]
name = "fwd" ;Required (do not remove)
command = $F
time = 1

[Command]
name = "downfwd"
command = $DF
time = 1

[Command]
name = "down" ;Required (do not remove)
command = $D
time = 1

[Command]
name = "downback"
command = $DB
time = 1

[Command]
name = "back" ;Required (do not remove)
command = $B
time = 1

[Command]
name = "upback"
command = $UB
time = 1

[Command]
name = "up" ;Required (do not remove)
command = $U
time = 1

[Command]
name = "upfwd"
command = $UF
time = 1

;-| Hold Button |--------------------------------------------------------------
[Command]
name = "hold_x"
command = /x
time = 1

[Command]
name = "hold_y"
command = /y
time = 1

[Command]
name = "hold_z"
command = /z
time = 1

[Command]
name = "hold_a"
command = /a
time = 1

[Command]
name = "hold_b"
command = /b
time = 1

[Command]
name = "hold_c"
command = /c
time = 1

[Command]
name = "hold_s"
command = /s
time = 1

;-| Hold Dir |--------------------------------------------------------------
[Command]
name = "holdfwd" ;Required (do not remove)
command = /$F
time = 1

[Command]
name = "holddownfwd"
command = /$DF
time = 1

[Command]
name = "holddown" ;Required (do not remove)
command = /$D
time = 1

[Command]
name = "holddownback"
command = /$DB
time = 1

[Command]
name = "holdback" ;Required (do not remove)
command = /$B
time = 1

[Command]
name = "holdupback"
command = /$UB
time = 1

[Command]
name = "holdup" ;Required (do not remove)
command = /$U
time = 1

[Command]
name = "holdupfwd"
command = /$UF
time = 1

;---------------------------------------------------------------------------
; Don't remove the following line. It's required by the CMD standard.
[Statedef -1]

[State -1, VarSet]
type = VarSet
trigger1 = Var(2) > 0
trigger1 = StateNo < 1000 || StateNo >= 5000
trigger1 = MoveType != A
ignorehitpause = 1
v = 2
value = 0

;--------------------
[State -1, VarAdd]
type = VarAdd
triggerall = Ctrl
triggerall = Var(59) <= 0
trigger1 = AILevel <= 0
trigger1 = Name != "SF2'CE Vega"
trigger1 = Command != "climb_s"
trigger1 = Command != "roll_s"
trigger1 = Command != "sid_s"
trigger1 = Command = "start"
ignorehitpause = 1
v = 51
value = 1

[State -1, VarSet]
type = VarSet
trigger1 = Var(51) > 1
trigger2 = AILevel > 0
trigger2 = Name != "SF2'CE Vega"
ignorehitpause = 1
v = 51
value = 0

[State -1, VarSet]
type = VarSet
trigger1 = Name = "SF2'CE Vega"
ignorehitpause = 1
v = 51
value = 1
;--------------------

;===========================================================================
;---------------------------------------------------------------------------
; Super Moves
;---------------------------------------------------------------------------
; Super Claw
[State -1, State]
type = ChangeState
value = 3000
triggerall = Var(20) <= 0
triggerall=  Var(51) <= 0
triggerall = AILevel <= 0
triggerall = Power >= 1000
triggerall = command = "superclaw"
trigger1 = statetype != A
trigger1 = ctrl

;---------------------------------------------------------------------------
; Super Kick Flip
[State -1, State]
type = ChangeState
value = 3500
triggerall = Var(51) <= 0
triggerall = AILevel <= 0
triggerall = Power >= 1000
triggerall = command = "skflip"
trigger1 = statetype != A
trigger1 = ctrl

;===========================================================================
;---------------------------------------------------------------------------
; Special Moves
;---------------------------------------------------------------------------
; Kick Flip
[State -1, State]
type = ChangeState
value = 1500
triggerall = Var(51) <= 0
triggerall = AILevel <= 0
triggerall = command = "kflip_a" || command = "kflip_b" || command = "kflip_c"
trigger1 = statetype != A
trigger1 = ctrl

;---------------------------------------------------------------------------
; Sky High Claw
[State -1, State]
type = ChangeState
value = 1210
triggerall = Var(51) <= 0
triggerall = AILevel <= 0
triggerall = command = "climb_x" || command = "climb_y" || command = "climb_z"
triggerall = statetype != A
trigger1 = ctrl
trigger2 = stateno = 40

;---------------------------------------------------------------------------
; Wall Cling Thrust Jump
[State -1, State]
type = ChangeState
value = 1100
triggerall = Var(51) <= 0
triggerall = Var(58) > 0
triggerall = AILevel <= 0
triggerall = Name != "SF2'CE Vega" 
triggerall = command = "climb_s" || Power >= 1000 && command = "sid_s"
triggerall = statetype != A
trigger1 = ctrl
trigger2 = stateno = 40

[State -1, State]
type = ChangeState
value = IfElse(Var(58) > 0, 1100, 1130)
triggerall = AILevel <= 0
triggerall = Name = "SF2'CE Vega" 
triggerall = command = "climb_a" || command = "climb_b" || command = "climb_c"
triggerall = statetype != A
trigger1 = ctrl
trigger2 = stateno = 40

[State -1, State]
type = ChangeState
value = IfElse(Var(51) > 0 && Var(58) > 0, 1100, 1130)
triggerall = AILevel <= 0
triggerall = Name != "SF2'CE Vega" 
triggerall = command = "climb_a" || command = "climb_b" || command = "climb_c" || Var(51) <= 0 && Power >= 1000 && command = "sid"
triggerall = statetype != A
trigger1 = ctrl
trigger2 = stateno = 40

;---------------------------------------------------------------------------
; Rolling Claw
[State -1, State]
type = ChangeState
value = 1000
triggerall = AILevel <= 0
triggerall = command = "roll_x" || command = "roll_y" || command = "roll_z" || Var(51) <= 0 && command = "roll_s"
trigger1 = statetype != A
trigger1 = ctrl

;---------------------------------------------------------------------------
; Backflip
[State -1, State]
type = ChangeState
value = 900
triggerall = ctrl
triggerall = AILevel <= 0
triggerall = statetype = S
triggerall = command != "holddown"
trigger1 = Var(51) <= 0
trigger1 = command = "backflip_p"
trigger2 = Var(51) > 0
trigger2 = command = "BB"

; Short Backflip
[State -1, State]
type = ChangeState
value = 910
triggerall = AILevel <= 0
triggerall = statetype = S
triggerall = command != "holddown"
trigger1 = command = "backflip_k"
trigger1 = Var(51) <= 0
trigger1 = ctrl

;===========================================================================
;---------------------------------------------------------------------------
; Throw
[State -1, State]
type = ChangeState
value = 800
triggerall = AILevel <= 0
triggerall = command = "y" || command = "z"
triggerall = statetype = S
triggerall = ctrl
triggerall = stateno != 100
trigger1 = command = "holdfwd"
trigger1 = p2bodydist X <= 20
trigger1 = (p2statetype = S) || (p2statetype = C)
trigger1 = p2movetype != H || p2stateno = 5300
trigger2 = command = "holdback"
trigger2 = p2bodydist X <= 20
trigger2 = (p2statetype = S) || (p2statetype = C)
trigger2 = p2movetype != H || p2stateno = 5300

; Air Throw
[State -1, State]
type = ChangeState
value = 810
triggerall = Vel X > 0
triggerall = Pos Y > P2BodyDist Y
triggerall = AILevel <= 0
triggerall = command = "y" || command = "z"
triggerall = statetype = A
triggerall = ctrl
trigger1 = command = "holdfwd" || command = "holdback"
trigger1 = p2bodydist X <= 20
trigger1 = p2bodydist Y < 40
trigger1 = p2statetype = A
trigger1 = p2movetype != H

;===========================================================================
;---------------------------------------------------------------------------
; Basic Moves - [SF2X Mode]
;---------------------------------------------------------------------------
; Stand Light Punch
[State -1, State]
type = ChangeState
value = 200
triggerall = Var(51) <= 0
triggerall = AILevel <= 0
triggerall = command = "x"
triggerall = command != "holddown"
trigger1 = statetype = S
trigger1 = ctrl

; Stand Medium Punch
[State -1, State]
type = ChangeState
value = 210
triggerall = Var(51) <= 0
triggerall = AILevel <= 0
triggerall = command = "y"
triggerall = command != "holddown"
trigger1 = statetype = S
trigger1 = ctrl

; Stand Strong Punch
[State -1, State]
type = ChangeState
value = 220
triggerall = Var(51) <= 0
triggerall = AILevel <= 0
triggerall = command = "z"
triggerall = command != "holddown"
trigger1 = statetype = S
trigger1 = ctrl

;---------------------------------------------------------------------------
; Stand Light Kick
[State -1, State]
type = ChangeState
value = 230
triggerall = Var(51) <= 0
triggerall = AILevel <= 0
triggerall = command = "a"
triggerall = command != "holddown"
trigger1 = statetype = S
trigger1 = ctrl

; Stand Medium Kick
[State -1, State]
type = ChangeState
value = 240
triggerall = Var(51) <= 0
triggerall = AILevel <= 0
triggerall = command = "b"
triggerall = command != "holddown"
trigger1 = statetype = S
trigger1 = ctrl

; Stand Strong Kick
[State -1, State]
type = ChangeState
value = 250
triggerall = Var(51) <= 0
triggerall = AILevel <= 0
triggerall = command = "c"
triggerall = command != "holddown"
trigger1 = statetype = S
trigger1 = ctrl

;---------------------------------------------------------------------------
; Crouch Light Punch
[State -1, State]
type = ChangeState
value = 400
triggerall = Var(51) <= 0
triggerall = AILevel <= 0
triggerall = command = "x"
triggerall = command = "holddown"
trigger1 = statetype = C
trigger1 = ctrl

; Crouch Medium Punch
[State -1, State]
type = ChangeState
value = 410
triggerall = Var(51) <= 0
triggerall = AILevel <= 0
triggerall = command = "y"
triggerall = command = "holddown"
trigger1 = statetype = C
trigger1 = ctrl

; Crouch Strong Punch
[State -1, State]
type = ChangeState
value = 420
triggerall = Var(51) <= 0
triggerall = AILevel <= 0
triggerall = command = "z"
triggerall = command = "holddown"
trigger1 = statetype = C
trigger1 = ctrl

;---------------------------------------------------------------------------
; Crouch Light Kick
[State -1, State]
type = ChangeState
value = 430
triggerall = Var(51) <= 0
triggerall = AILevel <= 0
triggerall = command = "a"
triggerall = command = "holddown"
trigger1 = statetype = C
trigger1 = ctrl

; Crouch Medium Kick
[State -1, State]
type = ChangeState
value = 440
triggerall = Var(51) <= 0
triggerall = AILevel <= 0
triggerall = command = "b"
triggerall = command = "holddown"
trigger1 = statetype = C
trigger1 = ctrl

; Crouch Slide Kick
[State -1, State]
type = ChangeState
value = 450
triggerall = Var(51) <= 0
triggerall = AILevel <= 0
triggerall = command = "c"
triggerall = command = "holddown"
trigger1 = statetype = C
trigger1 = ctrl

;---------------------------------------------------------------------------
; Jump Light Punch
[State -1, State]
type = ChangeState
value = 600
triggerall = Var(51) <= 0
triggerall = AILevel <= 0
triggerall = command = "x"
trigger1 = statetype = A
trigger1 = ctrl

; Jump Medium Punch
[State -1, State]
type = ChangeState
value = 610
triggerall = Var(51) <= 0
triggerall = AILevel <= 0
triggerall = command = "y"
trigger1 = statetype = A
trigger1 = ctrl

; Jump Strong Punch
[State -1, State]
type = ChangeState
value = 620
triggerall = Var(51) <= 0
triggerall = AILevel <= 0
triggerall = command = "z"
trigger1 = statetype = A
trigger1 = ctrl

;---------------------------------------------------------------------------
; Jump Light Kick
[State -1, State]
type = ChangeState
value = 630
triggerall = Var(51) <= 0
triggerall = AILevel <= 0
triggerall = command = "a"
trigger1 = statetype = A
trigger1 = ctrl

; Jump Medium Kick
[State -1, State]
type = ChangeState
value = 640
triggerall = Var(51) <= 0
triggerall = AILevel <= 0
triggerall = command = "b"
trigger1 = statetype = A
trigger1 = ctrl

; Jump Strong Kick
[State -1, State]
type = ChangeState
value = 650
triggerall = Var(51) <= 0
triggerall = AILevel <= 0
triggerall = command = "c"
trigger1 = statetype = A
trigger1 = ctrl

;===========================================================================
;---------------------------------------------------------------------------
; Basic Moves - [SF2'CE Mode]
;---------------------------------------------------------------------------
; Stand Light Punch
[State -1, State]
type = ChangeState
value = 300
triggerall = Var(51) > 0
triggerall = AILevel <= 0
triggerall = command = "x" || command = "a"
triggerall = command != "holddown"
trigger1 = statetype = S
trigger1 = ctrl

; Stand Medium Punch
[State -1, State]
type = ChangeState
value = 310
triggerall = Var(51) > 0
triggerall = AILevel <= 0
triggerall = command = "y" || command = "b"
triggerall = command != "holddown"
trigger1 = statetype = S
trigger1 = ctrl

; Stand Strong Punch
[State -1, State]
type = ChangeState
value = 320
triggerall = Var(51) > 0
triggerall = AILevel <= 0
triggerall = command = "z" || command = "c"
triggerall = command != "holddown"
trigger1 = statetype = S
trigger1 = ctrl

;---------------------------------------------------------------------------
; Crouch Light Punch
[State -1, State]
type = ChangeState
value = 500
triggerall = Var(51) > 0
triggerall = AILevel <= 0
triggerall = command = "x"
triggerall = command = "holddown"
trigger1 = statetype = C
trigger1 = ctrl

; Crouch Medium Punch
[State -1, State]
type = ChangeState
value = 510
triggerall = Var(51) > 0
triggerall = AILevel <= 0
triggerall = command = "y"
triggerall = command = "holddown"
trigger1 = statetype = C
trigger1 = ctrl

; Crouch Strong Punch
[State -1, State]
type = ChangeState
value = 520
triggerall = Var(51) > 0
triggerall = AILevel <= 0
triggerall = command = "z"
triggerall = command = "holddown"
trigger1 = statetype = C
trigger1 = ctrl

;---------------------------------------------------------------------------
; Crouch Light Kick
[State -1, State]
type = ChangeState
value = 530
triggerall = Var(51) > 0
triggerall = AILevel <= 0
triggerall = command = "a"
triggerall = command = "holddown"
trigger1 = statetype = C
trigger1 = ctrl

; Crouch Medium Kick
[State -1, State]
type = ChangeState
value = 540
triggerall = Var(51) > 0
triggerall = AILevel <= 0
triggerall = command = "b"
triggerall = command = "holddown"
trigger1 = statetype = C
trigger1 = ctrl

; Crouch Slide Kick
[State -1, State]
type = ChangeState
value = 550
triggerall = Var(51) > 0
triggerall = AILevel <= 0
triggerall = command = "c"
triggerall = command = "holddown"
trigger1 = statetype = C
trigger1 = ctrl

;---------------------------------------------------------------------------
; Jump Light Kick
[State -1, State]
type = ChangeState
value = 730
triggerall = Var(51) > 0
triggerall = AILevel <= 0
triggerall = command = "x" || command = "a"
trigger1 = statetype = A
trigger1 = ctrl

; Jump Medium Kick
[State -1, State]
type = ChangeState
value = 740
triggerall = Var(51) > 0
triggerall = AILevel <= 0
triggerall = command = "y" || command = "b"
trigger1 = statetype = A
trigger1 = ctrl

; Jump Strong Kick
[State -1, State]
type = ChangeState
value = 750
triggerall = Var(51) > 0
triggerall = AILevel <= 0
triggerall = command = "z" || command = "c"
trigger1 = statetype = A
trigger1 = ctrl

;===========================================================================
;---------------------------------------------------------------------------
; Wall Jump
[State -1]
type = ChangeState
value = 60
triggerall = ctrl || StateNo >= 600 && StateNo < 800 && Time > 10
triggerall = statetype = A
triggerall = stateno < 5000
triggerall = command = "holdup"
trigger1 = command = "holdfwd"
trigger1 = BackEdgeBodyDist <= 0
trigger2 = command = "holdback"
trigger2 = FrontEdgeBodyDist <= 0

;---------------------------------------------------------------------------

;===========================================================================
;---------------------------------------------------------------------------
; AI System
;---------------------------------------------------------------------------
; Random AI
[State -1, Random]
type = VarRandom
trigger1 = Ctrl
trigger1 = AILevel > 0
trigger1 = RoundState = 2
ignorehitpause = 1
v = 30
range = 0,100

[State -1, VarSet]
type = VarSet
trigger1 = AILevel <= 0
trigger2 = RoundState != 2
ignorehitpause = 1
v = 30
value = 0

;--------------------
; Walk & Throw
[State -1, State]
type = ChangeState
value = 25
triggerall = Ctrl
triggerall = AILevel > 0
triggerall = MoveType = I
triggerall = StateType != A
triggerall = P2Dist X <= 100
triggerall = EnemyNear, StateType != L
triggerall = StateNo < 25
trigger1 = Var(30) > 80
trigger1 = Var(30) < 90
;--------------------

;--------------------
; Wall Jump
[State -1]
type = ChangeState
value = 60
triggerall = AILevel > 0
triggerall = Var(30) > 0
triggerall = ctrl || StateNo >= 600 && StateNo < 800 && Time > 10
triggerall = Var(51) <= 0
triggerall = statetype = A
triggerall = stateno < 5000
trigger1 = BackEdgeBodyDist <= 0
trigger1 = Vel X <= 0
trigger1 = Vel Y > 0
trigger2 = FrontEdgeBodyDist <= 0
trigger2 = Vel X >= 0
trigger2 = Vel Y > 0

;=========================
; SSF2X CPU Commands
;=========================
;--------------------
; Super Claw
[State -1, State]
type = ChangeState
value = 3000
triggerall = Ctrl
triggerall = Power >= 1000
triggerall = AILevel > 0
triggerall = Var(51) <= 0
triggerall = MoveType = I
triggerall = StateType != A
triggerall = StateNo < 120
triggerall = P2Dist X <= 100
triggerall = EnemyNear, StateType != L
trigger1 = Var(30) = 10

; Super Kick Flip
[State -1, State]
type = ChangeState
value = 3500
triggerall = Ctrl
triggerall = Power >= 1000
triggerall = AILevel > 0
triggerall = Var(51) <= 0
triggerall = MoveType = I
triggerall = StateType != A
triggerall = StateNo < 120
triggerall = P2Dist X <= 100
triggerall = EnemyNear, StateType != L
trigger1 = Var(30) = 20

;--------------------
; Stand Light Punch
[State -1, State]
type = ChangeState
value = 200
triggerall = Ctrl
triggerall = AILevel > 0
triggerall = Var(51) <= 0
triggerall = MoveType = I
triggerall = StateType != A
triggerall = StateNo < 120
triggerall = P2Dist X <= 150
triggerall = EnemyNear, StateType != L
trigger1 = Var(30) = 5

; Stand Medium Punch
[State -1, State]
type = ChangeState
value = 210
triggerall = Ctrl
triggerall = AILevel > 0
triggerall = Var(51) <= 0
triggerall = MoveType = I
triggerall = StateType != A
triggerall = StateNo < 120
triggerall = P2Dist X <= 150
triggerall = EnemyNear, StateType != L
trigger1 = Var(30) = 10

; Stand Strong Punch
[State -1, State]
type = ChangeState
value = 220
triggerall = Ctrl
triggerall = AILevel > 0
triggerall = Var(51) <= 0
triggerall = MoveType = I
triggerall = StateType != A
triggerall = StateNo < 120
triggerall = P2Dist X <= 150
triggerall = EnemyNear, StateType != L
trigger1 = Var(30) = 15

; Stand Light Kick
[State -1, State]
type = ChangeState
value = 230
triggerall = Ctrl
triggerall = AILevel > 0
triggerall = Var(51) <= 0
triggerall = MoveType = I
triggerall = StateType != A
triggerall = StateNo < 120
triggerall = P2Dist X <= 150
triggerall = EnemyNear, StateType != L
trigger1 = Var(30) = 6

; Stand Medium Kick
[State -1, State]
type = ChangeState
value = 240
triggerall = Ctrl
triggerall = AILevel > 0
triggerall = Var(51) <= 0
triggerall = MoveType = I
triggerall = StateType != A
triggerall = StateNo < 120
triggerall = P2Dist X <= 150
triggerall = EnemyNear, StateType != L
trigger1 = Var(30) = 11

; Stand Strong Kick
[State -1, State]
type = ChangeState
value = 250
triggerall = Ctrl
triggerall = AILevel > 0
triggerall = Var(51) <= 0
triggerall = MoveType = I
triggerall = StateType != A
triggerall = StateNo < 120
triggerall = P2Dist X <= 150
triggerall = EnemyNear, StateType != L
trigger1 = Var(30) = 16

;--------------------
; Crouch Light Punch
[State -1, State]
type = ChangeState
value = 400
triggerall = Ctrl
triggerall = AILevel > 0
triggerall = Var(51) <= 0
triggerall = StateType = C
triggerall = MoveType = I
triggerall = StateType != A
triggerall = P2Dist X <= 150
triggerall = EnemyNear, StateType != L
trigger1 = Var(30) = 10

; Crouch Medium Punch
[State -1, State]
type = ChangeState
value = 410
triggerall = Ctrl
triggerall = AILevel > 0
triggerall = Var(51) <= 0
triggerall = StateType = C
triggerall = MoveType = I
triggerall = StateType != A
triggerall = P2Dist X <= 150
triggerall = EnemyNear, StateType != L
trigger1 = Var(30) = 15

; Crouch Strong Punch
[State -1, State]
type = ChangeState
value = 420
triggerall = Ctrl
triggerall = AILevel > 0
triggerall = Var(51) <= 0
triggerall = StateType = C
triggerall = MoveType = I
triggerall = StateType != A
triggerall = P2Dist X <= 150
triggerall = EnemyNear, StateType != L
trigger1 = Var(30) = 20

;--------------------
; Crouch Light Kick
[State -1, State]
type = ChangeState
value = 430
triggerall = Ctrl
triggerall = AILevel > 0
triggerall = Var(51) <= 0
triggerall = MoveType = I
triggerall = StateType != A
triggerall = StateNo < 120
triggerall = P2Dist X <= 100
triggerall = EnemyNear, StateType != L
trigger1 = Var(30) = 11
trigger2 = P2StateNo >= 200
trigger2 = P2StateNo < 400
trigger2 = Var(30) > 0
trigger2 = Var(30) < 25

; Crouch Medium Kick
[State -1, State]
type = ChangeState
value = 440
triggerall = Ctrl
triggerall = AILevel > 0
triggerall = Var(51) <= 0
triggerall = MoveType = I
triggerall = StateType != A
triggerall = StateNo < 120
triggerall = P2Dist X <= 100
triggerall = EnemyNear, StateType != L
trigger1 = Var(30) = 11
trigger2 = P2StateNo >= 200
trigger2 = P2StateNo < 400
trigger2 = Var(30) >= 25
trigger2 = Var(30) < 50

; Crouch Slide
[State -1, State]
type = ChangeState
value = 450
triggerall = Ctrl
triggerall = AILevel > 0
triggerall = Var(51) <= 0
triggerall = MoveType = I
triggerall = StateType != A
triggerall = StateNo < 120
triggerall = P2Dist X <= 100
triggerall = EnemyNear, StateType != L
trigger1 = Var(30) = 10
trigger2 = P2StateNo >= 200
trigger2 = P2StateNo < 400
trigger2 = Var(30) >= 50
trigger2 = Var(30) < 100
;--------------------

;--------------------
; Jump Light Punch
[State -1, State]
type = ChangeState
value = 600
triggerall = Ctrl
triggerall = AILevel > 0
triggerall = Var(51) <= 0
triggerall = MoveType = I
triggerall = StateType = A
triggerall = EnemyNear, StateType != L
trigger1 = StateNo < 120
trigger1 = Var(30) > 0
trigger1 = Var(30) < 5
trigger2 = EnemyNear, MoveType = A
trigger2 = Var(30) > 0
trigger2 = Var(30) < 5

; Jump Medium Punch
[State -1, State]
type = ChangeState
value = 610
triggerall = Ctrl
triggerall = AILevel > 0
triggerall = Var(51) <= 0
triggerall = MoveType = I
triggerall = StateType = A
triggerall = EnemyNear, StateType != L
trigger1 = StateNo < 120
trigger1 = Var(30) > 5
trigger1 = Var(30) < 10
trigger2 = EnemyNear, MoveType = A
trigger2 = Var(30) > 5
trigger2 = Var(30) < 10

; Jump Strong Punch
[State -1, State]
type = ChangeState
value = 620
triggerall = Ctrl
triggerall = AILevel > 0
triggerall = Var(51) <= 0
triggerall = MoveType = I
triggerall = StateType = A
triggerall = EnemyNear, StateType != L
trigger1 = StateNo < 120
trigger1 = Var(30) > 10
trigger1 = Var(30) < 15
trigger2 = EnemyNear, MoveType = A
trigger2 = Var(30) > 10
trigger2 = Var(30) < 15

; Jump Light Kick
[State -1, State]
type = ChangeState
value = 630
triggerall = Ctrl
triggerall = AILevel > 0
triggerall = Var(51) <= 0
triggerall = MoveType = I
triggerall = StateType = A
triggerall = EnemyNear, StateType != L
trigger1 = StateNo < 120
trigger1 = Var(30) > 20
trigger1 = Var(30) < 30
trigger2 = EnemyNear, MoveType = A
trigger2 = Var(30) > 25
trigger2 = Var(30) < 30

; Jump Medium Kick
[State -1, State]
type = ChangeState
value = 640
triggerall = Ctrl
triggerall = AILevel > 0
triggerall = Var(51) <= 0
triggerall = MoveType = I
triggerall = StateType = A
triggerall = EnemyNear, StateType != L
trigger1 = StateNo < 120
trigger1 = Var(30) > 30
trigger1 = Var(30) < 50
trigger2 = EnemyNear, MoveType = A
trigger2 = Var(30) > 35
trigger2 = Var(30) < 50

; Jump Strong Kick
[State -1, State]
type = ChangeState
value = 650
triggerall = Ctrl
triggerall = AILevel > 0
triggerall = Var(51) <= 0
triggerall = MoveType = I
triggerall = StateType = A
triggerall = EnemyNear, StateType != L
trigger1 = StateNo < 120
trigger1 = Var(30) > 50
trigger1 = Var(30) < 60
trigger2 = EnemyNear, MoveType = A
trigger2 = Var(30) > 55
trigger2 = Var(30) < 60
;--------------------
;=========================

;=========================
; SF2'CE CPU Commands
;=========================
;--------------------
; Stand Light Punch
[State -1, State]
type = ChangeState
value = 300
triggerall = Ctrl
triggerall = AILevel > 0
triggerall = Var(51) > 0
triggerall = MoveType = I
triggerall = StateType != A
triggerall = StateNo < 120
triggerall = P2Dist X <= 150
triggerall = EnemyNear, StateType != L
trigger1 = Var(30) = 5

; Stand Medium Punch
[State -1, State]
type = ChangeState
value = 310
triggerall = Ctrl
triggerall = AILevel > 0
triggerall = Var(51) > 0
triggerall = MoveType = I
triggerall = StateType != A
triggerall = StateNo < 120
triggerall = P2Dist X <= 150
triggerall = EnemyNear, StateType != L
trigger1 = Var(30) = 10

; Stand Strong Punch
[State -1, State]
type = ChangeState
value = 320
triggerall = Ctrl
triggerall = AILevel > 0
triggerall = Var(51) > 0
triggerall = MoveType = I
triggerall = StateType != A
triggerall = StateNo < 120
triggerall = P2Dist X <= 150
triggerall = EnemyNear, StateType != L
trigger1 = Var(30) = 15

;--------------------
; Crouch Light Punch
[State -1, State]
type = ChangeState
value = 500
triggerall = Ctrl
triggerall = AILevel > 0
triggerall = Var(51) > 0
triggerall = StateType = C
triggerall = MoveType = I
triggerall = StateType != A
triggerall = P2Dist X <= 150
triggerall = EnemyNear, StateType != L
trigger1 = Var(30) = 10

; Crouch Medium Punch
[State -1, State]
type = ChangeState
value = 510
triggerall = Ctrl
triggerall = AILevel > 0
triggerall = Var(51) > 0
triggerall = StateType = C
triggerall = MoveType = I
triggerall = StateType != A
triggerall = P2Dist X <= 150
triggerall = EnemyNear, StateType != L
trigger1 = Var(30) = 15

; Crouch Strong Punch
[State -1, State]
type = ChangeState
value = 520
triggerall = Ctrl
triggerall = AILevel > 0
triggerall = Var(51) > 0
triggerall = StateType = C
triggerall = MoveType = I
triggerall = StateType != A
triggerall = P2Dist X <= 150
triggerall = EnemyNear, StateType != L
trigger1 = Var(30) = 20

;--------------------
; Crouch Light Kick
[State -1, State]
type = ChangeState
value = 530
triggerall = Ctrl
triggerall = AILevel > 0
triggerall = Var(51) > 0
triggerall = MoveType = I
triggerall = StateType != A
triggerall = StateNo < 120
triggerall = P2Dist X <= 100
triggerall = EnemyNear, StateType != L
trigger1 = Var(30) = 11
trigger2 = P2StateNo >= 200
trigger2 = P2StateNo < 400
trigger2 = Var(30) > 0
trigger2 = Var(30) < 25

; Crouch Medium Kick
[State -1, State]
type = ChangeState
value = 540
triggerall = Ctrl
triggerall = AILevel > 0
triggerall = Var(51) > 0
triggerall = MoveType = I
triggerall = StateType != A
triggerall = StateNo < 120
triggerall = P2Dist X <= 100
triggerall = EnemyNear, StateType != L
trigger1 = Var(30) = 11
trigger2 = P2StateNo >= 200
trigger2 = P2StateNo < 400
trigger2 = Var(30) >= 25
trigger2 = Var(30) < 50

; Crouch Slide
[State -1, State]
type = ChangeState
value = 550
triggerall = Ctrl
triggerall = AILevel > 0
triggerall = Var(51) > 0
triggerall = MoveType = I
triggerall = StateType != A
triggerall = StateNo < 120
triggerall = P2Dist X <= 100
triggerall = EnemyNear, StateType != L
trigger1 = Var(30) = 10
trigger2 = P2StateNo >= 200
trigger2 = P2StateNo < 400
trigger2 = Var(30) >= 50
trigger2 = Var(30) < 100
;--------------------

;--------------------
; Jump Light Kick
[State -1, State]
type = ChangeState
value = 730
triggerall = Ctrl
triggerall = AILevel > 0
triggerall = Var(51) > 0
triggerall = MoveType = I
triggerall = StateType = A
triggerall = EnemyNear, StateType != L
trigger1 = StateNo < 120
trigger1 = Var(30) > 20
trigger1 = Var(30) < 30
trigger2 = EnemyNear, MoveType = A
trigger2 = Var(30) > 25
trigger2 = Var(30) < 30

; Jump Medium Kick
[State -1, State]
type = ChangeState
value = 740
triggerall = Ctrl
triggerall = AILevel > 0
triggerall = Var(51) > 0
triggerall = MoveType = I
triggerall = StateType = A
triggerall = EnemyNear, StateType != L
trigger1 = StateNo < 120
trigger1 = Var(30) > 30
trigger1 = Var(30) < 50
trigger2 = EnemyNear, MoveType = A
trigger2 = Var(30) > 35
trigger2 = Var(30) < 50

; Jump Strong Kick
[State -1, State]
type = ChangeState
value = 750
triggerall = Ctrl
triggerall = AILevel > 0
triggerall = Var(51) > 0
triggerall = MoveType = I
triggerall = StateType = A
triggerall = EnemyNear, StateType != L
trigger1 = StateNo < 120
trigger1 = Var(30) > 50
trigger1 = Var(30) < 60
trigger2 = EnemyNear, MoveType = A
trigger2 = Var(30) > 55
trigger2 = Var(30) < 60
;--------------------
;=========================

;--------------------
; Backflip
[State -1, State]
type = ChangeState
value = 900
triggerall = AILevel > 0
triggerall = statetype != A
triggerall = P2Dist X <= 200
trigger1 = EnemyNear, StateType = A
trigger1 = EnemyNear, MoveType = A
trigger1 = Ctrl
trigger2 = StateNo = 990
trigger2 = EnemyNear, MoveType = A

;--------------------
; CPU Begin Attack
[State -1, State]
type = ChangeState
value = 990
triggerall = Ctrl
triggerall = AILevel > 0
triggerall = MoveType = I
triggerall = StateType = S
triggerall = StateNo < 120
triggerall = P2Dist X > 150
triggerall = EnemyNear, StateType != L
trigger1 = Var(30) >= 20
trigger1 = Var(30) < 30

;--------------------
; Wall Cling Thrust
[State -1, State]
type = ChangeState
value = IfElse(Var(58) <= 0, 1130, 1100)
triggerall = Ctrl
triggerall = AILevel > 0
triggerall = MoveType = I
triggerall = StateType != A
triggerall = StateNo < 120
triggerall = P2Dist X > 150
triggerall = Name = "SF2'CE Vega"
triggerall = EnemyNear, StateType != L
trigger1 = Var(30) >= 70
trigger1 = Var(30) < 80

[State -1, State]
type = ChangeState
value = 1130
triggerall = Ctrl
triggerall = AILevel > 0
triggerall = MoveType = I
triggerall = StateType != A
triggerall = StateNo < 120
triggerall = P2Dist X > 150
triggerall = Name != "SF2'CE Vega"
triggerall = EnemyNear, StateType != L
trigger1 = Var(30) >= 70
trigger1 = Var(30) < 80

;--------------------
; Sky High Claw
[State -1, State]
type = ChangeState
value = 1210
triggerall = Ctrl
triggerall = AILevel > 0
triggerall = Var(51) <= 0
triggerall = MoveType = I
triggerall = StateType != A
triggerall = StateNo < 120
triggerall = P2Dist X > 100
triggerall = EnemyNear, StateType != L
trigger1 = Var(30) >= 50
trigger1 = Var(30) < 55

;--------------------
; Kick Flip
[State -1, State]
type = ChangeState
value = 1500
triggerall = Ctrl
triggerall = AILevel > 0
triggerall = Var(51) <= 0
triggerall = MoveType = I
triggerall = StateType != A
triggerall = StateNo < 120
triggerall = P2Dist X <= 100
triggerall = EnemyNear, StateType != L
trigger1 = Var(30) >= 60
trigger1 = Var(30) < 70

;--------------------
; Stand Guard
[State -1, Guard]
type = ChangeState
value = 130
triggerall = Ctrl
triggerall = AILevel > 0
triggerall = StateNo < 160
triggerall = StateType != A
trigger1 = EnemyNear, MoveType = A
trigger1 = P2Dist X <= 100

; Crouch Guard
[State -1, Guard]
type = ChangeState
value = 131
triggerall = Ctrl
triggerall = AILevel > 0
triggerall = StateNo < 160
triggerall = StateType != A
trigger1 = EnemyNear, StateType = C
trigger1 = EnemyNear, MoveType = A
trigger1 = P2Dist X <= 100
;--------------------

;---------------------------------------------------------------------------