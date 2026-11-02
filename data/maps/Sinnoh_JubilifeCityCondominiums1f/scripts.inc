Sinnoh_JubilifeCityCondominiums1f_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_JubilifeCityCondominiums1f_EventScript_Person1::
	goto SinnohP_jubilife_city_condominiums_1f_1
	end

Sinnoh_JubilifeCityCondominiums1f_EventScript_Person2::
	msgbox Sinnoh_JubilifeCityCondominiums1f_Text_Person2, MSGBOX_NPC
	end

Sinnoh_JubilifeCityCondominiums1f_EventScript_Person3::
	msgbox Sinnoh_JubilifeCityCondominiums1f_Text_Person3, MSGBOX_NPC
	end

Sinnoh_JubilifeCityCondominiums1f_EventScript_Person4::
	lock
	faceplayer
	goto_if_set 0x3DBA, Sinnoh_JubilifeCityCondominiums1f_EventScript_Person4_Given
	msgbox Sinnoh_JubilifeCityCondominiums1f_Text_Person4, MSGBOX_DEFAULT
	msgbox Sinnoh_JubilifeCityCondominiums1f_Text_Person4Gift, MSGBOX_DEFAULT
	giveitem ITEM_QUICK_CLAW, 1
	goto_if_eq VAR_RESULT, FALSE, Common_EventScript_ShowBagIsFull
	setflag 0x3DBA
	release
	end

Sinnoh_JubilifeCityCondominiums1f_EventScript_Person4_Given::
	msgbox Sinnoh_JubilifeCityCondominiums1f_Text_Person4, MSGBOX_DEFAULT
	release
	end

Sinnoh_JubilifeCityCondominiums1f_Text_Person2:
	.string "PACHIRISU: Chipa?\p"
	.string "It's staring at you curiously.$"

Sinnoh_JubilifeCityCondominiums1f_Text_Person3:
	.string "PIKACHU: Pika pika!\p"
	.string "Its cheeks crackle happily.$"

Sinnoh_JubilifeCityCondominiums1f_Text_Person4:
	.string "I just moved in. My neighbors are so\n"
	.string "friendly, and so are their MOéMON.$"

Sinnoh_JubilifeCityCondominiums1f_Text_Person4Gift:
	.string "This was a housewarming gift, but I\n"
	.string "have two. Please, take one!$"

@ --- end of SINNOH words ---
