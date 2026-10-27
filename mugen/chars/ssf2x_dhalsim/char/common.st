[StateDef 0];通常状態
Type = S
Physics = S
SprPriority = 0

[State 0]
Type = Turn
Trigger1 = Time = 0 && Var(18) > 0 && P2Dist X < 0
[State 0]
Type = ChangeState
Trigger1 = Time = 0 && Var(18) = [1,11]
Value=1999+(Var(18)=1)-(Var(18)=2)*799-(Var(18)=3)*798-(Var(18)=4)*797-(Var(18)=5)*796-(Var(18)=6)*899-(Var(18)=7)*895-(Var(18)=8)*894-(Var(18)=9)*699-(Var(18)=10)*695-(Var(18)=11)*694
[State 0]
Type = ChangeState
Trigger1 = Time = 0 && Var(18) > 11
Value=1000+(Var(18)=13)+(Var(18)=14)*2

[State 0]
Type = ChangeAnim
Trigger1 = (Anim != 0 && Anim != 5) || (Anim = 5 && AnimTime = 0)
Value = 0

[State 0]
Type = AssertSpecial
Trigger1 = RoundState > 2
Flag = NoWalk

[State 0]
Type = VelSet
Trigger1 = Time = 0
X = 0
Y = 0

[StateDef 10];立ち～しゃがみ
Type = C
Physics = N
Anim = 10

[State 10]
Type = ChangeState
Trigger1 = AnimTime = 0 || RoundState > 2
Value = 11 - (RoundState>2)*11

[StateDef 11];しゃがみ
Type = C
Physics = C
Anim = 11
SprPriority = 0

[State 11]
Type = ChangeAnim
Trigger1 = Anim = 6 && AnimTime = 0
Value = 11

[State 11]
Type = ChangeState
Trigger1 = RoundState > 2
Value = 0

[StateDef 12];しゃがみ～立ち
Type = S
Physics = S
Anim = 12

[State 12]
Type = AssertSpecial
Trigger1 = 1
Flag = NoWalk

[State 12]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0

[StateDef 20];歩行
Type = S
Physics = S
SprPriority = 0

[State 20]
Type = VelSet
Trigger1 = Command = "holdfwd" || Command = "holdback"
X = IFelse(Command = "holdfwd", Const(Velocity.Walk.Fwd.X), Const(Velocity.Walk.Back.X))*Const(Size.XScale)

[State 20]
Type = ChangeAnim
Trigger1 = (Vel X > 0 && ((Anim != 20 && Anim != 5) || (Anim = 5 && AnimTime = 0))) || (Vel X < 0 && ((Anim != 21 && Anim != 5) ||  (Anim = 5 && AnimTime = 0)))
Value = IFelse(Vel X > 0, 20, 21)

[StateDef 40];跳躍開始
Type = S
Physics = S
Anim = 40
Ctrl = 0
VelSet = 0, 0
SprPriority = 1

[State 40]
Type = VarSet
Trigger1 = Time = 1
sysvar(1) = IFelse(Command="holdfwd", 1, IFelse(Command="holdback", -1, 0))

[State 40]
Type = VelSet
Trigger1 = AnimTime = 0
X = IFelse(sysvar(1)=0, 0, IFelse(sysvar(1)=1, Const(Velocity.Jump.Fwd.X), Const(Velocity.Jump.Back.X)))*Const(Size.XScale)
Y = Const(Velocity.Jump.Y) - IFelse(Vel X=0, 0, .1)

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
Value = IFelse(Vel X=0, 41, IFelse(Vel X>0, 42, 43))

[StateDef 51];空中防御後
Type = A
Physics = A

[State 51]
Type = Null
Trigger1 = 1

[StateDef 52];着地
Type = S
Physics = S
Ctrl = 0
Anim = IFelse(PrevStateNo=1052&&NumExplod(7)>0,1016,47)
VelSet = 0, 0

[State 52]
Type = PosSet
Trigger1 = Time = 0
Y = 0

[State 52]
Type = PlaySnd
Trigger1 = Time = 0
Value = 0+Var(10)*2, 10-Var(10)*3

[State 52]
Type = AssertSpecial
Trigger1 = 1
Flag = NoWalk

[State 52]
Type = CtrlSet
Trigger1 = Time = 1 && (PrevStateNo!=[460,499]) && PrevStateNo != [2500,2502]
Value = 1

[State 52]
Type = NotHitBy
Trigger1 = PrevStateNo = 5040 && Time < 2
Value = SCA

