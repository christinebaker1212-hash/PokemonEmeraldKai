Johto_GoldenrodDeptStoreRoof_MapScripts::
	map_script MAP_SCRIPT_ON_TRANSITION, Johto_GoldenrodDeptStoreRoof_EventScript_83D491D
	.byte 0

Johto_GoldenrodDeptStoreRoof_EventScript_83D491D::
	call_if_set FLAG_QZ_0x1016, Johto_GoldenrodDeptStoreRoof_EventScript_83D4930
	call_if_unset FLAG_QZ_0x1016, Johto_GoldenrodDeptStoreRoof_EventScript_83D4934
	end

Johto_GoldenrodDeptStoreRoof_EventScript_83D4930::
	setmaplayoutindex LAYOUT_JOHTO_GOLDENROD_DEPT_STORE_ROOF_ALT   @ (Quetzal's layout 0x1FF)
	return

Johto_GoldenrodDeptStoreRoof_EventScript_83D4934::
	setmaplayoutindex LAYOUT_JOHTO_GOLDENROD_DEPT_STORE_ROOF   @ (Quetzal's layout 0x1FE)
	return
