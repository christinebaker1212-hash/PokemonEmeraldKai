Sinnoh_CanalaveCityMart_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_CanalaveCityMart_EventScript_Person1::
	goto SinnohP_canalave_city_mart_3
	end

Sinnoh_CanalaveCityMart_EventScript_Person2::
	goto SinnohP_canalave_city_mart_4
	end

Sinnoh_CanalaveCityMart_EventScript_Person3::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart 0x0
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Sinnoh_CanalaveCityMart_EventScript_Person5::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart Sinnoh_Mart_Canalave
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

@ --- end of SINNOH words ---
