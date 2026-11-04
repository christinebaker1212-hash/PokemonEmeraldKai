Johto_LakeOfRageMagikarpHouse_MapScripts::
	.byte 0

@ Johto_LakeOfRageMagikarpHouse object 1 (local id 1, gfx 0x3E95) at 5,4
@ Moemon Kai: Crystal's MAGIKARP length rater (pokecrystal LakeOfRageMagikarpHouse; Quetzal bought MAGIKARP), measured by
@ Emerald's size-record code in a JOHTO var (pokemon_size_record.c; docs/REGIONS_AUDIT.md 108). VAR_QZ_0x5026: 1 the
@ lake's story told, 2 (Quetzal's sales) / 3 asked for a big one
Johto_LakeOfRageMagikarpHouse_EventScript_83DDFD0::
	lock
	faceplayer
	goto_if_set FLAG_JOHTO_ELIXER_STANDBY, Johto_LakeOfRageMagikarpHouse_EventScript_Reward
	goto_if_ge VAR_QZ_0x5026, 2, Johto_LakeOfRageMagikarpHouse_EventScript_Asked
	goto_if_set FLAG_QZ_0x1051, Johto_LakeOfRageMagikarpHouse_EventScript_Cleared
	goto_if_eq VAR_QZ_0x5026, 1, Johto_LakeOfRageMagikarpHouse_EventScript_MenInBlack
	msgbox Johto_LakeOfRageMagikarpHouse_Text_History, MSGBOX_DEFAULT
	setvar VAR_QZ_0x5026, 1
	release
	end

Johto_LakeOfRageMagikarpHouse_EventScript_MenInBlack::
	msgbox Johto_LakeOfRageMagikarpHouse_Text_MenInBlack, MSGBOX_DEFAULT
	release
	end

Johto_LakeOfRageMagikarpHouse_EventScript_Cleared::
	msgbox Johto_LakeOfRageMagikarpHouse_Text_WorldsLargest, MSGBOX_DEFAULT
	setvar VAR_QZ_0x5026, 3
	release
	end

Johto_LakeOfRageMagikarpHouse_EventScript_Asked::
	setvar VAR_0x8004, SPECIES_MAGIKARP
	specialvar VAR_RESULT, QzPartyHasSpecies
	goto_if_eq VAR_RESULT, 0, Johto_LakeOfRageMagikarpHouse_EventScript_Cleared
	msgbox Johto_LakeOfRageMagikarpHouse_Text_YouHaveAMagikarp, MSGBOX_DEFAULT
	special GetJohtoMagikarpSizeRecordInfo
	special ChoosePartyMon
	waitstate
	copyvar VAR_RESULT, VAR_0x8004
	goto_if_ge VAR_RESULT, PARTY_SIZE, Johto_LakeOfRageMagikarpHouse_EventScript_Refused
	special CompareJohtoMagikarpSize
	goto_if_eq VAR_RESULT, 1, Johto_LakeOfRageMagikarpHouse_EventScript_NotMagikarp
	goto_if_eq VAR_RESULT, 3, Johto_LakeOfRageMagikarpHouse_EventScript_Record
	goto_if_unset FLAG_JOHTO_MAGIKARP_RECORD, Johto_LakeOfRageMagikarpHouse_EventScript_Record
	msgbox Johto_LakeOfRageMagikarpHouse_Text_TooShort, MSGBOX_DEFAULT
	release
	end

Johto_LakeOfRageMagikarpHouse_EventScript_Record::
	setflag FLAG_JOHTO_MAGIKARP_RECORD
Johto_LakeOfRageMagikarpHouse_EventScript_Reward::
	msgbox Johto_LakeOfRageMagikarpHouse_Text_Memento, MSGBOX_DEFAULT
	setflag FLAG_JOHTO_ELIXER_STANDBY
	setorcopyvar VAR_0x8000, ITEM_ELIXIR
	setorcopyvar VAR_0x8001, 1
	callstd STD_OBTAIN_ITEM
	goto_if_eq VAR_RESULT, 0, Johto_LakeOfRageMagikarpHouse_EventScript_NoRoom
	clearflag FLAG_JOHTO_ELIXER_STANDBY
	msgbox Johto_LakeOfRageMagikarpHouse_Text_Bonus, MSGBOX_DEFAULT
	release
	end

Johto_LakeOfRageMagikarpHouse_EventScript_NoRoom::
	release
	end

Johto_LakeOfRageMagikarpHouse_EventScript_NotMagikarp::
	msgbox Johto_LakeOfRageMagikarpHouse_Text_NotMagikarp, MSGBOX_DEFAULT
	release
	end

Johto_LakeOfRageMagikarpHouse_EventScript_Refused::
	msgbox Johto_LakeOfRageMagikarpHouse_Text_Refused, MSGBOX_DEFAULT
	release
	end

Johto_LakeOfRageMagikarpHouse_Text_History::
	.string "LAKE OF RAGE is actually a crater\n"
	.string "made by rampaging GYARADOS.\p"
	.string "The crater filled up with\n"
	.string "rainwater and the LAKE was formed.\p"
	.string "That’s the story passed on from my\n"
	.string "Grandpa’s greatgreat-grandpa.\p"
	.string "It used to be that you could catch\n"
	.string "lively MAGIKARP there, but…\p"
	.string "I don’t understand what’s\n"
	.string "happening.$"

Johto_LakeOfRageMagikarpHouse_Text_MenInBlack::
	.string "The LAKE hasn’t been normal since\n"
	.string "those men wearing black arrived.$"

Johto_LakeOfRageMagikarpHouse_Text_WorldsLargest::
	.string "LAKE OF RAGE is back to normal.\p"
	.string "The MAGIKARP have returned.\p"
	.string "I may yet realize my dream of\n"
	.string "seeing the world’s largest\l"
	.string "MAGIKARP.\p"
	.string "Do you have a ROD? Please help me\n"
	.string "if you do.$"

Johto_LakeOfRageMagikarpHouse_Text_YouHaveAMagikarp::
	.string "Ah, you have a MAGIKARP! Let’s see\n"
	.string "how big that baby is.$"

Johto_LakeOfRageMagikarpHouse_Text_Memento::
	.string "Wow! This one is outstanding!\p"
	.string "I tip my hat to you!\p"
	.string "Take this as a memento!$"

Johto_LakeOfRageMagikarpHouse_Text_Bonus::
	.string "The record is the important thing.\p"
	.string "Think of that as a bonus!$"

Johto_LakeOfRageMagikarpHouse_Text_TooShort::
	.string "Wow! This one is outstanding!\p"
	.string "…I wish I could say that, but I’ve\n"
	.string "seen a bigger one before.$"

Johto_LakeOfRageMagikarpHouse_Text_NotMagikarp::
	.string "What? That’s not a MAGIKARP!$"

Johto_LakeOfRageMagikarpHouse_Text_Refused::
	.string "Oh… So you didn’t get one good\n"
	.string "enough to show me? Maybe next\l"
	.string "time.$"
