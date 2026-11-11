@ Moemon Kai: UNOVA's shared scripts (docs/UNOVA_HANDOFF.md "Black's words and scripts"). Every UNOVA map's people,
@ signs and trigger spots come here; Black's own script then runs in UNOVA's VM (src/unova/u3d_vm.c) and the field
@ waits until it ends. Nothing of Black's is written here.

@ ON_TRANSITION of every UNOVA map: Black's zone-change and field-init scripts (people placed by the story)
Unova_EventScript_MapLoad::
	special U3D_VmMapLoad
	end

@ A person talked to
Unova_EventScript_Talk::
	lockall
	special U3D_VmTalk
	waitstate
	releaseall
	end

@ A sign or a spot looked at
Unova_EventScript_Sign::
	lockall
	special U3D_VmSign
	waitstate
	releaseall
	end

@ A trigger spot stepped on (its work held its value)
Unova_EventScript_Pos::
	lockall
	special U3D_VmPos
	waitstate
	releaseall
	end

@ A scene of the zone's own (script.c TryRunOnFrameMapScript -> U3D_Vm_SceneStart)
Unova_EventScript_Scene::
	lockall
	special U3D_VmHold
	waitstate
	releaseall
	end
