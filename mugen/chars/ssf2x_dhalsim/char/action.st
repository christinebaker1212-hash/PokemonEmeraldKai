[StateDef 200];弱パンチ
Type = S
MoveType = A
Physics = N
Ctrl = 0
VelSet = IFelse(PrevStateNo=100, Vel X, 0)
Anim = 200
SprPriority = 2
PowerAdd = Var(10)*12 - (Var(14))*12

[State 200]
Type = VelAdd
Trigger1 = MoveContact && PrevStateNo = 100
Persistent = 0
IgnoreHitPause = 0
X = -1.8*Const(Size.XScale)

[State 200]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0+Var(10)*2, 7-Var(10)*3

[State 200]
Type = HitDef
Trigger1 = AnimElem = 3
Attr = S, NA
HitFlag = MA
GuardFlag = M
Damage = 63
GetPower = 40 + Var(10)*60 - Var(14)*100
GivePower = 0
PauseTime = 14 - Var(10)*2, 14 - Var(10)*2
SparkNo = S(IFelse(Var(10), 61, 50+TeamSide))
Guard.SparkNo = S50 + Var(10)*10
SparkXY = -30+Floor(P2Dist X)/2, -65
HitSound = S0 + Var(10)*2, 1
GuardSound = S0 + Var(10)*2, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 11 + Var(10)
Ground.HitTime = 12
Guard.HitTime = 19 - Var(10)*3
Air.HitTime = 99
Ground.Velocity = -5 + Var(10)*Const(Size.XScale)
Air.Velocity = -2 - Var(10)*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -10 + Var(10)*2*Const(Size.XScale)
YAccel = .25 + Var(10)*.15
P2Facing = 1

[State 200]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 201];後ろ押し弱パンチ
Type = S
MoveType = A
Physics = N
Ctrl = 0
VelSet = IFelse(PrevStateNo=100, Vel X, 0)
Anim = 201
SprPriority = 2
PowerAdd = Var(10)*12 - (Var(14))*12

[State 201]
Type = VelAdd
Trigger1 = MoveContact && PrevStateNo = 100
Persistent = 0
IgnoreHitPause = 0
X = -1.8*Const(Size.XScale)

[State 201]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0+Var(10)*2, 7-Var(10)*3

[State 201]
Type = HitDef
Trigger1 = AnimElem = 2
Attr = S, NA
HitFlag = MA
GuardFlag = M
Damage = 98
GetPower = 40 + Var(10)*60 - Var(14)*100
GivePower = 0
PauseTime = 14 - Var(10)*2, 14 - Var(10)*2
SparkNo = S(IFelse(Var(10), 61, 50+TeamSide))
Guard.SparkNo = S50 + Var(10)*10
SparkXY = -20+Floor(P2Dist X)/2, -80
HitSound = S0 + Var(10)*2, 1
GuardSound = S0 + Var(10)*2, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 11 + Var(10)
Ground.HitTime = 12
Guard.HitTime = 19 - Var(10)*3
Air.HitTime = 99
Ground.Velocity = -5 + Var(10)*Const(Size.XScale)
Air.Velocity = -2 - Var(10)*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -10 + Var(10)*2*Const(Size.XScale)
YAccel = .25 + Var(10)*.15
P2Facing = 1

[State 201]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 210];中パンチ
Type = S
MoveType = A
Physics = N
Ctrl = 0
VelSet = IFelse(PrevStateNo=100, Vel X, 0)
Anim = 210
SprPriority = 2
PowerAdd = Var(10)*25 - (Var(14))*25

[State 210]
Type = VelAdd
Trigger1 = MoveContact && PrevStateNo = 100
Persistent = 0
IgnoreHitPause = 0
X = -1.8*Const(Size.XScale)

[State 210]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0+Var(10)*2, 8-Var(10)*3

[State 210]
Type = HitDef
Trigger1 = AnimElem = 3
Attr = S, NA
HitFlag = MA
GuardFlag = M
Damage = 119 - Var(10)*21
GetPower = 80 + Var(10)*70 - Var(14)*150
GivePower = 0
PauseTime = 14 - Var(10)*2, 14 - Var(10)*2
SparkNo = S(IFelse(Var(10), 61, 52+TeamSide))
Guard.SparkNo = S50 + Var(10)*10
SparkXY = -50+Floor(P2Dist X)/3, -32
HitSound = S0 + Var(10)*2, 2
GuardSound = S0 + Var(10)*2, 0
AnimType = Middle
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 17
Ground.HitTime = 16
Guard.HitTime = 24 - Var(10)*4
Air.HitTime = 99
Ground.Velocity = -7 + Var(10)*Const(Size.XScale)
Air.Velocity = -2 - Var(10)*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -14 + Var(10)*2*Const(Size.XScale)
YAccel = .25 + Var(10)*.15
P2Facing = 1

[State 210]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 211];後ろ押し中パンチ
Type = S
MoveType = A
Physics = N
Ctrl = 0
VelSet = IFelse(PrevStateNo=100, Vel X, 0)
Anim = 211
SprPriority = 2
PowerAdd = Var(10)*25 - (Var(14))*25

[State 211]
Type = VelAdd
Trigger1 = MoveContact && PrevStateNo = 100
Persistent = 0
IgnoreHitPause = 0
X = -1.8*Const(Size.XScale)

[State 211]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0+Var(10)*2, 8-Var(10)*3

[State 211]
Type = HitDef
Trigger1 = (AnimElem = 2 || (!MoveContact && AnimElem = 3))
Attr = S, NA
Hitflag = MA
GuardFlag = M
Damage = 119 - Var(10)*21
GetPower = 80 + Var(10)*70 - Var(14)*150
GivePower = 0
PauseTime = 14 - Var(10)*2, 14 - Var(10)*2
SparkNo = S(IFelse(Var(10), 61, 52+TeamSide))
Guard.SparkNo = S50 + Var(10)*10
SparkXY = -20+Floor(P2Dist X)/2, -54 - (AnimElem=3)*46
HitSound = S0 + Var(10)*2, 2
GuardSound = S0 + Var(10)*2, 0
Animtype = Middle
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 17
Ground.HitTime = 16
Guard.HitTime = 24 - Var(10)*4
Air.HitTime = 99
Ground.Velocity = -7 + Var(10)*Const(Size.XScale)
Air.Velocity = -2 - Var(10)*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -14 + Var(10)*2*Const(Size.XScale)
YAccel = .25 + Var(10)*.15
P2Facing = 1

[State 211]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 220];強パンチ
Type = S
MoveType = A
Physics = N
Ctrl = 0
VelSet = IFelse(PrevStateNo=100, Vel X, 0)
Anim = 220
SprPriority = 2
PowerAdd = Var(10)*37 - (Var(14))*37

[State 220]
Type = VelAdd
Trigger1 = MoveContact && PrevStateNo = 100
Persistent = 0
IgnoreHitPause = 0
X = -1.8*Const(Size.XScale)

[State 220]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0+Var(10)*2, 9-Var(10)*4

