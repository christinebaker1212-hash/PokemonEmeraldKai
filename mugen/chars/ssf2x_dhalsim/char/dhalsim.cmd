[Command]
Name = "waza1_light"
Command = ~D, F, x
[Command]
Name = "waza1_middle"
Command = ~D, F, y
[Command]
Name = "waza1_hard"
Command = ~D, F, z

[Command]
Name = "waza2_light"
Command = ~B, D, F, x
[Command]
Name = "waza2_middle"
Command = ~B, D, F, y
[Command]
Name = "waza2_hard"
Command = ~B, D, F, z

[Command]
Name = "waza3_fp"
Command = F, ~D, DF, x+y+z
Time = 20
[Command]
Name = "waza3_bp"
Command = B, ~D, DB, x+y+z
Time = 20
[Command]
Name = "waza3_fk"
Command = F, ~D, DF, a+b+c
Time = 20
[Command]
Name = "waza3_bk"
Command = B, ~D, DB, a+b+c
Time = 20

[Command]
Name = "waza4_light"
Command = ~B, D, F, a
[Command]
Name = "waza4_middle"
Command = ~B, D, F, b
[Command]
Name = "waza4_hard"
Command = ~B, D, F, c

[Command]
Name = "waza5_light"
Command = ~D, B, a
[Command]
Name = "waza5_middle"
Command = ~D, B, b
[Command]
Name = "waza5_hard"
Command = ~D, B, c

[Command]
Name = "super"
Command = ~B, D, F, B, D, F, x
Time = 30
[Command]
Name = "super"
Command = ~B, D, F, B, D, F, y
Time = 30
[Command]
Name = "super"
Command = ~B, D, F, B, D, F, z
Time = 30

[Command]
Name = "super2"
Command = D, F, DF, x+y+z
Time = 20

[Command]
Name = "FF"
Command = F, F
[Command]
Name = "BB"
Command = B, B

[Command]
Name = "a"
Command = a
Time = 1
[Command]
Name = "b"
Command = b
Time = 1
[Command]
Name = "c"
Command = c
Time = 1
[Command]
Name = "x"
Command = x
Time = 1
[Command]
Name = "y"
Command = y
Time = 1
[Command]
Name = "z"
Command = z
Time = 1

[Command]
Name = "start"
Command = s
Time = 1

[Command]
Name = "fwd"
Command = F
Time = 1
[Command]
Name = "back"
Command = B
Time = 1
[Command]
Name = "up" 
Command = U
Time = 1
[Command]
Name = "down"
Command = D
Time = 1

[Command]
Name = "holda"
Command = /$a
Time = 1
[Command]
Name = "holdb"
Command = /$b
Time = 1
[Command]
Name = "holdc"
Command = /$c
Time = 1
[Command]
Name = "holdx"
Command = /$x
Time = 1
[Command]
Name = "holdy"
Command = /$y
Time = 1
[Command]
Name = "holdz"
Command = /$z
Time = 1

[Command]
Name = "holdfwd"
Command = /$F
Time = 1
[Command]
Name = "holdback"
Command = /$B
Time = 1
[Command]
Name = "holdup" 
Command = /$U
Time = 1
[Command]
Name = "holddown"
Command = /$D
Time = 1

[Command]
Name = "recovery"
Command = a+b+c+x+y+z+s
Time = 0

[StateDef -1]
[State -1];REVERSAL ATTACK準備
Type = VarSet
Trigger1 = Var(18) > 0 && StateNo != 0 && StateNo != [5000,5999]
V = 18
Value = 0
[State -1]
Type = VarSet
TriggerAll = Var(1) < 401 && (((StateNo=5001||StateNo=5011)&&Time>(GetHitVar(HitTime))-3)||(StateNo=5120&&AnimTime>-3)) && NumProjID(0) = 0 && NumProjID(1) = 0 && NumProjID(2) = 0 && NumProjID(3) = 0 && !Var(10)
Trigger1 = Command = "waza1_light" || Command = "waza1_middle" || Command = "waza1_hard"
Trigger3 = Command = "waza2_middle" || Command = "waza2_hard"
Trigger2 = Command = "waza4_light" || Command = "waza4_middle" || Command = "waza4_hard"
V = 18
Value = IFelse(Command="waza2_middle",7,IFelse(Command="waza2_hard",8,IFelse(Command="waza4_light",9,IFelse(Command="waza4_middle",10,IFelse(Command="waza4_hard",11,IFelse(Command="waza1_light",12,IFelse(Command="waza1_middle",13,14)))))))
[State -1]
Type = VarSet
TriggerAll = Var(1) < 401 && (((StateNo=5001||StateNo=5011)&&Time>(GetHitVar(HitTime))-3)||(StateNo=5120&&AnimTime>-3)) && !Var(10)
Trigger1 = Command="super" && NumProjID(0) = 0 && NumProjID(1) = 0 && NumProjID(2) = 0 && NumProjID(3) = 0 && Power=1000
Trigger2 = Command = "waza3_fp" || Command = "waza3_bp" || Command = "waza3_fk" || Command = "waza3_bk" || Command = "waza2_light"
V = 18
Value = IFelse(Time=0,0,IFelse(Command="super",1,IFelse(Command="waza3_fp",2,IFelse(Command="waza3_bp",3,IFelse(Command="waza3_fk",4,IFelse(Command="waza3_bk",5,6))))))

