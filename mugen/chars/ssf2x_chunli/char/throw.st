[StateDef 500];虎襲倒
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
Ground.Velocity = -5*Const(Size.XScale), 0
Fall = 1
Fall.Recover = 0
Fall.YVelocity = -4
YAccel = .25
ID = 1
P1StateNo = 501
P2StateNo = 502
P1Facing = IFelse(Command="holdback", -1, 1)
P2Facing = 1

[State 500]
Type = ChangeState
Trigger1 = Time = 1
Value = 220+(P2BodyDist X<(55*Const(Size.XScale)))

[StateDef 501];虎襲倒中
Type = S
MoveType = A
Physics = S
Ctrl = 0
VelSet = 0
Anim = 500

[State 501]
Type = Width
Trigger1 = AnimElem = 3
Value = Floor(60*Const(Size.XScale)), 0

[State 501]
Type = TargetBind
Trigger1 = Time = 0 || AnimElem = 2 || AnimElem = 3 || AnimElem = 4
ID = 1
Pos = IFelse(Time=0,4,IFelse(AnimElem=2,0,20,IFelse(AnimElem=3,0,85,41)))*Const(Size.XScale), IFelse(Time=0,-92,IFelse(AnimElem=2,0,-80,IFelse(AnimElem=3,0,-47,17)))

[State 501]
Type = PlaySnd
Trigger1 = Time = 6
Value = 10, 1

[State 501]
Type = VarSet
Trigger1 = (Time<10&&(Target(1),Command="holdfwd"||Target(1),Command="holdback")&&(Target(1),Command="z"||Target(1),Command="y")) || Time = 0
V = 23
Value = IFelse(Time=0, 0, 1)
[State 501]
Type = TargetState
Trigger1 = AnimElem = 4,1 && Var(23)=1
ID = 1
Value = 506

[State 501]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0
Ctrl = 1

[StateDef 502];虎襲倒られる相手
Type = S
MoveType = H
Physics = S
VelSet = 0, 0
SprPriority = -1

[State 502]
Type = VarSet
Trigger1 = Time = 0
sysvar(1) = 1

[State 502]
Type = ChangeAnim2
Trigger1 = Time = 0
Value = 501

[State 502]
Type = ScreenBound
Trigger1 = AnimElem = 4,<0
Value = 0

[State 502]
Type = ChangeState
Trigger1 = AnimElem = 4,2
Value = 503

[StateDef 503];虎襲倒られる相手開放
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
Y = .2

[State 503]
Type = LifeAdd
Trigger1 = ((Vel Y>0&&Pos Y>=20) || (Life<=Floor(224/(Const(Data.Defence)/100)))) && PrevStateNo != 505
Persistent = 0
Value = IFelse((Life<=Floor(224/(Const(Data.Defence)/100))), -1000, -224)

[State 503]
Type = ChangeState
TriggerAll = BackEdgeDist < 2 && Time > 0 && PrevStateNo != 505
Trigger1 = Name="SSF2Xリュウ"||Name="SF2'リュウ"||Name="SF2'ケン"||Name="SF2'サガット"||Name="SF2'ガイル"||Name="SF2'春麗"||Name="SF2'ザンギエフ"||(Name="SSF2Xダルシム"&&!Var(10))
Trigger2 = Name="SF2'ベガ"||Name="SF2'ブランカ"
Value = 505

[State 503]
Type = SelfState
Trigger1 = Vel Y > 0 &&  Pos Y >= 20
Value = 5100

[StateDef 504];虎襲倒 中パンチ時
Type = S
MoveType = A
Physics = N
Ctrl = 0
VelSet = 0

[State 504]
Type = HitDef
Trigger1 = Time = 0
Attr = S, NT
AnimType = Light
HitFlag = M-
Priority = 0, Miss
Damage = 0
GetPower = 0
NumHits = 0
SparkNo = -1
HitSound = -1
AnimType = Light
Ground.Velocity = -5*Const(Size.XScale), 0
Fall = 1
Fall.Recover = 0
Fall.YVelocity = -4
YAccel = .25
ID = 1
P1StateNo = 501
P2StateNo = 502
P1Facing = IFelse(Command = "holdback", -1, 1)
P2Facing = 1

[State 504]
Type = ChangeState
Trigger1 = Time = 1
Value = 210+(P2BodyDist X<(43*Const(Size.XScale)))

[StateDef 505];相手降下中の画面端引っ掛かり
Type = A
MoveType = H
Physics = N

[State 505]
Type = VelSet
Trigger1 = Time = 0
Y = 0

[State 505]
Type = PosFreeze
Trigger1 = 1

[State 505]
Type = NotHitBy
Trigger1 = 1
Value = SCA

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
Facing = -1
RemoveOnGetHit = 1
Scale = Const(Size.XScale), 1