[State 220]
Type = HitDef
Trigger1 = AnimElem = 3
Attr = S, NA
HitFlag = MA
GuardFlag = M
Damage = 140 + Var(10)*14
GetPower = 100 + Var(10)*100 - Var(14)*200
GivePower = 0
PauseTime = 14 - Var(10)*2, 14 - Var(10)*2
SparkNo = S(IFelse(Var(10), 62, 54+TeamSide))
Guard.SparkNo = S50 + Var(10)*10
SparkXY = -50+Floor(P2Dist X)/3, -36
HitSound = S0 + Var(10)*2, 3
GuardSound = S0 + Var(10)*2, 0
Animtype = Hard
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 29 - Var(10)*5
Air.HitTime = 99
Ground.Velocity = -9 + Var(10)*4*Const(Size.XScale)
Air.Velocity = -2 - Var(10)*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -18 + Var(10)*2*Const(Size.XScale)
YAccel = .25 + Var(10)*.15
P2Facing = 1

[State 220]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 221];後ろ押し強パンチ
Type = S
MoveType = A
Physics = N
Ctrl = 0
VelSet = IFelse(PrevStateNo=100, Vel X, 0)
Anim = 221
SprPriority = 2
PowerAdd = Var(10)*37 - (Var(14))*37

[State 221]
Type = VelAdd
Trigger1 = MoveContact && PrevStateNo = 100
Persistent = 0
IgnoreHitPause = 0
X = -1.8*Const(Size.XScale)

[State 221]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0+Var(10)*2, 9-Var(10)*4

[State 221]
Type = HitDef
Trigger1 = AnimElem = 3 || AnimElem = 4
Attr = S, NA
HitFlag = MA
GuardFlag = M
Damage = 133 + Var(10)*14
GetPower = 100 + Var(10)*100 - Var(14)*200
GivePower = 0
PauseTime = 14 - Var(10)*2, 14 - Var(10)*2
SparkNo = S(IFelse(Var(10), 62, 54+TeamSide))
Guard.SparkNo = S50 + Var(10)*10
SparkXY = -28+Floor(P2Dist X)/2, -46
HitSound = S0 + Var(10)*2, 3
GuardSound = S0 + Var(10)*2, 0
Animtype = Hard
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 29 - Var(10)*5
Air.HitTime = 99
Ground.Velocity = -9 + Var(10)*4*Const(Size.XScale)
Air.Velocity = -2 - Var(10)*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -18 + Var(10)*2*Const(Size.XScale)
YAccel = .25 + Var(10)*.15
P2Facing = 1

[State 221]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 230];弱キック
Type = S
MoveType = A
Physics = N
Ctrl = 0
VelSet = IFelse(PrevStateNo=100, Vel X, 0)
Anim = 230
SprPriority = 2
PowerAdd = Var(10)*12 - (Var(14))*12

[State 230]
Type = VelAdd
Trigger1 = MoveContact && PrevStateNo = 100
Persistent = 0
IgnoreHitPause = 0
X = -1.8*Const(Size.XScale)

[State 230]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0+Var(10)*2, 7-Var(10)*3

[State 230]
Type = HitDef
Trigger1 = AnimElem = 2
Attr = S, NA
HitFlag = MA
GuardFlag = M
Damage = 84 - Var(10) * 14
GetPower = 40 + Var(10)*60 - Var(14)*100
GivePower = 0
PauseTime = 14 - Var(10)*2, 14 - Var(10)*2
SparkNo = S(IFelse(Var(10), 61, 50+TeamSide))
Guard.SparkNo = S50 + Var(10)*10
SparkXY = -36+Floor(P2Dist X)/2, -84
HitSound = S0 + Var(10)*2, 4 - Var(10)*3
GuardSound = S0 + Var(10)*2, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 11 + Var(10)
Ground.HitTime = 12
Guard.HitTime = 19 - Var(10)*3
Air.HitTime = 99
Ground.Velocity = -5 + Var(10)*Const(Size.XScale)
Air.Velocity = -2 - Var(10)*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -10 + Var(10)*2*Const(Size.XScale)
YAccel = .25 + Var(10)*.15
P2Facing = 1

[State 230]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 231];前押し弱キック
Type = S
MoveType = A
Physics = N
Ctrl = 0
VelSet = IFelse(PrevStateNo=100, Vel X, 0)
Anim = 231
SprPriority = 2
PowerAdd = Var(10)*12 - (Var(14))*12

[State 231]
Type = VelAdd
Trigger1 = MoveContact && PrevStateNo = 100
Persistent = 0
IgnoreHitPause = 0
X = -1.8*Const(Size.XScale)

[State 231]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0+Var(10)*2, 7-Var(10)*3

[State 231]
Type = HitDef
Trigger1 = AnimElem = 3
Attr = S, NA
HitFlag = MA
GuardFlag = M
Damage = 77
GetPower = 40 + Var(10)*60 - Var(14)*100
GivePower = 0
PauseTime = 14 - Var(10)*2, 14 - Var(10)*2
SparkNo = S(IFelse(Var(10), 61, 50+TeamSide))
Guard.SparkNo = S50 + Var(10)*10
SparkXY = -36+Floor(P2Dist X)/3, -84
HitSound = S0 + Var(10)*2, 4 - Var(10)*3
GuardSound = S0 + Var(10)*2, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 11 + Var(10)
Ground.HitTime = 12
Guard.HitTime = 19 - Var(10)*3
Air.HitTime = 99
Ground.Velocity = -5 + Var(10)*Const(Size.XScale)
Air.Velocity = -2 - Var(10)*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -10 + Var(10)*2*Const(Size.XScale)
YAccel = .25 + Var(10)*.15

[State 231]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 240];中キック
Type = S
MoveType = A
Physics = N
Ctrl = 0
VelSet = IFelse(PrevStateNo=100, Vel X, 0)
Anim = 240
SprPriority = 2
PowerAdd = Var(10)*25 - (Var(14))*25

[State 240]
Type = VelAdd
Trigger1 = MoveContact && PrevStateNo = 100
Persistent = 0
IgnoreHitPause = 0
X = -1.8*Const(Size.XScale)

[State 240]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0+Var(10)*2, 8-Var(10)*3

[State 240]
Type = HitDef
Trigger1 = AnimElem = 3
Attr = S, NA
HitFlag = MA
GuardFlag = M
Damage = 98 - Var(10)*14
GetPower = 80 + Var(10)*70 - Var(14)*150
GivePower = 0
PauseTime = 14 - Var(10)*2, 14 - Var(10)*2
SparkNo = S(IFelse(Var(10), 61, 52+TeamSide))
Guard.SparkNo = S50 + Var(10)*10
SparkXY = -36+Floor(P2Dist X)/3, -84
HitSound = S0 + Var(10)*2, 5 - Var(10)*3
GuardSound = S0 + Var(10)*2, 0
AnimType = Middle
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 17
Ground.HitTime = 16
Guard.HitTime = 24 - Var(10)*4
Air.HitTime = 99
Ground.Velocity = -7 + Var(10)*Const(Size.XScale)
Air.Velocity = -2 - Var(10)*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -14 + Var(10)*2*Const(Size.XScale)
YAccel = .25 + Var(10)*.15
P2Facing = 1

[State 240]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 241];後ろ押し中キック
Type = S
MoveType = A
Physics = N
Ctrl = 0
VelSet = IFelse(PrevStateNo=100, Vel X, 0)
Anim = 241
SprPriority = 2
PowerAdd = Var(10)*25 - (Var(14))*25

