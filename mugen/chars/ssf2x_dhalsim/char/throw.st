[StateDef 500];ヨガスマッシュ
Type = S
MoveType = A
Physics = N
Ctrl = 0
VelSet = 0

[State 500]
Type = HitDef
Trigger1 = Time = 0
Attr = S, NT
HitFlag = M-
Priority = 0, Miss
Damage = 0
GetPower = 0
GivePower = 0
NumHits = 0
SparkNo = -1
HitSound = -1
AnimType = Light
Ground.Velocity = -2 - Var(10)*Const(Size.XScale), -6
Fall = 1
Fall.YVelocity = -4+Var(10)
YAccel = .25 + Var(10)*.15
ID = 1
P1StateNo = 501
P2StateNo = 502
P1Facing = 1 - (Command="holdback")*2
P2Facing = 1

[State 500]
Type = ChangeState
Trigger1 = Time = 1
Value = IFelse(Command="holdback", 211, 210)

[StateDef 501];ヨガスマッシュ中
Type = S
MoveType = A
Physics = S
Ctrl = 0
VelSet = 0
Anim = 500

[State 501]
Type = ChangeAnim
Trigger1 = Var(16) > 24 && AnimElem = 4,[6,24]
Value = 500
Elem = 5

[State 501]
Type = Width
Trigger1 = Time = 0
Value = Floor(120*Const(Size.XScale)), 0

[State 501]
Type = VarSet
Trigger1 = Time = 0
V = 15
Value = 204
[State 501]
Type = VarAdd
Trigger1 = 1
V = 15
Value = -1
[State 501]
Type = VarAdd
Trigger1 = Target(1),Command="a"||Target(1),Command="b"||Target(1),Command="c"||Target(1),Command="x"||Target(1),Command="y"||Target(1),Command="z"||Target(1),Command="fwd"||Target(1),Command="back"||Target(1),Command="up"||Target(1),Command="down"
V = 15
Value = -3
[State 501]
Type = VarAdd
Trigger1 = (Command="fwd"||Command="back"||Command="up"||Command="down"||Command="a"||Command="b"||Command="c"||Command="x"||Command="y"||Command="z") && Var(15) > 10
Persistent = 4
V = 15
Value = 1

[State 501]
Type = VarSet
Trigger1 = Time = 0 || AnimElem = 5,1
V = 16
Value = 0
[State 501]
Type = VarAdd
Trigger1 = Command="fwd"||Command="back"||Command="up"||Command="down"||Command="a"||Command="b"||Command="c"||Command="x"||Command="y"||Command="z"
V = 16
Value = 6
[State 501]
Type = VarAdd
Trigger1 = Var(16) > 0
V = 16
Value = -1

[State 501]
Type = TargetBind
Trigger1 =  Time = 0 || AnimElem = 2
ID = 1
Pos = Floor(43 + (Time=0)*47)*Const(Size.XScale), 0

[State 501]
Type = PlaySnd
Trigger1 = AnimElem = 4
Value = 10, 0 + Var(10)*4
[State 501]
Type = PlaySnd
Trigger1 = AnimElem = 4
Value = 0 + Var(10)*2, 3

[State 501]
Type = Explod
Trigger1 = AnimElem = 4
Anim = IFelse(Var(10), 62, 54+TeamSide)
Pos = Floor(30*Const(Size.XScale)), -90
PosType = P1
SprPriority = 10
OwnPal = 1
Scale = Const(Size.XScale), 1

[State 501]
Type = TargetState
Trigger1 = AnimTime = 0
ID = 1
Value = 502

[State 501]
Type = TargetState
Trigger1 = (Var(15) < 0 && AnimElem = 5,>5) || !Target(1),Alive
ID = 1
Value = 503
[State 501]
Type = ChangeState
Trigger1 = (Var(15) < 0 && AnimElem = 5,>5) || !Target(1),Alive
Value = 0
Ctrl = 1

[State 501]
Type = ChangeAnim
Trigger1 = AnimTime = 0
Value = 500
Elem = 2

[StateDef 502];ヨガスマれる相手
Type = S
MoveType = H
Physics = S
VelSet = 0, 0
SprPriority = -1

[State 502]
Type = LifeAdd
Trigger1 = AnimElem = 4
Value = -184 + (PrevStateNo=502)*156

