@ Moemon Kai: UNOVA's shared scripts (docs/UNOVA_HANDOFF.md "Black's words and scripts"). Every UNOVA map's people,
@ signs and trigger spots come here; Black's own script then runs in UNOVA's VM (src/unova/u3d_vm.c) and the field
@ waits until it ends. When Black's script opens a screen (its starter choice, a battle...), the VM lets the wait go,
@ the screen opens from here, and the VM goes on when it's back. Nothing of Black's is written here.

@ ON_TRANSITION of every UNOVA map: Black's zone-change and field-init scripts (people placed by the story)
Unova_EventScript_MapLoad::
	special U3D_VmMapLoad
	end

@ A person talked to
Unova_EventScript_Talk::
	lockall
	special U3D_VmTalk
	goto Unova_EventScript_Wait

@ A sign or a spot looked at
Unova_EventScript_Sign::
	lockall
	special U3D_VmSign
	goto Unova_EventScript_Wait

@ A trigger spot stepped on (its work held its value)
Unova_EventScript_Pos::
	lockall
	special U3D_VmPos
	goto Unova_EventScript_Wait

@ A scene of the zone's own (script.c TryRunOnFrameMapScript -> U3D_Vm_SceneStart)
Unova_EventScript_Scene::
	lockall
	special U3D_VmHold
	goto Unova_EventScript_Wait

@ Until Black's script ends (VAR_RESULT 0), or wants a screen of the game's
Unova_EventScript_Wait::
	waitstate
	specialvar VAR_RESULT, U3D_VmScreen
	goto_if_ne VAR_RESULT, 0, Unova_EventScript_Screen
	releaseall
	end

Unova_EventScript_Screen::
	special U3D_VmOpenScreen
	waitstate
	special U3D_VmResume
	goto Unova_EventScript_Wait

@ ELESA's gym's coasters (src/unova/u3d_vm.c U3D_Vm_DockStep): a dock walked onto facing the rails. A lane still locked
@ runs Black's own scripts there (its trainer); else the cart carries the player to the other stop of its loop
Unova_EventScript_Ride::
	lockall
	specialvar VAR_RESULT, U3D_VmRideKind
	goto_if_eq VAR_RESULT, 1, Unova_EventScript_RideCart
	special U3D_VmRideLocked
	goto Unova_EventScript_Wait

@ SKYLA's gym's cannons (src/unova/u3d_vm.c U3D_Vm_CannonAhead): walked into, Black's script climbs in, the shot
@ lands the player on the cell Black's table gives
Unova_EventScript_Cannon::
	lockall
	special U3D_VmCannon
	waitstate
	playse SE_M_EXPLOSION
	waitse
	warpsilent MAP_UNOVA_MISTRALTONCITY_Z108, VAR_0x8004, VAR_0x8005
	waitstate
	releaseall
	end

Unova_EventScript_RideCart::
	playse SE_TRUCK_MOVE
	warpsilent MAP_UNOVA_NIMBASACITY_Z63, VAR_0x8004, VAR_0x8005
	waitstate
	releaseall
	end