[State 241]
Type = VelAdd
Trigger1 = MoveContact && PrevStateNo = 100
Persistent = 0
IgnoreHitPause = 0
X = -1.8*Const(Size.XScale)

[State 241]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0+Var(10)*2, 8-Var(10)*3

[State 241]
Type = HitDef
Trigger1 = AnimElem = 2
Attr = S, NA
HitFlag = MA
GuardFlag = M
Damage = 112 - Var(10)*14
GetPower = 80 + Var(10)*70 - Var(14)*150
GivePower = 0
PauseTime = 14 - Var(10)*2, 14 - Var(10)*2
SparkNo = S(IFelse(Var(10), 61, 52+TeamSide))
Guard.SparkNo = S50 + Var(10)*10
SparkXY = -36+Floor(P2Dist X)/2, -84
HitSound = S0 + Var(10)*2, 5 - Var(10)*3
GuardSound = S0 + Var(10)*2, 0
AnimType = Middle
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 17
Ground.HitTime = 16
Guard.HitTime = 24 - Var(10)*4
Air.HitTime = 99
Ground.Velocity = -7 + Var(10)*Const(Size.XScale)
Air.Velocity = -2 - Var(10)*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -14 + Var(10)*2*Const(Size.XScale)
YAccel = .25 + Var(10)*.15
P2Facing = 1

[State 241]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 250];強キック
Type = S
MoveType = A
Physics = N
Ctrl = 0
VelSet = IFelse(PrevStateNo=100, Vel X, 0)
Anim = 250
SprPriority = 2
PowerAdd = Var(10)*37 - (Var(14))*37

[State 250]
Type = VelAdd
Trigger1 = MoveContact && PrevStateNo = 100
Persistent = 0
IgnoreHitPause = 0
X = -1.8*Const(Size.XScale)

[State 250]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0+Var(10)*2, 9-Var(10)*4

[State 250]
Type = HitDef
Trigger1 = AnimElem = 5
Attr = S, NA
HitFlag = MA
GuardFlag = M
Damage = 119 + Var(10)*14
GetPower = 100 + Var(10)*100 - Var(14)*200
GivePower = 0
PauseTime = 14 - Var(10)*2, 14 - Var(10)*2
SparkNo = S(IFelse(Var(10), 62, 54+TeamSide))
Guard.SparkNo = S50 + Var(10)*10
SparkXY = -36+Floor(P2Dist X)/3, -84
HitSound = S0 + Var(10)*2, 6 - Var(10)*3
GuardSound = S0 + Var(10)*2, 0
Animtype = Hard
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 29 - Var(10)*5
Air.HitTime = 99
Ground.Velocity = -9 + Var(10)*4*Const(Size.XScale)
Air.Velocity = -2 - Var(10)*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -18 + Var(10)*2*Const(Size.XScale)
YAccel = .25 + Var(10)*.15
P2Facing = 1

[State 250]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 251];後ろ押し強キック
Type = S
MoveType = A
Physics = N
Ctrl = 0
VelSet = IFelse(PrevStateNo=100, Vel X, 0)
Anim = 251
SprPriority = 2
PowerAdd = Var(10)*37 - (Var(14))*37

[State 251]
Type = VelAdd
Trigger1 = MoveContact && PrevStateNo = 100
Persistent = 0
IgnoreHitPause = 0
X = -1.8*Const(Size.XScale)

[State 251]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0+Var(10)*2, 9-Var(10)*4

[State 251]
Type = HitDef
Trigger1 = AnimElem = 4
Attr = S, NA
HitFlag = MA
GuardFlag = M
Damage = 133 + Var(10)*14
GetPower = 100 + Var(10)*100 - Var(14)*200
GivePower = 0
PauseTime = 14 - Var(10)*2, 14 - Var(10)*2
SparkNo = S(IFelse(Var(10), 62, 54+TeamSide))
Guard.SparkNo = S50 + Var(10)*10
SparkXY = -28+Floor(P2Dist X)/2, -58
HitSound = S0 + Var(10)*2, 6 - Var(10)*3
GuardSound = S0 + Var(10)*2, 0
Animtype = Hard
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 29 - Var(10)*5
Air.HitTime = 99
Ground.Velocity = -9 + Var(10)*4*Const(Size.XScale)
Air.Velocity = -2 - Var(10)*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -18 + Var(10)*2*Const(Size.XScale)
YAccel = .25 + Var(10)*.15
P2Facing = 1

[State 251]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 300];しゃがみ弱パンチ
Type = C
MoveType = A
Physics = C
Ctrl = 0
VelSet = 0
Anim = 300
SprPriority = 2
PowerAdd = Var(10)*12 - (Var(14))*12

[State 300]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0+Var(10)*2, 7-Var(10)*3

[State 300]
Type = HitDef
Trigger1 = AnimElem = 4
Attr = C, NA
HitFlag = MA
GuardFlag = L
Damage = 77 + Var(10)*14
GetPower = 40 + Var(10)*60 - Var(14)*100
GivePower = 0
PauseTime = 14 - Var(10)*2, 14 - Var(10)*2
SparkNo = S(IFelse(Var(10), 61, 50+TeamSide))
Guard.SparkNo = S50 + Var(10)*10
SparkXY = -30+Floor(P2Dist X)/3, -22
HitSound = S0 + Var(10)*2, 1
GuardSound = S0 + Var(10)*2, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 11 + Var(10)
Ground.HitTime = 12
Guard.HitTime = 19 - Var(10)*3
Air.HitTime = 99
Ground.Velocity = -5 + Var(10)*Const(Size.XScale)
Air.Velocity = -2 - Var(10)*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -10 + Var(10)*2*Const(Size.XScale)
YAccel = .25 + Var(10)*.15
P2Facing = 1

[State 300]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 11
Ctrl = 1

[StateDef 301];後ろ押ししゃがみ弱パンチ
Type = C
MoveType = A
Physics = C
Ctrl = 0
VelSet = 0
Anim = 301
SprPriority = 2
PowerAdd = Var(10)*12 - (Var(14))*12

[State 301]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0+Var(10)*2, 7-Var(10)*3

[State 301]
Type = HitDef
Trigger1 = AnimElem = 2
Attr = C, NA
HitFlag = MA
GuardFlag = M
Damage = 98 + Var(10)*14
GetPower = 40 + Var(10)*60 - Var(14)*100
GivePower = 0
PauseTime = 14 - Var(10)*2, 14 - Var(10)*2
SparkNo = S(IFelse(Var(10), 61, 50+TeamSide))
Guard.SparkNo = S50 + Var(10)*10
SparkXY = -30+Floor(P2Dist X)/2, -52
HitSound = S0 + Var(10)*2, 1
GuardSound = S0 + Var(10)*2, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 11 + Var(10)
Ground.HitTime = 12
Guard.HitTime = 19 - Var(10)*3
Air.HitTime = 99
Ground.Velocity = -5 + Var(10)*Const(Size.XScale)
Air.Velocity = -2 - Var(10)*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -10 + Var(10)*2*Const(Size.XScale)
YAccel = .25 + Var(10)*.15
P2Facing = 1