[State 52]
Type = ChangeState
Trigger1 = AnimTime = 0 || (P2Dist X<-5&&Time=0)
Value = 0 + (P2Dist X<-5&&Time=0&&Command="holddown")*11
Ctrl = 1

[StateDef 100];ダッシュ
Type = S
Physics = N
SprPriority = 1
Anim = 181
VelSet = IFelse(Command="holdfwd", 3.6, -3.6)*Const(Size.XScale)
Ctrl = 0

[State 100]
Type = ChangeAnim
Trigger1 = Time = 2 || Time = 4 || Time = 6 || Time = 8
Value = 181
Elem = IFelse(Time=2, 2, IFelse(Time=4, 3, IFelse(Time=6, 4, 5)))

[State 100]
Type = PlaySnd
Trigger1 = AnimElem = 2
Value = 2, 13

[State 100]
Type = ChangeState
Trigger1 = (Command = "x" || Command = "y" || Command = "z") && Time > 11 && Var(30) != 1
Value = IFelse(Command="holdback", IFelse(Command = "x", 201, IFelse(Command = "y", 211, 221)), IFelse(Command = "x", 200, IFelse(Command = "y", 210, 220)))
[State 100]
Type = ChangeState
Trigger1 = (Command = "a" || Command = "b" || Command = "c") && Time > 11 && Var(30) != 1
Value = IFelse(Command = "a", IFelse(Command="holdfwd", 231, 230), IFelse(Command="holdback", IFelse(Command = "b", 241, 251), IFelse(Command = "b", 240, 250)))

[State 100]
Type = ChangeState
Trigger1 = (Time > 31 && (Command != "holdfwd" && Command != "holdback" )) || Time = 40
Value = 101

[StateDef 101];ダッシュ終了
Type = S
Physics = N
SprPriority = 1
Anim = 181
VelSet = 0

[State 101]
Type = ChangeAnim
Trigger1 = Time = 0 || Time = 4 || Time = 8 || Time = 12
Value = 181
Elem = IFelse(Time=0, 4, IFelse(Time=4, 3, IFelse(Time=8, 2, 1)))

[State 101]
Type = ChangeState
Trigger1 = Time = 16
Value = 0
Ctrl = 1

[StateDef 120];防御開始
Type = U
Physics = U

[State 120]
Type = ChangeAnim
Trigger1 = Time = 0 && StateType != A
Value = 120 + (StateType = C)

[State 120]
Type = StateTypeSet
Trigger1 = Time = 0 && StateType = S
Physics = S
[State 120]
Type = StateTypeSet
Trigger1 = Time = 0 && StateType = C
Physics = C
[State 120]
Type = StateTypeSet
Trigger1 = Time = 0 && StateType = A
Physics = A

[State 120]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 130 + (StateType = C) + (StateType = A)*2

[StateDef 180];勝利時
Type = S

[State 180]
Type = ChangeState
Trigger1 = Time = 0
Value = 181 + (Var(10)&&Random<500)

[StateDef 181];勝利
Type = S
Ctrl = 0
VelSet = 0, 0
SprPriority = 1

[State 181]
Type = NotHitBy
Trigger1 = 1
Value = SCA

[State 181]
Type = ChangeAnim
Trigger1 = Time = 0
Value = 180+MatchOver

[State 181]
Type = AssertSpecial
Trigger1 = Time < 200
Flag = RoundNotOver

[StateDef 182];勝利 ダークストーカー
Type = S
Ctrl = 0
VelSet = 0, 0
SprPriority = 1

[State 182]
Type = NotHitBy
Trigger1 = 1
Value = SCA
[State 182]
Type = PlayerPush
Trigger1 = 1
Value = 0
[State 182]
Type = ScreenBound
Trigger1 = !Var(15) && AnimElem = 5,1
Value = 0

[State 182]
Type = Helper
Trigger1 = Time = 0
Pos = 0, 0
PosType = P1
StateNo = 183
HelperType = Player

[State 182]
Type = VarRandom
Trigger1 = Time = 0
V = 15
Range = 0, 1

[State 182]
Type = ChangeAnim
Trigger1 = ((Time = 0 || AnimElem=5,1)&&!Var(15)) || (Time=20&&Var(15))
Value = 181 + Var(15)*1019
Elem = 1 + (AnimElem=5,1)*4

[State 182]
Type = VarSet
Trigger1 = Time = 0
FV = 0
Value = 30
[State 182]
Type = VarAdd
Trigger1 = 1
FV = 0
Value = .05