[State 502]
Type = ChangeAnim2
Trigger1 = Time = 0
Value = 502
Elem = 1 + (PrevStateNo=502)

[StateDef 503];ヨガスマれる相手開放
Type = A
MoveType = H
Physics = N

[State 503]
Type = HitVelSet
Trigger1 = Time = 0 && PrevStateNo != 505
X = 1
Y = 1
[State 503]
Type = VelAdd
Trigger1 = 1
Y = GetHitVar(YAccel)

[State 503]
Type = NotHitBy
Trigger1 = 1
Value = SCA

[State 503]
Type = ChangeAnim
Trigger1 = (Time = 0 && PrevStateNo != 505) || (Time = 13 && Anim != 5040)
Value = IFelse(Time=0, 5000 + (SelfAnimExist(5035))*35, 5040)

[State 503]
Type = ChangeState
TriggerAll = BackEdgeDist < 2 && Time > 0 && PrevStateNo != 505
Trigger1 =  Name="SSF2Xリュウ"||(Name="SSF2Xダルシム"&&!Var(10))||Name="SF2'リュウ"||Name="SF2'ケン"||Name="SF2'ガイル"||Name="SF2'サガット"||Name="SF2'春麗"||Name="SF2'ザンギエフ"
Trigger2 = Name="SF2'ベガ"||Name="SF2'ブランカ"
Value = 505

[State 503]
Type = SelfState
Trigger1 = (Time = 0 && !Alive) || (Vel Y > 0 && Pos Y >= 0)
Value = 52 + (Time=0)*4948

[StateDef 505];相手降下中の画面端引っ掛かり
Type = A
MoveType = H
Physics = N

[State 505]
Type = VelSet
Trigger1 = Time = 0
Y = 0

[State 505]
Type = AssertSpecial
Trigger1 = PrevStateNo = 1005
Flag = Invisible

[State 505]
Type = PosFreeze
Trigger1 = 1

[State 505]
Type = NotHitBy
Trigger1 = 1
Value = SCA

[State 505]
Type = ChangeAnim
Trigger1 = PrevStateNo = 503 || PrevStateNo = 514
Value = Anim
Elem = AnimElemNo(0)

[State 505]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0+(Var(10)&&Name="SSF2Xリュウ")*2, 11+(Var(10)&&Name="SSF2Xリュウ")

[State 505]
Type = Explod
Trigger1 = Time = 0 && Name = "SSF2Xリュウ" && Var(10)
Anim = 96-TeamSide
Pos = 0, 0
PosType = P1
SprPriority = -10
OwnPal = 1
Facing = -1 + ((Facing=-1&&ScreenPos X>160)||(Facing=1&&ScreenPos X<160))*2
RemoveOnGetHit = 1
Scale = Const(Size.XScale), 1

[State 505]
Type = LifeAdd
Trigger1 = Time = 0 && PrevStateNo = 513
Value = -238

[State 505]
Type = ChangeState
Trigger1 = Time = 12 || (Time=4&&Var(10)&&Name="SSF2Xリュウ")
Value = PrevStateNo

[StateDef 510];ヨガスルー
Type = S
MoveType = A
Physics = S
Ctrl = 0
VelSet = 0

[State 510]
Type = HitDef
Trigger1 = Time = 0
Attr = S, NT
HitFlag = M-
Priority = 0, Miss
Damage = 0
GetPower = 0
NumHits = 0
SparkNo = -1
HitSound = -1
AnimType = Light
Ground.Velocity = 3.5*Const(Size.XScale), -3
Fall.YVelocity = -4+Var(10)
YAccel = .2 + Var(10)*.2
ID = 1
P1StateNo = 511
P2StateNo = 512
P1Facing = IFelse(Command = "holdback", 1, -1)
P2Facing = 1

[State 510]
Type = ChangeState
Trigger1 = Time = 1
Value = 220 + (Command="holdback")

[StateDef 511];ヨガスルー中
Type = S
MoveType = A
Physics = S
Ctrl = 0
VelSet = 0
Anim = 510

[State 511]
Type = Turn
Trigger1 = AnimTime = 0

[State 511]
Type = Width
Trigger1 = AnimElem = 4
Value = 0, Floor(120*Const(Size.XScale))