[State 301]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 11
Ctrl = 1

[StateDef 310];しゃがみ中パンチ
Type = C
MoveType = A
Physics = C
Ctrl = 0
VelSet = 0
Anim = 300
SprPriority = 2
PowerAdd = Var(10)*25 - (Var(14))*25

[State 310]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0+Var(10)*2, 8-Var(10)*3

[State 310]
Type = HitDef
Trigger1 = AnimElem = 4
Attr = C, NA
HitFlag = MA
GuardFlag = L
Damage = 98 - Var(10)*14
GetPower = 80 + Var(10)*70 - Var(14)*150
GivePower = 0
PauseTime = 14 - Var(10)*2, 14 - Var(10)*2
SparkNo = S(IFelse(Var(10), 61, 52+TeamSide))
Guard.SparkNo = S50 + Var(10)*10
SparkXY = -30+Floor(P2Dist X)/3, -22
HitSound = S0 + Var(10)*2, 2
GuardSound = S0 + Var(10)*2, 0
AnimType = Middle
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 17
Ground.HitTime = 16
Guard.HitTime = 24 - Var(10)*4
Air.HitTime = 99
Ground.Velocity = -7 + Var(10)*Const(Size.XScale)
Air.Velocity = -2 - Var(10)*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -14 + Var(10)*2*Const(Size.XScale)
YAccel = .25 + Var(10)*.15
P2Facing = 1

[State 310]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 11
Ctrl = 1

[StateDef 311];後ろ押ししゃがみ中パンチ
Type = C
MoveType = A
Physics = C
Ctrl = 0
VelSet = 0
Anim = 311
SprPriority = 2
PowerAdd = Var(10)*25 - (Var(14))*25

[State 311]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0+Var(10)*2, 8-Var(10)*3

[State 311]
Type = HitDef
Trigger1 = AnimElem = 2
Attr = C, NA
HitFlag = MA
GuardFlag = M
Damage = 112 - Var(10)*14
GetPower = 80 + Var(10)*70 - Var(14)*150
GivePower = 0
PauseTime = 14 - Var(10)*2, 14 - Var(10)*2
SparkNo = S(IFelse(Var(10), 61, 52+TeamSide))
Guard.SparkNo = S50 + Var(10)*10
SparkXY = -30+Floor(P2Dist X)/2, -52
HitSound = S0 + Var(10)*2, 2
GuardSound = S0 + Var(10)*2, 0
AnimType = Middle
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 17
Ground.HitTime = 16
Guard.HitTime = 24 - Var(10)*4
Air.HitTime = 99
Ground.Velocity = -7 + Var(10)*Const(Size.XScale)
Air.Velocity = -2 - Var(10)*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -14 + Var(10)*2*Const(Size.XScale)
YAccel = .25 + Var(10)*.15
P2Facing = 1

[State 311]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 11
Ctrl = 1

[StateDef 320];しゃがみ強パンチ
Type = C
MoveType = A
Physics = C
Ctrl = 0
VelSet = 0
Anim = 320
SprPriority = 2
PowerAdd = Var(10)*37 - (Var(14))*37

[State 320]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0+Var(10)*2, 9-Var(10)*4

[State 320]
Type = HitDef
Trigger1 = AnimElem = 4
Attr = C, NA
HitFlag = MA
GuardFlag = L
Damage = 133 + Var(10)*14
GetPower = 100 + Var(10)*100 - Var(14)*200
GivePower = 0
PauseTime = 14 - Var(10)*2, 14 - Var(10)*2
SparkNo = S(IFelse(Var(10), 62, 54+TeamSide))
Guard.SparkNo = S50 + Var(10)*10
SparkXY = -30+Floor(P2Dist X)/3, -22
HitSound = S0 + Var(10)*2, 3
GuardSound = S0 + Var(10)*2, 0
Animtype = Hard
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 29 - Var(10)*5
Air.HitTime = 99
Ground.Velocity = -9 + Var(10)*4*Const(Size.XScale)
Air.Velocity = -2 - Var(10)*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -18 + Var(10)*2*Const(Size.XScale)
YAccel = .25 + Var(10)*.15
P2Facing = 1

[State 320]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 11
Ctrl = 1

[StateDef 321];後ろ押ししゃがみ強パンチ
Type = C
MoveType = A
Physics = C
Ctrl = 0
VelSet = 0
Anim = 321
SprPriority = 2
PowerAdd = Var(10)*37 - (Var(14))*37

[State 321]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0+Var(10)*2, 9-Var(10)*4

[State 321]
Type = HitDef
Trigger1 = AnimElem = 2
Attr = C, NA
HitFlag = MA
GuardFlag = M
Damage = 119 + Var(10)*14
GetPower = 100 + Var(10)*100 - Var(14)*200
GivePower = 0
PauseTime = 14 - Var(10)*2, 14 - Var(10)*2
SparkNo = S(IFelse(Var(10), 62, 54+TeamSide))
Guard.SparkNo = S50 + Var(10)*10
SparkXY = -30+Floor(P2Dist X)/2, -52
HitSound = S0 + Var(10)*2, 3
GuardSound = S0 + Var(10)*2, 0
Animtype = Hard
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 29 - Var(10)*5
Air.HitTime = 99
Ground.Velocity = -9 + Var(10)*4*Const(Size.XScale)
Air.Velocity = -2 - Var(10)*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -18 + Var(10)*2*Const(Size.XScale)
YAccel = .25 + Var(10)*.15

[State 321]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 11
Ctrl = 1

[StateDef 330];しゃがみ弱キック
Type = C
MoveType = A
Physics = N
Ctrl = 0
VelSet = 0
Anim = 330
SprPriority = 2
PowerAdd = Var(10)*12 - (Var(14))*12

[State 330]
Type = VelSet
Trigger1 = AnimElem = 2, 2 || AnimElem = 3
X = IFelse(AnimElem=2,2, 9, 0)*Const(Size.XScale)
[State 330]
Type = VelAdd
Trigger1 = AnimElem = 2,>2 && AnimElem = 3,<0
X = -.9*Const(Size.XScale)

[State 330]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0+Var(10)*2, 7-Var(10)*3

[State 330]
Type = HitDef
Trigger1 = AnimElem = 2
Attr = C, NA
HitFlag = MA
GuardFlag = L
Damage = 77 - Var(10) * 14
GetPower = 40 + Var(10)*60 - Var(14)*100
GivePower = 0
PauseTime = 14 - Var(10)*2, 14 - Var(10)*2
SparkNo = S(IFelse(Var(10), 61, 50+TeamSide))
Guard.SparkNo = S50 + Var(10)*10
SparkXY = -35+Floor(P2Dist X)/2, -16
HitSound = S0 + Var(10)*2, 4 - Var(10)*3
GuardSound = S0 + Var(10)*2, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 11 + Var(10)
Ground.HitTime = 12
Guard.HitTime = 19 - Var(10)*3
Air.HitTime = 99
Ground.Velocity = -5 + Var(10)*Const(Size.XScale)
Air.Velocity = -2 - Var(10)*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -10 + Var(10)*2*Const(Size.XScale)
YAccel = .25 + Var(10)*.15
P2Facing = 1