[State 182]
Type = VelSet
Trigger1 = AnimElem = 5 && !Var(15)
X = ((Time-32)/10) * Cos(2 * FVar(0))
Y = ((Time-32)/10) * -Sin(2 * FVar(0))

[State 182]
Type = AssertSpecial
Trigger1 = Time > 43 && Var(15)
Flag = Invisible

[State 182]
Type = AssertSpecial
Trigger1 = Time < 200 && !Var(15)
Flag = RoundNotOver

[StateDef 183];勝利時 カメラ位置固定ヘルパー
Type = S
MoveType = I
Physics = N
Anim = 5111

[State 183]
Type = NotHitBy
Trigger1 = 1
Value = SCA

[StateDef 190];イントロ準備
Type = S
Ctrl = 0
VelSet = 0, 0

[State 190]
Type = ChangeState
Trigger1 = Time = 0
Value = IFelse(Var(10), 191,0)

[StateDef 191];イントロ ダークストーカー時
Type = S
Ctrl = 0

[State 191]
Type = AssertSpecial
Trigger1 = 1
Flag = Intro

[State 191]
Type = NotHitBy
Trigger1 = 1
Value = SCA

[State 191]
Type = PlaySnd
Trigger1 = Time = 41
Value = 1, 0

[State 191]
Type = ChangeAnim
Trigger1 = Time < 41
Value = 1200
Elem = 18

[State 191]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 0

[StateDef 5000];地上ダメージ喰らい中
Type = S
MoveType = H
Physics = N
VelSet = 0, 0

[State 5000]
Type = ChangeState
Trigger1 = Var(1) > 400 && !GetHitVar(Fall)
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
Trigger1 = Var(1) > 400 && !GetHitVar(Fall)
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
Trigger1 = Var(1) > 400 && !GetHitVar(Fall)
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

[StateDef 5030];空中喰らい 上昇
Type = A
MoveType = H
Physics = N
Ctrl = 0

[State 5030]
Type = VelAdd
Trigger1 = 1
Y = GetHitVar(YAccel)

[State 5030]
Type = HitVelSet
Trigger1 = Time = 0
X = 1
Y = 1

[State 5030]
Type = ChangeState
trigger1 = HitOver || (Vel Y > 0 && Pos Y >= 0)
Value = IFelse(HitFall, 5050, 5040)
[State 5030]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = 5035

[StateDef 5035];空中喰らい 上昇移行
Type = A
MoveType = H
Physics = N

[State 5035]
Type = ChangeAnim
Trigger1 = Time = 0 && SelfAnimExist(5035) && Anim != 5090 && Anim != [5051,5059]
Value = 5035 + Var(10)

[State 5035]
Type = VelAdd
Trigger1 = 1
Y = GetHitVar(yaccel)

[State 5035]
Type = ChangeState
Trigger1 = HitOver || AnimTime = 0 || (Vel Y > 0 && Pos Y >= 10) || (Time = 0 && Anim != [5035,5036])
Value = IFelse(HitFall, 5050, 5040)

[StateDef 5040];空中復帰
Type = A
MoveType = H
Physics = N

[State 5040]
Type = CtrlSet
Trigger1 = 1
Value = 0

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
Value = IFelse(PrevStateNo = 5030, IFelse(HitFall, 5050, 5040), PrevStateNo)

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
Trigger1 = (AnimTime = 0 && Anim = 5035) || (Time = 0 && Anim != 5035 && (Anim != [5051,5059]) && (Anim != [5061,5069]) && Anim != 5090)
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
Trigger1 = Vel Y > 0 && Pos Y >= IFelse((Anim=[5051,5059])||(Anim=[5061,5069])||Anim=5077, 0, 25)
Value = IFelse(GetHitVar(YVel)=0, 5111, 5100)

[StateDef 5070];足払い転倒 喰らい中
Type = A
MoveType = H
Physics = N
VelSet = 0, 0

[State 5070]
Type = ChangeState
Trigger1 = Var(1) > 400 && !GetHitVar(Fall)
Value = 4000

[State 5070]
Type = ChangeAnim 
Trigger1 = Time = 0
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
Trigger1 = Vel Y > 0 && Pos Y >= 15-Var(10)*15
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
Type = ChangeState
Trigger1 = Time = 18
Value = 5120

[StateDef 5120];起き上がり
Type = L
MoveType = I
Physics = N
Ctrl = 0
Anim = 5120
VelSet = 0, 0

