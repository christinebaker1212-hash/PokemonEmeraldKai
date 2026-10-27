[StateDef -2]
[State -2];仕様の初期設定。
Type = VarSet
Trigger1 = RoundState = 0 && Var(30) != 2
V = 10
;Value = 0;←ここの値が0でノーマルモード。1でＸスーパーモード。
Value = IFelse((PalNo=[9,10]),1,0);←こう書くとPalNoが9,10の時にＸスーパーモード。

[State -2];気絶可否。
Type = VarSet
Trigger1 = RoundState = 0 && Var(30) != 2
V = 5
Value = 0;←ここの値が0で気絶あり。1で気絶なし。

[State -2];空中ガードなし
Type = AssertSpecial
Trigger1 = 1
Flag = NoAirGuard

[State -2];喰らい系ステートではコントロール不可
Type = CtrlSet
Trigger1 = StateNo = [5000,5999]
Value = 0

[State -2];相手が喰らい回復時は追撃不可
Type = StateTypeSet
Trigger1 = MoveType = A && (StateNo!=[1300,1399]) && EnemyNear,StateNo = [5020,5099]
IgnoreHitPause = 1
MoveType = I
[State -2]
Type = StateTypeSet
Trigger1 = MoveType = I && (StateNo = [200,2000]) && EnemyNear,StateNo != [5020,5099]
MoveType = A

[State -2];画面端で転倒すると何かに引っ掛かる
Type = ChangeState
Trigger1 = !Var(21) && Time > 0 && BackEdgeDist < 2 && ((StateNo = [5030, 5050]) || StateNo = 5071 || StateNo = 4000)
Value = 5042
[State -2]
Type = VarSet
Trigger1 = Var(21) && MoveType != H && StateNo != [5040,5049]
V = 21
Value = 0

[State -2];ダウン時の音とエフェクト
Type = PlaySnd
Trigger1 = Time = 1 && (StateNo = 5100 || (StateNo = 5111 && (PrevStateNo=5050||PrevStateNo=5071||PrevStateNo=5100)))
Value = 0, 11
[State -2]
Type = Explod
Trigger1 = Time = 1 && (StateNo = 5100 || StateNo = 5111)
Anim = 93
Pos = 0, 0
PosType = P1
SprPriority = -10
OwnPal = 1
Scale = Const(Size.XScale), 1

[State -2];立ち強攻撃喰らいエフェクト
Type = Explod
Trigger1 = GetHitVar(AnimType) = 2 && (GetHitVar(GroundType)=[1,2]) && StateNo = 5000 && Time = 1 && NumExplod(1) = 0 && Random < 750
IgnoreHitPause = 0
Anim = IFelse(GetHitVar(GroundType)=1, 71, 73)+(Random<500) + (Var(12)=-1)*10
ID = 1
Pos = 0, 0
PosType = P1
BindTime = -1
SprPriority = 1
RemoveOnGetHit = 1
Scale = Const(Size.XScale), 1
[State -2];攻撃喰らい演出エフェクト消去
Type = RemoveExplod
Trigger1 = NumExplod(1) = 1 && (MoveType!=H&&StateNo!=[5040,5041])
IgnoreHitPause = 0
ID = 1
[State -2];攻撃喰らい時の埃
Type = Explod
Trigger1 = (StateNo = 151 || StateNo = 153 || StateNo = 5001 || StateNo = 5011) && Time = 1
Anim = 70 + (Var(12)=-1)*10
Pos = 0, 0
PosType = P1
SprPriority = 10
OwnPal = 1
BindTime = -1
Scale = Const(Size.XScale), 1
[State -2]
Type = Explod
Trigger1 = StateNo = 5100
Anim = 75 + (Var(12)=-1)*10
Pos = Floor((-40 + Random/12.5)*Const(Size.XScale)), 0 - Random/50
PosType = P1
Velocity = 0, -3
SprPriority = 10
OwnPal = 1
Scale = Const(Size.XScale), 1

[State -2];気絶音消去
Type = StopSnd
Trigger1 = RoundState = 3 || ((StateNo=5000||StateNo=5010||StateNo=5020||StateNo=5070||StateNo!=[5000,5120]) && StateNo != [4000,4009])
Channel = 6

[State -2];気絶処理
Type = VarAdd
Trigger1 = !Var(5) && !Var(8) && GetHitVar(Damage) > 0 && (StateNo != [150,199])
V = 1
Value = GetHitVar(Damage)
[State -2]
Type = NotHitBy
Trigger1 = Var(1) > 400 && StateNo = [5100,5120]
Value = SCA
[State -2]
Type = RemoveExplod
Trigger1 = NumExplod(2) != 0 && StateNo < 5030 && StateNo != [4000,4009]
ID = 2
[State -2]
Type = VarSet
Trigger1 = (!Var(8) && StateNo = 4001) || (Var(8) && MoveType != H)
V = 8
Value = IFelse(Var(8), 0, 1)