[State 330]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 11
Ctrl = 1

[StateDef 331];後ろ押ししゃがみ弱キック
Type = C
MoveType = A
Physics = C
Ctrl = 0
VelSet = 0
Anim = 331
SprPriority = 2
PowerAdd = Var(10)*12 - (Var(14))*12

[State 331]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0+Var(10)*2, 7-Var(10)*3

[State 331]
Type = HitDef
Trigger1 = AnimElem = 2
Attr = C, NA
HitFlag = MA
GuardFlag = L
Damage = 84 - Var(10) * 14
GetPower = 40 + Var(10)*60 - Var(14)*100
GivePower = 0
PauseTime = 14 - Var(10)*2, 14 - Var(10)*2
SparkNo = S(IFelse(Var(10), 61, 50+TeamSide))
Guard.SparkNo = S50 + Var(10)*10
SparkXY = -35+Floor(P2Dist X)/2, -16
HitSound = S0 + Var(10)*2, 4 - Var(10)*3
GuardSound = S0 + Var(10)*2, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 11 + Var(10)
Ground.HitTime = 12
Guard.HitTime = 19 - Var(10)*3
Air.HitTime = 99
Ground.Velocity = -5 + Var(10)*Const(Size.XScale)
Air.Velocity = -2 - Var(10)*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -10 + Var(10)*2*Const(Size.XScale)
YAccel = .25 + Var(10)*.15
P2Facing = 1

[State 331]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 11
Ctrl = 1

[StateDef 340];しゃがみ中キック
Type = C
MoveType = A
Physics = N
Ctrl = 0
VelSet = 0
Anim = 340
SprPriority = 2
PowerAdd = Var(10)*25 - (Var(14))*25

[State 340]
Type = VelSet
Trigger1 = AnimElem = 2, 2 || AnimElem = 3
X = IFelse(AnimElem=2,2, 11, 0)*Const(Size.XScale)
[State 340]
Type = VelAdd
Trigger1 = AnimElem = 2,>2 && AnimElem = 3,<0
X = -.9*Const(Size.XScale)

[State 340]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0+Var(10)*2, 8-Var(10)*3

[State 340]
Type = HitDef
Trigger1 = AnimElem = 2
Attr = C, NA
HitFlag = MA
GuardFlag = L
Damage = 98 - Var(10)*14
GetPower = 80 + Var(10)*70 - Var(14)*150
GivePower = 0
PauseTime = 14 - Var(10)*2, 14 - Var(10)*2
SparkNo = S(IFelse(Var(10), 61, 52+TeamSide))
Guard.SparkNo = S50 + Var(10)*10
SparkXY = -38+Floor(P2Dist X)/2, -16
HitSound = S0 + Var(10)*2, 5 - Var(10)*3
GuardSound = S0 + Var(10)*2, 0
AnimType = Middle
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 17
Ground.HitTime = 16
Guard.HitTime = 24 - Var(10)*4
Air.HitTime = 99
Ground.Velocity = -7 + Var(10)*Const(Size.XScale)
Air.Velocity = -2 - Var(10)*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -14 + Var(10)*2*Const(Size.XScale)
YAccel = .25 + Var(10)*.15
P2Facing = 1

[State 340]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 11
Ctrl = 1

[StateDef 341];後ろ押ししゃがみ中キック
Type = C
MoveType = A
Physics = C
Ctrl = 0
VelSet = 0
Anim = 341
SprPriority = 2
PowerAdd = Var(10)*25 - (Var(14))*25

[State 341]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0+Var(10)*2, 8-Var(10)*3

[State 341]
Type = HitDef
Trigger1 = AnimElem = 2
Attr = C, NA
HitFlag = MA
GuardFlag = L
Damage = 119 - Var(10)*14
GetPower = 80 + Var(10)*70 - Var(14)*150
GivePower = 0
PauseTime = 14 - Var(10)*2, 14 - Var(10)*2
SparkNo = S(IFelse(Var(10), 61, 52+TeamSide))
Guard.SparkNo = S50 + Var(10)*10
SparkXY = -36+Floor(P2Dist X)/2, -16
HitSound = S0 + Var(10)*2, 5 - Var(10)*3
GuardSound = S0 + Var(10)*2, 0
AnimType = Middle
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 17
Ground.HitTime = 16
Guard.HitTime = 24 - Var(10)*4
Air.HitTime = 99
Ground.Velocity = -7 + Var(10)*Const(Size.XScale)
Air.Velocity = -2 - Var(10)*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -14 + Var(10)*2*Const(Size.XScale)
YAccel = .25 + Var(10)*.15
P2Facing = 1

[State 341]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 11
Ctrl = 1

[StateDef 350];しゃがみ強キック
Type = C
MoveType = A
Physics = N
Ctrl = 0
VelSet = 0
Anim = 350
SprPriority = 2
PowerAdd = Var(10)*37 - (Var(14))*37

[State 350]
Type = VelSet
Trigger1 = AnimElem = 2, 2 || AnimElem = 3
X = IFelse(AnimElem=2,2, 13, 0)*Const(Size.XScale)
[State 350]
Type = VelAdd
Trigger1 = AnimElem = 2,>2 && AnimElem = 3,<-2
X = -1*Const(Size.XScale)

[State 350]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0+Var(10)*2, 9-Var(10)*4

[State 350]
Type = HitDef
Trigger1 = AnimElem = 2
Attr = C, NA
HitFlag = MA
GuardFlag = L
Damage = 133 + Var(10)*14
GetPower = 100 + Var(10)*100 - Var(14)*200
GivePower = 0
PauseTime = 14 - Var(10)*2, 14 - Var(10)*2
SparkNo = S(IFelse(Var(10), 62, 54+TeamSide))
Guard.SparkNo = S50 + Var(10)*10
SparkXY = -36+Floor(P2Dist X)/2, -16
HitSound = S0 + Var(10)*2, 6 - Var(10)*3
GuardSound = S0 + Var(10)*2, 0
Animtype = Hard
Ground.Type = Trip
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 29 - Var(10)*5
Ground.Velocity = -2 - Var(10)*Const(Size.XScale), -6
Guard.Velocity = -9 + Var(10)*4*Const(Size.XScale)
Air.Velocity = -2 - Var(10)*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -18 + Var(10)*2*Const(Size.XScale)
Fall = 1
Fall.Recover = 0
Fall.YVelocity = 0 - Var(10)*3
YAccel = .25 + Var(10)*.15
P2Facing = 1

[State 350]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 11
Ctrl = 1

[StateDef 351];後ろ押ししゃがみ強キック
Type = C
MoveType = A
Physics = C
Ctrl = 0
VelSet = 0
Anim = 351
SprPriority = 2
PowerAdd = Var(10)*37 - (Var(14))*37

[State 351]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0+Var(10)*2, 9-Var(10)*4

