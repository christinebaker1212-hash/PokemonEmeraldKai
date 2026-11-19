@ Moemon Kai: CHATTER from the party menu, Platinum's own script and words (res/field/scripts/scripts_record_chatot_cry.s,
@ res/text/record_chatot_cry.json): CHATOT learns a saying from the microphone (src/mp/mp_chatter.c; the party slot
@ in VAR_0x8004)
EventScript_RecordChatotCry::
	lockall
	special MP_ChatterField_Prepare
	showmonpic VAR_0x8005, 10, 3
	goto_if_eq VAR_RESULT, FALSE, EventScript_RecordChatotCry_Try
	msgbox Text_RecordChatotCry_Forget, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_RecordChatotCry_End
EventScript_RecordChatotCry_Try:
	message Text_RecordChatotCry_PressAOrB
	waitmessage
	waitbuttonpress
	specialvar VAR_RESULT, MP_ChatterField_Start
	goto_if_eq VAR_RESULT, FALSE, EventScript_RecordChatotCry_Failed
	special MP_ChatterField_Wait
	waitstate
	goto_if_eq VAR_RESULT, FALSE, EventScript_RecordChatotCry_Failed
	bufferpartymonnick STR_VAR_1, VAR_0x8004
	message Text_RecordChatotCry_Learned
	waitmessage
	playmoncry SPECIES_CHATOT, CRY_MODE_NORMAL
	waitmoncry
	waitbuttonpress
EventScript_RecordChatotCry_End:
	hidemonpic
	closemessage
	releaseall
	end

EventScript_RecordChatotCry_Failed:
	bufferpartymonnick STR_VAR_1, VAR_0x8004
	message Text_RecordChatotCry_FailedToLearn
	waitmessage
	waitbuttonpress
	msgbox Text_RecordChatotCry_TryAgain, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, YES, EventScript_RecordChatotCry_Try
	goto EventScript_RecordChatotCry_End

Text_RecordChatotCry_Forget:
	.string "It will forget the saying it now\n"
	.string "knows. Is that OK?$"

Text_RecordChatotCry_PressAOrB:
	.string "Press the A or B Button, then speak\n"
	.string "into the microphone.$"

Text_RecordChatotCry_Learned:
	.string "{STR_VAR_1} learned a new saying!$"

Text_RecordChatotCry_FailedToLearn:
	.string "{STR_VAR_1} failed to learn a new\n"
	.string "saying.$"

Text_RecordChatotCry_TryAgain:
	.string "Would you like to try again?$"