[State 511]
Type = TargetBind
Trigger1 =  Time = 0 || AnimElem = 2 || AnimElem = 3 || AnimElem = 4
ID = 1
Pos = IFelse(Time=0,90,IFelse(AnimElem=2,0,54,IFelse(AnimElem=3,0,-22,-107)))*Const(Size.XScale), IFelse(AnimElem=3,0,-73,IFelse(AnimElem=4,0,-50,0))

[State 511]
Type = PlaySnd
Trigger1 = AnimElem = 4
Value = 10, 3

[State 511]
Type = VarSet
Trigger1 = (Time < 10 && (Target(1),Command="holdfwd"||Target(1),Command="holdback") && (Target(1),Command="z"||Target(1),Command="y")) || Time = 0
V = 15
Value = IFelse(Time=0, 0, 1)

[State 511]
Type = TargetState
Trigger1 = AnimElem = 4,6 && Var(15)
ID = 1
Value = 514

[State 511]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 512];ヨガスられる相手
Type = S
MoveType = H
Physics = S
VelSet = 0, 0
SprPriority = -1

[State 512]
Type = ChangeAnim2
Trigger1 = Time = 0
Value = 511

[State 512]
Type = ScreenBound
Trigger1 = AnimElem = 3,<=0
Value = 0

[State 512]
Type = ChangeState
Trigger1 = AnimElem = 3,9
Value = 513

[StateDef 513];ヨガスられる相手開放
Type = A
MoveType = H
Physics = N

[State 513]
Type = HitVelSet
Trigger1 = Time = 0 && PrevStateNo != 505
X = 1
Y = 1
[State 513]
Type = VelAdd
Trigger1 = 1
Y = .2

[State 513]
Type = LifeAdd
Trigger1 = ((Vel Y > 0 &&  Pos Y >= 15) || (Life <= Floor(238/(Const(Data.Defence)/100)))) && PrevStateNo != 505
Persistent = 0
Value = IFelse((Life<=Floor(238/(Const(Data.Defence)/100))), -1000, -238)

[State 513]
Type = ChangeState
TriggerAll = FrontEdgeDist < 2 && Time > 0 && PrevStateNo != 505
Trigger1 =  Name="SSF2Xリュウ"||(Name="SSF2Xダルシム"&&!Var(10))||Name="SF2'リュウ"||Name="SF2'ケン"||Name="SF2'ガイル"||Name="SF2'サガット"||Name="SF2'春麗"||Name="SF2'ザンギエフ"
Trigger2 = Name="SF2'ベガ"||Name="SF2'ブランカ"
Value = 505

[State 513]
Type = SelfState
Trigger1 = Vel Y > 0 &&  Pos Y >= 20
Value = 5100

[StateDef 514];ヨガスル抜ける相手
Type = A
MoveType = H
Physics = N
VelSet = 0

[State 514]
Type = Turn
Trigger1 = Time = 0 && PrevStateNo != 505

[State 514]
Type = VelSet
Trigger1 = Time = 0 && (Name!="SSF2Xリュウ"||(!Var(10)&&Name="SSF2Xリュウ"))
Y = 0

[State 514]
Type = HitVelSet
Trigger1 = Time = 0 && PrevStateNo != 505
X = 1
Y = 1
[State 514]
Type = VelAdd
Trigger1 = 1
Y = GetHitVar(YAccel)

[State 514]
Type = NotHitBy
Trigger1 = 1
Value = SCA

[State 514]
Type = LifeAdd
Trigger1 = PrevStateNo != 505 && Time = 0
Value = -119

[State 514]
Type = ChangeAnim
Trigger1 = ((Time=0||Time=13)&&PrevStateNo!=505) || (Time=0&&PrevStateNo=505&&Anim!=5040)
Value = IFelse(Time=0&&PrevStateNo!=505, 5000 + (SelfAnimExist(5035))*35, 5040)

[State 514]
Type = ChangeState
TriggerAll = BackEdgeDist < 2 && Time > 0 && PrevStateNo != 505
Trigger1 =  Name="SSF2Xリュウ"||(Name="SSF2Xダルシム"&&!Var(10))||Name="SF2'リュウ"||Name="SF2'ケン"||Name="SF2'ガイル"||Name="SF2'サガット"||Name="SF2'春麗"||Name="SF2'ザンギエフ"
Trigger2 = Name="SF2'ベガ"||Name="SF2'ブランカ"
Value = 505