[State 351]
Type = HitDef
Trigger1 = AnimElem = 2
Attr = C, NA
HitFlag = MA
GuardFlag = L
Damage = 112 + Var(10)*14
GetPower = 100 + Var(10)*100 - Var(14)*200
GivePower = 0
PauseTime = 14 - Var(10)*2, 14 - Var(10)*2
SparkNo = S(IFelse(Var(10), 62, 54+TeamSide))
Guard.SparkNo = S50 + Var(10)*10
SparkXY = -35+Floor(P2Dist X)/2, -16
HitSound = S0 + Var(10)*2, 6 - Var(10)*3
GuardSound = S0 + Var(10)*2, 0
Animtype = Hard
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 21
Ground.HitTime = 20
Guard.HitTime = 29 - Var(10)*5
Air.HitTime = 99
Ground.Velocity = -9 + Var(10)*4*Const(Size.XScale)
Air.Velocity = -2 - Var(10)*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -18 + Var(10)*2*Const(Size.XScale)
YAccel = .25 + Var(10)*.15
P2Facing = 1

[State 351]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 11
Ctrl = 1

[StateDef 400];垂直空中弱パンチ
Type = A
MoveType = A
Physics = A
Ctrl = 0
Anim = 400
PowerAdd = Var(10)*12 - (Var(14))*12

[State 400]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0+Var(10)*2, 7-Var(10)*3

[State 400]
Type = HitDef
Trigger1 = AnimElem = 4 && Var(10)
Attr = A, NA
HitFlag = MA
GuardFlag = HA
Damage = 70
GetPower = 100 - Var(14)*100
GivePower = 0
PauseTime = 12, 12
SparkNo = S61
Guard.SparkNo = S60
SparkXY = -28+Floor(P2Dist X)/2, -20
HitSound = S2, 1
GuardSound = S2, 0
Animtype = Light
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 12
Ground.HitTime = 12
Guard.HitTime = 16
Air.HitTime = 99
Ground.Velocity = -4*Const(Size.XScale)
Air.Velocity = -3*Const(Size.XScale), -6
AirGuard.Velocity = -3*Const(Size.XScale), -3
Ground.CornerPush.VelOff = -8*Const(Size.XScale)
YAccel = .4
P2Facing = 1
[State 400]
Type = HitDef
Trigger1 = AnimElem = 4 && !Var(10)
Attr = A, NA
HitFlag = MA
GuardFlag = H
Damage = 84
GetPower = 40
GivePower = 0
PauseTime = 14, 14
SparkNo = S50+TeamSide
Guard.SparkNo = S50
SparkXY = -28+Floor(P2Dist X)/2, -20
HitSound = S0, 1
GuardSound = S0, 0
Guard.SparkNo = -1
Animtype = Light
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -10*Const(Size.XScale)
YAccel = .25
P2Facing = 1

[StateDef 401];空中弱パンチ
Type = A
MoveType = A
Physics = A
Ctrl = 0
Anim = 401
PowerAdd = Var(10)*12 - (Var(14))*12

[State 401]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0+Var(10)*2, 7-Var(10)*3

[State 401]
Type = HitDef
Trigger1 = AnimElem = 3 && Var(10)
Attr = A, NA
HitFlag = MA
GuardFlag = HA
Damage = 77
GetPower = 100 - Var(14)*100
GivePower = 0
PauseTime = 12, 12
SparkNo = S61
Guard.SparkNo = S60
SparkXY = -28+Floor(P2Dist X)/3, -38
HitSound = S2, 1
GuardSound = S2, 0
Animtype = Light
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 12
Ground.HitTime = 12
Guard.HitTime = 16
Air.HitTime = 99
Ground.Velocity = -4*Const(Size.XScale)
Air.Velocity = -3*Const(Size.XScale), -6
AirGuard.Velocity = -3*Const(Size.XScale), -3
Ground.CornerPush.VelOff = -8*Const(Size.XScale)
YAccel = .4
P2Facing = 1
[State 401]
Type = HitDef
Trigger1 = AnimElem = 3 && !Var(10)
Attr = A, NA
HitFlag = MA
GuardFlag = H
Damage = 91
GetPower = 40
GivePower = 0
PauseTime = 14, 14
SparkNo = S50+TeamSide
Guard.SparkNo = S50
SparkXY = -28+Floor(P2Dist X)/3, -38
HitSound = S0, 1
GuardSound = S0, 0
Guard.SparkNo = -1
Animtype = Light
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -10*Const(Size.XScale)
YAccel = .25
P2Facing = 1

[Statedef 410];垂直空中中パンチ
Type = A
MoveType = A
Physics = A
Ctrl = 0
Anim = 410
PowerAdd = Var(10)*25 - (Var(14))*25

[State 410]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0+Var(10)*2, 8-Var(10)*3

[State 410]
Type = HitDef
Trigger1 = AnimElem = 3 && Var(10)
Attr = A, NA
Animtype = Light
HitFlag = MA
GuardFlag = HA
Damage = 98
GetPower = 150 - Var(14)*150
GivePower = 0
PauseTime = 12, 12
SparkNo = S61
Guard.SparkNo = S60
SparkXY = -28+Floor(P2Dist X)/3, -38
HitSound = S2, 2
GuardSound = S2, 0
AnimType = Middle
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 12
Ground.HitTime = 12
Guard.HitTime = 16
Air.HitTime = 99
Ground.Velocity = -4*Const(Size.XScale)
Air.Velocity = -3*Const(Size.XScale), -6
AirGuard.Velocity = -3*Const(Size.XScale), -3
Ground.CornerPush.VelOff = -8*Const(Size.XScale)
YAccel = .4
P2Facing = 1
[State 410]
Type = HitDef
Trigger1 = AnimElem = 3 && !Var(10)
Attr = A, NA
HitFlag = MA
GuardFlag = H
Damage = 112
GetPower = 80
GivePower = 0
PauseTime = 14, 14
SparkNo = S52+TeamSide
Guard.SparkNo = S50
SparkXY = -28+Floor(P2Dist X)/3, -38
HitSound = S0, 2
GuardSound = S0, 0
Animtype = Light
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Ground.CornerPush.VelOff = -10*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
YAccel = .25
P2Facing = 1

[Statedef 411];空中中パンチ
Type = A
MoveType = A
Physics = A
Ctrl = 0
Anim = 411
PowerAdd = Var(10)*25 - (Var(14))*25

[State 411]
Type = ChangeAnim
Trigger1 = Var(10) && AnimElem = 9
Value = 48

[State 411]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0+Var(10)*2, 8-Var(10)*3

[State 411]
Type = HitDef
Trigger1 = AnimElem = 4 && Var(10)
Attr = A, NA
HitFlag = MA
GuardFlag = HA
Damage = 98
GetPower = 150 - Var(14)*150
GivePower = 0
PauseTime = 12, 12
SparkNo = S61
Guard.SparkNo = S60
SparkXY = -36+Floor(P2Dist X)/3, -80
HitSound = S2, 2
GuardSound = S2, 0
AnimType = Middle
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 12
Ground.HitTime = 12
Guard.HitTime = 16
Air.HitTime = 99
Ground.Velocity = -4*Const(Size.XScale)
Air.Velocity = -3*Const(Size.XScale), -6
AirGuard.Velocity = -3*Const(Size.XScale), -3
Ground.CornerPush.VelOff = -8*Const(Size.XScale)
YAccel = .4
P2Facing = 1
[State 411]
Type = HitDef
Trigger1 = AnimElem = 4 && !Var(10)
Attr = A, NA
HitFlag = MAF
GuardFlag = H
Damage = 112
GetPower = 80
GivePower = 0
PauseTime = 14, 14
SparkNo = S52+TeamSide
Guard.SparkNo = S50
SparkXY = -36+Floor(P2Dist X)/3, -80
HitSound = S0, 2
GuardSound = S0, 0
Animtype = Light
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -10*Const(Size.XScale)
Air.Fall = 1
Fall.YVelocity = -4
YAccel = .25
P2Facing = 1