[State 505]
Type = LifeAdd
Trigger1 = Time = 0 && PrevStateNo != 506
Value = -224 -(PrevStateNo=513)*74 -(PrevStateNo=523)*14

[State 505]
Type = ChangeState
Trigger1 = (Time=4&&Var(10)&&Name="SSF2Xリュウ") || Time = 12
Value = PrevStateNo

[StateDef 506];投げ抜ける相手
Type = A
MoveType = H
Physics = N
VelSet = 0

[State 506]
Type = PosAdd
Trigger1 = Time = 0 && PrevStateNo != 505
Y = -16

[State 506]
Type = VelSet
Trigger1 = Time = 0 && (Name!="SSF2Xリュウ"||(Name="SSF2Xリュウ"&&!Var(10)))
Y = 0

[State 506]
Type = VelSet
Trigger1 = Time = 0 && PrevStateNo != 505
X = -2
Y = -6
[State 506]
Type = VelAdd
Trigger1 = 1
Y = GetHitVar(YAccel)

[State 506]
Type = NotHitBy
Trigger1 = 1
Value = SCA

[State 506]
Type = LifeAdd
Trigger1 = PrevStateNo != 505 && Time = 0
Value = -119

[State 506]
Type = ChangeAnim
Trigger1 = ((Time=0||Time=13)&&PrevStateNo!=505) || (Time=0&&PrevStateNo=505&&Anim!=5040)
Value = IFelse(Time=0&&PrevStateNo!=505, 5000 + (SelfAnimExist(5035))*35, 5040)

[State 506]
Type = ChangeState
TriggerAll = BackEdgeDist < 2 && Time > 0 && PrevStateNo != 505
Trigger1 = Name="SSF2Xリュウ"||Name="SF2'リュウ"||Name="SF2'ケン"||Name="SF2'サガット"||Name="SF2'ガイル"||Name="SF2'春麗"||Name="SF2'ザンギエフ"||(Name="SSF2Xダルシム"&&!Var(10))
Trigger2 = Name="SF2'ベガ"||Name="SF2'ブランカ"
Value = 505

[State 506]
Type = SelfState
Trigger1 = (Time=0&&!Alive) || (Vel Y>0&&Pos Y>=0)
Value = 52 + (Time=0)*4948

[StateDef 510];流星落
Type = A
MoveType = A
Physics = A
Ctrl = 0

[State 510]
Type = VarSet
Trigger1 = Time = 0
sysvar(0) = (PrevStateNo=460||PrevStateNo=51)

[State 510]
Type = HitDef
Trigger1 = Time = 0 && EnemyNear,StateNo != [5020, 5099]
Attr = A, NT
HitFlag = A-
Priority = 0, Miss
Damage = 0
GetPower = 0
GivePower = 0
NumHits = 0
SparkNo = -1
HitSound = -1
AnimType = Light
Air.Velocity = (-3 - (Vel Y>0)*2)*Const(Size.XScale), 6 - (Vel Y>0)*6
Fall = 1
Fall.Recover = 0
Fall.YVelocity = -4
YAccel = .25
ID = 1
P1StateNo = 515 - (Vel Y>0)*4
P2StateNo = 516 - (Vel Y>0)*4
P1Facing = IFelse(Command="holdback", -1, 1)
P2Facing = 1

[State 510]
Type = ChangeState
Trigger1 = Time = 1
Value = IFelse(Vel X=0, 420, 421)

[StateDef 511];下降流星落中
Type = A
MoveType = A
Physics = N
Ctrl = 0

[State 511]
Type = PosAdd
Trigger1 = Time = 0 && PrevStateNo != 511
Y = -8

[State 511]
Type = PosFreeze
Trigger1 = Time < 13 && PrevStateNo != 511

[State 511]
Type = ChangeAnim
Trigger1 = Time = 0 || (PrevStateNo=511&&AnimElem=4,21)
Value = 510 - (PrevStateNo=511)*10
Elem = 1 + (PrevStateNo=511)*3 + (AnimElem=4,21)

[State 511]
Type = Width
Trigger1 = Time = 0 && PrevStateNo != 511
Value = 20, 0

[State 511]
Type = VelSet
Trigger1 = Time = 0 && PrevStateNo = 511
X = 0
Y = 0
[State 511]
Type = VelAdd
Trigger1 = Time > 12 && PrevStateNo != 511
Y = .44 - sysvar(0)*.17

[State 511]
Type = StateTypeSet
Trigger1 = Time = 0 && PrevStateNo = 511
Value = S
[State 511]
Type = PosSet
Trigger1 = Time = 0 && PrevStateNo = 511
Y = 0