[State 5120]
Type = PosSet
Trigger1 = 1
Y = 0

[State 5120]
Type = StateTypeSet
Trigger1 = Time = 0 && Var(10)
Value = S

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
Pos = 0, 0
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
Pos = 0, 0
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
Pos = 0, 0
SprPriority = 1
BindTime = -1
RemoveOnGetHit = 1
RemoveTime = -1

[State 5120]
Type = ChangeState
Trigger1 = AnimTime = 0
Value = IFelse(Var(1) > 400, 4001, 0)
Ctrl = IFelse(Var(1) > 400, 0, 1)

[StateDef 5150];死亡
Type = L
MoveType = H
Physics = N
SprPriority = -3

[State 5150]
Type = Helper
Trigger1 = (Command="a"||Command="b"||Command="c"||Command="x"||Command="y"||Command="z") && RoundState  = 2 && Var(10) && (ScreenPos X = [0,320]) && ScreenPos Y = [0,240]
Persistent = 0
Name = "Dhalsim_Ghost"
Pos = 0, 0
PosType = P1
StateNo = 5151
KeyCtrl = 1
OwnPal = 1

[State 5150]
Type = ChangeAnim
Trigger1 = (Time = 0 && SelfAnimExist(5140)) && ((Anim != [5111, 5119]) && (Anim != [5171, 5179]) || !SelfAnimExist(5140 + (Anim % 10)))
Value = 5140

[State 5150]
Type = ChangeAnim
Trigger1 = Time = 0 && (Anim = [5111, 5119]) || (Anim = [5171, 5179]) && SelfAnimExist(5140 + (Anim % 10))
Value = 5140 + (Anim % 10)

[State 5150]
Type = ChangeAnim
Trigger1 = MatchOver && SelfAnimExist(Anim + 10) && Anim = [5140, 5149]
Persistent = 0
Value = Anim + 10

[State 5150]
Type = ChangeAnim
Trigger1 = Time = 0 && (Anim != [5140, 5159]) && Anim != [5110, 5119]
Value = 5110

[State 5150]
Type = VelMul
Trigger1 = 1
X = .85

[State 5150]
Type = VelSet
Trigger1 = Vel X < .05
Persistent = 0
X = 0

[State 5150]
Type = NotHitBy
Trigger1 = 1
Value = SCA

[StateDef 5151];幽霊ダルシム 起き上がり～浮き上がり
Type = S
Physics = N
SprPriority = -9
Anim = 5120
Ctrl = 0

[State 5151]
Type = PalFx
Trigger1 = 1
Time = 300
Color = 0
Add = 80,80,130

[State 5151]
Type = NotHitBy
Trigger1 = 1
Value = SCA
[State 5151]
Type = PlayerPush
Trigger1 = 1
Value = 0
[State 5151]
Type = AssertSpecial
Trigger1 = TimeMod = 3,0
Flag = Invisible
[State 5151]
Type = AssertSpecial
Trigger1 = 1
Flag = NoShadow

[State 5151]
Type = ChangeAnim
Trigger1 = AnimTime = 0
Value = 1200

[State 5151]
Type = ChangeState
Trigger1 = Anim = 1200 && AnimElem = 4
Value = 5152

[StateDef 5152];幽霊ダルシム
Type = A
Physics = N

[State 5152]
Type = PalFx
Trigger1 = 1
Time = 300
Color = 0
Add = 80,80,130

[State 5152]
Type = NotHitBy
Trigger1 = 1
Value = SCA
[State 5152]
Type = PlayerPush
Trigger1 = 1
Value = 0
[State 5152]
Type = AssertSpecial
Trigger1 = TimeMod = 3,0
Flag = Invisible
[State 5152]
Type = AssertSpecial
Trigger1 = 1
Flag = NoShadow

[State 5152]
Type = Turn
Trigger1 = Vel X < -1

[State 5152]
Type = ChangeAnim
Trigger1 = (Parent,Command!="holda"&&AnimElem=4,<2) || AnimElem = 30
Value = 1200
Elem = 4
[State 5152]
Type = ChangeAnim
Trigger1 = Parent,Command = "holda" && AnimElem = 17,1
Value = 1200
Elem = 17

[State 5152]
Type = PlaySnd
Trigger1 = Parent,Command="x"||Parent,Command="y"||Parent,Command="z"||Parent,Command="b"||Parent,Command="c"
Value = IFelse(Parent,Command="b",11,IFelse(Parent,Command="c",1,10)),IFelse(Parent,Command="c",1,IFelse(Parent,Command="y",4,IFelse(Parent,Command="z",3,0)))

