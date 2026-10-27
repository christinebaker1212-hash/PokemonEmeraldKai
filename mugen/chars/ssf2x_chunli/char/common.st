[StateDef 0];通常状態
Type = S
Physics = S
SprPriority = 0
VelSet = 0, 0

[State 0]
Type = Turn
Trigger1 = Time = 0 && Var(18) > 0 && P2Dist X < 0
[State 0]
Type = ChangeState
Trigger1 = Time = 0 && Var(18) = [1,9]
Value=1999+(Var(18)=13)-(Var(18)=1)*999-(Var(18)=2)*997-(Var(18)=3)*895-(Var(18)=4)*799-(Var(18)=5)*798-(Var(18)=6)*797-(Var(18)=7)*899-(Var(18)=8)*898-(Var(18)=9)*897
[State 0]
Type = ChangeState
Trigger1 = Time = 0 && Var(18) > 9
Value=1300+(Var(18)=11)+(Var(18)=12)*2+(Var(18)=13)*3

[State 0]
Type = PosSet
Trigger1 = Time = 0
Y = 0

[State 0]
Type = ChangeAnim
Trigger1 = (Anim != 0 && Anim != 5) || (Anim = 5 && AnimTime = 0)
Value = 0

[State 0]
Type = AssertSpecial
Trigger1 = RoundState > 2
Flag = NoWalk

[StateDef 10];立ち～しゃがみ
Type = C
Physics = N
Anim = 10
VelSet = 0

[State 10]
Type = ChangeState
Trigger1 = AnimTime = 0 || RoundState > 2
Value = 11 - (RoundState>2)*11

[StateDef 20];歩行
Type = S
Physics = S
SprPriority = 0

[State 20]
Type = VelSet
Trigger1 = Command = "holdfwd" || Command = "holdback"
X = IFelse(Command="holdfwd", 4, -3)*Const(Size.XScale)

[State 20]
Type = ChangeAnim
Trigger1 = (Vel X>0&&((Anim!=20&&Anim!=5)||(Anim=5&&AnimTime=0))) || (Vel X<0&&((Anim!=21&&Anim!= 5)|| (Anim=5&&AnimTime=0)))
Value = IFelse(Vel X > 0, 20, 21)

[StateDef 40];跳躍開始
Type = S
Physics = S
Anim = 40
Ctrl = 0
VelSet = 0, 0
SprPriority = 1

[State 40]
Type = Turn
Trigger1 = Time = 0 && P2Dist X < -5

[State 40]
Type = VarSet
Trigger1 = Time = 1
sysvar(1) = IFelse(Command="holdfwd", 1, IFelse(Command="holdback", -1, 0))

[State 40]
Type = VelSet
Trigger1 = AnimTime = 0
X = IFelse(sysvar(1)=0, 0, IFelse(sysvar(1)=1, 4, -4.2))*Const(Size.XScale)
Y = -10

[State 40]
Type = ChangeState
Trigger1 = AnimTime = 0 || RoundState > 2
Value = 50 - (RoundState>2)*50
Ctrl = 1

[StateDef 50];ジャンプ中
Type = A
Physics = A

[State 50]
Type = VarSet
Trigger1 = Time = 0
sysvar(1) = 0

[State 50]
Type = ChangeAnim
Trigger1 = Time = 0
Value = IFelse(Vel X=0||(PrevStateNo=[1000,1002]), 41, IFelse(Vel X>0, 42, 43))
Elem = IFelse((PrevStateNo=[1000,1002]), 4, 1)

[StateDef 51];壁反蹴
Type = A
Physics = N
Anim = 44
VelSet = 0, 0
Ctrl = 0

[State 51]
Type = Turn
Trigger1 = FrontEdgeDist < 3 && Time = 0

[State 51]
Type = PosAdd
Trigger1 = Anim = 44
X = IFelse(Facing=1, Pos X, -Pos X)

[State 51]
Type = VelSet
Trigger1 = Time < 6
Y = 0

[State 51]
Type = VelSet
Trigger1 = Time = 6
X = 4*Const(Size.XScale)
Y = -6
[State 51]
Type = VelAdd
Trigger1 = Time > 5
Y = .27

