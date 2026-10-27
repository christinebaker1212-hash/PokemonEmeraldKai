
;-| Super Motions |--------------------------------------------------------
[command]
name = "hyper#1_a"
command = ~D,DB,B,D,DB,B, a
time = 20

[command]
name = "hyper#1_b"
command = ~D,DB,B,D,DB,B, b
time = 20

[command]
name = "hyper#1_c"
command = ~D,DB,B,D,DB,B, c
time = 20


[command]
name = "hyper#2_a"
command = ~D,DF,F,D,DF,F, a
time = 20

[command]
name = "hyper#2_b"
command = ~D,DF,F,D,DF,F, b
time = 20

[command]
name = "hyper#2_c"
command = ~D,DF,F,D,DF,F, c
time = 20


[command]
name = "hyper#3_x"
command = ~D,DF,F,D,DF,F, x
time = 20

[command]
name = "hyper#3_y"
command = ~D,DF,F,D,DF,F, y
time = 20

[command]
name = "hyper#3_z"
command = ~D,DF,F,D,DF,F, z
time = 20

;-| Special Motions |------------------------------------------------------
[Command]
name = "proj_x"
command = ~D,DF,F,x

[Command]
name = "proj_y"
command = ~D,DF,F,y

[Command]
name = "proj_z"
command = ~D,DF,F,z

[Command]
name = "proj_x_"
command = ~30$B,F,x

[Command]
name = "proj_y_"
command = ~30$B,F,y

[Command]
name = "proj_z_"
command = ~30$B,F,z



[command]
name = "kick_a"
command = ~D,DB,B, a

[command]
name = "kick_b"
command = ~D,DB,B, b

[command]
name = "kick_c"
command = ~D,DB,B, c

[command]
name = "kick_a_"
command = ~30$B,F, a

[command]
name = "kick_b_"
command = ~30$B,F, b

[command]
name = "kick_c_"
command = ~30$B,F, c


[command]
name = "maximum_a"
command = ~F,D,DF,a

[command]
name = "maximum_b"
command = ~F,D,DF,b

[command]
name = "maximum_c"
command = ~F,D,DF,c

[command]
name = "maximum_a_"
command = ~30$D,$U,a

[command]
name = "maximum_b_"
command = ~30$D,$U,b

[command]
name = "maximum_c_"
command = ~30$D,$U,c


[command]
name = "machine_x"
command = ~F,D,DF,x

[command]
name = "machine_y"
command = ~F,D,DF,y

[command]
name = "machine_z"
command = ~F,D,DF,z

[command]
name = "machine_x_"
command = ~30$D,$U,x

[command]
name = "machine_y_"
command = ~30$D,$U,y

[command]
name = "machine_z_"
command = ~30$D,$U,z


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
name = "recovery";Required (do not remove)
command = x+y
time = 1

;-| Dir + Button |---------------------------------------------------------
[Command]
name = "down_a"
command = /$D,a
time = 1

[Command]
name = "down_b"
command = /$D,b
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

;-| Hold Dir |--------------------------------------------------------------
[Command]
name = "holdfwd";Required (do not remove)
command = /$F
time = 1

[Command]
name = "holdback";Required (do not remove)
command = /$B
time = 1

[Command]
name = "holdup" ;Required (do not remove)
command = /$U
time = 1

[Command]
name = "holddown";Required (do not remove)
command = /$D
time = 1

;---------------------------------------------------------------------------
[Statedef -1]

;===========================================================================
;---------------------------------------------------------------------------