[State 511]
Type = TargetBind
Trigger1 = PrevStateNo != 511
ID = 1
Pos = IFelse(AnimElemNo(0)=1,4,IFelse(AnimElemNo(0)=2,20,IFelse(AnimElemNo(0)=3,85,41)))*Const(Size.XScale), IFelse(AnimElemNo(0)=1,-92,IFelse(AnimElemNo(0)=2,-80,IFelse(AnimElemNo(0)=3,-47,17)))

[State 511]
Type = PlaySnd
Trigger1 = Time = 6 && PrevStateNo != 511
Value = 10, 1

[State 511]
Type = TargetState
Trigger1 = PrevStateNo = 511 && (Time=0||Time=2)
ID = 1
Value = 513 -(Time=0)

[State 511]
Type = VarSet
Trigger1 = ((Time<10&&PrevStateNo!=511)&&(Target(1),Command="holdfwd"||Target(1),Command="holdback") && (Target(1),Command="z"||Target(1),Command="y")) || (Time=0&&PrevStateNo!=511)
V = 23
Value = IFelse(Time=0, 0, 1)
[State 511]
Type = TargetBind
Trigger1 = PrevStateNo = 511 && Var(23)=1 && Time = 0
ID = 1
Pos = 41*Const(Size.XScale), 17
[State 511]
Type = TargetState
Trigger1 = PrevStateNo = 511 && Var(23)=1 && Time = 2
ID = 1
Value = 506

[State 511]
Type = ChangeState
Trigger1 = (Pos Y >= 0 && PrevStateNo != 511) || AnimTime = 0
Value = IFelse(PrevStateNo = 511, 0, 511)
Ctrl = IFelse(PrevStateNo = 511, 1, 0)

[StateDef 512];下降流星落られる相手
Type = S
MoveType = H
Physics = S
VelSet = 0, 0
SprPriority = -1

[State 512]
Type = ScreenBound
Trigger1 = 1
Value = 0

[State 512]
Type = VarSet
Trigger1 = Time = 0 && PrevStateNo != 522
sysvar(1) = 0

[State 512]
Type = ChangeAnim2
Trigger1 = Time = 0
Value = 511
Elem = 1 + (PrevStateNo=512)*3

[State 512]
Type = PosSet
Trigger1 = Time = 0 && PrevStateNo=512
Y = 17

[StateDef 513];下降流星落られる相手解放
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
Y = .25

[State 513]
Type = LifeAdd
Trigger1 = ((Vel Y>0&&Pos Y>=20) || (Life<=Floor(294/(Const(Data.Defence)/100)))) && PrevStateNo != 505
Persistent = 0
Value = IFelse((Life<=Floor(294/(Const(Data.Defence)/100))), -1000, -294)

[State 513]
Type = ChangeState
TriggerAll = BackEdgeDist < 2 && Time > 0 && PrevStateNo != 505
Trigger1 = Name="SSF2Xリュウ"||Name="SF2'リュウ"||Name="SF2'ケン"||Name="SF2'サガット"||Name="SF2'ガイル"||Name="SF2'春麗"||Name="SF2'ザンギエフ"||(Name="SSF2Xダルシム"&&!Var(10))
Trigger2 = Name="SF2'ベガ"||Name="SF2'ブランカ"
Value = 505

[State 513]
Type = SelfState
Trigger1 = Vel Y > 0 &&  Pos Y >= 20
Value = 5100

[StateDef 514];流星落 中パンチ
Type = A
MoveType = A
Physics = A
Ctrl = 0

[State 514]
Type = VarSet
Trigger1 = Time = 0
sysvar(0) = (PrevStateNo=460||PrevStateNo=51)

[State 514]
Type = HitDef
Trigger1 = Time = 0 && EnemyNear,StateNo != [5020, 5099]
Attr = A, NT
HitFlag = A-
Priority = 0, Miss
Damage = 0
GetPower = 0
GivePower = 0
NumHits = 0
SparkNo = -1
HitSound = -1
AnimType = Light
Air.Velocity = (-3 - (Vel Y>0)*2)*Const(Size.XScale), 6 - (Vel Y>0)*6
Fall = 1
Fall.Recover = 0
Fall.YVelocity = -4
YAccel = .25
ID = 1
P1StateNo = 515 - (Vel Y>0)*4
P2StateNo = 516 - (Vel Y>0)*4
P1Facing = IFelse(Command="holdback", -1, 1)
P2Facing = 1

[State 514]
Type = ChangeState
Trigger1 = Time = 1
Value = IFelse(Vel X=0, 410, 411)

[StateDef 515];流星落中
Type = A
MoveType = A
Physics = N
Ctrl = 0
Anim = 510

[State 515]
Type = Width
Trigger1 = AnimElemNo(0) = 3
Value = Floor(60*Const(Size.XScale)), 0