[State 51]
Type = CtrlSet
Trigger1 = Time = 6
Value = 1

[State 51]
Type = ChangeAnim
Trigger1 = AnimTime = 0
Value = 42

[State 51]
Type = ChangeState
Trigger1 = Pos Y >= 0
Value = 52

[StateDef 52];着地
Type = S
Physics = S
Ctrl = 0
Anim = 47
VelSet = 0, 0

[State 52]
Type = PosSet
Trigger1 = Time = 0
Y = 0

[State 52]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0, 10

[State 52]
Type = AssertSpecial
Trigger1 = 1
Flag = NoWalk

[State 52]
Type = CtrlSet
Trigger1 = Time = 1
Value = 1

[State 52]
Type = NotHitBy
Trigger1 = PrevStateNo = 5040 && Time < 2
Value = SCA

[State 52]
Type = ChangeState
Trigger1 = AnimTime = 0 || (P2Dist X<-5&&Time=0) || PrevStateNo = 470
Value = 0 + (P2Dist X<-5&&Time=0&&Command="holddown")*11
Ctrl = 1

[StateDef 180];勝利時
Type = S

[State 180]
Type = ChangeState
Trigger1 = Time = 0
Value = 181

[StateDef 181];勝利
Type = S
Ctrl = 0
VelSet = 0, 0
SprPriority = 1

[State 182]
Type = ChangeAnim
Trigger1 = Time = 0
Value = IFelse(Random<500, 180, 181)

[State 182]
Type = PlaySnd
Trigger1 = Anim = 180 && (AnimElem=3||AnimElem=21)
Value = 10, 2 + (AnimElem=21)

[State 182]
Type = AssertSpecial
Trigger1 = Anim = 180 && Time < 240
Flag = RoundNotOver

[StateDef 190];イントロ準備
Type = S
Ctrl = 0
VelSet = 0, 0

[State 190]
Type = ChangeState
Trigger1 = Time = 0
Value = 0

[StateDef 5000];地上ダメージ喰らい中
Type = S
MoveType = H
Physics = N
VelSet = 0, 0

[State 5000]
Type = ChangeState
Trigger1 = Var(1) > 400
Value = 4000

[State 5000]
Type = ChangeAnim
Trigger1 = Time = 0
Value = IFelse((GetHitVar(AnimType) = [3,5]), 5030, IFelse((GetHitVar(GroundType)=1), IFelse(GetHitVar(AnimType)=0,5000,5000+GetHitVar(AnimType)), IFelse(GetHitVar(AnimType)=0,5010,5010+GetHitVar(AnimType))))
[State 5000]
Type = ChangeAnim
Trigger1 = Time = 0 && (GetHitVar(AnimType) = [4,5]) && (SelfAnimExist(5047 + GetHitVar(AnimType)))
Value = 5047 + GetHitVar(AnimType)
[State 5000]
Type = ChangeAnim
Trigger1 = Time > 0
Value = Anim

[State 5000]
Type = StateTypeSet
Trigger1 = (Time = 0 && GetHitVar(YVel) != 0 && GetHitVar(Fall)) || Pos Y != 0
Type = A

[State 5000]
Type = ChangeState
Trigger1 = HitShakeOver
Value = IFelse(GetHitVar(YVel) = 0, 5001, 5030)

[State 5000]
Type = ForceFeedback
Trigger1 = Anim = 5000 || Anim = 5010
Persistent = 0
Time = 6
WaveForm = Square
[State 5000]
Type = ForceFeedback
Trigger1 = Anim = 5001 || Anim = 5011
Persistent = 0
Time = 8
WaveForm = SineSquare
Ampl = 110, -1, -.3
[State 5000]
Type = ForceFeedback
Trigger1 = Anim = 5012 || Anim = 5002 || Anim = 5030
Persistent = 0
Time = 15
WaveForm = SineSquare
Ampl = 140

[StateDef 5010];しゃがみ喰らい中
Type = C
MoveType = H
Physics = N
VelSet = 0, 0