;===========================================================================
;---------------------------------------------------------------------------
;Run Fwd
[State -1, Run Fwd]
type = ChangeState
value = 100
trigger1 = command = "FF"
trigger1 = statetype = S
trigger1 = ctrl
trigger1 = stateno != 100
;---------------------------------------------------------------------------
;Run Back
[State -1, Run Back]
type = ChangeState
value = 105
trigger1 = command = "BB"
trigger1 = statetype = S
trigger1 = ctrl
;-------------------------------------------------------------------------------
;Hyper#1
[state -1, HYper#1]
type = changestate
triggerall = command = "hyper#1_a" || command = "hyper#1_b" || command = "hyper#1_c"
triggerall = power >= 1000 
trigger1 = ctrl = 1 && statetype = s
trigger2 = stateno = 220 && movecontact = 1 
trigger3 = stateno = 240 && movecontact = 1 
value = 2000

;Hyper#2
[state -1, HYper#2]
type = changestate
triggerall = command = "hyper#2_a" || command = "hyper#2_b" || command = "hyper#2_c"
triggerall = power >= 1000 
trigger1 = ctrl = 1 && statetype = s
value = 2100

;Hyper#3
[state -1, HYper#3]
type = null;changestate
triggerall = command = "hyper#3_x" || command = "hyper#3_y" || command = "hyper#3_z"
triggerall = power >= 1000 
trigger1 = ctrl = 1 && statetype = s
value = 2200


;-------------------------------------------------

;Macine Gun Upper
[state -1, Maximum]
type = changestate
trigger1 = command = "machine_x"
trigger2 = command = "machine_y"
trigger3 = command = "machine_z"
triggerall = ctrl = 1
triggerall = statetype = s
triggerall = var(30) = 0
value = 1600

[state -1, Maximum]
type = changestate
trigger1 = command = "machine_x_"
trigger2 = command = "machine_y_"
trigger3 = command = "machine_z_"
triggerall = ctrl = 1 || stateno = 40
triggerall = statetype != a 
triggerall = var(30) = 1
value = 1600


;Projectile
[state -1, Projectile]
type = varset
trigger1 = command = "proj_x" && var(30) = 0
trigger2 = command = "proj_x_" && var(30) = 1
triggerall = ctrl
triggerall = statetype = s  
var(10) = 1

[state -1, Projectile]
type = varset
trigger1 = command = "proj_y" && var(30) = 0
trigger2 = command = "proj_y_" && var(30) = 1
triggerall = ctrl
triggerall = statetype = s  
var(10) = 2

[state -1, Projectile]
type = varset
trigger1 = command = "proj_z" && var(30) = 0
trigger2 = command = "proj_z_" && var(30) = 1
triggerall = ctrl
triggerall = statetype = s  
var(10) = 3


[state -1, Projectile]
type = ChangeState
trigger1 = command = "proj_x"
trigger2 = command = "proj_y"
trigger3 = command = "proj_z"
;triggerall = projnumid(1000) = 0
triggerall = ctrl
triggerall = statetype = s  
triggerall = var(30) = 0
value = 1000

[state -1, Projectile]
type = ChangeState
trigger1 = command = "proj_x_"
trigger2 = command = "proj_y_"
trigger3 = command = "proj_z_"
;triggerall = projnumid(1000) = 0
triggerall = ctrl
triggerall = statetype = s  
triggerall = var(30) = 1
value = 1000

;-------------------------------------------------------------------------------
;SP KIck
[state -1, SPckick]
type = varset
trigger1 = command = "kick_a" && var(30) = 0
trigger2 = command = "kick_a_" && var(30) = 1
var(11) = 1

[state -1, SPkick]
type = varset
trigger1 = command = "kick_b" && var(30) = 0
trigger2 = command = "kick_b_" && var(30) = 1
var(11) = 2

[state -1, SPkick]
type = varset
trigger1 = command = "kick_c" && var(30) = 0
trigger2 = command = "kick_c_" && var(30) = 1
var(11) = 3

[state -1, SPKick]
type = changestate
trigger1 = command = "kick_a"
trigger2 = command = "kick_b"
trigger3 = command = "kick_c"
triggerall = ctrl = 1
triggerall = statetype = s
triggerall = var(30) = 0
value = 1200

[state -1, SPKick]
type = changestate
trigger1 = command = "kick_a_"
trigger2 = command = "kick_b_"
trigger3 = command = "kick_c_"
triggerall = ctrl = 1
triggerall = statetype = s
triggerall = var(30) = 1
value = 1200

[state -1, SPKick cnacel]
type = changestate
trigger1 = stateno = 220 && movecontact = 1 
trigger2 = stateno = 240 && movecontact = 1 
triggerall = command = "kick_a" || command = "kick_b" || command = "kick_c" 
triggerall = var(30) = 0
value = 1200

[state -1, SPKick cnacel]
type = changestate
trigger1 = stateno = 220 && movecontact = 1 
trigger2 = stateno = 240 && movecontact = 1 
triggerall = command = "kick_a_" || command = "kick_b_" || command = "kick_c_" 
triggerall = var(30) = 1
value = 1200

;-------------------------------------------------------------------------------
;Maximum Jack kinfe

[state -1, Maximum]
type = varset
trigger1 = command = "maximum_a" && var(30) = 0
trigger2 = command = "maximum_a_" && var(30) = 1
var(12) = 1

[state -1, Maximum]
type = varset
trigger1 = command = "maximum_b" && var(30) = 0
trigger2 = command = "maximum_b_" && var(30) = 1
var(12) = 2

[state -1, Maximum]
type = varset
trigger1 = command = "maximum_c" && var(30) = 0
trigger2 = command = "maximum_c_" && var(30) = 1
var(12) = 3

[state -1, Maximum]
type = changestate
trigger1 = command = "maximum_a"
trigger2 = command = "maximum_b"
trigger3 = command = "maximum_c"
triggerall = ctrl = 1 || stateno = 52
triggerall = var(30) = 0
value = 1400

[state -1, Maximum]
type = changestate
trigger1 = command = "maximum_a_"
trigger2 = command = "maximum_b_"
trigger3 = command = "maximum_c_"
triggerall = ctrl = 1 || stateno = 52 || stateno = 40
triggerall = var(30) = 1
value = 1400

[state -1, Maximum_Cancel]
type = changestate
triggerall = command = "maximum_a" || command = "maximum_b" || command = "maximum_c"
triggerall = p2statetype = a || stateno = 52
triggerall = var(30) = 0
trigger1 = stateno = 600 && movecontact
trigger2 = stateno = 610 && movecontact
trigger3 = stateno = 611 && movecontact
trigger4 = stateno = 620 && movecontact
trigger5 = stateno = 621 && movecontact
trigger6 = stateno = 630 && movecontact
trigger7 = stateno = 631 && movecontact
trigger8 = stateno = 640 && movecontact
trigger9 = stateno = 641 && movecontact
trigger10 = stateno = 650 && movecontact
trigger11 = stateno = 651 && movecontact
value = 1400

[state -1, Maximum_Cancel]
type = changestate
triggerall = command = "maximum_a_" || command = "maximum_b_" || command = "maximum_c_"
triggerall = p2statetype = a || stateno = 52
triggerall = var(30) = 1
trigger1 = stateno = 600 && movecontact
trigger2 = stateno = 610 && movecontact
trigger3 = stateno = 611 && movecontact
trigger4 = stateno = 620 && movecontact
trigger5 = stateno = 621 && movecontact
trigger6 = stateno = 630 && movecontact
trigger7 = stateno = 631 && movecontact
trigger8 = stateno = 640 && movecontact
trigger9 = stateno = 641 && movecontact
trigger10 = stateno = 650 && movecontact
trigger11 = stateno = 651 && movecontact
value = 1400

[state -1, cancle2]
type = changestate 
trigger1 = stateno = 210 && movecontact && time > 9
triggerall = command = "maximum_a" || command = "maximum_b" || command = "maximum_c"
triggerall = var(30) = 0
value = 1400

[state -1, cancle2]
type = changestate 
trigger1 = stateno = 210 && movecontact && time > 9
triggerall = command = "maximum_a_" || command = "maximum_b_" || command = "maximum_c_"
triggerall = var(30) = 1
value = 1400

;---------------------------------------------------------------------------
;Grab #1
[State -1, Greb#1]
type = ChangeState
value = 1300
triggerall = command = "c"
triggerall = statetype = S
triggerall = ctrl
triggerall = stateno != 100
trigger1 = command = "holdfwd"
trigger1 = p2bodydist X < 3
trigger1 = (p2statetype = S) || (p2statetype = C) 
trigger1 = p2movetype != h

;---------------------------------------------------------------------------
;Grab #2
[State -1, Greb#2]
type = ChangeState
value = 1500
triggerall = command = "z"
triggerall = statetype = S
triggerall = ctrl
triggerall = stateno != 100
trigger1 = command = "holdfwd"
trigger1 = p2bodydist X < 3
trigger1 = (p2statetype = S) || (p2statetype = C) 
trigger1 = p2movetype != h


;===========================================================================
;---------------------------------------------------------------------------
;Stand Light Punch
[State -1, Stand Light Punch]
type = ChangeState
value = 200
triggerall = command = "x"
triggerall = command != "holddown"
trigger1 = statetype = S
trigger1 = ctrl

;---------------------------------------------------------------------------
;Stand Medium Punch
[State -1, Stand Medium Punch]
type = ChangeState
value = 210
triggerall = command = "y"
triggerall = command != "holddown"
trigger1 = statetype = S
trigger1 = ctrl
trigger2 = (stateno = 200) && movecontact && time > 6

;---------------------------------------------------------------------------
;Stand Strong Punch2 (close)
[State -1, Stand Strong Punch]
type = ChangeState
value = 221
triggerall = command = "z"
triggerall = command != "holddown"
trigger1 = statetype = S
trigger1 = p2bodydist x < 23 
trigger1 = ctrl

;---------------------------------------------------------------------------
;Stand Strong Punch
[State -1, Stand Strong Punch]
type = ChangeState
value = 220
triggerall = command = "z"
triggerall = command != "holddown"
trigger1 = statetype = S
trigger1 = ctrl
trigger2 = (stateno = 210) && movecontact && time > 7
trigger3 = (stateno = 200) && movecontact 

;---------------------------------------------------------------------------
;Stand Light Kick2 (close)
[State -1, Stand Light Kick]
type = ChangeState
value = 231
triggerall = command = "a"
triggerall = command != "holddown"
trigger1 = statetype = S
trigger1 = ctrl
trigger1 = p2bodydist x < 12

;---------------------------------------------------------------------------
;Stand Light Kick
[State -1, Stand Light Kick]
type = ChangeState
value = 230
triggerall = command = "a"
triggerall = command != "holddown"
trigger1 = statetype = S
trigger1 = ctrl

;---------------------------------------------------------------------------
;Standing Medium Kick2 (close)
[State -1, Medium Strong Kick]
type = ChangeState
value = 241
triggerall = command = "b"
triggerall = command != "holddown"
trigger1 = statetype = S
trigger1 = ctrl
trigger1 = p2bodydist x < 23  

;Standing Medium Kick
[State -1, Medium Strong Kick]
type = ChangeState
value = 240
triggerall = command = "b"
triggerall = command != "holddown"
trigger1 = statetype = S
trigger1 = ctrl
trigger2 = (stateno = 230) && movecontact ;&& time > 6  

;Standing Strong Kick2(close)
[State -1, Standing Strong Kick]
type = ChangeState
value = 251
triggerall = command = "c"
triggerall = command != "holddown"
trigger1 = statetype = S
trigger1=  p2bodydist x < 5
trigger1 = ctrl


;Standing Strong Kick
[State -1, Standing Strong Kick]
type = ChangeState
value = 250
triggerall = command = "c"
triggerall = command != "holddown"
trigger1 = statetype = S
trigger1 = ctrl
trigger2 = (stateno = 230) && movecontact 
trigger3 = (stateno = 241) && movecontact && time > 8

;---------------------------------------------------------------------------
;Taunt
[State -1, Taunt]
type = null;ChangeState
value = 195
triggerall = command = "start"
trigger1 = statetype != A
trigger1 = ctrl

;---------------------------------------------------------------------------
;Crouching Light Punch
[State -1, Crouching Light Punch]
type = ChangeState
value = 400
triggerall = command = "x"
triggerall = command = "holddown"
trigger1 = statetype = C
trigger1 = ctrl

;---------------------------------------------------------------------------
;Crouching Medium Punch
[State -1, Crouching Medium Punch]
type = ChangeState
value = 410
triggerall = command = "y"
triggerall = command = "holddown"
trigger1 = statetype = C
trigger1 = ctrl
;trigger2 = (stateno = 430) && movecontact  

;---------------------------------------------------------------------------
;Crouching Strong Punch
[State -1, Crouching Strong Punch]
type = ChangeState
value = 420
triggerall = command = "z"
triggerall = command = "holddown"
trigger1 = statetype = C
trigger1 = ctrl
;trigger2 = (stateno = 440) && movecontact  

;---------------------------------------------------------------------------
;Crouching Light Kick
[State -1, Crouching Light Kick]
type = ChangeState
value = 430
triggerall = command = "a"
triggerall = command = "holddown"
trigger1 = statetype = C
trigger1 = ctrl

;---------------------------------------------------------------------------
;Crouching Medium Kick
[State -1, Crouching Medium Kick]
type = ChangeState
value = 440
triggerall = command = "b"
triggerall = command = "holddown"
trigger1 = statetype = C
trigger1 = ctrl
;trigger2 = (stateno = 400) && movecontact  

;---------------------------------------------------------------------------
;Crouching Strong Kick
[State -1, Crouching Strong Kick]
type = ChangeState
value = 450
triggerall = command = "c"
triggerall = command = "holddown"
trigger1 = statetype = C
trigger1 = ctrl
;trigger2 = (stateno = 410) && movecontact  

;---------------------------------------------------------------------------
;Jump Light Punch
[State -1, Jump Light Punch]
type = ChangeState
value = 600
triggerall = command = "x"
trigger1 = statetype = A
trigger1 = ctrl

;---------------------------------------------------------------------------
;Jump Medium Punch(v)
[State -1, Jump Medium Punch v]
type = ChangeState
value = 610
triggerall = command = "y"
triggerall = vel x = 0
trigger1 = statetype = A
trigger1 = ctrl

;---------------------------------------------------------------------------
;Jump Medium Punch
[State -1, Jump Medium Punch v]
type = ChangeState
value = 611
triggerall = command = "y"
trigger1 = statetype = A
trigger1 = ctrl

;---------------------------------------------------------------------------
;Jump Strong Punch(v)
[State -1, Jump Strong Punch v]
type = ChangeState
value = 620
triggerall = command = "z"
triggerall = vel x = 0
trigger1 = statetype = A
trigger1 = ctrl

;---------------------------------------------------------------------------
;Jump Strong Punch
[State -1, Jump Strong Punch]
type = ChangeState
value = 621
triggerall = command = "z"
trigger1 = statetype = A
trigger1 = ctrl

;---------------------------------------------------------------------------
;Jump Light Kick(v)
[State -1, Jump Light Kick v]
type = ChangeState
value = 630
triggerall = command = "a"
triggerall = vel x = 0
trigger1 = statetype = A
trigger1 = ctrl

;---------------------------------------------------------------------------
;Jump Light Kick(d)
[State -1, Jump Light Kick d]
type = ChangeState
value = 632
triggerall = command = "a" && command = "holddown"
trigger1 = statetype = A
trigger1 = ctrl

;---------------------------------------------------------------------------
;Jump Light Kick
[State -1, Jump Light Kick]
type = ChangeState
value = 631
triggerall = command = "a"
trigger1 = statetype = A
trigger1 = ctrl

;---------------------------------------------------------------------------
;Jump Medium Kick(v)
[State -1, Jump Medium Kick v]
type = ChangeState
value = 640
triggerall = command = "b"
triggerall = vel x = 0
trigger1 = statetype = A
trigger1 = ctrl


;---------------------------------------------------------------------------
;Jump Medium Kick
[State -1, Jump Medium Kick]
type = ChangeState
value = 641
triggerall = command = "b"
trigger1 = statetype = A
trigger1 = ctrl



;---------------------------------------------------------------------------
;Jump Strong Kick(v)
[State -1, Jump Strong Kick v]
type = ChangeState
value = 650
triggerall = command = "c"
triggerall = vel x = 0
trigger1 = statetype = A
trigger1 = ctrl


;---------------------------------------------------------------------------
;Jump Strong Kick
[State -1, Jump Strong Kick]
type = ChangeState
value = 651
triggerall = command = "c"
trigger1 = statetype = A
trigger1 = ctrl