[State -2];キャンセル処理
Type = VarSet
Trigger1 = Var(6) = -1 && MoveContact && (((StateNo=220||StateNo=260)&&Var(10))||StateNo=221||StateNo=231||StateNo=241||(StateNo=[300,310])||(StateNo=[330,340])||StateNo=[200,211])
V = 6
Value = 3
[State -2]
Type = VarAdd
Trigger1 = Var(6) > 0 || (Var(6)=0&&StateNo!=[200,399])
V = 6
Value = -1
[State -2];スーパーキャンセル処理
Type = VarSet
Trigger1 = Var(19) = -1 && MoveContact && !Var(10) && StateNo = 230
V = 19
Value = 3
[State -2]
Type = VarAdd
Trigger1 = Var(19) > 0 || (Var(19)=0&&StateNo!=[200,499]) && !Var(10)
V = 19
Value = -1

[State -2];残像消去
Type = AfterImageTime
Trigger1 = PrevStateNo = 2000 || MoveType = H
Time = 0

[State -2];試合開始時パワー0
Type = PowerSet
Trigger1 = RoundState = 0 && Power > 0
Value = 0

[State -2];技表
Type = Helper
Trigger1 = NumHelper(3000) = 0
Name = "Pause"
ID = 3000
Pos = 0, -600
PosType = Back
StateNo = 3000
KeyCtrl = 1
OwnPal = 1

[State -2];振り向き対応
Type = AssertSpecial
Trigger1 = P2Name = "madgear" || P4Name = "madgear" || P2Name = "BONUS AREA" || P4Name = "BONUS AREA"
Flag = NoAutoTurn
[State -2]
Type = Turn
Trigger1=Command="holdback"&&RoundState=2&&Ctrl&&StateType=S&&(P2Name="madgear"||P4Name="madgear"||P2Name="BONUS AREA"||P4Name="BONUS AREA")

;ここから下はいんちきプログラム。
[State -2];いんちきプログラム開始用ヘルパー出現
Type = Helper
Trigger1 = NumHelper(30) = 0 && Var(30) < 2
ID = 30
StateNo = 990
KeyCtrl = 1

[State -2];たまに気功拳。
Type = ChangeState
Trigger1 = StateType != A && (Ctrl||(Time<2&&StateNo=[200,399])||Var(6)>0) && !NumProjID(1) && Random < 10 && Var(30) = 1 && RoundState = 2
Value = IFelse(Random<333, 1200, IFelse(Random<666, 1201, 1202))

[State -2];たまに百裂脚。
Type = ChangeState
Trigger1 = StateType != A && (Ctrl||(Time<2&&StateNo=[200,399])||Var(6)>0) && Random < 5 && Var(30) = 1 && RoundState = 2
Value = IFelse(Random<333, 1000, IFelse(Random<666, 1002, 1004))

[State -2];たまに回転的鶴脚蹴。
Type = ChangeState
Trigger1 = StateType != A && (Ctrl||(Time<2&&StateNo=[200,399])||Var(6)>0) && Random < 5 && Var(30) = 1 && RoundState = 2
Value = IFelse(Random<333, 1100, IFelse(Random<666, 1101, 1102))
[State -2];TURBO時はたまに空中回転的鶴脚蹴。
Type = ChangeState
Trigger1 = StateType=A&&Vel X>0&&Pos Y<-25 && (Ctrl||(Time<2&&StateNo=[200,399])||Var(6)>0) && P2Dist X < 60 && ((P2StateType=A&&P2Dist Y=[10,80])||(P2StateType=S&&P2MoveType=A)) && Random < 20 && Var(10) && Var(30) = 1 && RoundState = 2
Value = IFelse(Random<333, 1104, IFelse(Random<666, 1105, 1106))

[State -2];敵がしゃがみ攻撃を出してる時はジャンプし易く。
Type = ChangeState
Trigger1 = StateType = S && Ctrl && P2Dist X > 40 && EnemyNear,StateType = C && Random < 20 && Var(30) = 1 && RoundState = 2
Value = 40
[State -2];前後ジャンプを状況に合わせて
Type = VarSet
Trigger1 = Time = 1 && StateNo = 40 && Var(30) = 1 && RoundState = 2
sysvar(1) = IFelse(P2Dist X>120||Random<333||EnemyNear,NumProjID(0)!=0, 1, IFelse(P2Dist X<80||Random<666, -1, 0))

[State -2];敵が近くで飛び道具を撃ったらジャンプ。
Type = ChangeState
Trigger1 = StateType != A && Ctrl && P2Dist X < 70 && EnemyNear,NumProjID(0) != 0 && Random < 100 && Var(30) = 1 && RoundState = 2
Value = 40