[State 5010]
Type = ChangeState
Trigger1 = Var(1) > 400
Value = 4000

[State 5010]
Type = ChangeAnim
Trigger1 = Time = 0
Value = IFelse((GetHitVar(AnimType) = [3,5]), 5030, IFelse(GetHitVar(AnimType)=0, 5020, 5020+GetHitVar(AnimType)))
[State 5010]
Type = ChangeAnim
Trigger1 = Time = 0 && (GetHitVar(AnimType) = [4,5]) && (SelfAnimExist(5047 + GetHitVar(AnimType)))
Value = 5047 + GetHitVar(AnimType)
[State 5010]
Type = ChangeAnim
Trigger1 = Time > 0
Value = Anim

[State 5010]
Type = ChangeState
Trigger1 = HitShakeOver
Value = IFelse(GetHitVar(YVel) = 0, 5011, 5030)

[State 5010]
Type = ForceFeedback
Trigger1 = Anim = 5020
Persistent = 0
Time = 6
WaveForm = Square
[State 5010]
Type = ForceFeedback
Trigger1 = Anim = 5021
Persistent = 0
Time = 8
WaveForm = SineSquare
Ampl = 110, -1, -.3
[State 5010]
Type = ForceFeedback
Trigger1 = Anim = 5022
Persistent = 0
Time = 15
WaveForm = SineSquare
Ampl = 140

[StateDef 5020];空中喰らい中
Type = A
MoveType = H
Physics = N
VelSet = 0, 0

[State 5020]
Type = ChangeState
Trigger1 = Var(1) > 400
Value = 4000

[State 5020]
Type = ChangeAnim
Trigger1 = Time = 0
Value = IFelse((GetHitVar(AnimType) = [3,5]), 5030, IFelse(GetHitVar(GroundType)=1, 5000+GetHitVar(AnimType), 5010+GetHitVar(AnimType)))
[State 5020]
Type = ChangeAnim
Trigger1 = Time = 0 && (GetHitVar(AnimType) = [4,5]) && (SelfAnimExist(5047 + GetHitVar(AnimType)))
Value = 5047 + GetHitVar(AnimType)
[State 5020]
Type = ChangeAnim
Trigger1 = Time > 0
Value = Anim

[State 5020]
Type = PosSet
Trigger1 = Pos Y > 0
Y = 0

[State 5020]
Type = ChangeState
Trigger1 = HitShakeOver
Value = 5030

[State 5020]
Type = ForceFeedback
Trigger1 = Anim = 5000 || Anim = 5010
Persistent = 0
Time = 6
WaveForm = Square
[State 5020]
Type = ForceFeedback
Trigger1 = Anim = 5001 || Anim = 5011
Persistent = 0
Time = 8
WaveForm = SineSquare
Ampl = 110, -1, -.3

[State 5020]
Type = ForceFeedback
Trigger1 = Anim = 5012 || Anim = 5002 || Anim = 5030 || Anim = [5051, 5059]
Persistent = 0
Time = 15
WaveForm = SineSquare
Ampl = 140

[StateDef 5035];空中喰らい 上昇移行
Type = A
MoveType = H
Physics = N

[State 5035]
Type = ChangeAnim
Trigger1 = Time = 0 && SelfAnimExist(5035) && Anim != 5090 && Anim != [5051,5059]
Value = 5035

[State 5035]
Type = VelAdd
Trigger1 = 1
Y = GetHitVar(yaccel)

[State 5035]
Type = ChangeState
Trigger1 = HitOver || AnimTime = 0 || (Vel Y > 0 && Pos Y >= 10) || (Time = 0 && Anim != 5035)
Value = IFelse(HitFall, 5050, 5040)

[StateDef 5040];空中復帰
Type = A
MoveType = H
Physics = N

[State 5040]
Type = ChangeState
Trigger1 = !Alive
Value = 5050

[State 5040]
Type = ChangeAnim
Trigger1 = (PrevStateNo != 5042 || Anim != 5040) && ((AnimTime = 0 && Anim != 5040) || (Time = 0 && Anim != 5035))
Value = 5040