[StateDef 420];空中強パンチ
Type = A
MoveType = A
Physics = A
Ctrl = 0
Anim = 420
PowerAdd = Var(10)*37 - (Var(14))*37

[State 420]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0+Var(10)*2, 9-Var(10)*4

[State 420]
Type = HitDef
Trigger1 = AnimElem = 3 && Var(10)
Attr = A, NA
HitFlag = MA
GuardFlag = HA
Damage = 140
GetPower = 200 - Var(14)*200
GivePower = 0
PauseTime = 12, 12
SparkNo = S62
Guard.SparkNo = S60
SparkXY = -28+Floor(P2Dist X)/3, -38
HitSound = S2, 3
GuardSound = S2, 0
AnimType = Hard
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 12
Ground.HitTime = 12
Guard.HitTime = 16
Air.HitTime = 99
Ground.Velocity = -4*Const(Size.XScale)
Air.Velocity = -3*Const(Size.XScale), -6
AirGuard.Velocity = -3*Const(Size.XScale), -3
Ground.CornerPush.VelOff = -8*Const(Size.XScale)
YAccel = .4
P2Facing = 1
[State 420]
Type = HitDef
Trigger1 = AnimElem = 3 && !Var(10)
Attr = A, NA
HitFlag = MA
GuardFlag = H
Damage = 152
GetPower = 100
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S50
SparkXY = -28+Floor(P2Dist X)/3, -38
HitSound = S0, 3
GuardSound = S0, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -10*Const(Size.XScale)
YAccel = .25
P2Facing = 1

[StateDef 430];空中弱キック
Type = A
MoveType = A
Physics = A
Ctrl = 0
Anim = 430
PowerAdd = Var(10)*12 - (Var(14))*12

[State 430]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0+Var(10)*2, 7-Var(10)*3

[State 430]
Type = HitDef
Trigger1 = AnimElem = 3 && Var(10)
Attr = A, NA
HitFlag = MA
GuardFlag = HA
Damage = 70
GetPower = 100 - Var(14)*100
GivePower = 0
PauseTime = 12, 12
SparkNo = S61
Guard.SparkNo = S60
SparkXY = -36+Floor(P2Dist X)/3, -42
HitSound = S2, 1
GuardSound = S2, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 12
Ground.HitTime = 12
Guard.HitTime = 16
Air.HitTime = 99
Ground.Velocity = -4*Const(Size.XScale)
Air.Velocity = -3*Const(Size.XScale), -6
AirGuard.Velocity = -3*Const(Size.XScale), -3
Ground.CornerPush.VelOff = -8*Const(Size.XScale)
YAccel = .4
P2Facing = 1
[State 430]
Type = HitDef
Trigger1 = AnimElem = 3 && !Var(10)
Attr = A, NA
HitFlag = MA
GuardFlag = H
Damage = 84
GetPower = 40
GivePower = 0
PauseTime = 14, 14
SparkNo = S50+TeamSide
Guard.SparkNo = S50
SparkXY = -36+Floor(P2Dist X)/3, -42
HitSound = S0, 4
GuardSound = S0, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -10*Const(Size.XScale)
YAccel = .25
P2Facing = 1

[StateDef 440];空中中キック
Type = A
MoveType = A
Physics = A
Ctrl = 0
Anim = 440
PowerAdd = Var(10)*25 - (Var(14))*25

[State 440]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0+Var(10)*2, 8-Var(10)*3

[State 440]
Type = HitDef
Trigger1 = AnimElem = 3 && Var(10)
Attr = A, NA
HitFlag = MA
GuardFlag = HA
Damage = 105
GetPower = 150 - Var(14)*150
GivePower = 0
PauseTime = 12, 12
SparkNo = S61
Guard.SparkNo = S60
SparkXY = -36+Floor(P2Dist X)/3, -42
HitSound = S2, 2
GuardSound = S2, 0
AnimType = Middle
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 12
Ground.HitTime = 12
Guard.HitTime = 16
Air.HitTime = 99
Ground.Velocity = -4*Const(Size.XScale)
Air.Velocity = -3*Const(Size.XScale), -6
AirGuard.Velocity = -3*Const(Size.XScale), -3
Ground.CornerPush.VelOff = -8*Const(Size.XScale)
YAccel = .4
P2Facing = 1
[State 440]
Type = HitDef
Trigger1 = AnimElem = 3 && !Var(10)
Attr = A, NA
HitFlag = MA
GuardFlag = H
Damage = 119
GetPower = 80
GivePower = 0
PauseTime = 14, 14
SparkNo = S52+TeamSide
Guard.SparkNo = S50
SparkXY = -36+Floor(P2Dist X)/3, -42
HitSound = S0, 5
GuardSound = S0, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -10*Const(Size.XScale)
YAccel = .25
P2Facing = 1

[StateDef 450];空中強キック
Type = A
MoveType = A
Physics = A
Ctrl = 0
Anim = 450
PowerAdd = Var(10)*37 - (Var(14))*37

[State 450]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0+Var(10)*2, 9-Var(10)*4

[State 450]
Type = HitDef
Trigger1 = AnimElem = 5 && Var(10)
Attr = A, NA
HitFlag = MA
GuardFlag = HA
Damage = 140
GetPower = 200 - Var(14)*200
GivePower = 0
PauseTime = 12, 12
SparkNo = S62
Guard.SparkNo = S60
SparkXY = -36+Floor(P2Dist X)/3, -84
HitSound = S2, 3
GuardSound = S2, 0
AnimType = Hard
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 12
Ground.HitTime = 12
Guard.HitTime = 16
Air.HitTime = 99
Ground.Velocity = -4*Const(Size.XScale)
Air.Velocity = -3*Const(Size.XScale), -6
AirGuard.Velocity = -3*Const(Size.XScale), -3
Ground.CornerPush.VelOff = -8*Const(Size.XScale)
YAccel = .4
P2Facing = 1
[State 450]
Type = HitDef
Trigger1 = AnimElem = 5 && !Var(10)
Attr = A, NA
HitFlag = MA
GuardFlag = H
Damage = 154
GetPower = 100
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S50
SparkXY = -36+Floor(P2Dist X)/3, -84
HitSound = S0, 6
GuardSound = S0, 0
AnimType = Light
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -10*Const(Size.XScale)
YAccel = .25
P2Facing = 1

[StateDef 460];ドリル頭突き
Type = A
MoveType = A
Physics = A
Ctrl = 0
VelSet = 0, 0
Anim = 460
PowerAdd = Var(10)*37 - (Var(14))*37

[State 460]
Type = VelSet
Trigger1 = Time > 1
X = 4.5*Const(Size.XScale)
Y = 1.75

[State 460]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0+Var(10)*2, 9-Var(10)*4

