Johto_Route48_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Quetzal_EventScript_840EA66
	.byte 0

@ Johto_Route48 trigger at 21,0 (VAR_QZ_0x500F == 2)
@ Johto_Route48 trigger at 22,0 (VAR_QZ_0x500F == 2)
@ Johto_Route48 trigger at 23,0 (VAR_QZ_0x500F == 2)
@ Johto_Route48 trigger at 24,0 (VAR_QZ_0x500F == 2)
Johto_Route48_EventScript_83F8810::
	setflag FLAG_QZ_0x11C2
	setvar VAR_QZ_0x500F, 3
	end

@ Johto_Route48 trigger at 21,1 (VAR_TEMP_0 == 0)
@ Johto_Route48 trigger at 22,1 (VAR_TEMP_0 == 0)
@ Johto_Route48 trigger at 23,1 (VAR_TEMP_0 == 0)
@ Johto_Route48 trigger at 24,1 (VAR_TEMP_0 == 0)
Johto_Route48_EventScript_83F8819::
	setflag FLAG_QZ_0x11C2
	setvar VAR_TEMP_0, 1
	end