[State -1];ヨガインフェルノ or ヨガインドプレジャー
Type = ChangeState
Trigger1 = ((!Var(10)&&Command="super" && NumProjID(0)=0&&NumProjID(1)=0&&NumProjID(2)=0&&NumProjID(3)=0&&Power=1000)||(Var(10)&&Command="super2"&&Var(14)))
Trigger1 = StateType != A && (Ctrl||((Var(10)&&Var(6)>0)||(!Var(10)&&Var(19)>0))||(Time<3&&StateNo=[200,399])) && RoundState = 2
Value = 2000 + Var(10)*100

[State -1];ヨガテレポート
Type = ChangeState
Trigger1 = (Command="waza3_fp"||Command="waza3_bp"||Command="waza3_fk"||Command="waza3_bk")  && StateType != A && (Ctrl||Var(6)>0||(Time<3&&StateNo=[200,399])) && RoundState = 2
Value = IFelse(Command="waza3_fp", 1200, IFelse(Command="waza3_bp", 1201, IFelse(Command="waza3_fk", 1202, 1203)))

[State -1];ヨガフレイム
Type = ChangeState
Trigger1 = (Command = "waza2_light" || Command = "waza2_middle" || Command = "waza2_hard") && NumProjID(1) = 0 && NumProjID(2) = 0 && NumProjID(3) = 0 &&  StateType != A && (Ctrl||Var(6)>0||(Time<3&&StateNo=[200,399])) && RoundState = 2
Value = IFelse(Var(14)&&Var(10), 1106, IFelse(Command = "waza2_light", 1100, IFelse(Command = "waza2_middle", 1104, 1105)))

[State -1];ヨガブラスト
Type = ChangeState
Trigger1 = (Command = "waza4_light" || Command = "waza4_middle" || Command = "waza4_hard") && NumProjID(1) = 0 && NumProjID(2) = 0 && NumProjID(3) = 0 &&  StateType != A && (Ctrl||Var(6)>0||(Time<3&&StateNo=[200,399])) && RoundState = 2
Value = IFelse(Var(14)&&Var(10), 1306, IFelse(Command="waza4_light", 1300, IFelse(Command="waza4_middle", 1304, 1305)))

[State -1];ヨガファイヤー or ヨガカレー
Type = ChangeState
Trigger1 = (Command = "waza1_light" || Command = "waza1_middle" || Command = "waza1_hard") && NumProjID(1) = 0 && NumProjID(2) = 0 && NumProjID(3) = 0 && StateType != A && (Ctrl||Var(6)>0||(Time<3&&StateNo=[200,399])) && RoundState = 2
Value = IFelse(Var(14)&&Var(10), 2603, IFelse(Command="waza1_light", 1000, IFelse(Command="waza1_middle", 1001, 1002))+Var(10)*1600)

[State -1];ヨガスピン
Type = ChangeState
Trigger1 = (Command = "waza5_light" || Command = "waza5_middle" || Command = "waza5_hard") && (Ctrl||Var(6)>0||(Time<3&&StateNo=[200,499])) && Var(10) && RoundState = 2
Value = 2500 + IFelse(Command="waza5_light",0,IFelse(Command="waza5_hard",2,1))

[State -1];ダッシュ
Type = ChangeState
Trigger1 = (Command="FF"||Command="BB") && StateType = S && Ctrl && Var(10) && RoundState = 2
Value = 100

