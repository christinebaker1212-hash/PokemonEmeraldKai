EventScript_RepelWoreOff::
	msgbox Text_RepelWoreOff, MSGBOX_SIGN
	@ Moemon Co-op QoL: offer another of the same REPEL
	special MP_Repel_CheckAnother
	goto_if_eq VAR_RESULT, FALSE, EventScript_RepelWoreOffEnd
	lock
	msgbox Text_MpUseAnotherRepel, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, EventScript_RepelWoreOffRelease
	special MP_Repel_UseAnother
	playse SE_REPEL
	waitse
	msgbox Text_MpUsedRepel, MSGBOX_DEFAULT
EventScript_RepelWoreOffRelease::
	release
EventScript_RepelWoreOffEnd::
	end

Text_RepelWoreOff:
	.string "REPEL's effect wore off…$"

Text_MpUseAnotherRepel:
	.string "Use another {STR_VAR_1}?$"

Text_MpUsedRepel:
	.string "{PLAYER} used the\n{STR_VAR_1}.$"