[State 514]
Type = SelfState
Trigger1 = (Time = 0 && !Alive) || (Vel Y > 0 && Pos Y >= 0)
Value = 52 + (Time=0)*4948

[StateDef 4000];気絶直前 倒れ
Type = A
MoveType = H
Physics = N
SprPriority = -1
Anim = 5030

[State 4000]
Type = ChangeAnim
Trigger1 = Time = 0 || (Anim=5030&&HitShakeOver)
IgnoreHitPause = 1
Value = IFelse(Time=0, 5030, 5050)

[State 4000]
Type = VelSet
Trigger1 = Time = 0 && PrevStateNo != 5042
X = 0
Y = 0

[State 4000]
Type = ModifyExplod
Trigger1 = Pos Y >= 0
ID = 2
PosType = P1
Pos = 0, Floor(RootDist Y - Pos Y)

[State 4000]
Type = VarRandom
Trigger1 = PrevStateNo != 5042
V = 0
Range = 0, 3

[State 4000]
Type = PlaySnd
Trigger1 = Time = 0 && PrevStateNo != 5042
Value = 0, IFelse(Var(0) = 0, 14, IFelse(Var(0) = 1, 13, IFelse(Var(0) = 2, 15, 16)))
Channel = 6
Loop = 1

[State 4000]
Type = Explod
Trigger1 = Time = 0 && PrevStateNo != 5042
Anim = 4000 + Var(0)
ID = 2
PosType = P1
Pos = 0, 0
SprPriority = -1
BindTime = -1
RemoveOnGetHit = 1
RemoveTime = -1
[State 4000]
Type = Explod
Trigger1 = Time = 0 && PrevStateNo != 5042
Anim = 4004 + Var(0)
ID = 2
PosType = P1
Pos = 0, 0
SprPriority = 1
BindTime = -1
RemoveOnGetHit = 1
RemoveTime = -1
[State 4000]
Type = Explod
Trigger1 = Time = 0 && PrevStateNo != 5042
Anim = 4008 + Var(0)
ID = 2
PosType = P1
Pos = 0, 0
SprPriority = 1
BindTime = -1
RemoveOnGetHit = 1
RemoveTime = -1

[State 4000]
Type = NotHitBy
Trigger1 = 1
IgnoreHitPause = 1
Value = SCA

[State 4000]
Type = HitVelSet
Trigger1 = HitShakeOver && PrevStateNo != 5042 && GetHitVar(YVel) < 0
Persistent = 0
X = 1
Y = 1
[State 4000]
Type = VelSet
Trigger1 = HitShakeOver && PrevStateNo != 5042 && GetHitVar(YVel) >= 0
Persistent = 0
X = -2*Const(Size.XScale)
Y = -6
[State 4000]
Type = VelAdd
Trigger1 = HitShakeOver || PrevStateNo = 5042
Y = IFelse(GetHitVar(YAccel)>0, GetHitVar(YAccel), .25)

[State 4000]
Type = ChangeState
Trigger1 = Vel Y > 0 &&  Pos Y >= 25
Value = 5100

[StateDef 4001];気絶中
Type = S
MoveType = I
Physics = S
Anim = 5300

[State 4001]
Type = VarSet
Trigger1 = Time = 0
V = 1
Value = -400
[State 4001]
Type = VarSet
Trigger1 = Time = 0
V = 2
Value = 0
[State 4001]
Type = VarAdd
Trigger1 = 1
V = 2
Value = 1
[State 4001]
Type = VarAdd
Trigger1 = Command = "up" || Command = "down" || Command = "fwd" || Command = "back" || Command = "a" || Command = "b" || Command = "c" || Command = "x" || Command = "y" || Command = "z"
V = 2
Value = 2

[State 4001]
Type = ChangeAnim
Trigger1 = 1
Value = 5300
Elem = IFelse((Var(2)=[0,21])||(Var(2)=[86,107])||(Var(2)=[172,193]), 1, IFelse((Var(2)=[22,39])||(Var(2)=[66,85])||(Var(2)=[108,125])||(Var(2)=[152,171])||Var(2)>193, 2, 3))

[State 4001]
Type = ChangeState
Trigger1 = (Var(0) = 0 && Var(2) >= 120) || (Var(0) = 1 && Var(2) >= 150) || (Var(0) = 2 && Var(2) >= 60) || (Var(0) = 3 && Var(2) >= 210)
Value = 0
Ctrl = 1