[State -1];投げ
Type = ChangeState
Trigger1 = StateNo = 20 && (Command="y"||Command="z") && P2BodyDist X < (54*Const(Size.XScale)) && Var(30) != 1 && RoundState = 2
Value = 500 + (Command="z")*10

[State -1];立ちパンチ ダークストーカー時
Type = ChangeState
TriggerAll = StateType != A && Command != "holddown" && Var(10) && Var(6)=[1,6]
Trigger1 = StateNo=201&&MoveContact&&(Command="y"||Command="z")
Trigger2 = StateNo=211&&MoveContact&&Command="z"
Value = IFelse(Command="holdback", IFelse(Command = "x", 201, IFelse(Command = "y", 211, 221)), IFelse(Command = "x", 200, IFelse(Command = "y", 210, 220)))
[State -1];立ちキック ダークストーカー時
Type = ChangeState
TriggerAll = StateType != A && Command != "holddown" && Var(10) && Var(6)=[1,6]
Trigger1 = StateNo=230&&MoveContact&&(Command="b"||Command="c")
Trigger2 = StateNo=241&&MoveContact&&Command="c"
Value = IFelse(Command = "a", IFelse(Command="holdfwd", 231, 230), IFelse(Command="holdback", IFelse(Command = "b", 241, 251), IFelse(Command = "b", 240, 250)))
[State -1];しゃがみパンチ ダークストーカー時
Type = ChangeState
TriggerAll = StateType != A && Command = "holddown" && Var(10) && Var(6)=[1,6]
Trigger1 = StateNo=301&&MoveContact&&(Command="y"||Command="z")
Trigger2 = StateNo=311&&MoveContact&&Command="z"
Value=IFelse(Command="holdback",IFelse(Command="x",301,IFelse(Command="y",311,321)),IFelse(Command="x",300,IFelse(Command="y",310,320)))
[State -1];しゃがみキック ダークストーカー時
Type = ChangeState
TriggerAll = StateType != A && Command = "holddown" && Var(10) && Var(6)=[1,6]
Trigger1 = StateNo=331&&MoveContact&&(Command="b"||Command="c")
Trigger2 = StateNo=341&&MoveContact&&Command="c"
Value=IFelse(Command="holdback",IFelse(Command="a",331,IFelse(Command="b",341,351)),IFelse(Command="a",330,IFelse(Command="b",340,350)))

[State -1];立ちパンチ系攻撃
Type = ChangeState
Trigger1 = (Command = "x" || Command = "y" || Command = "z") && StateType = S && Ctrl && RoundState = 2
Value = IFelse(Command="holdback", IFelse(Command = "x", 201, IFelse(Command = "y", 211, 221)), IFelse(Command = "x", 200, IFelse(Command = "y", 210, 220)))
[State -1];立ちキック系攻撃
Type = ChangeState
Trigger1 = (Command = "a" || Command = "b" || Command = "c") && StateType = S && Ctrl && RoundState = 2
Value = IFelse(Command = "a", IFelse(Command="holdfwd", 231, 230), IFelse(Command="holdback", IFelse(Command = "b", 241, 251), IFelse(Command = "b", 240, 250)))
[State -1];しゃがみ攻撃
Type = ChangeState
Trigger1 = (Command = "x" || Command = "y" || Command = "z" || Command = "a" || Command = "b" || Command = "c") && StateType = C && Command = "holddown" && Ctrl && RoundState = 2
Value = IFelse(Command="x",300,IFelse(Command="y",310,IFelse(Command="z",320,IFelse(Command="a",330, IFelse(Command="b",340,350)))))+(Command="holdback")
[State -1];空中パンチ
Type = ChangeState
Trigger1 = (Command = "x" || Command = "y" || Command = "z") && StateType = A && Ctrl && RoundState = 2
Value = IFelse(Command="x", IFelse(Vel X=0, 400, 401), IFelse(Command="y", IFelse(Vel X=0, 410, 411), IFelse(Command="holddown"&&!Var(10), 460, 420)))
[State -1];空中キック
Type = ChangeState
Trigger1 = (Command = "a" || Command = "b" || Command = "c") && StateType = A && Ctrl && RoundState = 2
Value = IFelse(Command="a", IFelse(Command="holddown"&&!Var(10), 470, 430), IFelse(Command="b", IFelse(Command="holddown"&&!Var(10), 471, 440), IFelse(Command="holddown"&&!Var(10), 472, 450)))

