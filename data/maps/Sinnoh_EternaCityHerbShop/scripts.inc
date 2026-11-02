Sinnoh_EternaCityHerbShop_MapScripts::
	.byte 0

@ --- SINNOH words (tools/sinnoh/gen_words.py from tools/sinnoh/words.py): not to be edited here ---
Sinnoh_EternaCityHerbShop_EventScript_Person1::
	goto SinnohP_eterna_city_herb_shop_2
	end

Sinnoh_EternaCityHerbShop_EventScript_Person2::
	lock
	faceplayer
	message gText_HowMayIServeYou
	waitmessage
	pokemart Sinnoh_Mart_EternaHouse
	msgbox gText_PleaseComeAgain, MSGBOX_DEFAULT
	release
	end

Sinnoh_EternaCityHerbShop_EventScript_Person3::
	goto SinnohP_eterna_city_herb_shop_3
	end

@ --- end of SINNOH words ---
