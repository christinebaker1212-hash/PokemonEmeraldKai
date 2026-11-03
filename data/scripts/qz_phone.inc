@ Moemon Kai: JOHTO's phone (src/mp/mp_qzphone.c): Crystal's converted scripts run step by step; each step says what
@ to show (VAR_RESULT): 0 the end, 1 a text box (gStringVar4), 2 YES / NO after gStringVar4, 3 an item (VAR_0x8004),
@ 4 a rematch, 5 the number registered (a fanfare with gStringVar4)

QzPhone_EventScript_TrainerTalk::
	goto QzPhone_EventScript_Run

QzPhone_EventScript_Run::
	callnative QzPhone_Step
	goto_if_eq VAR_RESULT, 1, QzPhone_EventScript_Text
	goto_if_eq VAR_RESULT, 2, QzPhone_EventScript_YesNo
	goto_if_eq VAR_RESULT, 3, QzPhone_EventScript_Item
	goto_if_eq VAR_RESULT, 4, QzPhone_EventScript_Battle
	goto_if_eq VAR_RESULT, 5, QzPhone_EventScript_Registered
	closemessage
	releaseall
	end

QzPhone_EventScript_Text::
	msgbox gStringVar4, MSGBOX_DEFAULT
	goto QzPhone_EventScript_Run

QzPhone_EventScript_YesNo::
	message gStringVar4
	waitmessage
	yesnobox 20, 8
	callnative QzPhone_Answer
	goto QzPhone_EventScript_Run

QzPhone_EventScript_Item::
	giveitem VAR_0x8004, VAR_0x8005
	callnative QzPhone_ItemGiven
	goto QzPhone_EventScript_Run

QzPhone_EventScript_Registered::
	playfanfare MUS_REGISTER_MATCH_CALL
	message gStringVar4
	waitmessage
	waitfanfare
	waitbuttonpress
	goto QzPhone_EventScript_Run

QzPhone_EventScript_Battle::
	closemessage
	callnative QzPhone_SetupBattle
	special PlayTrainerEncounterMusic
	dotrainerbattle
	callnative MP_QzPhone_BattleOver
	goto QzPhone_EventScript_Run

@ A contact calls (field_control_avatar.c, a step in JOHTO)
QzPhone_EventScript_IncomingCall::
	lockall
	playse SE_POKENAV_CALL
	waitse
	playse SE_POKENAV_CALL
	waitse
	goto QzPhone_EventScript_Run
