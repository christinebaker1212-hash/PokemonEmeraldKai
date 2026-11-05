EventScript_UseFlash::
	animateflash 1
	setflashlevel 1
	end

@ Moemon Kai: FLASH in the RUINS OF ALPH's AERODACTYL chamber, lit already (fldeff_flash.c QzRuinsFlash): a flash of
@ light only
EventScript_RuinsFlash::
	lockall
	fadescreenspeed FADE_TO_WHITE, 0
	fadescreenspeed FADE_FROM_WHITE, 0
	releaseall
	end