[State 5040]
Type = StateTypeSet
Trigger1 = HitOver
MoveType = I

[State 5040]
Type = VelAdd
Trigger1 = 1
Y = GetHitVar(YAccel)

[State 5040]
Type = VarSet
Trigger1 = Time = 0
FV = 1
Value = 0
[State 5040]
Type = VarAdd
Trigger1 = 1
FV = 1
Value = -.5

[State 5040]
Type = ChangeState
Trigger1 = Vel Y > 0 && Pos Y >= 0
Value = 52

[StateDef 5041];空中復帰からバウンド復帰
Type = A
MoveType = H
Physics = N
Anim = 5041
Ctrl = 1

[State 5041]
Type = Explod
Trigger1 = Time = 0
Anim = 93
Pos = 0, 0
PosType = P1
SprPriority = 10
OwnPal = 1
Scale = Const(Size.XScale), 1

[State 5041]
Type = PlaySnd
Trigger1 = Time = 0
Value = 2, 12

[State 5041]
Type = PosSet
Trigger1 = Time = 0
Y = 0
[State 5041]
Type = VelSet
Trigger1 = Time = 0
Y = -1+FVar(1)

[State 5041]
Type = VelAdd
Trigger1 = 1
Y = .4

[State 5041]
Type = ChangeState
Trigger1 = Vel Y > 0 && Pos Y >= 0
Value = 52

[StateDef 5042];降下中の画面端引っ掛かり
Type = A
MoveType = H
Physics = N

[State 5042]
Type = VarSet
Trigger1 = Time = 0
V = 21
Value = 1

[State 5042]
Type = PosFreeze
Trigger1 = 1

[State 5042]
Type = ModifyExplod
Trigger1 = Pos Y >= 0
ID = 2
PosType = P1
Pos = 0, Floor(RootDist Y - Pos Y)

[State 5042]
Type = ChangeAnim
Trigger1 = 1
Value = Anim
Elem = AnimElemNo(0)

[State 5042]
Type = NotHitBy
Trigger1 = PrevStateNo = 4000 || Pos Y > -20
Value = SCA

[State 5042]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0, 11

[State 5042]
Type = VelSet
Trigger1 = Time = 12
Y = 1

[State 5042]
Type = ChangeState
Trigger1 = Time = 12
Value = IFelse(PrevStateNo=5030, IFelse(HitFall, 5050, 5040), PrevStateNo)

[StateDef 5050];空中喰らい 降下中
Type = A
MoveType= H
Physics = N

[State 5050]
Type = ModifyExplod
Trigger1 = Pos Y >= 0
ID = 2
PosType = P1
Pos = 0, Floor(RootDist Y - Pos Y)

[State 5050]
Type = ChangeAnim
Trigger1 = ((AnimTime = 0 && Anim = 5035) || (PrevStateNo != 5042 && Time = 0 && Anim != 5035 && (Anim != [5051,5059]) && (Anim != [5061,5069]) && Anim != 5090))
Value = 5050

[State 5050]
Type = ChangeAnim
Trigger1 = Vel Y >= IFelse(Anim = 5050, 1, -2) && SelfAnimExist(Anim + 10) && Anim = [5050,5059]
Persistent = 0
Value = Anim + 10

[State 5050]
Type = VelAdd
Trigger1 = 1
Y = GetHitVar(YAccel)

[State 5050]
Type = ChangeState
Trigger1 = Vel Y > 0 && Pos Y >= 0
Value = IFelse(GetHitVar(YVel)=0, 5111, 5100)

[StateDef 5070];足払い転倒 喰らい中
Type = A
MoveType = H
Physics = N
VelSet = 0, 0

[State 5070]
Type = ChangeState
Trigger1 = Var(1) > 400
Value = 4000

[State 5070]
Type = ChangeAnim 
Trigger1 = 1
Value = 5070

[State 5070]
Type = ChangeState
Trigger1 = HitShakeOver
Value = 5071

[State 5070]
Type = ForceFeedback
Trigger1 = time = 0
Time = 12
XaveForm = SineSquare
Ampl = 140

[StateDef 5071];足払い転倒 降下中
Type = A
MoveType = H
Physics = N

