Sinnoh_SandgemTownMart_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_SandgemTownMart_EventScript_Person1::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart 0x0
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Sinnoh_SandgemTownMart_EventScript_Person2::
	goto SinnohP_sandgem_town_mart_4
	end

Sinnoh_SandgemTownMart_EventScript_Person3::
	goto SinnohP_sandgem_town_mart_3
	end

@ --- end of SINNOH words ---