[State 5152]
Type = VelAdd
Trigger1 = (FrontEdgeDist>0&&Command="holdfwd") || (BackEdgeDist>0&&Command="holdback")
X = IFelse(Command="holdfwd", .2, -.2)
[State 5152]
Type = VelAdd
Trigger1 = FrontEdgeDist <= 0 || BackEdgeDist <= 0
X = IFelse(FrontEdgeDist<=0, -.4, .4)
[State 5152]
Type = VelAdd
Trigger1 = Vel X != [-4,4]
X = IFelse(Vel X>0, -.2, .2)
[State 5152]
Type = VelAdd
Trigger1 = (ScreenPos Y>70&&Command="holdup") || (Pos Y<0&&Command="holddown")
Y = IFelse(Command="holdup", -.2, .2)
[State 5152]
Type = VelAdd
Trigger1 = ScreenPos Y <= 50 || Pos Y >= 0
Y = IFelse(ScreenPos Y<=70, .4, -.4)
[State 5152]
Type = VelAdd
Trigger1 = Vel Y != [-4,4]
Y = IFelse(Vel Y>0, -.2, .2)

[State 5152]
Type = VelAdd
Trigger1 = Command != "holdfwd" && Command != "holdback" && Command != "holdup" && Command != "holddown"
X = (Random/500) * Cos(2 * Time/3)
Y = (Random/500) * -Sin(2 * Time/3)

[State 5152]
Type = ChangeState
Trigger1 = RoundState = 3
Value = 5153 + Win

[StateDef 5153];幽霊ダルシム 負け
Type = A
Physics = N
Anim = 5051

[State 5153]
Type = PalFx
Trigger1 = 1
Time = 300
Color = 0
Add = 80,80,130

[State 5153]
Type = NotHitBy
Trigger1 = 1
Value = SCA
[State 5153]
Type = PlayerPush
Trigger1 = 1
Value = 0
[State 5153]
Type = AssertSpecial
Trigger1 = 1
Persistent = 2
Flag = Invisible
[State 5153]
Type = AssertSpecial
Trigger1 = 1
Flag = NoShadow

[State 5153]
Type = VelAdd
Trigger1 = Time = 0
Y = 3 - Vel Y/2
[State 5153]
Type = VelAdd
Trigger1 = 1
X = - Vel X / 10
Y = -.15

[StateDef 5154];幽霊ダルシム 勝ち
Type = A
Physics = N

[State 5154]
Type = PalFx
Trigger1 = 1
Time = 300
Color = 0
Add = 80,80,130

[State 5154]
Type = NotHitBy
Trigger1 = 1
Value = SCA
[State 5154]
Type = PlayerPush
Trigger1 = 1
Value = 0
[State 5154]
Type = AssertSpecial
Trigger1 = TimeMod = 3,0
Flag = Invisible
[State 5154]
Type = AssertSpecial
Trigger1 = 1
Flag = NoShadow

[State 5154]
Type = ChangeAnim
Trigger1 = (Time = 0 || AnimElem = 32,1) && PrevStateNo != 5154
Value = 1200
Elem = 32 - (Time=0)*2
[State 5154]
Type = ChangeAnim
Trigger1 = (Time = 0 || (Anim=47&&AnimTime=0)) && PrevStateNo = 5154
Value = 0 + (Time=0)*47
[State 5154]
Type = ChangeAnim
Trigger1 = Anim = 0 && RoundState = 4 && PrevStateNo = 5154
Persistent = 0
Value = 180 + (Random<500)

[State 5154]
Type = VelSet
Trigger1 = PrevStateNo != 5154 && Time = 0
X = 0
[State 5154]
Type = VelAdd
Trigger1 = PrevStateNo != 5154
Y = .33
[State 5154]
Type = VelSet
Trigger1 = PrevStateNo = 5154 && Time = 0
Y = 0

[State 5154]
Type = ChangeState
Trigger1 = Vel Y > 0 && Pos Y >= 0
Value = 5154

[StateDef 5900];Varの初期化設定
Type = S

[State 5900]
Type = VarRangeSet
Trigger1 = RoundNo = 1
Value = 0
First = 0
Last = 4
[State 5900]
Type = VarRangeSet
Trigger1 = RoundNo = 1
Value = 0
First = 6
Last = 9
[State 5900]
Type = VarRangeSet
Trigger1 = RoundNo = 1
Value = 0
First = 11
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