[State 5071]
Type = ModifyExplod
Trigger1 = Pos Y >= 0
ID = 2
PosType = P1
Pos = 0, Floor(RootDist Y - Pos Y)

[State 5071]
Type = HitVelSet
Trigger1 = Time = 0 && PrevStateNo != 5042
X = 1
Y = 1

[State 5071]
Type = VelAdd
Trigger1 = 1
Y = GetHitVar(YAccel)

[State 5071]
Type = ChangeState
Trigger1 = Vel Y > 0 && Pos Y >= 0
Value = IFelse(GetHitVar(Fall.YVel)=0, 5111, 5100)

[StateDef 5100];喰らいバウンド
Type = L
MoveType = H
Physics = N

[State 5100]
Type = ModifyExplod
Trigger1 = Pos Y >= 0
ID = 2
PosType = P1
Pos = 0, 0

[State 5100]
Type = VelSet
Trigger1 = Time = 0 && GetHitVar(Fall.YVel) = 0 && Vel X > 1
X = 1

[State 5100]
Type = FallEnvShake
Trigger1 = Time = 0

[State 5100]
Type = VarSet
Trigger1 = Time = 0
sysvar(1) = Floor(Vel Y)

[State 5100]
Type = ChangeAnim
Trigger1 = Time = 0
Value = IFelse(PrevStateNo = 5071, 5071, 5100)

[State 5100]
Type = PosSet
Trigger1 = Time = 0
Y = 0

[State 5100]
Type = VelSet
Trigger1 = Time = 0
Y = 0

[State 5100]
Type = ChangeState
Trigger1 = (Time = 0 && GetHitVar(Fall.YVel) = 0) || AnimTime = 0
Value = IFelse(AnimTime = 0, 5101, 5111)

[State 5100]
Type = HitFallDamage
Trigger1 = Time = 3

[State 5100]
Type = PosFreeze
Trigger1 = 1

[State 5100]
Type = ForceFeedback
Trigger1 = Time = 0
WaveForm = SineSquare
Ampl = 128, -3, -.2, .005
Time = 20

[StateDef 5101];喰らいバウンド後
Type = L
MoveType = H
Physics = N

[State 5101]
Type = ChangeAnim
Trigger1 = Time = 0
Value = IFelse((Anim = [5101, 5109]) && SelfAnimExist(5160 + (Anim % 10)), 5160 + (Anim % 10), 5160)

[State 5101]
Type = HitFallVel
Trigger1 = Time = 0

[State 5101]
Type = PosSet
Trigger1 = Time = 0
Y = 20

[State 5101]
Type = ModifyExplod
Trigger1 = Pos Y >= 0
ID = 2
PosType = P1
Pos = 0, Floor(RootDist Y - Pos Y)

[State 5101]
Type = VelAdd
Trigger1 = 1
Y = .4

[State 5101]
Type = ChangeState
Trigger1 = Vel Y > 0 && Pos Y >= 12
Value = 5110

[StateDef 5110];ダウン中へ
Type = L
MoveType = H
Physics = N
VelSet = 0, 0

[State 5110]
Type = ChangeState
Trigger1 = 1
Value = 5111

[StateDef 5111];ダウン中へ2
Type = L
MoveType = H
Physics = N
VelSet = 0, 0
Anim = IFelse(PrevStateNo=5050||PrevStateNo=5071||PrevStateNo=5100, 5071, 5110)

[State 5111]
Type = ModifyExplod
Trigger1 = Time = 0
ID = 2
PosType = P1
Pos = 0, 0

[State 5111]
Type = FallEnvShake
Trigger1 = Time = 0

[State 5111]
Type = HitFallDamage
Trigger1 = Time = 0

[State 5111]
Type = PosSet
Trigger1 = 1
Y = 0

[State 5111]
Type = VarSet
Trigger1 = Time = 0 && GetHitVar(Fall.YVel) != 0
sysvar(1) = Floor(Vel Y)

[State 5111]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = IFelse(Alive, 5112, 5150)

[State 5111]
Type = VarSet
Trigger1 = sysvar(0) && Time = 0
sysvar(0) = 0

