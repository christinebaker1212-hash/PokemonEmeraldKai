@ Moemon Kai: ORRE's shared scripts (docs/ORRE_DECISIONS.md)

@ A place's exits (tools/orre/maps.py "exits"): Colosseum's world map (src/orre/worldmap.c), then off to the place
@ chosen there, or back to the field
Orre_EventScript_WorldMap::
	lockall
	special Orre_WorldMap
	special Orre_WorldMapTravel
	goto_if_eq VAR_RESULT, TRUE, Orre_EventScript_WorldMapGone
	releaseall
	end
Orre_EventScript_WorldMapGone:
	end