[State 515]
Type = PosFreeze
Trigger1 = Time < 13

[State 515]
Type = VelAdd
Trigger1 = Time > 12
Y = .44 - sysvar(0)*.17

[State 515]
Type = TargetBind
Trigger1 = AnimElem = 4,<1
ID = 1
Pos = IFelse(AnimElemNo(0)=1,4,IFelse(AnimElemNo(0)=2,20,IFelse(AnimElemNo(0)=3,85,41)))*Const(Size.XScale), IFelse(AnimElemNo(0)=1,-92,IFelse(AnimElemNo(0)=2,-80,IFelse(AnimElemNo(0)=3,-47,17)))

[State 515]
Type = PlaySnd
Trigger1 = Time = 6
Value = 10, 1

[State 515]
Type = VarSet
Trigger1 = (Time < 10 && (Target(1),Command="holdfwd"||Target(1),Command="holdback") && (Target(1),Command="z"||Target(1),Command="y")) || Time = 0
V = 23
Value = IFelse(Time=0, 0, 1)
[State 515]
Type = TargetState
Trigger1 = AnimElem = 4,1 && Var(23)
ID = 1
Value = 506

[State 515]
Type = ChangeState
Trigger1 = Pos Y >= 0
Value = 52

[StateDef 516];流星落られる相手
Type = S
MoveType = H
Physics = N
VelSet = 0, 0
SprPriority = -1

[State 516]
Type = ScreenBound
Trigger1 = 1
Value = 0

[State 516]
Type = VarSet
Trigger1 = Time = 0
sysvar(1) = 0

[State 516]
Type = ChangeAnim2
Trigger1 = Time  = 0
Value = 511

[State 516]
Type = VelSet
Trigger1 = AnimElem = 4
X = 0
Y = 0

[State 516]
Type = ChangeState
Trigger1 = AnimElem = 4,3
Value = 517

[StateDef 517];流星落られる相手解放へ
Type = A
MoveType = H
Physics = N

[State 517]
Type = HitVelSet
Trigger1 = Time = 0 && PrevStateNo != 505
X = 1
Y = 1

[State 517]
Type = VelAdd
Trigger1 = 1
Y = .25

[State 517]
Type = LifeAdd
Trigger1 = Life <= Floor((294+sysvar(1)*42)/(Const(Data.Defence)/100)) && PrevStateNo != 505
Value = -1000

[State 517]
Type = ChangeState
TriggerAll = BackEdgeDist < 2 && Time > 0 && PrevStateNo != 505
Trigger1 = Name="SSF2Xリュウ"||Name="SF2'リュウ"||Name="SF2'ケン"||Name="SF2'サガット"||Name="SF2'ガイル"||Name="SF2'春麗"||Name="SF2'ザンギエフ"||(Name="SSF2Xダルシム"&&!Var(10))
Trigger2 = Name="SF2'ベガ"||Name="SF2'ブランカ"
Value = 505

[State 517]
Type = VarSet
Trigger1 =  Time = 0
sysvar(0) = (PrevStateNo=505)

[State 517]
Type = ChangeState
Trigger1 = Vel Y > 0 && Pos Y >= 20
Value = 518

[StateDef 518];流星落られる相手体力減らして解放
Type = S
MoveType = H
Physics = N

[State 518]
Type = EnvShake
Trigger1 = Time = 0
Freq = 135
Ampl = -3
Time = 60
Phase = 0

[State 518]
Type = VelSet
Trigger1 = Time = 0
Y = 0

[State 518]
Type = LifeAdd
Trigger1 = Time = 0 && !sysvar(0)
Value = -238

[State 518]
Type = SelfState
Trigger1 = Time = 1
Value = 5100

[StateDef 4000];気絶直前 倒れ
Type = A
MoveType = H
Physics = N
SprPriority = -1

[State 4000]
Type = ChangeAnim
Trigger1 = (Time = 0 && PrevStateNo != 5042) || (Anim=5030&&HitShakeOver)
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
Pos = 20, 0
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
Pos = 20, 0
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
Pos = 20, 0
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
Trigger1 = Vel Y > 0 &&  Pos Y >= 0
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
Value = 4 -(Var(0)=3)*2

[State 4001]
Type = ChangeAnim
Trigger1 = 1
Value = 5300
Elem = IFelse((Var(2)=[0,23])||(Var(2)=[112,135]), 1, IFelse((Var(2)=[24,39])||(Var(2)=[96,111])||(Var(2)=[136,151]), 2, IFelse((Var(2)=[58,77]),4, 3)))

[State 4001]
Type = ChangeState
Trigger1 = Var(2) >= (90 +(Var(0)=1||Var(0)=3)*60 -(Var(0)=2)*30)
Value = 0
Ctrl = 1