[State 5111]
Type = ForceFeedback
Trigger1 = Alive && Time = 0
Time = 8
Ampl = 240
WaveForm = Sine
[State 5110]
Type = ForceFeedback
Trigger1 = !Alive && Time = 0
Ampl = 200, 7, -.467
Time = 30
WaveForm = Sine

[StateDef 5112];ダウン中
Type = L
MoveType = H
Physics = N
VelSet = 0, 0

[State 5112]
Type = ChangeAnim
Trigger1 = Time= 0
Value = 5110

[State 5112]
Type = VarAdd
Trigger1 =  Var(16) > 6
V = 16
Value = -1

[State 5112]
Type = ChangeState
Trigger1 = (Time = 23 && GetHitVar(AnimType)!=3) || (Time = 35 && GetHitVar(AnimType)=3)
Value = IFelse(Time=23||Time=35,5120,5121)

[StateDef 5120];起き上がり
Type = L
MoveType = I
Physics = N
VelSet = 0, 0

[State 5120]
Type = PosSet
Trigger1 = 1
Y = 0

[State 5120]
Type = ChangeAnim
Trigger1 = Time = 0
Value = 5120
Elem = IFelse(PrevStateNo = 5121, 2, 1)

[State 5120]
Type = NotHitBy
Trigger1 = 1
Value = SCA

[State 5120]
Type = HitFallSet
Trigger1 = AnimTime = 0
Value = 1

[State 5120]
Type = NotHitBy
Trigger1 = AnimTime = 0
Value = , NT, ST, HT
Time = 12

[State 5120]
Type = NotHitBy
Trigger1 = AnimTime = 0
Value2 = SCA
Time = 3

[State 5120]
Type = VarRandom
Trigger1 = Var(1) > 400 && NumExplod(2) = 0
V = 0
Range = 0, 3

[State 5120]
Type = PlaySnd
Trigger1 = Var(1) > 400 && NumExplod(2) = 0
Value = 0, IFelse(Var(0) = 0, 14, IFelse(Var(0) = 1, 13, IFelse(Var(0) = 2, 15, 16)))
Channel = 6
Loop = 1

[State 5120]
Type = Explod
Trigger1 = Var(1) > 400 && NumExplod(2) < 3
Anim = 4000 + Var(0)
ID = 2
PosType = P1
Pos = 20, 0
SprPriority = 1
BindTime = -1
RemoveOnGetHit = 1
RemoveTime = -1
[State 5120]
Type = Explod
Trigger1 = Var(1) > 400 && NumExplod(2) < 3
Anim = 4004 + Var(0)
ID = 2
PosType = P1
Pos = 20, 0
SprPriority = 1
BindTime = -1
RemoveOnGetHit = 1
RemoveTime = -1
[State 5120]
Type = Explod
Trigger1 = Var(1) > 400 && NumExplod(2) < 3
Anim = 4008 + Var(0)
ID = 2
PosType = P1
Pos = 20, 0
SprPriority = 1
BindTime = -1
RemoveOnGetHit = 1
RemoveTime = -1

[State 5120]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = IFelse(Var(1) > 400, 4001, 0)
Ctrl = IFelse(Var(1) > 400, 0, 1)

[StateDef 5900];Varの初期化設定
Type = S

[State 5900]
Type = VarRangeSet
Trigger1 = RoundNo = 1
Value = 0
First = 0
Last = 3
[State 5900]
Type = VarSet
Trigger1 = RoundNo = 1
V = 6
Value = 0
[State 5900]
Type = VarSet
Trigger1 = RoundNo = 1
V = 8
Value = 0
[State 5900]
Type = VarRangeSet
Trigger1 = RoundNo = 1
Value = 0
First = 14
Last = 29
[State 5900]
Type = VarRangeSet
Trigger1 = RoundNo = 1
Value = 0
First = 31
Last = 59

[State 5900]
Type = VarRangeSet
Trigger1 = RoundNo = 1
fvalue = 0

[State 5900]
Type = ChangeState
Trigger1 = RoundNo = 1
Value = 190