[State -2];たまに投げ。
Type = ChangeState
Trigger1 = P2Dist X < 20&&(StateNo=20||(Pos Y<-59&&Vel Y!=0&&StateNo=[50,51])) && Random < 20 && Var(30) = 1 && RoundState = 2
Value = IFelse(StateType=S, IFelse(Random<500,504,500), IFelse(Random<500,514,510))

[State -2];通常技キャンセル必殺技。
Type = ChangeState
Trigger1 = Var(6) > 0 && StateType = S && Random < 100 && Var(30) = 1
Value = IFelse(Random<333, 1002, IFelse(Random<666&&Var(10), 1201, 1102))

[State -2];たまに特殊攻撃。
Type = ChangeState
Trigger1=((StateType=S&&P2Dist X<(85*Const(Size.XScale)))||StateType=A)&& Ctrl&&Random<20&&Var(30)=1&&RoundState=2
Value = IFelse(StateType=A, 460, IFelse(Random<500, 260, 270))

[State -2];立ちパンチ系攻撃
Type = ChangeState
Trigger1 = P2Dist X < 90 && Random < 50 && StateType = S && Ctrl && Var(30) = 1 && RoundState = 2
Value = IFelse(Random<333, 200+(P2BodyDist X<(28*Const(Size.XScale))), IFelse(Random<666, 210+(P2BodyDist X<(43*Const(Size.XScale))), 220+(P2BodyDist X<(55*Const(Size.XScale)))))
[State -2];立ちキック系攻撃
Type = ChangeState
Trigger1 = P2Dist X < 90 && Random < 70 && StateType = S && Ctrl && Var(30) = 1 && RoundState = 2
Value = IFelse(Random<333, 230+(P2BodyDist X<(43*Const(Size.XScale))), IFelse(Random<666, 240+(P2BodyDist X<(48*Const(Size.XScale))), 250+(P2BodyDist X<(48*Const(Size.XScale)))))
[State -2];しゃがみ攻撃
Type = ChangeState
Trigger1 = P2Dist X < 60 && Random < 100 && StateType = C && Ctrl && Var(30) = 1 && RoundState = 2
Value = IFelse(Random<100, 300, IFelse(Random<200, 310, IFelse(Random<300, 320, IFelse(Random<550, 330, IFelse(Random<650, 340, 350)))))
[State -2];空中パンチ
Type = ChangeState
Trigger1 = P2Dist X < 100 && (P2Dist Y=[-10,20]) && Random < 50 && StateType = A && Ctrl && Var(30) = 1
Value = IFelse(Vel X=0,IFelse(Random<60, 400, IFelse(Random<120, 410, 420)), IFelse(Random<60, 401, IFelse(Random<120, 411, 421)))
[State -2];空中キック
Type = ChangeState
Trigger1 = P2Dist X < 100 && (P2Dist Y=[-20,20]) && Random < 50 && StateType = A && Ctrl && Var(30) = 1 && RoundState = 2
Value = IFelse(Vel X=0, IFelse(Random<60, 430, IFelse(Random<120, 440, 450)), IFelse(Random<60, 431, IFelse(Random<120, 441, 451)))
[State -2];弱攻撃連打処理
Type = ChangeState
Type = ChangeState
Trigger1 = Random<100 && (StateNo=[200,201]) && AnimElem = 3,>1 && Var(30)=1 && RoundState=2
Value = IFelse(P2Dist X>(84*Const(Size.XScale)),200,201)

[State -2];壁反蹴
Type = ChangeState
Trigger1 = ((BackEdgeDist<3&&Vel X<0)||(FrontEdgeDist<3&&Vel X>0)) && Pos Y < -59 && ((Stateno=[50,51])||StateNo=[400,499]) && Random < 100 && Var(30)=1 && RoundState=2
Value = 51

[StateDef 990];いんちきぷろぐらむ発動ヘルパー
Type = S
Physics = S
Ctrl = 0
Anim = 99
[State 990]
Type = LifeSet
Trigger1 = Time = 0
Value = 1
[State 990]
Type = ParentVarSet
Trigger1 = Time = 300
V = 30
Value = 1
[State 990]
Type = ChangeState
TrjggerAll = RoundState = 2
Trigger1 = (Command="holdup"&&Parent,Command="holdup")||(Command="holddown"&&Parent,Command="holddown")||(Command="holdfwd"&&Parent,Command="holdfwd")||(Command="holdback"&&Parent,Command="holdback")
Trigger2 = (Command="a"&&Parent,Command="a")||(Command="b"&&Parent,Command="b")||(Command="c"&&Parent,Command="c")||(Command="x"&&Parent,Command="x")||(Command="y"&&Parent,Command="y")||(Command="z"&&Parent,Command="z")
Trigger3 = Life > 1
Value = 991
[StateDef 991]
Type = S
Physics = S
[State 991]
Type = ParentVarSet
Trigger1 = Time = 0
V = 30
Value = 2
[State 991]
Type = DestroySelf
Trigger1 = Time = 0