[StateDef -2]
[State -2];仕様の初期設定。
Type = VarSet
Trigger1 = RoundState = 0 && Var(30) != 2
V = 10
;Value = 0;←ここが0だとSSF2Xモード。1だとVAMPIREモード。
Value = IFelse((PalNo=[7,8]),1,0);←こう書くとENTER押しながらAかBで選択した時のみVAMPIREモード。

[State -2];大柄な敵の名前設定。近くにいる時。
Type = VarSet
TriggerAll = RoundState = 2 && !Var(17)
Trigger1 = EnemyNear,Name = "Anakaris By Third"
Trigger2 = EnemyNear,Name = "raiden"
Trigger3 = EnemyNear,Name = "Zangief"
Trigger4 = EnemyNear,Name = "Cain Marko by TAMAGO"
Trigger5 = EnemyNear,Name = "Tin's bigbear"
Trigger6 = EnemyNear,Name = "SF2'ザンギエフ"
V = 17
Value = 1
[State -2];大柄な敵の名前設定。近くにいない時。
Type = VarSet
Trigger1 = RoundState = 2 && Var(17)
Trigger1 = EnemyNear,Name != "Anakaris By Third"
Trigger1 = EnemyNear,Name != "raiden"
Trigger1 = EnemyNear,Name != "Zangief"
Trigger1 = EnemyNear,Name != "Cain Marko by TAMAGO"
Trigger1 = EnemyNear,Name != "Tin's bigbear"
Trigger1 = EnemyNear,Name != "SF2'ザンギエフ"
V = 17
Value = 0

[State -2];格闘家の時は空中ガードなし
Type = AssertSpecial
Trigger1 = !Var(10)
Flag = NoAirGuard

[State -2];喰らい系ステートではコントロール不可
Type = CtrlSet
Trigger1 = !Var(10) && StateNo = [5000, 5999]
Value = 0

[State -2];相手が喰らい回復時は追撃不可
Type = StateTypeSet
Trigger1 = (StateNo != [1010, 1029]) && StateNo!=411 && StateNo != 2430 && StateNo != 2490 && (StateNo != [2495, 2499]) && MoveType = A && EnemyNear,StateNo = [5020, 5099]
IgnoreHitPause = 1
MoveType = I
[State -2]
Type = StateTypeSet
Trigger1 = MoveType = I && StateNo != 290 && (StateNo != [1023, 1039]) && (StateNo = [200, 2000]) && EnemyNear,StateNo != [5020, 5099]
MoveType = A

[State -2];気絶音消去
Type = StopSnd
Trigger1 = StateNo < 5030 && StateNo != 5070 && StateNo != [4000, 4009]
Channel = 6

[State -2];画面端で転倒すると何かに引っ掛かる
Type = ChangeState
Trigger1 = !Var(21) && Time > 0 && BackEdgeDist < 3 && ((StateNo = [5030, 5050]) || StateNo = 5071 || StateNo = 4000) && !Var(10)
Value = 5042
[State -2]
Type = VarSet
Trigger1 = Var(21) && MoveType != H && StateNo != [5040,5049]
V = 21
Value = 0

[State -2];ダウン時の音
Type = PlaySnd
Trigger1 = Time = 1 && (StateNo = 5100 || (StateNo = 5111 && (PrevStateNo=5050||PrevStateNo=5071||PrevStateNo=5100)))
Value = 0+Var(10)*2, 11+Var(10)*5

[State -2];立ち強攻撃喰らいエフェクト
Type = Explod
Trigger1 = (Anim = 5002 || Anim = 5012) && StateNo = 5000 && NumExplod(1) = 0 && Random > 500 && !Var(10)
IgnoreHitPause = 0
Anim = IFelse(Anim = 5002, IFelse(Random<500,71,72), IFelse(Random<500,73,74))
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
Trigger1 = (StateNo = 151 || StateNo = 153 || StateNo = 5001 || StateNo = 5011) && Time = 1 && !Var(10)
Anim = 70
Pos = 0, 0
PosType = P1
SprPriority = 10
OwnPal = 1
BindTime = -1
Scale = Const(Size.XScale), 1
[State -2]
Type = Explod
Trigger1 = StateNo = 5100 && !Var(10); && Time = 1
Anim = 75
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
Trigger1 = !Var(5) && !Var(10) && !Var(8) && GetHitVar(Damage) > 0 && (StateNo != [150, 199])
V = 1
Value = GetHitVar(Damage)
[State -2]
Type = NotHitBy
Trigger1 = Var(1) > 400 && StateNo = [5100,5120]
Value = SCA
[State -2]
Type = RemoveExplod
Trigger1 = NumExplod(2) != 0 && StateNo < 5030 && StateNo != [4000, 4009]
ID = 2
[State -2]
Type = VarSet
Trigger1 = (!Var(8) && StateNo = 4001) || (Var(8) && MoveType != H)
V = 8
Value = IFelse(Var(8), 0, 1)

[State -2];キャンセル処理 格闘家時
Type = VarSet
TriggerAll = Var(6) = -1 && MoveContact && !Var(10)
Trigger1 = (StateNo=201||StateNo=211) && AnimElem = 3,<0
Trigger2 = StateNo = 221 && AnimElem = 4,<0 && PalNo = [9,10]
Trigger3 = StateNo = 230 || StateNo = 241 || StateNo = 301 || StateNo = 311 || StateNo = 341 || StateNo = [330,331]
Trigger4 = StateNo = 240 && PalNo = [9,10]
Trigger5 = StateNo = 351 && PalNo != [9,10]
V = 6
Value = 3
[State -2]
Type = VarAdd
Trigger1 = Var(6) > 0 || (Var(6)=0&&StateNo!=[200,499]) && !Var(10)
V = 6
Value = -1
[State -2];キャンセル処理 ダークストーカー時
Type = VarSet
TriggerAll = Var(6) = 0 && MoveContact && Var(10)
Trigger1 = StateNo=201||StateNo=211||StateNo=221||StateNo=230||StateNo=241||StateNo=301||StateNo=311||StateNo=341||StateNo=[330,331]
V = 6
Value = 8
[State -2]
Type = VarAdd
Trigger1 = Var(6) > 0 && Var(10)
V = 6
Value = -1
[State -2]
Type = VarSet
Trigger1 = Var(6) = -1 && Var(10)
V = 6
Value = 0
[State -2];スーパーキャンセル処理
Type = VarSet
TriggerAll = Var(19) = -1 && MoveContact && !Var(10)
Trigger1 = (StateNo=201||StateNo=211) && AnimElem = 3,<0
Trigger2 = StateNo = 221 && AnimElem = 4,<0
Trigger3 = StateNo = 230 || (StateNo = [240,241]) || StateNo = 301 || StateNo = 311 || StateNo = 341 || StateNo = 351 || StateNo = [330,331]
V = 19
Value = 3
[State -2]
Type = VarAdd
Trigger1 = Var(19) > 0 || (Var(19)=0&&StateNo!=[200,499]) && !Var(10)
V = 19
Value = -1

[State -2];ダークストーカー時のパワー減少
Type = PowerAdd
Trigger1 = Var(10) && Var(13)
Value = -12
[State -2];格闘家時の試合開始時パワー0
Type = PowerSet
Trigger1 = !Var(10) && RoundState = 0 && Power > 0
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

[State -2];スーパーコンボフィニッシュ演出用KOスロー止め
Type = AssertSpecial
Trigger1 = StateNo = 2000 && ProjContact2 = 1;Helper(3000),StateNo = 3003
Flag = NoKOSlow

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