;ここから下はいんちきプログラム。
[State -1];敵と距離が離れてる時はヨガファイヤーをたまに。
Type = ChangeState
Trigger1 = NumProjID(1) = 0 && NumProjID(2) = 0 && NumProjID(3) = 0 && StateType != A && (Ctrl||Var(6)>0||(Time<3&&StateNo=[200,399])) && Random < (P2Dist X / 40) && Var(30) = 1 && RoundState = 2 && !Var(10)
Value = IFelse(Random < 100, 1000, IFelse(Random < 600, 1001, 1002))
[State -1];ダークストーカー時はヨガカレーをたまに。
Type = ChangeState
Trigger1 = NumProjID(1) = 0 && NumProjID(2) = 0 && NumProjID(3) = 0 && StateType != A && (Ctrl||Var(6)>0||(Time<3&&StateNo=[200,399])) && P2Dist X > 30 && Random < 20 && Var(30) = 1 && RoundState = 2 && Var(10)
Value = IFelse(Var(14), 2603, IFelse(P2Dist X<80, 2600, IFelse(P2Dist X<140, 2601, 2602)))
[State -1];ヨガカレースペシャルの効果をランダムで決定。
Type = VarSet
Trigger1 = StateNo = 2603 && Time = 1 && Var(30) = 1
V = 15
Value = (Random<600)*2 + (Random=[600,649]) + (Random=[650,899])*3 + (Random>899)*4

[State -1];敵の攻撃を避けるのに使おうヨガテレポート。
Type = ChangeState
Trigger1 = StateType != A && (Ctrl||Var(6)>0||(Time<3&&StateNo=[200,399])) && P2Dist X < 70 && P2MoveType = A && (Random < 100 || (Var(14)&&Random<500)) && Var(30) = 1 && RoundState = 2
Value = IFelse(Random<500, 1200, IFelse(Random<600, 1201, IFelse(Random<900, 1202, 1203)))

[State -1];敵が近くで飛び道具を撃ったらヨガテレポートかジャンプ。
Type = ChangeState
Trigger1 = StateType != A && (Ctrl||Var(6)>0||(Time<3&&StateNo=[200,399])) && P2Dist X < 80 && EnemyNear,NumProjID(0) != 0 && Random < 300 && Var(30) = 1 && RoundState = 2
Value = IFelse(Random < 500, 40, 1200)

[State -1];近づかばヨガせっかん！
Type = ChangeState
Trigger1 = P2Dist X < 20 && Ctrl && Random < 20 && StateType != A && Var(30) = 1 && RoundState = 2
Value = 500

[State -1];ヨガスピンを敵との位置関係で。
Type = ChangeState
Trigger1 = (Ctrl||Var(6)>0||(Time<3&&StateNo=[200,499])) && Random < 25 && Var(10) && Var(30) = 1 && RoundState = 2
Value = 2500 + IFelse(P2Dist X<70,0,IFelse(P2Dist X<120,2,1))

[State -1];通常技キャンセル必殺技。
Type = ChangeState
Trigger1 = Var(6)>0 && Random < 50 && Var(30) = 1
Value = IFelse(Var(10), 2500, 1000)

[State -1];目押しコンボを狙おう。
Type = ChangeState
Trigger1 = (StateNo=201||StateNo=211)&&Random<300&&Var(30)=1&&Var(10)&&Var(6)=[1,6]
Value = IFelse(StateNo=201, IFelse(Random<800, 211, 210), IFelse(Random<800, 220, 221))
[State -1]
Type = ChangeState
Trigger1 = (StateNo=230||StateNo=241)&&Random<300&&Var(30)=1&&Var(10)&&Var(6)=[1,6]
Value = IFelse(StateNo=230, IFelse(Random<800, 241, 240), IFelse(Random<800, 250, 251))
[State -1]
Type = ChangeState
Trigger1 = (StateNo=301||StateNo=311)&&Random<300&&Var(30)=1&&Var(10)&&Var(6)=[1,6]
Value = IFelse(StateNo=301, IFelse(Random<800, 311, 310), IFelse(Random<800, 320, 321))
[State -1]
Type = ChangeState
Trigger1 = (StateNo=331||StateNo=341)&&Random<300&&Var(30)=1&&Var(10)&&Var(6)=[1,6]
Value = IFelse(StateNo=331, IFelse(Random<800, 341, 340), IFelse(Random<800, 350, 351))