[State 460]
Type = HitDef
Trigger1 = AnimElem = 2,>=0 && !MoveContact && Var(10)
Attr = A, NA
HitFlag = MA
GuardFlag = HA
Damage = 133
GetPower = 200 - Var(14)*200
GivePower = 0
PauseTime = 12, 12
SparkNo = S62
Guard.SparkNo = S60
SparkXY = -28+Floor(P2Dist X)/2, -62
HitSound = S2, 3
GuardSound = S2, 0
AnimType = Hard
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 12
Ground.HitTime = 12
Guard.HitTime = 16
Air.HitTime = 99
Ground.Velocity = -4*Const(Size.XScale)
Air.Velocity = -3*Const(Size.XScale), -6
AirGuard.Velocity = -3*Const(Size.XScale), -3
Ground.CornerPush.VelOff = -8*Const(Size.XScale)
YAccel = .4
P2Facing = 1
[State 460]
Type = HitDef
Trigger1 = AnimElem = 2,>=0 && !MoveContact && !Var(10)
Attr = A, NA
HitFlag = MA
GuardFlag = H
Damage = 147
GetPower = 100
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S50
SparkXY = -28+Floor(P2Dist X)/2, -62
HitSound = S0, 6
GuardSound = S0, 0
AnimType = Hard
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -10*Const(Size.XScale)
YAccel = .25
P2Facing = 1

[StateDef 470];ドリルキック 弱
Type = A
MoveType = A
Physics = A
Ctrl = 0
VelSet = 0, 0
Anim = 470
PowerAdd = Var(10)*37 - (Var(14))*37

[State 470]
Type = VelSet
Trigger1 = Time > 1
X = 4.5*Const(Size.XScale)
Y = 1.75

[State 470]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0+Var(10)*2, 9-Var(10)*4

[State 470]
Type = HitDef
Trigger1 = AnimElem = 2,>=0 && !MoveContact && Var(10)
Attr = A, NA
HitFlag = MA
GuardFlag = HA
Damage = 126
GetPower = 100 - Var(14)*100
GivePower = 0
PauseTime = 12, 12
SparkNo = S61
Guard.SparkNo = S60
SparkXY = -34+Floor(P2Dist X)/2, -42
HitSound = S2, 3
GuardSound = S2, 0
AnimType = Hard
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 12
Ground.HitTime = 12
Guard.HitTime = 16
Air.HitTime = 99
Ground.Velocity = -4*Const(Size.XScale)
Air.Velocity = -3*Const(Size.XScale), -6
AirGuard.Velocity = -3*Const(Size.XScale), -3
Ground.CornerPush.VelOff = -8*Const(Size.XScale)
YAccel = .4
P2Facing = 1
[State 470]
Type = HitDef
Trigger1 = AnimElem = 2,>=0 && !MoveContact && !Var(10)
Attr = A, NA
HitFlag = MA
GuardFlag = H
Damage = 140
GetPower = 100
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S50
SparkXY = -34+Floor(P2Dist X)/2, -42
HitSound = S0, 6
GuardSound = S0, 0
AnimType = Hard
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -10*Const(Size.XScale)
YAccel = .25
P2Facing = 1

[StateDef 471];ドリルキック 中
Type = A
MoveType = A
Physics = A
Ctrl = 0
VelSet = 0, 0
Anim = 471
PowerAdd = Var(10)*37 - (Var(14))*37

[State 471]
Type = VelSet
Trigger1 = Time > 1
X = 4.25*Const(Size.XScale)
Y = 2.5

[State 471]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0+Var(10)*2, 9-Var(10)*4

[State 471]
Type = HitDef
Trigger1 = AnimElem = 2,>=0 && !MoveContact && Var(10)
Attr = A, NA
HitFlag = MA
GuardFlag = HA
Damage = 140
GetPower = 150 - Var(14)*150
GivePower = 0
PauseTime = 12, 12
SparkNo = S61
Guard.SparkNo = S60
SparkXY = -30+Floor(P2Dist X)/2, -32
HitSound = S2, 3
GuardSound = S2, 0
AnimType = Hard
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 12
Ground.HitTime = 12
Guard.HitTime = 16
Air.HitTime = 99
Ground.Velocity = -4*Const(Size.XScale)
Air.Velocity = -3*Const(Size.XScale), -6
AirGuard.Velocity = -3*Const(Size.XScale), -3
Ground.CornerPush.VelOff = -8*Const(Size.XScale)
YAccel = .4
P2Facing = 1
[State 471]
Type = HitDef
Trigger1 = AnimElem = 2,>=0 && !MoveContact && !Var(10)
Attr = A, NA
HitFlag = MA
GuardFlag = H
Damage = 154
GetPower = 100
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S50
SparkXY = -30+Floor(P2Dist X)/2, -32
HitSound = S0, 6
GuardSound = S0, 0
AnimType = Hard
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -10*Const(Size.XScale)
YAccel = .25
P2Facing = 1

[StateDef 472];ドリルキック 強
Type = A
MoveType = A
Physics = A
Ctrl = 0
VelSet = 0, 0
Anim = 472
PowerAdd = Var(10)*37 - (Var(14))*37

[State 472]
Type = VelSet
Trigger1 = Time > 1
X = 2.75*Const(Size.XScale)
Y = 3.75

[State 472]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0+Var(10)*2, 9-Var(10)*4

[State 472]
Type = HitDef
Trigger1 = AnimElem = 2,>=0 && !MoveContact && Var(10)
Attr = A, NA
HitFlag = MA
GuardFlag = HA
Damage = 98
GetPower = 200 - Var(14)*200
GivePower = 0
PauseTime = 12, 12
SparkNo = S62
Guard.SparkNo = S60
SparkXY = -26+Floor(P2Dist X)/2, -10
HitSound = S2, 3
GuardSound = S2, 0
AnimType = Hard
Air.AnimType = Back
Ground.Type = High
Ground.SlideTime = 12
Ground.HitTime = 12
Guard.HitTime = 16
Air.HitTime = 99
Ground.Velocity = -4*Const(Size.XScale)
Air.Velocity = -3*Const(Size.XScale), -6
AirGuard.Velocity = -3*Const(Size.XScale), -3
Ground.CornerPush.VelOff = -8*Const(Size.XScale)
YAccel = .4
P2Facing = 1
[State 472]
Type = HitDef
Trigger1 = AnimElem = 2,>=0 && !MoveContact && !Var(10)
Attr = A, NA
HitFlag = MA
GuardFlag = H
Damage = 112
GetPower = 100
GivePower = 0
PauseTime = 14, 14
SparkNo = S54+TeamSide
Guard.SparkNo = S50
SparkXY = -26+Floor(P2Dist X)/2, -10
HitSound = S0, 6
GuardSound = S0, 0
AnimType = Hard
Air.AnimType = Back
Ground.Type = Low
Ground.SlideTime = 11
Ground.HitTime = 12
Guard.HitTime = 19
Air.HitTime = 99
Ground.Velocity = -5*Const(Size.XScale)
Air.Velocity = -2*Const(Size.XScale), -6
Ground.CornerPush.VelOff = -10*Const(Size.XScale)
YAccel = .25
P2Facing = 1