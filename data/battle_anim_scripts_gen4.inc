@ Gen 4 move animations (Move_<MOVE>). A move still borrowing a Gen 3 animation
@ just jumps to it; tools/moemon_gen4/gen_moves.py adds a borrowing stub for any move missing here.

Move_ROOST:
	loadspritegfx ANIM_TAG_WHITE_FEATHER
	monbg ANIM_ATTACKER
	splitbgprio_all
	playsewithpan SE_M_PETAL_DANCE, SOUND_PAN_TARGET
	delay 0
	createsprite gFallingFeatherSpriteTemplate, ANIM_ATTACKER, 0, 0, -16, 64, 2, 104, 11304, 32, 1
	delay 6
	createsprite gFallingFeatherSpriteTemplate, ANIM_ATTACKER, 0, 0, -16, 32, 2, 104, 11304, 32, 1
	createsprite gFallingFeatherSpriteTemplate, ANIM_ATTACKER, 0, 0, -16, 0, 2, 104, 11304, 32, 1
	delay 6
	createsprite gFallingFeatherSpriteTemplate, ANIM_ATTACKER, 0, 0, -16, 224, 2, 104, 11304, 32, 1
	createsprite gFallingFeatherSpriteTemplate, ANIM_ATTACKER, 0, 0, -16, 128, 2, 104, 11304, 32, 1
	delay 6
	createsprite gFallingFeatherSpriteTemplate, ANIM_ATTACKER, 0, 0, -16, 192, 2, 104, 11304, 32, 1
	createsprite gFallingFeatherSpriteTemplate, ANIM_ATTACKER, 0, 0, -16, 160, 2, 104, 11304, 32, 1
	delay 6
	createsprite gFallingFeatherSpriteTemplate, ANIM_ATTACKER, 0, 0, -16, 96, 2, 104, 11304, 32, 1
	waitforvisualfinish
	clearmonbg ANIM_ATTACKER
	end

Move_GRAVITY:
	goto Move_IMPRISON

Move_MIRACLE_EYE:
	loadspritegfx ANIM_TAG_OPENING_EYE
	loadspritegfx ANIM_TAG_ROUND_WHITE_HALO
	loadspritegfx ANIM_TAG_TEAL_ALERT
	monbg ANIM_DEF_PARTNER
	call G4_SetPsychicBackground
	playsewithpan SE_M_CONFUSE_RAY, SOUND_PAN_TARGET
	createsprite gOpeningEyeSpriteTemplate, ANIM_ATTACKER, 5, 0, 0, 1, 0
	createsprite gWhiteHaloSpriteTemplate, ANIM_ATTACKER, 5
	delay 40
	playsewithpan SE_M_LEER, SOUND_PAN_TARGET
	blend_color_cycle selector=F_PAL_BG, delay=1, num_blends=2, initial_blend_y=0, target_blend_y=10, color=RGB_BLACK
	call G4_MindReaderEyeSpikeEffect
	waitforvisualfinish
	call G4_UnsetBackground
	clearmonbg ANIM_DEF_PARTNER
	end

Move_WAKE_UP_SLAP:
	loadspritegfx ANIM_TAG_HANDS_AND_FEET
	loadspritegfx ANIM_TAG_IMPACT
	loadspritegfx ANIM_TAG_SMELLINGSALT_EFFECT
	monbg ANIM_DEF_PARTNER
	splitbgprio ANIM_TARGET
	setalpha 12, 8
	playsewithpan SE_M_DOUBLE_TEAM, SOUND_PAN_TARGET
	createsprite gKarateChopSpriteTemplate, ANIM_ATTACKER, 2, -16, 0, 0, 0, 10, 1, 3, 0
	waitforvisualfinish
	playsewithpan SE_M_DOUBLE_SLAP, SOUND_PAN_TARGET
	create_basic_hitsplat_sprite ANIM_ATTACKER, 3, x=0, y=0, relative_to=ANIM_TARGET, animation=2
	createvisualtask AnimTask_ShakeMon, 5, ANIM_TARGET, 4, 0, 6, 2
	waitforvisualfinish
	createsprite gSmellingSaltExclamationSpriteTemplate, ANIM_TARGET, 2, 1, 8, 3
	loopsewithpan SE_M_SWAGGER2, +63, 16, 3
	clearmonbg ANIM_DEF_PARTNER
	blendoff
	end

Move_HAMMER_ARM:
	goto Move_MEGA_PUNCH

Move_GYRO_BALL:
	loadspritegfx ANIM_TAG_IMPACT
	loopsewithpan SE_M_HARDEN, SOUND_PAN_ATTACKER, 28, 2
	metallic_shine permanent=0
	waitforvisualfinish
	playsewithpan SE_M_SWAGGER, SOUND_PAN_ATTACKER
	waitplaysewithpan SE_M_SWAGGER, SOUND_PAN_ATTACKER, 8
	createvisualtask AnimTask_TranslateMonEllipticalRespectSide, 2, ANIM_ATTACKER, 18, 6, 2, 4
	waitforvisualfinish
	createsprite gSlideMonToOffsetSpriteTemplate, ANIM_ATTACKER, 2, ANIM_ATTACKER, 20, 0, 0, 4
	delay 3
	waitforvisualfinish
	playsewithpan SE_M_MEGA_KICK2, SOUND_PAN_TARGET
	create_basic_hitsplat_sprite ANIM_TARGET, 4, x=-10, y=0, relative_to=ANIM_TARGET, animation=0
	createsprite gSlideMonToOffsetSpriteTemplate, ANIM_ATTACKER, 2, ANIM_TARGET, -32, 0, 0, 3
	waitforvisualfinish
	createvisualtask AnimTask_RotateMonSpriteToSide, 2, 8, -256, ANIM_ATTACKER, 0
	createvisualtask AnimTask_RotateMonSpriteToSide, 2, 8, -256, ANIM_TARGET, 0
	createvisualtask AnimTask_ShakeMonInPlace, 2, ANIM_ATTACKER, 4, 0, 12, 1
	createvisualtask AnimTask_ShakeMonInPlace, 2, ANIM_TARGET, 4, 0, 12, 1
	waitforvisualfinish
	createvisualtask AnimTask_RotateMonSpriteToSide, 2, 8, -256, ANIM_ATTACKER, 1
	createvisualtask AnimTask_RotateMonSpriteToSide, 2, 8, -256, ANIM_TARGET, 1
	waitforvisualfinish
	createsprite gSlideMonToOriginalPosSpriteTemplate, ANIM_ATTACKER, 2, 0, 0, 5
	delay 3
	createsprite gSlideMonToOriginalPosSpriteTemplate, ANIM_ATTACKER, 2, 1, 0, 7
	waitforvisualfinish
	end

Move_HEALING_WISH:
	goto Move_WISH

Move_BRINE:
	goto Move_WATER_PULSE

Move_NATURAL_GIFT:
	loadspritegfx ANIM_TAG_HOLLOW_ORB
	loadspritegfx ANIM_TAG_SPARKLE_2
	monbg ANIM_ATTACKER
	splitbgprio ANIM_ATTACKER
	blend_color_cycle selector=F_PAL_ATTACKER, delay=2, num_blends=2, initial_blend_y=0, target_blend_y=16, color=RGB(27, 31, 18)
	playsewithpan SE_M_MEGA_KICK, -64
	call G4_GrantingStarsEffect
	waitforvisualfinish
	delay 1
	createvisualtask AnimTask_DragonDanceWaver, 5
	playsewithpan SE_M_TELEPORT, -64
	delay 8
	createvisualtask AnimTask_BlendPalInAndOutByTag, 5, ANIM_TAG_HOLLOW_ORB, RGB(0, 0, 19), 14, 0, 3
	call G4_CreateDragonDanceOrbs
	delay 30
	playsewithpan SE_M_TELEPORT, -64
	delay 30
	playsewithpan SE_M_TELEPORT, -64
	waitforvisualfinish
	clearmonbg ANIM_ATTACKER
	delay 1
	end

Move_FEINT:
	goto Move_QUICK_ATTACK

Move_PLUCK:
	goto Move_PECK

Move_TAILWIND:
	loadspritegfx ANIM_TAG_FLYING_DIRT
	playsewithpan SE_M_GUST, SOUND_PAN_ATTACKER
	call G4_SetHighSpeedBg
	monbg ANIM_ATK_PARTNER
	setalpha 12, 8
	createvisualtask AnimTask_TranslateMonEllipticalRespectSide, 2, ANIM_ATTACKER, 24, 6, 4, 4
	createvisualtask AnimTask_TraceMonBlended, 2, 0, 4, 7, 10
	playsewithpan SE_M_DOUBLE_TEAM, SOUND_PAN_ATTACKER
	call G4_CreateTailwindCrescents
	waitforvisualfinish
	stopsound
	call G4_UnsetBackground
	clearmonbg ANIM_ATK_PARTNER
	blendoff
	delay 1
	end

Move_ACUPRESSURE:
	goto Move_FOCUS_ENERGY

Move_METAL_BURST:
	loadspritegfx ANIM_TAG_RED_ORB
	loopsewithpan SE_M_TRI_ATTACK, -64, 18, 3
	set_grayscale_pal battler=ANIM_ATTACKER
	createvisualtask AnimTask_DefenseCurlDeformMon, 5
	waitforvisualfinish
	setarg 7, -1
	playsewithpan SE_M_REFLECT, -64
	createvisualtask AnimTask_ScaleMonAndRestore, 5, -7, -7, 11, ANIM_ATTACKER, 0
	createsprite gHiddenPowerOrbScatterSpriteTemplate, ANIM_TARGET, 2, 0
	createsprite gHiddenPowerOrbScatterSpriteTemplate, ANIM_TARGET, 2, 32
	createsprite gHiddenPowerOrbScatterSpriteTemplate, ANIM_TARGET, 2, 64
	createsprite gHiddenPowerOrbScatterSpriteTemplate, ANIM_TARGET, 2, 96
	createsprite gHiddenPowerOrbScatterSpriteTemplate, ANIM_TARGET, 2, 128
	createsprite gHiddenPowerOrbScatterSpriteTemplate, ANIM_TARGET, 2, 160
	createsprite gHiddenPowerOrbScatterSpriteTemplate, ANIM_TARGET, 20, 192
	createsprite gHiddenPowerOrbScatterSpriteTemplate, ANIM_TARGET, 2, 224
	waitforvisualfinish
	set_original_pal battler=ANIM_ATTACKER
	waitforvisualfinish
	end

Move_U_TURN:
	goto Move_FURY_CUTTER

Move_CLOSE_COMBAT:
	loadspritegfx ANIM_TAG_HANDS_AND_FEET
	monbg ANIM_DEF_PARTNER
	call G4_SetHighSpeedBg
	createsprite gFistFootRandomPosSpriteTemplate, ANIM_TARGET, 3, 1, 10, 0
	createvisualtask AnimTask_ShakeMonInPlace, 2, ANIM_TARGET, 2, 0, 7, 1
	playsewithpan SE_M_COMET_PUNCH, +63
	delay 2
	createsprite gFistFootRandomPosSpriteTemplate, ANIM_TARGET, 3, 1, 10, 0
	createvisualtask AnimTask_ShakeMonInPlace, 2, ANIM_TARGET, 2, 0, 7, 1
	playsewithpan SE_M_VITAL_THROW2, +63
	delay 2
	createsprite gFistFootRandomPosSpriteTemplate, ANIM_TARGET, 3, 1, 10, 0
	createvisualtask AnimTask_ShakeMonInPlace, 2, ANIM_TARGET, 2, 0, 7, 1
	playsewithpan SE_M_MEGA_KICK2, +63
	delay 2
	createsprite gFistFootRandomPosSpriteTemplate, ANIM_TARGET, 3, 1, 10, 0
	createvisualtask AnimTask_ShakeMonInPlace, 2, ANIM_TARGET, 2, 0, 7, 1
	playsewithpan SE_M_COMET_PUNCH, +63
	delay 2
	createsprite gFistFootRandomPosSpriteTemplate, ANIM_TARGET, 3, 1, 10, 0
	createvisualtask AnimTask_ShakeMonInPlace, 2, ANIM_TARGET, 2, 0, 7, 1
	playsewithpan SE_M_VITAL_THROW2, +63
	delay 2
	createsprite gFistFootRandomPosSpriteTemplate, ANIM_TARGET, 3, 1, 10, 0
	createvisualtask AnimTask_ShakeMonInPlace, 2, ANIM_TARGET, 2, 0, 7, 1
	playsewithpan SE_M_MEGA_KICK2, +63
	delay 2
	createsprite gFistFootRandomPosSpriteTemplate, ANIM_TARGET, 3, 1, 10, 0
	createvisualtask AnimTask_ShakeMonInPlace, 2, ANIM_TARGET, 2, 0, 7, 1
	playsewithpan SE_M_COMET_PUNCH, +63
	delay 2
	createsprite gFistFootRandomPosSpriteTemplate, ANIM_TARGET, 3, 1, 10, 0
	createvisualtask AnimTask_ShakeMonInPlace, 2, ANIM_TARGET, 2, 0, 7, 1
	playsewithpan SE_M_VITAL_THROW2, +63
	delay 2
	createsprite gFistFootRandomPosSpriteTemplate, ANIM_TARGET, 3, 1, 10, 0
	createvisualtask AnimTask_ShakeMonInPlace, 2, ANIM_TARGET, 2, 0, 7, 1
	playsewithpan SE_M_MEGA_KICK2, +63
	delay 2
	createsprite gFistFootRandomPosSpriteTemplate, ANIM_TARGET, 3, 1, 10, 0
	createvisualtask AnimTask_ShakeMonInPlace, 2, ANIM_TARGET, 2, 0, 7, 1
	playsewithpan SE_M_COMET_PUNCH, +63
	delay 2
	createsprite gFistFootRandomPosSpriteTemplate, ANIM_TARGET, 3, 1, 10, 0
	createvisualtask AnimTask_ShakeMonInPlace, 2, ANIM_TARGET, 2, 0, 7, 1
	playsewithpan SE_M_VITAL_THROW2, +63
	delay 2
	createsprite gFistFootRandomPosSpriteTemplate, ANIM_TARGET, 3, 1, 10, 0
	createvisualtask AnimTask_ShakeMonInPlace, 2, ANIM_TARGET, 2, 0, 7, 1
	playsewithpan SE_M_MEGA_KICK2, +63
	delay 2
	createsprite gFistFootRandomPosSpriteTemplate, ANIM_TARGET, 3, 1, 10, 0
	createvisualtask AnimTask_ShakeMonInPlace, 2, ANIM_TARGET, 2, 0, 7, 1
	playsewithpan SE_M_COMET_PUNCH, +63
	delay 2
	createsprite gFistFootRandomPosSpriteTemplate, ANIM_TARGET, 3, 1, 10, 0
	createvisualtask AnimTask_ShakeMonInPlace, 2, ANIM_TARGET, 2, 0, 7, 1
	playsewithpan SE_M_VITAL_THROW2, +63
	delay 2
	createsprite gFistFootRandomPosSpriteTemplate, ANIM_TARGET, 3, 1, 10, 0
	createvisualtask AnimTask_ShakeMonInPlace, 2, ANIM_TARGET, 2, 0, 7, 1
	playsewithpan SE_M_MEGA_KICK2, +63
	delay 2
	createsprite gFistFootRandomPosSpriteTemplate, ANIM_TARGET, 3, 1, 10, 0
	createvisualtask AnimTask_ShakeMonInPlace, 2, ANIM_TARGET, 2, 0, 7, 1
	playsewithpan SE_M_COMET_PUNCH, +63
	delay 2
	createsprite gFistFootRandomPosSpriteTemplate, ANIM_TARGET, 3, 1, 10, 0
	createvisualtask AnimTask_ShakeMonInPlace, 2, ANIM_TARGET, 2, 0, 7, 1
	playsewithpan SE_M_VITAL_THROW2, +63
	delay 2
	createsprite gFistFootRandomPosSpriteTemplate, ANIM_TARGET, 3, 1, 10, 0
	createvisualtask AnimTask_ShakeMonInPlace, 2, ANIM_TARGET, 2, 0, 7, 1
	playsewithpan SE_M_MEGA_KICK2, +63
	delay 1
	call G4_UnsetBackground
	clearmonbg ANIM_DEF_PARTNER
	blendoff
	delay 1
	setarg 7, 4096
	delay 1
	end

Move_PAYBACK:
	loadspritegfx ANIM_TAG_IMPACT
	monbg ANIM_TARGET
	fadetobg BG_DARK
	waitbgfadein
	delay 0
	setalpha 12, 8
	createvisualtask AnimTask_ShakeMon2, 5, ANIM_TARGET, 4, 0, 30, 1
	create_basic_hitsplat_sprite ANIM_ATTACKER, 3, x=0, y=0, relative_to=ANIM_TARGET, animation=0
	playsewithpan SE_M_COMET_PUNCH, +63
	delay 5
	create_basic_hitsplat_sprite ANIM_ATTACKER, 3, x=24, y=8, relative_to=ANIM_TARGET, animation=0
	playsewithpan SE_M_COMET_PUNCH, +63
	delay 5
	create_basic_hitsplat_sprite ANIM_ATTACKER, 3, x=-24, y=-16, relative_to=ANIM_TARGET, animation=0
	playsewithpan SE_M_COMET_PUNCH, +63
	delay 5
	create_basic_hitsplat_sprite ANIM_ATTACKER, 3, x=8, y=4, relative_to=ANIM_TARGET, animation=0
	playsewithpan SE_M_COMET_PUNCH, +63
	delay 5
	create_basic_hitsplat_sprite ANIM_ATTACKER, 3, x=-16, y=19, relative_to=ANIM_TARGET, animation=0
	playsewithpan SE_M_COMET_PUNCH, +63
	delay 5
	create_basic_hitsplat_sprite ANIM_ATTACKER, 3, x=18, y=-18, relative_to=ANIM_TARGET, animation=0
	playsewithpan SE_M_COMET_PUNCH, +63
	waitforvisualfinish
	clearmonbg ANIM_TARGET
	blendoff
	restorebg
	waitbgfadein
	end

Move_ASSURANCE:
	goto Move_FAINT_ATTACK

Move_EMBARGO:
	goto Move_TORMENT

Move_FLING:
	loadspritegfx ANIM_TAG_BLACK_BALL
	loadspritegfx ANIM_TAG_IMPACT
	monbg ANIM_DEF_PARTNER
	splitbgprio ANIM_TARGET
	setalpha 12, 8
	createsprite gBlackBallSpriteTemplate, ANIM_TARGET, 2, 20, 0, 0, 0, 35, -25
	playsewithpan SE_M_SWAGGER, -64
	waitforvisualfinish
	create_basic_hitsplat_sprite ANIM_ATTACKER, 2, x=0, y=0, relative_to=ANIM_TARGET, animation=2
	createvisualtask AnimTask_ShakeMon, 2, ANIM_TARGET, 3, 0, 6, 1
	playsewithpan SE_M_VITAL_THROW, +63
	clearmonbg ANIM_DEF_PARTNER
	blendoff
	end

Move_PSYCHO_SHIFT:
	loadspritegfx ANIM_TAG_HOLLOW_ORB
	loadspritegfx ANIM_TAG_THIN_RING
	monbg ANIM_ATTACKER
	setalpha 12, 8
	playsewithpan SE_M_MEGA_KICK, -64
	create_dragon_dance_orb_sprite ANIM_ATTACKER, 2, angle=0/256
	delay 30
	waitforvisualfinish
	createsprite gThinRingShrinkingSpriteTemplate, ANIM_ATTACKER, 40, 0, 0, 1, 0
	playsewithpan SE_M_SUPERSONIC, -64
	delay 14
	createsprite gThinRingShrinkingSpriteTemplate, ANIM_ATTACKER, 40, 0, 0, 1, 0
	playsewithpan SE_M_SUPERSONIC, -64
	delay 14
	createsprite gThinRingShrinkingSpriteTemplate, ANIM_ATTACKER, 40, 0, 0, 1, 0
	playsewithpan SE_M_SUPERSONIC, -64
	waitforvisualfinish
	clearmonbg ANIM_ATK_PARTNER
	end
	clearmonbg ANIM_ATTACKER
	blendoff
	end

Move_TRUMP_CARD:
	goto Move_SWIFT

Move_HEAL_BLOCK:
	goto Move_IMPRISON

Move_WRING_OUT:
	goto Move_CONSTRICT

Move_POWER_TRICK:
	goto Move_SKILL_SWAP

Move_GASTRO_ACID:
	goto Move_SLUDGE

Move_LUCKY_CHANT:
	goto Move_SAFEGUARD

Move_ME_FIRST:
	loadspritegfx ANIM_TAG_FINGER
	loadspritegfx ANIM_TAG_ORBS
	loadspritegfx ANIM_TAG_THOUGHT_BUBBLE
	createvisualtask AnimTask_BlendParticle, 5, ANIM_TAG_ORBS, 0, 12, 12, RGB(15, 29, 31)
	setalpha 11, 5
	monbg_static ANIM_DEF_PARTNER
	splitbgprio_all
	createsprite gThoughtBubbleSpriteTemplate, ANIM_ATTACKER, 11, 0, 100
	playsewithpan SE_M_METRONOME, SOUND_PAN_ATTACKER
	delay 6
	createsprite gMetronomeFingerSpriteTemplate, ANIM_ATTACKER, 12, 0
	delay 24
	loopsewithpan SE_M_TAIL_WHIP, SOUND_PAN_ATTACKER, 22, 3
	waitforvisualfinish
	panse SE_M_MINIMIZE, SOUND_PAN_TARGET, SOUND_PAN_ATTACKER, -3, 0
	shrink_target_copy unk0=128, unk1=24
	delay 15
	create_mimic_orb_sprite ANIM_TARGET, 2, initial_x=-12, initial_y=24
	delay 10
	setarg 7, -1
	waitforvisualfinish
	playsewithpan SE_M_TAKE_DOWN, SOUND_PAN_ATTACKER
	blend_color_cycle selector=F_PAL_ATTACKER, delay=0, num_blends=2, initial_blend_y=0, target_blend_y=11, color=RGB_WHITE
	waitforvisualfinish
	clearmonbg_static ANIM_DEF_PARTNER
	blendoff
	end

Move_COPYCAT:
	loadspritegfx ANIM_TAG_GRAY_ORB
	loadspritegfx ANIM_TAG_THIN_RING
	playsewithpan SE_M_MEGA_KICK, SOUND_PAN_ATTACKER
	blend_color_cycle selector=F_PAL_ATTACKER, delay=8, num_blends=1, initial_blend_y=0, target_blend_y=12, color=RGB_WHITE
	createvisualtask AnimTask_StockpileDeformMon, 5
	call G4_StockpileAbsorb
	call G4_StockpileAbsorb
	waitforvisualfinish
	simple_palette_blend selector=F_PAL_ATTACKER, delay=0, initial_blend_y=12, target_blend_y=0, color=RGB_WHITE
	waitforvisualfinish
	createsprite gThinRingShrinkingSpriteTemplate, ANIM_ATTACKER, 40, 0, 0, 0, 0
	playsewithpan SE_M_SUPERSONIC, -64
	delay 14
	createsprite gThinRingShrinkingSpriteTemplate, ANIM_ATTACKER, 40, 0, 0, 0, 0
	playsewithpan SE_M_SUPERSONIC, -64
	delay 14
	createsprite gThinRingShrinkingSpriteTemplate, ANIM_ATTACKER, 40, 0, 0, 0, 0
	playsewithpan SE_M_SUPERSONIC, -64
	waitforvisualfinish
	end

Move_POWER_SWAP:
	goto Move_SKILL_SWAP

Move_GUARD_SWAP:
	goto Move_SKILL_SWAP

Move_PUNISHMENT:
	goto Move_FAINT_ATTACK

Move_LAST_RESORT:
	loadspritegfx ANIM_TAG_IMPACT
	fadetobg BG_COSMIC
	waitbgfadeout
	createvisualtask AnimTask_StartSlidingBg, 2, 0, 128, 0, -1
	waitbgfadein
	playsewithpan SE_M_SWIFT, SOUND_PAN_ATTACKER
	complex_palette_blend selector=F_PAL_ATTACKER, delay=4, num_blends=2, color1=RGB_WHITE, blend_y1=10, color2=RGB_BLACK, blend_y2=0
	waitforvisualfinish
	delay 10
	playsewithpan SE_M_SWAGGER, SOUND_PAN_ATTACKER
	waitplaysewithpan SE_M_SWAGGER, SOUND_PAN_ATTACKER, 8
	createvisualtask AnimTask_TranslateMonEllipticalRespectSide, 2, ANIM_ATTACKER, 18, 6, 2, 4
	waitforvisualfinish
	simple_palette_blend selector=F_PAL_BG, delay=0, initial_blend_y=16, target_blend_y=16, color=RGB_WHITE
	createsprite gSlideMonToOffsetSpriteTemplate, ANIM_ATTACKER, 2, ANIM_ATTACKER, 20, 0, 0, 4
	delay 3
	waitforvisualfinish
	playsewithpan SE_M_MEGA_KICK2, SOUND_PAN_TARGET
	create_basic_hitsplat_sprite ANIM_TARGET, 132, x=-10, y=0, relative_to=ANIM_TARGET, animation=0
	createsprite gSlideMonToOffsetSpriteTemplate, ANIM_TARGET, 2, ANIM_TARGET, -32, 0, 0, 3
	waitforvisualfinish
	createvisualtask AnimTask_RotateMonSpriteToSide, 2, 8, -256, ANIM_ATTACKER, 1
	createvisualtask AnimTask_RotateMonSpriteToSide, 2, 8, -256, ANIM_TARGET, 1
	createvisualtask AnimTask_ShakeMonInPlace, 2, ANIM_ATTACKER, 4, 0, 12, 1
	createvisualtask AnimTask_ShakeMonInPlace, 2, ANIM_TARGET, 4, 0, 12, 1
	simple_palette_blend selector=F_PAL_BG, delay=2, initial_blend_y=16, target_blend_y=0, color=RGB_WHITE
	waitforvisualfinish
	createvisualtask AnimTask_RotateMonSpriteToSide, 2, 8, -256, ANIM_ATTACKER, 1
	createvisualtask AnimTask_RotateMonSpriteToSide, 2, 8, -256, ANIM_TARGET, 1
	waitforvisualfinish
	createsprite gSlideMonToOriginalPosSpriteTemplate, ANIM_ATTACKER, 2, 0, 0, 5
	delay 3
	createsprite gSlideMonToOriginalPosSpriteTemplate, ANIM_TARGET, 2, 1, 0, 7
	waitforvisualfinish
	call G4_UnsetBackground
	end

Move_WORRY_SEED:
	goto Move_LEECH_SEED

Move_SUCKER_PUNCH:
	goto Move_FAINT_ATTACK

Move_TOXIC_SPIKES:
	goto Move_SPIKES

Move_HEART_SWAP:
	goto Move_SKILL_SWAP

Move_AQUA_RING:


Move_MAGNET_RISE:
	loadspritegfx ANIM_TAG_SPARK_2
	delay 0
	blend_color_cycle selector=(F_PAL_BG | F_PAL_BATTLERS), delay=0, num_blends=1, initial_blend_y=5, target_blend_y=5, color=RGB(31, 31, 20)
	playsewithpan SE_M_THUNDERBOLT2, SOUND_PAN_ATTACKER
	createvisualtask AnimTask_WindUpLunge, 5, ANIM_ATTACKER, -12, 4, 10, 10, 12, 6
	createsprite gSparkElectricitySpriteTemplate, ANIM_ATTACKER, 0, 32, 24, 190, 12, ANIM_ATTACKER, 1, 0
	delay 0
	createsprite gSparkElectricitySpriteTemplate, ANIM_ATTACKER, 0, 80, 24, 22, 12, ANIM_ATTACKER, 1, 0
	createsprite gSparkElectricitySpriteTemplate, ANIM_ATTACKER, 0, 156, 24, 121, 13, ANIM_ATTACKER, 1, 1
	delay 0
	playsewithpan SE_M_THUNDERBOLT2, SOUND_PAN_ATTACKER
	delay 4
	createsprite gSparkElectricitySpriteTemplate, ANIM_ATTACKER, 0, 100, 24, 60, 10, ANIM_ATTACKER, 1, 0
	createsprite gSparkElectricitySpriteTemplate, ANIM_ATTACKER, 0, 170, 24, 42, 11, ANIM_ATTACKER, 1, 1
	delay 0
	createsprite gSparkElectricitySpriteTemplate, ANIM_ATTACKER, 0, 238, 24, 165, 10, ANIM_ATTACKER, 1, 1
	delay 0
	createsprite gSparkElectricitySpriteTemplate, ANIM_ATTACKER, 0, 32, 24, 190, 12, ANIM_ATTACKER, 1, 0
	delay 0
	createsprite gSparkElectricitySpriteTemplate, ANIM_ATTACKER, 0, 80, 24, 22, 12, ANIM_ATTACKER, 1, 0
	createsprite gSparkElectricitySpriteTemplate, ANIM_ATTACKER, 0, 156, 24, 121, 13, ANIM_ATTACKER, 1, 1
	delay 0
	createsprite gSparkElectricitySpriteTemplate, ANIM_ATTACKER, 0, 100, 24, 60, 10, ANIM_ATTACKER, 1, 0
	createsprite gSparkElectricitySpriteTemplate, ANIM_ATTACKER, 0, 170, 24, 42, 11, ANIM_ATTACKER, 1, 1
	delay 0
	createsprite gSparkElectricitySpriteTemplate, ANIM_ATTACKER, 0, 238, 24, 165, 10, ANIM_ATTACKER, 1, 1
	delay 0
	blend_color_cycle selector=(F_PAL_BG | F_PAL_BATTLERS), delay=0, num_blends=1, initial_blend_y=0, target_blend_y=0, color=RGB(31, 31, 20)
	delay 20
	blend_color_cycle selector=(F_PAL_BG | F_PAL_BATTLERS), delay=0, num_blends=1, initial_blend_y=7, target_blend_y=7, color=RGB(31, 31, 20)
	waitforvisualfinish
	blend_color_cycle selector=(F_PAL_BG | F_PAL_BATTLERS), delay=0, num_blends=1, initial_blend_y=0, target_blend_y=0, color=RGB(31, 31, 20)
	waitforvisualfinish
	end

Move_FLARE_BLITZ:
	loadspritegfx ANIM_TAG_IMPACT
	loadspritegfx ANIM_TAG_SMALL_EMBER
	monbg ANIM_TARGET
	splitbgprio_foes ANIM_TARGET
	setalpha 12, 8
	createsprite gFireSpiralOutwardSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, 56, 0
	playsewithpan SE_M_FLAME_WHEEL, SOUND_PAN_ATTACKER
	delay 2
	createsprite gFireSpiralOutwardSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, 56, 2
	playsewithpan SE_M_FLAME_WHEEL, SOUND_PAN_ATTACKER
	delay 2
	createsprite gFireSpiralOutwardSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, 56, 4
	playsewithpan SE_M_FLAME_WHEEL, SOUND_PAN_ATTACKER
	delay 2
	createsprite gFireSpiralOutwardSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, 56, 6
	playsewithpan SE_M_FLAME_WHEEL, SOUND_PAN_ATTACKER
	delay 2
	createsprite gFireSpiralOutwardSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, 56, 8
	playsewithpan SE_M_FLAME_WHEEL, SOUND_PAN_ATTACKER
	delay 2
	createsprite gFireSpiralOutwardSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, 56, 10
	playsewithpan SE_M_FLAME_WHEEL, SOUND_PAN_ATTACKER
	delay 2
	createsprite gFireSpiralOutwardSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, 56, 12
	playsewithpan SE_M_FLAME_WHEEL, SOUND_PAN_ATTACKER
	delay 2
	waitforvisualfinish
	createvisualtask AnimTask_TranslateMonEllipticalRespectSide, 2, ANIM_ATTACKER, 12, 4, 1, 4
	waitforvisualfinish
	createsprite gSlideMonToOffsetSpriteTemplate, ANIM_ATTACKER, 2, ANIM_ATTACKER, 24, 0, 0, 6
	waitforvisualfinish
	call G4_SetImpactBackground
	createvisualtask AnimTask_ShakeMon2, 2, ANIM_TARGET, 4, 0, 22, 1
	createvisualtask AnimTask_BlendMonInAndOut, 3, ANIM_TARGET, RGB_RED, 12, 1, 1
	create_basic_hitsplat_sprite ANIM_ATTACKER, 3, x=0, y=0, relative_to=ANIM_TARGET, animation=0
	createsprite gFireSpreadSpriteTemplate, ANIM_TARGET, 1, 0, 10, 192, 176, 40
	playsewithpan SE_M_COMET_PUNCH, +63
	delay 5
	create_basic_hitsplat_sprite ANIM_ATTACKER, 3, x=24, y=8, relative_to=ANIM_TARGET, animation=0
	createvisualtask AnimTask_BlendMonInAndOut, 3, ANIM_TARGET, RGB_RED, 12, 1, 1
	createsprite gFireSpreadSpriteTemplate, ANIM_TARGET, 1, 0, 10, -192, 240, 40
	playsewithpan SE_M_COMET_PUNCH, +63
	delay 5
	create_basic_hitsplat_sprite ANIM_ATTACKER, 3, x=-24, y=-16, relative_to=ANIM_TARGET, animation=0
	createvisualtask AnimTask_BlendMonInAndOut, 3, ANIM_TARGET, RGB_RED, 12, 1, 1
	createsprite gFireSpreadSpriteTemplate, ANIM_TARGET, 1, 0, 10, 192, -160, 40
	playsewithpan SE_M_COMET_PUNCH, +63
	delay 5
	create_basic_hitsplat_sprite ANIM_ATTACKER, 3, x=8, y=4, relative_to=ANIM_TARGET, animation=0
	createvisualtask AnimTask_BlendMonInAndOut, 3, ANIM_TARGET, RGB_RED, 12, 1, 1
	createsprite gFireSpreadSpriteTemplate, ANIM_TARGET, 1, 0, 10, -192, -112, 40
	playsewithpan SE_M_COMET_PUNCH, +63
	delay 5
	create_basic_hitsplat_sprite ANIM_ATTACKER, 3, x=-16, y=19, relative_to=ANIM_TARGET, animation=0
	createvisualtask AnimTask_BlendMonInAndOut, 3, ANIM_TARGET, RGB_RED, 12, 1, 1
	createsprite gFireSpreadSpriteTemplate, ANIM_TARGET, 1, 0, 10, 160, 48, 40
	playsewithpan SE_M_COMET_PUNCH, +63
	delay 5
	create_basic_hitsplat_sprite ANIM_ATTACKER, 3, x=18, y=-18, relative_to=ANIM_TARGET, animation=0
	createvisualtask AnimTask_BlendMonInAndOut, 3, ANIM_TARGET, RGB_RED, 12, 1, 1
	createsprite gFireSpreadSpriteTemplate, ANIM_TARGET, 1, 0, 10, -224, -32, 40
	createsprite gFireSpreadSpriteTemplate, ANIM_TARGET, 1, 0, 10, 112, -128, 40
	playsewithpan SE_M_COMET_PUNCH, +63
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_TARGET, 2, 0, 0, RGB_BLACK
	playsewithpan SE_M_COMET_PUNCH, +63
	waitforvisualfinish
	createsprite gSlideMonToOriginalPosSpriteTemplate, ANIM_ATTACKER, 2, 0, 0, 9
	waitforvisualfinish
	clearmonbg ANIM_TARGET
	blendoff
	delay 2
	restorebg
	waitbgfadein
	end

Move_FORCE_PALM:
	goto Move_ARM_THRUST

Move_AURA_SPHERE:
	goto Move_SHADOW_BALL

Move_ROCK_POLISH:
	goto Move_AGILITY

Move_POISON_JAB:
	goto Move_POISON_FANG

Move_DARK_PULSE:
	goto Move_NIGHT_SHADE

Move_NIGHT_SLASH:
	goto Move_SLASH

Move_AQUA_TAIL:
	goto Move_CRABHAMMER

Move_SEED_BOMB:
	loadspritegfx ANIM_TAG_EXPLOSION
	loadspritegfx ANIM_TAG_SEED
	createsprite gBulletSeedSpriteTemplate, ANIM_TARGET, 2, 20, 0
	delay 5
	createsprite gBulletSeedSpriteTemplate, ANIM_TARGET, 2, 20, 0
	delay 5
	createsprite gBulletSeedSpriteTemplate, ANIM_TARGET, 2, 20, 0
	delay 5
	createvisualtask AnimTask_ShakeMon2, 2, ANIM_TARGET, 2, 0, 30, 1
	createsprite gBulletSeedSpriteTemplate, ANIM_TARGET, 2, 20, 0
	playsewithpan SE_M_SELF_DESTRUCT, SOUND_PAN_TARGET
	createsprite gExplosionSpriteTemplate, ANIM_TARGET, 4, 6, 5, 1, 0
	delay 6
	createsprite gBulletSeedSpriteTemplate, ANIM_TARGET, 2, 20, 0
	playsewithpan SE_M_SELF_DESTRUCT, SOUND_PAN_TARGET
	createsprite gExplosionSpriteTemplate, ANIM_TARGET, 4, -16, -15, 1, 0
	delay 6
	createsprite gBulletSeedSpriteTemplate, ANIM_TARGET, 2, 20, 0
	playsewithpan SE_M_SELF_DESTRUCT, SOUND_PAN_TARGET
	createsprite gExplosionSpriteTemplate, ANIM_TARGET, 4, 16, -5, 1, 0
	delay 6
	createsprite gBulletSeedSpriteTemplate, ANIM_TARGET, 2, 20, 0
	playsewithpan SE_M_SELF_DESTRUCT, SOUND_PAN_TARGET
	createsprite gExplosionSpriteTemplate, ANIM_TARGET, 4, -12, 18, 1, 0
	delay 6
	createsprite gBulletSeedSpriteTemplate, ANIM_TARGET, 2, 20, 0
	playsewithpan SE_M_SELF_DESTRUCT, SOUND_PAN_TARGET
	createsprite gExplosionSpriteTemplate, ANIM_TARGET, 4, 0, 5, 1, 0
	delay 6
	createsprite gBulletSeedSpriteTemplate, ANIM_TARGET, 2, 20, 0
	playsewithpan SE_M_SELF_DESTRUCT, SOUND_PAN_TARGET
	createsprite gExplosionSpriteTemplate, ANIM_TARGET, 4, 6, 5, 1, 0
	delay 6
	createsprite gBulletSeedSpriteTemplate, ANIM_TARGET, 2, 20, 0
	playsewithpan SE_M_SELF_DESTRUCT, SOUND_PAN_TARGET
	createsprite gExplosionSpriteTemplate, ANIM_TARGET, 4, -16, -15, 1, 0
	delay 6
	createsprite gExplosionSpriteTemplate, ANIM_TARGET, 4, 16, -5, 1, 0
	playsewithpan SE_M_SELF_DESTRUCT, SOUND_PAN_TARGET
	delay 6
	playsewithpan SE_M_SELF_DESTRUCT, SOUND_PAN_TARGET
	createsprite gExplosionSpriteTemplate, ANIM_TARGET, 4, -12, 18, 1, 0
	delay 6
	playsewithpan SE_M_SELF_DESTRUCT, SOUND_PAN_TARGET
	createsprite gExplosionSpriteTemplate, ANIM_TARGET, 4, 0, 5, 1, 0
	waitforvisualfinish
	end

Move_AIR_SLASH:
	loadspritegfx ANIM_TAG_SLASH
	call G4_SetSkyBg
	createsprite gSlashSliceSpriteTemplate, ANIM_TARGET, 2, 1, -8, 0
	playsewithpan SE_M_RAZOR_WIND, SOUND_PAN_TARGET
	delay 4
	createsprite gSlashSliceSpriteTemplate, ANIM_TARGET, 2, 1, 8, 0
	createvisualtask AnimTask_ShakeMon2, 2, ANIM_TARGET, 4, 0, 18, 1
	playsewithpan SE_M_RAZOR_WIND, SOUND_PAN_TARGET
	waitforvisualfinish
	call G4_UnsetBackground
	end

Move_X_SCISSOR:
	loadspritegfx ANIM_TAG_CUT
	monbg ANIM_TARGET
	setalpha 12, 8
	createvisualtask AnimTask_TranslateMonEllipticalRespectSide, 2, ANIM_ATTACKER, 24, 6, 1, 5
	createvisualtask AnimTask_TraceMonBlended, 2, 0, 4, 7, 3
	createsprite gCuttingSliceSpriteTemplate, ANIM_ATTACKER, 2, 40, -32, 0
	createsprite gCuttingSliceSpriteTemplate, ANIM_ATTACKER, 2, 40, -32, 1
	playsewithpan SE_M_RAZOR_WIND2, SOUND_PAN_ATTACKER
	delay 5
	createvisualtask AnimTask_ShakeMon, 2, ANIM_TARGET, 0, 3, 10, 1
	complex_palette_blend selector=F_PAL_BG | F_PAL_BATTLERS, delay=3, num_blends=1, color1=RGB_BLACK, blend_y1=10, color2=RGB_BLACK, blend_y2=0
	playsewithpan SE_M_RAZOR_WIND, SOUND_PAN_TARGET
	waitforvisualfinish
	clearmonbg ANIM_TARGET
	blendoff
	end

Move_BUG_BUZZ:
	loadspritegfx ANIM_TAG_JAGGED_MUSIC_NOTE
	loadspritegfx ANIM_TAG_THIN_RING
	monbg ANIM_DEF_PARTNER
	call G4_SetBugBg
	createvisualtask AnimTask_UproarDistortion, 2, 0
	createsprite gUproarRingSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, 0, 0, RGB_RED, 8
	playsewithpan SE_M_UPROAR, SOUND_PAN_ATTACKER
	createsprite gJaggedMusicNoteSpriteTemplate, ANIM_ATTACKER, 2, 0, 29, -12, 0
	createsprite gJaggedMusicNoteSpriteTemplate, ANIM_ATTACKER, 2, 0, -12, -29, 1
	delay 16
	createvisualtask AnimTask_UproarDistortion, 2, 0
	createsprite gUproarRingSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, 0, 0, RGB_RED, 8
	playsewithpan SE_M_UPROAR, SOUND_PAN_ATTACKER
	createsprite gJaggedMusicNoteSpriteTemplate, ANIM_ATTACKER, 2, 0, 12, -29, 1
	createsprite gJaggedMusicNoteSpriteTemplate, ANIM_ATTACKER, 2, 0, -29, -12, 0
	delay 16
	createvisualtask AnimTask_UproarDistortion, 2, 0
	createsprite gUproarRingSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, 0, 0, RGB_RED, 8
	playsewithpan SE_M_UPROAR, SOUND_PAN_ATTACKER
	createsprite gJaggedMusicNoteSpriteTemplate, ANIM_ATTACKER, 2, 0, 24, -24, 1
	createsprite gJaggedMusicNoteSpriteTemplate, ANIM_ATTACKER, 2, 0, -24, -24, 0
	waitforvisualfinish
	clearmonbg ANIM_DEF_PARTNER
	call G4_UnsetBugBg
	end

Move_DRAGON_PULSE:
	goto Move_DRAGON_BREATH

Move_DRAGON_RUSH:
	goto Move_DRAGON_CLAW

Move_POWER_GEM:
	goto Move_ANCIENT_POWER

Move_DRAIN_PUNCH:
	loadspritegfx ANIM_TAG_BLUE_STAR
	loadspritegfx ANIM_TAG_HANDS_AND_FEET
	loadspritegfx ANIM_TAG_IMPACT
	loadspritegfx ANIM_TAG_ORBS
	monbg ANIM_DEF_PARTNER
	setalpha 12, 8
	playsewithpan SE_M_VITAL_THROW2, -64
	create_fist_sprite ANIM_TARGET, 3, x=0, y=0, duration=8, initPosition=1
	create_basic_hitsplat_sprite ANIM_TARGET, 2, x=0, y=0, relative_to=ANIM_TARGET, animation=1
	createvisualtask AnimTask_ShakeMon, 2, ANIM_TARGET, 0, 3, 15, 1
	delay 20
	call G4_GigaDrainAbsorbEffect
	waitforvisualfinish
	delay 15
	call G4_HealingEffect
	waitforvisualfinish
	clearmonbg ANIM_DEF_PARTNER
	blendoff
	end

Move_VACUUM_WAVE:
	loadspritegfx ANIM_TAG_HANDS_AND_FEET
	loadspritegfx ANIM_TAG_IMPACT
	loadspritegfx ANIM_TAG_THIN_RING
	monbg ANIM_TARGET
	setalpha 12, 8
	create_basic_hitsplat_sprite ANIM_ATTACKER, 2, x=8, y=-8, relative_to=ANIM_TARGET, animation=2
	create_fist_sprite ANIM_ATTACKER, 3, x=8, y=0, duration=8, initPosition=1
	createsprite gUproarRingSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, 1, 0, RGB_WHITE, 3
	createvisualtask AnimTask_ShakeMon, 2, ANIM_TARGET, 3, 0, 6, 1
	playsewithpan SE_M_COMET_PUNCH, SOUND_PAN_TARGET
	waitforvisualfinish
	clearmonbg ANIM_TARGET
	blendoff
	end

Move_FOCUS_BLAST:
	goto Move_FOCUS_PUNCH

Move_ENERGY_BALL:
	goto Move_MAGICAL_LEAF

Move_BRAVE_BIRD:
	loadspritegfx ANIM_TAG_BIRD
	call G4_SetSkyBg
	monbg ANIM_ATTACKER
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_ATTACKER, 0, 0, 16, RGB_WHITE
	delay 4
	attacker_fade_to_invisible priority=5, step_delay=0
	waitforvisualfinish
	createvisualtask SoundTask_PlaySE2WithPanning, 5, 238, SOUND_PAN_ATTACKER
	createsprite gSkyAttackBirdSpriteTemplate, ANIM_TARGET, 2
	delay 14
	createvisualtask AnimTask_ShakeMon2, 2, ANIM_TARGET, 10, 0, 18, 1
	createvisualtask SoundTask_PlaySE1WithPanning, 5, 141, SOUND_PAN_TARGET
	delay 20
	attacker_fade_from_invisible priority=5, step_delay=1
	delay 2
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_ATTACKER, 0, 15, 0, RGB_WHITE
	waitforvisualfinish
	clearmonbg ANIM_ATTACKER
	call G4_UnsetBackground
	end

Move_EARTH_POWER:
	loadspritegfx ANIM_TAG_FIRE_PLUME
	createvisualtask AnimTask_HorizontalShake, 3, ANIM_TARGET, 10, 50
	playsewithpan SE_M_EARTHQUAKE, SOUND_PAN_TARGET
	delay 32
	fadetobg BG_FISSURE
	waitbgfadeout
	createvisualtask AnimTask_PositionFissureBgOnBattler, 5, ANIM_TARGET, 5, -1
	waitbgfadein
	loopsewithpan SE_M_FLAME_WHEEL2, SOUND_PAN_TARGET 11, 3
	createvisualtask AnimTask_ShakeMon, 5, ANIM_TARGET, 0, 3, 25, 1
	call G4_CreateDragonRageFirePlumes
	waitforvisualfinish
	createsprite gSlideMonToOriginalPosSpriteTemplate, ANIM_TARGET, 66, 0, 0, 4
	waitforvisualfinish
	call G4_UnsetBackground
	end

Move_SWITCHEROO:
	goto Move_TRICK

Move_GIGA_IMPACT:
	goto Move_DOUBLE_EDGE

Move_NASTY_PLOT:
	loadspritegfx ANIM_TAG_AMNESIA
	fadetobg BG_DARK
	waitbgfadeout
	createvisualtask AnimTask_FadeScreenToWhite, 5
	waitbgfadein
	delay 8
	createsprite gQuestionMarkSpriteTemplate, ANIM_ATTACKER, 20, 0
	playsewithpan SE_M_METRONOME, SOUND_PAN_ATTACKER
	delay 54
	loopsewithpan SE_M_METRONOME, SOUND_PAN_ATTACKER, 16, 3
	waitforvisualfinish
	call G4_UnsetBackground
	end

Move_BULLET_PUNCH:
	loadspritegfx ANIM_TAG_HANDS_AND_FEET
	loadspritegfx ANIM_TAG_IMPACT
	monbg ANIM_TARGET
	setalpha 12, 8
	create_fist_sprite ANIM_ATTACKER, 3, x=-8, y=0, duration=8, initPosition=1
	create_basic_hitsplat_sprite ANIM_ATTACKER, 2, x=-8, y=0, relative_to=ANIM_TARGET, animation=2
	playsewithpan SE_M_COMET_PUNCH, SOUND_PAN_TARGET
	delay 2
	create_fist_sprite ANIM_ATTACKER, 3, x=-16, y=8, duration=8, initPosition=1
	create_basic_hitsplat_sprite ANIM_ATTACKER, 2, x=-16, y=8, relative_to=ANIM_TARGET, animation=2
	playsewithpan SE_M_COMET_PUNCH, SOUND_PAN_TARGET
	delay 2
	create_fist_sprite ANIM_ATTACKER, 3, x=8, y=-16, duration=8, initPosition=1
	create_basic_hitsplat_sprite ANIM_ATTACKER, 2, x=-16, y=8, relative_to=ANIM_TARGET, animation=2
	playsewithpan SE_M_COMET_PUNCH, SOUND_PAN_TARGET
	delay 2
	create_fist_sprite ANIM_ATTACKER, 3, x=16, y=16, duration=8, initPosition=1
	create_basic_hitsplat_sprite ANIM_ATTACKER, 2, x=16, y=8, relative_to=ANIM_TARGET, animation=2
	playsewithpan SE_M_COMET_PUNCH, SOUND_PAN_TARGET
	delay 2
	create_fist_sprite ANIM_ATTACKER, 3, x=0, y=0, duration=8, initPosition=1
	create_basic_hitsplat_sprite ANIM_ATTACKER, 2, x=0, y=0, relative_to=ANIM_TARGET, animation=2
	playsewithpan SE_M_COMET_PUNCH, SOUND_PAN_TARGET
	delay 2
	create_fist_sprite ANIM_ATTACKER, 3, x=24, y=0, duration=8, initPosition=1
	create_basic_hitsplat_sprite ANIM_ATTACKER, 2, x=24, y=0, relative_to=ANIM_TARGET, animation=2
	playsewithpan SE_M_COMET_PUNCH, SOUND_PAN_TARGET
	delay 2
	create_fist_sprite ANIM_ATTACKER, 3, x=24, y=16, duration=8, initPosition=1
	create_basic_hitsplat_sprite ANIM_ATTACKER, 2, x=24, y=16, relative_to=ANIM_TARGET, animation=2
	playsewithpan SE_M_COMET_PUNCH, SOUND_PAN_TARGET
	delay 2
	create_fist_sprite ANIM_ATTACKER, 3, x=-24, y=16, duration=8, initPosition=1
	create_basic_hitsplat_sprite ANIM_ATTACKER, 2, x=-24, y=16, relative_to=ANIM_TARGET, animation=2
	playsewithpan SE_M_COMET_PUNCH, SOUND_PAN_TARGET
	delay 2
	create_fist_sprite ANIM_ATTACKER, 3, x=-24, y=-8, duration=8, initPosition=1
	create_basic_hitsplat_sprite ANIM_ATTACKER, 2, x=-24, y=-8, relative_to=ANIM_TARGET, animation=2
	playsewithpan SE_M_COMET_PUNCH, SOUND_PAN_TARGET
	delay 2
	create_fist_sprite ANIM_ATTACKER, 3, x=-24, y=8, duration=8, initPosition=1
	create_basic_hitsplat_sprite ANIM_ATTACKER, 2, x=-24, y=8, relative_to=ANIM_TARGET, animation=2
	playsewithpan SE_M_COMET_PUNCH, SOUND_PAN_TARGET
	delay 2
	waitforvisualfinish
	createvisualtask AnimTask_ShakeMon, 2, ANIM_TARGET, 9, 0, 8, 1
	playsewithpan SE_M_COMET_PUNCH, SOUND_PAN_TARGET
	waitforvisualfinish

	clearmonbg ANIM_TARGET
	blendoff
	end

Move_AVALANCHE:
	goto Move_ICE_BALL

Move_ICE_SHARD:
	goto Move_ICICLE_SPEAR

Move_SHADOW_CLAW:
	loadspritegfx ANIM_TAG_CLAW_SLASH
	fadetobg BG_GHOST
	waitbgfadein
	monbg ANIM_TARGET
	setalpha 12, 8
	createsprite gHorizontalLungeSpriteTemplate ANIM_ATTACKER, 2, 6, 4
	delay 4
	createvisualtask AnimTask_ShakeMon2, 2, ANIM_TARGET, 2, 0, 18, 1
	call G4_CreateClawSlashes1
	playsewithpan SE_M_RAZOR_WIND, SOUND_PAN_TARGET
	delay 12
	call G4_CreateClawSlashes2
	playsewithpan SE_M_RAZOR_WIND, SOUND_PAN_TARGET
	waitforvisualfinish
	restorebg
	waitbgfadein
	waitforvisualfinish
	blendoff
	clearmonbg ANIM_TARGET
	end

Move_THUNDER_FANG:
	loadspritegfx ANIM_TAG_IMPACT
	loadspritegfx ANIM_TAG_LIGHTNING
	loadspritegfx ANIM_TAG_SHARP_TEETH
	monbg ANIM_TARGET
	setalpha 12, 8
	simple_palette_blend selector=F_PAL_BG, delay=2, initial_blend_y=0, target_blend_y=16, color=RGB_BLACK
	waitforvisualfinish
	invert_screen_color scenery=1, attacker=1, target=1
	delay 1
	createsprite gLightningSpriteTemplate, ANIM_TARGET, 2, 0, -48
	delay 1
	createsprite gLightningSpriteTemplate, ANIM_ATTACKER, 2, 0, -16
	delay 1
	createsprite gLightningSpriteTemplate, ANIM_ATTACKER, 2, 0, 16
	delay 1
	call G4_CreateBite
	delay 1
	playsewithpan SE_M_TRI_ATTACK2, SOUND_PAN_TARGET
	invert_screen_color scenery=1, attacker=1, target=1
	delay 1
	createvisualtask AnimTask_ShakeMon, 2, ANIM_TARGET, 0, 3, 15, 1
	create_basic_hitsplat_sprite ANIM_ATTACKER, 3, x=0, y=0, relative_to=ANIM_TARGET, animation=2
	delay 1
	simple_palette_blend selector=F_PAL_BG, delay=2, initial_blend_y=16, target_blend_y=0, color=RGB_BLACK
	delay 20
	waitforvisualfinish
	clearmonbg ANIM_TARGET
	blendoff
	delay 1
	end

Move_ICE_FANG:
	loadspritegfx ANIM_TAG_ICE_CRYSTALS
	loadspritegfx ANIM_TAG_IMPACT
	loadspritegfx ANIM_TAG_SHARP_TEETH
	monbg ANIM_TARGET
	setalpha 12, 8
	simple_palette_blend selector=F_PAL_BG, delay=1, initial_blend_y=0, target_blend_y=7, color=RGB_BLACK
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_TARGET, 2, 0, 9, RGB(12, 26, 31)
	delay 20
	call G4_CreateIceSpiral
	call G4_CreateBite
	waitforvisualfinish
	create_basic_hitsplat_sprite ANIM_ATTACKER, 2, x=0, y=0, relative_to=ANIM_TARGET, animation=2
	createvisualtask AnimTask_ShakeMon, 5, ANIM_TARGET, 0, 4, 7, 1
	delay 15
	call G4_IceCrystalEffectShort
	delay 5
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_TARGET, 2, 9, 0, RGB(12, 26, 31)
	waitforvisualfinish
	simple_palette_blend selector=F_PAL_BG, delay=0, initial_blend_y=7, target_blend_y=0, color=RGB_BLACK
	waitforvisualfinish
	clearmonbg ANIM_TARGET
	blendoff
	delay 1
	end

Move_FIRE_FANG:
	loadspritegfx ANIM_TAG_IMPACT
	loadspritegfx ANIM_TAG_SHARP_TEETH
	loadspritegfx ANIM_TAG_SMALL_EMBER
	monbg ANIM_TARGET
	setalpha 12, 8
	call G4_CreateFireSpiral
	call G4_CreateBite
	delay 10
	waitforvisualfinish
	create_basic_hitsplat_sprite ANIM_ATTACKER, 2, x=0, y=0, relative_to=ANIM_TARGET, animation=2
	createvisualtask AnimTask_ShakeMon, 5, ANIM_TARGET, 0, 4, 7, 1
	call G4_FireSpreadEffect
	delay 4
	playsewithpan SE_M_FIRE_PUNCH, SOUND_PAN_TARGET
	waitforvisualfinish
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_TARGET, 0, 9, 0, RGB_RED
	waitforvisualfinish
	clearmonbg ANIM_TARGET
	blendoff
	delay 1
	end

Move_SHADOW_SNEAK:
	goto Move_ASTONISH

Move_MUD_BOMB:
	goto Move_MUD_SHOT

Move_PSYCHO_CUT:
	goto Move_SLASH

Move_ZEN_HEADBUTT:
	goto Move_HEADBUTT

Move_MIRROR_SHOT:
	loadspritegfx ANIM_TAG_IMPACT
	loopsewithpan SE_M_HARDEN, SOUND_PAN_ATTACKER, 28, 2
	metallic_shine permanent=1
	waitforvisualfinish
	createvisualtask AnimTask_BlendBattleAnimPalExclude, 5, 5, 2, 0, 10, RGB_WHITEALPHA
	createvisualtask AnimTask_BlendParticle, 5, ANIM_TAG_IMPACT, 0, 12, 12, RGB(21, 21, 21)
	waitforvisualfinish
	create_random_pos_hitsplat_sprite ANIM_TARGET, 3, relative_to=ANIM_TARGET, animation=2
	createvisualtask SoundTask_PlaySE1WithPanning, 5, SE_M_VITAL_THROW2, SOUND_PAN_TARGET
	delay 3
	create_random_pos_hitsplat_sprite ANIM_TARGET, 3, relative_to=ANIM_TARGET, animation=2
	createvisualtask SoundTask_PlaySE1WithPanning, 5, SE_M_VITAL_THROW2, SOUND_PAN_TARGET
	delay 3
	create_random_pos_hitsplat_sprite ANIM_TARGET, 3, relative_to=ANIM_TARGET, animation=2
	createvisualtask SoundTask_PlaySE1WithPanning, 5, SE_M_VITAL_THROW2, SOUND_PAN_TARGET
	delay 3
	create_random_pos_hitsplat_sprite ANIM_TARGET, 3, relative_to=ANIM_TARGET, animation=2
	createvisualtask SoundTask_PlaySE1WithPanning, 5, SE_M_VITAL_THROW2, SOUND_PAN_TARGET
	delay 3
	create_random_pos_hitsplat_sprite ANIM_TARGET, 3, relative_to=ANIM_TARGET, animation=2
	createvisualtask SoundTask_PlaySE1WithPanning, 5, SE_M_VITAL_THROW2, SOUND_PAN_TARGET
	delay 3
	create_random_pos_hitsplat_sprite ANIM_TARGET, 3, relative_to=ANIM_TARGET, animation=2
	createvisualtask SoundTask_PlaySE1WithPanning, 5, SE_M_VITAL_THROW2, SOUND_PAN_TARGET
	waitforvisualfinish
	createvisualtask AnimTask_BlendBattleAnimPalExclude, 5, 5, 2, 10, 0, RGB_WHITEALPHA
	createvisualtask AnimTask_HorizontalShake, 5, ANIM_TARGET, 5, 14
	waitforvisualfinish
	blendoff
	end

Move_FLASH_CANNON:
	goto Move_HYPER_BEAM

Move_ROCK_CLIMB:
	loadspritegfx ANIM_TAG_IMPACT
	delay 8
	loopsewithpan SE_M_TAIL_WHIP, SOUND_PAN_ATTACKER, 38, 3
	waitforvisualfinish
	monbg ANIM_DEF_PARTNER
	splitbgprio ANIM_TARGET
	setalpha 12, 8
	createvisualtask AnimTask_Rollout, 2, 1
	waitforvisualfinish
	createvisualtask AnimTask_ShakeTargetBasedOnMovePowerOrDmg, 2, 0, 1, 30, 1, 0
	create_basic_hitsplat_sprite ANIM_TARGET, 3, x=-15, y=8, relative_to=ANIM_TARGET, animation=1
	playsewithpan SE_M_VITAL_THROW2, SOUND_PAN_TARGET
	delay 1
	playsewithpan SE_M_VITAL_THROW2, SOUND_PAN_TARGET
	create_basic_hitsplat_sprite ANIM_TARGET, 3, x=-5, y=-12, relative_to=ANIM_TARGET, animation=1
	delay 1
	playsewithpan SE_M_VITAL_THROW2, SOUND_PAN_TARGET
	create_basic_hitsplat_sprite ANIM_TARGET, 3, x=0, y=-32, relative_to=ANIM_TARGET, animation=1
	delay 1
	playsewithpan SE_M_VITAL_THROW2, SOUND_PAN_TARGET
	create_basic_hitsplat_sprite ANIM_TARGET, 3, x=5, y=-52, relative_to=ANIM_TARGET, animation=1
	createsprite gSlideMonToOffsetSpriteTemplate, ANIM_TARGET, 2, ANIM_TARGET, -25, 16, 1, 4
	delay 4
	createvisualtask AnimTask_ShakeMonInPlace, 2, ANIM_TARGET, 0, 3, 6, 1
	delay 30
	createsprite gSlideMonToOriginalPosSpriteTemplate, ANIM_ATTACKER, 2, 0, 0, 6
	delay 4
	createsprite gSlideMonToOriginalPosSpriteTemplate, ANIM_ATTACKER, 2, 1, 0, 6
	clearmonbg ANIM_DEF_PARTNER
	blendoff
	end

Move_DEFOG:
	loadspritegfx ANIM_TAG_WHITE_CIRCLE_OF_LIGHT
	monbg ANIM_ATTACKER
	setalpha 12, 8
	playsewithpan SE_M_SOLAR_BEAM, SOUND_PAN_ATTACKER
	createsprite gLusterPurgeCircleSpriteTemplate, ANIM_ATTACKER, 41, 0, 0, 0, 0
	delay 20
	createvisualtask AnimTask_BlendBattleAnimPalExclude, 5, 5, 2, 0, 16, RGB_WHITEALPHA
	createvisualtask AnimTask_BlendParticle, 5, ANIM_TAG_WHITE_CIRCLE_OF_LIGHT, 2, 0, 16, -1
	waitforvisualfinish
	waitforvisualfinish
	delay 16
	createvisualtask AnimTask_BlendBattleAnimPalExclude, 5, 5, 2, 16, 0, RGB_WHITEALPHA
	waitforvisualfinish
	clearmonbg ANIM_ATTACKER
	blendoff
	end

Move_TRICK_ROOM:
	call G4_InitRoomAnimation

Move_DRACO_METEOR:
	goto Move_METEOR_MASH

Move_DISCHARGE:
	loadspritegfx ANIM_TAG_IMPACT
	loadspritegfx ANIM_TAG_SPARK_2
	loadspritegfx ANIM_TAG_WHITE_CIRCLE_OF_LIGHT
	delay 0
	blend_color_cycle selector=(F_PAL_BG | F_PAL_ATTACKER), delay=-31, num_blends=1, initial_blend_y=5, target_blend_y=5, color=RGB(31, 31, 20)
	playsewithpan SE_M_THUNDERBOLT2, SOUND_PAN_ATTACKER
	createsprite gSparkElectricitySpriteTemplate, ANIM_ATTACKER, 0, 32, 24, 190, 12, ANIM_ATTACKER, 1, 0
	delay 0
	createsprite gSparkElectricitySpriteTemplate, ANIM_ATTACKER, 0, 80, 24, 22, 12, ANIM_ATTACKER, 1, 0
	createsprite gSparkElectricitySpriteTemplate, ANIM_ATTACKER, 0, 156, 24, 121, 13, ANIM_ATTACKER, 1, 1
	delay 0
	blend_color_cycle selector=(F_PAL_BG | F_PAL_ATTACKER), delay=-31, num_blends=1, initial_blend_y=0, target_blend_y=0, color=RGB(31, 31, 20)
	delay 10
	blend_color_cycle selector=(F_PAL_BG | F_PAL_ATTACKER), delay=-31, num_blends=1, initial_blend_y=5, target_blend_y=5, color=RGB(31, 31, 20)
	playsewithpan SE_M_THUNDERBOLT2, SOUND_PAN_ATTACKER
	createsprite gSparkElectricitySpriteTemplate, ANIM_ATTACKER, 0, 100, 24, 60, 10, ANIM_ATTACKER, 1, 0
	createsprite gSparkElectricitySpriteTemplate, ANIM_ATTACKER, 0, 170, 24, 42, 11, ANIM_ATTACKER, 1, 1
	waitbgfadein
	monbg ANIM_ATTACKER
	setalpha 12, 8
	playsewithpan SE_M_SOLAR_BEAM, SOUND_PAN_ATTACKER
	createsprite gLusterPurgeCircleSpriteTemplate, ANIM_ATTACKER, 41, 0, 0, 0, 0
	delay 20
	createvisualtask AnimTask_BlendBattleAnimPalExclude, 5, 5, 2, 0, 16, RGB_WHITEALPHA
	createvisualtask AnimTask_BlendParticle, 5, ANIM_TAG_WHITE_CIRCLE_OF_LIGHT, 2, 0, 16, RGB_WHITEALPHA
	waitforvisualfinish
	createvisualtask AnimTask_BlendParticle, 5, ANIM_TAG_IMPACT, 0, 12, 12, RGB(0, 0, 23)
	waitforvisualfinish
	create_random_pos_hitsplat_sprite ANIM_TARGET, 3, relative_to=ANIM_TARGET, animation=2
	create_random_pos_hitsplat_sprite ANIM_TARGET, 3, relative_to=ANIM_TARGET, animation=2
	create_random_pos_hitsplat_sprite ANIM_TARGET, 3, relative_to=ANIM_TARGET, animation=2
	createvisualtask SoundTask_PlaySE1WithPanning, 5, 215, SOUND_PAN_TARGET
	delay 3
	create_random_pos_hitsplat_sprite ANIM_TARGET, 3, relative_to=ANIM_TARGET, animation=2
	create_random_pos_hitsplat_sprite ANIM_TARGET, 3, relative_to=ANIM_DEF_PARTNER, animation=2
	create_random_pos_hitsplat_sprite ANIM_TARGET, 3, relative_to=ANIM_ATK_PARTNER, animation=2
	createvisualtask SoundTask_PlaySE1WithPanning, 5, 215, SOUND_PAN_TARGET
	delay 3
	create_random_pos_hitsplat_sprite ANIM_TARGET, 3, relative_to=ANIM_TARGET, animation=2
	create_random_pos_hitsplat_sprite ANIM_TARGET, 3, relative_to=ANIM_DEF_PARTNER, animation=2
	create_random_pos_hitsplat_sprite ANIM_TARGET, 3, relative_to=ANIM_ATK_PARTNER, animation=2
	createvisualtask SoundTask_PlaySE1WithPanning, 5, 215, SOUND_PAN_TARGET
	delay 3
	create_random_pos_hitsplat_sprite ANIM_TARGET, 3, relative_to=ANIM_TARGET, animation=2
	create_random_pos_hitsplat_sprite ANIM_TARGET, 3, relative_to=ANIM_DEF_PARTNER, animation=2
	create_random_pos_hitsplat_sprite ANIM_TARGET, 3, relative_to=ANIM_ATK_PARTNER, animation=2
	createvisualtask SoundTask_PlaySE1WithPanning, 5, 215, SOUND_PAN_TARGET
	delay 3
	create_random_pos_hitsplat_sprite ANIM_TARGET, 3, relative_to=ANIM_TARGET, animation=2
	create_random_pos_hitsplat_sprite ANIM_TARGET, 3, relative_to=ANIM_DEF_PARTNER, animation=2
	create_random_pos_hitsplat_sprite ANIM_TARGET, 3, relative_to=ANIM_ATK_PARTNER, animation=2
	createvisualtask SoundTask_PlaySE1WithPanning, 5, 215, SOUND_PAN_TARGET
	delay 3
	create_random_pos_hitsplat_sprite ANIM_TARGET, 3, relative_to=ANIM_TARGET, animation=2
	create_random_pos_hitsplat_sprite ANIM_TARGET, 3, relative_to=ANIM_DEF_PARTNER, animation=2
	create_random_pos_hitsplat_sprite ANIM_TARGET, 3, relative_to=ANIM_ATK_PARTNER, animation=2
	createvisualtask SoundTask_PlaySE1WithPanning, 5, 215, SOUND_PAN_TARGET
	waitforvisualfinish
	createvisualtask AnimTask_BlendBattleAnimPalExclude, 5, 5, 2, 16, 0, RGB_WHITEALPHA
	createvisualtask AnimTask_HorizontalShake, 5, ANIM_TARGET, 5, 14
	createvisualtask AnimTask_HorizontalShake, 5, ANIM_DEF_PARTNER, 5, 14
	createvisualtask AnimTask_HorizontalShake, 5, ANIM_ATK_PARTNER, 5, 14
	waitforvisualfinish
	clearmonbg ANIM_ATTACKER
	blendoff
	playsewithpan SE_M_THUNDERBOLT2, SOUND_PAN_TARGET
	createsprite gElectricitySpriteTemplate, ANIM_TARGET, 2, 5, 0, 5, 0, ANIM_TARGET
	createsprite gElectricitySpriteTemplate, ANIM_TARGET, 2, 5, 0, 5, 0, ANIM_DEF_PARTNER
	createsprite gElectricitySpriteTemplate, ANIM_TARGET, 2, 5, 0, 5, 0, ANIM_ATK_PARTNER
	delay 2
	createsprite gElectricitySpriteTemplate, ANIM_TARGET, 2, -5, 10, 5, 1, ANIM_TARGET
	createsprite gElectricitySpriteTemplate, ANIM_TARGET, 2, -5, 10, 5, 1, ANIM_DEF_PARTNER
	createsprite gElectricitySpriteTemplate, ANIM_TARGET, 2, -5, 10, 5, 1, ANIM_ATK_PARTNER
	delay 2
	createsprite gElectricitySpriteTemplate, ANIM_TARGET, 2, 15, 20, 5, 2, ANIM_TARGET
	createsprite gElectricitySpriteTemplate, ANIM_TARGET, 2, 15, 20, 5, 2, ANIM_DEF_PARTNER
	createsprite gElectricitySpriteTemplate, ANIM_TARGET, 2, 15, 20, 5, 2, ANIM_ATK_PARTNER
	delay 2
	createsprite gElectricitySpriteTemplate, ANIM_TARGET, 2, -15, -10, 5, 0, ANIM_TARGET
	createsprite gElectricitySpriteTemplate, ANIM_TARGET, 2, -15, -10, 5, 0, ANIM_DEF_PARTNER
	createsprite gElectricitySpriteTemplate, ANIM_TARGET, 2, -15, -10, 5, 0, ANIM_ATK_PARTNER
	delay 2
	createsprite gElectricitySpriteTemplate, ANIM_TARGET, 2, 25, 0, 5, 1, ANIM_TARGET
	createsprite gElectricitySpriteTemplate, ANIM_TARGET, 2, 25, 0, 5, 1, ANIM_DEF_PARTNER
	createsprite gElectricitySpriteTemplate, ANIM_TARGET, 2, 25, 0, 5, 1, ANIM_ATK_PARTNER
	delay 2
	createsprite gElectricitySpriteTemplate, ANIM_TARGET, 2, -8, 8, 5, 2, ANIM_TARGET
	createsprite gElectricitySpriteTemplate, ANIM_TARGET, 2, -8, 8, 5, 2, ANIM_DEF_PARTNER
	createsprite gElectricitySpriteTemplate, ANIM_TARGET, 2, -8, 8, 5, 2, ANIM_ATK_PARTNER
	delay 2
	createsprite gElectricitySpriteTemplate, ANIM_TARGET, 2, 2, -8, 5, 0, ANIM_TARGET
	createsprite gElectricitySpriteTemplate, ANIM_TARGET, 2, 2, -8, 5, 0, ANIM_DEF_PARTNER
	createsprite gElectricitySpriteTemplate, ANIM_TARGET, 2, 2, -8, 5, 0, ANIM_ATK_PARTNER
	delay 2
	createsprite gElectricitySpriteTemplate, ANIM_TARGET, 2, -20, 15, 5, 1, ANIM_TARGET
	createsprite gElectricitySpriteTemplate, ANIM_TARGET, 2, -20, 15, 5, 1, ANIM_DEF_PARTNER
	createsprite gElectricitySpriteTemplate, ANIM_TARGET, 2, -20, 15, 5, 1, ANIM_ATK_PARTNER
	waitforvisualfinish
	end

Move_LAVA_PLUME:
	goto Move_ERUPTION

Move_LEAF_STORM:
	goto Move_RAZOR_LEAF

Move_POWER_WHIP:
	goto Move_VINE_WHIP

Move_ROCK_WRECKER:
	goto Move_ROCK_TOMB

Move_CROSS_POISON:
	loadspritegfx ANIM_TAG_CROSS_IMPACT
	loadspritegfx ANIM_TAG_POISON_BUBBLE
	monbg ANIM_TARGET
	setalpha 12, 8
	playsewithpan SE_M_RAZOR_WIND, SOUND_PAN_TARGET
	complex_palette_blend selector=F_PAL_BG | F_PAL_BATTLERS, delay=3, num_blends=1, color1=RGB_WHITE, blend_y1=10, color2=RGB_BLACK, blend_y2=10
	create_cross_impact_sprite ANIM_ATTACKER, 3, x=0, y=0, relative_to=1, duration=20
	createvisualtask AnimTask_ShakeMon, 5, ANIM_TARGET, 7, 0, 9, 1
	waitforvisualfinish
	call G4_PoisonBubblesEffect
	waitforvisualfinish
	clearmonbg ANIM_TARGET
	blendoff
	end

Move_GUNK_SHOT:
	goto Move_SLUDGE_BOMB

Move_IRON_HEAD:
	loadspritegfx ANIM_TAG_IMPACT
	loopsewithpan SE_M_HARDEN, SOUND_PAN_ATTACKER, 28, 2
	metallic_shine permanent=1
	waitforvisualfinish
	createsprite gBowMonSpriteTemplate, ANIM_ATTACKER, 2, 0
	playsewithpan SE_M_HEADBUTT, SOUND_PAN_ATTACKER
	waitforvisualfinish
	delay 2
	createsprite gBowMonSpriteTemplate, ANIM_ATTACKER, 2, 1
	waitforvisualfinish
	createvisualtask AnimTask_ShakeMonInPlace, 2, ANIM_ATTACKER, 2, 0, 4, 1
	createvisualtask AnimTask_ShakeMon, 2, ANIM_TARGET, 5, 0, 7, 1
	createsprite gBowMonSpriteTemplate, ANIM_ATTACKER, 2, 2
	create_flashing_hitsplat_sprite ANIM_TARGET, 3, x=0, y=0, relative_to=1, animation=1
	playsewithpan SE_M_VITAL_THROW2, SOUND_PAN_TARGET
	waitforvisualfinish
	set_original_pal battler=ANIM_ATTACKER
	waitforvisualfinish
	end

Move_MAGNET_BOMB:
	goto Move_SPARK

Move_STONE_EDGE:
	goto Move_ROCK_SLIDE

Move_CAPTIVATE:
	loadspritegfx ANIM_TAG_MAGENTA_HEART
	loadspritegfx ANIM_TAG_SPARKLE_2
	call G4_CreateMagentaHearts
	playsewithpan SE_M_MORNING_SUN, SOUND_PAN_ATTACKER
	call G4_GrantingStarsEffect
	waitforvisualfinish
	end

Move_STEALTH_ROCK:
	goto Move_SPIKES

Move_GRASS_KNOT:
	goto Move_VINE_WHIP

Move_CHATTER:
	goto Move_UPROAR

Move_JUDGMENT:
	goto Move_HYPER_BEAM

Move_BUG_BITE:
	loadspritegfx ANIM_TAG_ROCKS
	monbg ANIM_TARGET
	setalpha 12, 8
	delay 1
	createvisualtask AnimTask_ShakeMon, 2, ANIM_TARGET, 3, 0, 5, 1
	waitforvisualfinish
	playsewithpan SE_M_BITE, SOUND_PAN_TARGET
	createsprite gRockFragmentSpriteTemplate, ANIM_ATTACKER, 2, 0, 0, 20, 24, 14, 2
	createsprite gRockFragmentSpriteTemplate, ANIM_ATTACKER, 2, 5, 0, -20, 24, 14, 2
	createsprite gRockFragmentSpriteTemplate, ANIM_ATTACKER, 2, 0, 5, 20, -24, 14, 2
	createsprite gRockFragmentSpriteTemplate, ANIM_ATTACKER, 2, -5, 0, -20, -24, 14, 2
	createsprite gRockFragmentSpriteTemplate, ANIM_ATTACKER, 2, 0, -5, 30, 18, 8, 2
	createsprite gRockFragmentSpriteTemplate, ANIM_ATTACKER, 2, 0, 0, 30, -18, 8, 2
	createvisualtask AnimTask_ShakeMon, 2, ANIM_TARGET, 0, 3, 7, 1
	delay 8
	playsewithpan SE_M_BITE, SOUND_PAN_TARGET
	createsprite gRockFragmentSpriteTemplate, ANIM_ATTACKER, 2, 10, 5, 30, 29, 14, 2
	createsprite gRockFragmentSpriteTemplate, ANIM_ATTACKER, 2, 15, 5, -10, 29, 14, 2
	createsprite gRockFragmentSpriteTemplate, ANIM_ATTACKER, 2, 10, 10, 30, -19, 14, 2
	createsprite gRockFragmentSpriteTemplate, ANIM_ATTACKER, 2, 5, 5, -30, -19, 14, 2
	createsprite gRockFragmentSpriteTemplate, ANIM_ATTACKER, 2, 10, 0, 40, 23, 8, 2
	createsprite gRockFragmentSpriteTemplate, ANIM_ATTACKER, 2, 10, 5, 40, -13, 8, 2
	createvisualtask AnimTask_ShakeMon, 2, ANIM_TARGET, 0, 3, 7, 1
	delay 8
	playsewithpan SE_M_BITE, SOUND_PAN_TARGET
	createsprite gRockFragmentSpriteTemplate, ANIM_ATTACKER, 2, -10, -5, 10, 19, 14, 2
	createsprite gRockFragmentSpriteTemplate, ANIM_ATTACKER, 2, -5, -5, -30, 19, 14, 2
	createsprite gRockFragmentSpriteTemplate, ANIM_ATTACKER, 2, -10, 0, 10, -29, 14, 2
	createsprite gRockFragmentSpriteTemplate, ANIM_ATTACKER, 2, -15, -5, -30, -29, 14, 2
	createsprite gRockFragmentSpriteTemplate, ANIM_ATTACKER, 2, -10, -10, 20, 13, 8, 2
	createsprite gRockFragmentSpriteTemplate, ANIM_ATTACKER, 2, -10, -5, 20, -23, 8, 2
	createvisualtask AnimTask_ShakeMon, 2, ANIM_TARGET, 0, 3, 7, 1
	delay 8
	waitforvisualfinish
	clearmonbg ANIM_TARGET
	blendoff
	end

Move_CHARGE_BEAM:
	goto Move_THUNDER_SHOCK

Move_WOOD_HAMMER:
	goto Move_NEEDLE_ARM

Move_AQUA_JET:
	loadspritegfx ANIM_TAG_SMALL_BUBBLES
	loadspritegfx ANIM_TAG_SPLASH
	loadspritegfx ANIM_TAG_SWEAT_BEAD
	loadspritegfx ANIM_TAG_WATER_IMPACT
	createvisualtask AnimTask_AttackerStretchAndDisappear, 2
	playsewithpan SE_M_RAZOR_WIND2, SOUND_PAN_ATTACKER
	waitforvisualfinish
	delay 1
	createvisualtask AnimTask_SetAttackerInvisibleWaitForSignal, 2
	monbg ANIM_DEF_PARTNER
	setalpha 12, 8
	playsewithpan SE_M_EXPLOSION, SOUND_PAN_TARGET
	createsprite gDiveWaterSplashSpriteTemplate, ANIM_TARGET, 3, 1
	call G4_DiveAttackWaterDroplets
	call G4_DiveAttackWaterDroplets
	call G4_DiveAttackWaterDroplets
	call G4_DiveAttackWaterDroplets
	call G4_DiveAttackWaterDroplets
	delay 12
	call G4_RisingWaterHitEffect
	waitforvisualfinish
	createvisualtask AnimTask_ExtremeSpeedMonReappear, 2
	setarg 7, 4096
	waitforvisualfinish
	visible ANIM_ATTACKER
	clearmonbg ANIM_DEF_PARTNER
	blendoff
	setarg 7, 4096
	end

Move_ATTACK_ORDER:
	goto Move_PIN_MISSILE

Move_DEFEND_ORDER:
	goto Move_IRON_DEFENSE

Move_HEAL_ORDER:
	goto Move_MILK_DRINK

Move_HEAD_SMASH:
	loadspritegfx ANIM_TAG_IMPACT
	loadspritegfx ANIM_TAG_ROCKS
	createvisualtask AnimTask_SkullBashPosition, 2, 0
	playsewithpan SE_M_TAKE_DOWN, SOUND_PAN_ATTACKER
	waitforvisualfinish
	playse SE_BANG
	call G4_SetImpactBackground
	createvisualtask AnimTask_ShakeMonInPlace, 2, ANIM_ATTACKER, 2, 0, 40, 1
	createvisualtask AnimTask_ShakeMonInPlace, 2, ANIM_TARGET, 10, 0, 40, 1
	create_flashing_hitsplat_sprite ANIM_TARGET, 4, x=0, y=0, relative_to=ANIM_TARGET, animation=0
	playsewithpan SE_M_ROCK_THROW, SOUND_PAN_TARGET
	createsprite gRockScatterSpriteTemplate, ANIM_TARGET, 2, -12, 32, 3, 4
	createsprite gRockScatterSpriteTemplate, ANIM_TARGET, 2, 8, 31, 2, 2
	createsprite gRockScatterSpriteTemplate, ANIM_TARGET, 2, -4, 28, 2, 3
	createsprite gRockScatterSpriteTemplate, ANIM_TARGET, 2, 12, 30, 4, 3
	waitforvisualfinish
	createvisualtask AnimTask_SkullBashPosition, 2, 1
	restorebg
	waitbgfadein
	end

Move_DOUBLE_HIT:
	loadspritegfx ANIM_TAG_IMPACT
	monbg ANIM_TARGET
	setalpha 12, 8
	createsprite gHorizontalLungeSpriteTemplate, ANIM_ATTACKER, 2, 4, 4
	delay 6
	create_basic_hitsplat_sprite ANIM_TARGET, 2, x=0, y=0, relative_to=ANIM_TARGET, animation=2
	createvisualtask AnimTask_ShakeMon, 2, ANIM_TARGET, 3, 0, 6, 1
	playsewithpan SE_M_COMET_PUNCH, SOUND_PAN_TARGET
	delay 8
	waitforvisualfinish
	@ why does the anim hit twice?
	@ that just means the anim plays 4 times
	@ which doesn't make sense
	@      I don't know, but I'm not gonna argue with it
	createsprite gHorizontalLungeSpriteTemplate, ANIM_ATTACKER, 2, 4, 4
	delay 6
	create_basic_hitsplat_sprite ANIM_TARGET, 2, x=0, y=0, relative_to=ANIM_TARGET, animation=2
	createvisualtask AnimTask_ShakeMon, 2, ANIM_TARGET, 3, 0, 6, 1
	playsewithpan SE_M_COMET_PUNCH, SOUND_PAN_TARGET
	delay 8
	waitforvisualfinish
	clearmonbg ANIM_TARGET
	blendoff
	end

Move_ROAR_OF_TIME:
	goto Move_HYPER_BEAM

Move_SPACIAL_REND:
	goto Move_PSYCHO_BOOST

Move_LUNAR_DANCE:
	goto Move_MOONLIGHT

Move_CRUSH_GRIP:
	goto Move_VICE_GRIP

Move_MAGMA_STORM:
	goto Move_FIRE_SPIN

Move_DARK_VOID:
	goto Move_HYPNOSIS

Move_SEED_FLARE:
	goto Move_SOLAR_BEAM

Move_OMINOUS_WIND:
	goto Move_SILVER_WIND

Move_SHADOW_FORCE:
	choosetwoturnanim ShadowForceVanish, ShadowForceStrike
ShadowForceEnd:
	end
ShadowForceVanish:
	monbg ANIM_ATTACKER
	fadetobg BG_DARK
	waitbgfadein
	delay 0
	playsewithpan SE_M_FAINT_ATTACK, SOUND_PAN_ATTACKER
	createvisualtask AnimTask_TranslateMonEllipticalRespectSide, 2, ANIM_ATTACKER, 18, 6, 1, 3
	attacker_fade_to_invisible step_delay=1
	waitforvisualfinish
	clearmonbg ANIM_ATTACKER
	invisible ANIM_ATTACKER
	delay 1
	restorebg
	waitbgfadein
	goto ShadowForceEnd
ShadowForceStrike:
	loadspritegfx ANIM_TAG_IMPACT
	fadetobg BG_DARK
	waitbgfadein
	createvisualtask AnimTask_SetAttackerInvisibleWaitForSignal, 2
	setalpha 12, 8
	monbg ANIM_TARGET
	delay 1
	playsewithpan SE_M_VITAL_THROW2, SOUND_PAN_TARGET
	create_basic_hitsplat_sprite ANIM_ATTACKER, 2, x=0, y=0, relative_to=ANIM_TARGET, animation=1
	createvisualtask AnimTask_ShakeMon2, 2, ANIM_TARGET, 2, 0, 9, 1
	waitforvisualfinish
	clearmonbg ANIM_TARGET
	blendoff
	delay 1
	setarg 7, 0x1000
	delay 32
	createvisualtask AnimTask_InitAttackerFadeFromInvisible, 2
	monbg ANIM_ATTACKER
	attacker_fade_from_invisible step_delay=1
	waitforvisualfinish
	clearmonbg ANIM_ATTACKER
	delay 1
	restorebg
	waitbgfadein
	goto ShadowForceEnd

@ ---- routines shared by the animations above (from pokeemerald-expansion, prefixed)
G4_CreateBite:
	playsewithpan SE_M_BITE, SOUND_PAN_TARGET
	create_sharp_teeth_sprite ANIM_ATTACKER, 2, x=0, y=-32, animation=0, x_velocity=0/256, y_velocity=819/256, half_duration=10
	create_sharp_teeth_sprite ANIM_ATTACKER, 2, x=0, y=32, animation=4, x_velocity=0/256, y_velocity=-819/256, half_duration=10
	return
G4_CreateClawSlashes1:
	create_claw_slash_sprite ANIM_TARGET, 2, x=-10, y=-10, animation=0
	create_claw_slash_sprite ANIM_TARGET, 2, x=-10, y=10, animation=0
	return
G4_CreateClawSlashes2:
	create_claw_slash_sprite ANIM_TARGET, 2, x=10, y=-10, animation=1
	create_claw_slash_sprite ANIM_TARGET, 2, x=10, y=10, animation=1
	return
G4_CreateDragonDanceOrbs:
	create_dragon_dance_orb_sprite ANIM_ATTACKER, 2, angle=0/256
	create_dragon_dance_orb_sprite ANIM_ATTACKER, 2, angle=43/256
	create_dragon_dance_orb_sprite ANIM_ATTACKER, 2, angle=85/256
	create_dragon_dance_orb_sprite ANIM_ATTACKER, 2, angle=128/256
	create_dragon_dance_orb_sprite ANIM_ATTACKER, 2, angle=170/256
	create_dragon_dance_orb_sprite ANIM_ATTACKER, 2, angle=213/256
	return
G4_CreateDragonRageFirePlumes:
	create_dragon_rage_fire_plume_sprite ANIM_TARGET, 66, relative_to=1, x=5, y=0
	delay 1
	create_dragon_rage_fire_plume_sprite ANIM_TARGET, 66, relative_to=1, x=-10, y=-15
	delay 1
	create_dragon_rage_fire_plume_sprite ANIM_TARGET, 2, relative_to=1, x=0, y=25
	delay 1
	create_dragon_rage_fire_plume_sprite ANIM_TARGET, 66, relative_to=1, x=15, y=5
	delay 1
	create_dragon_rage_fire_plume_sprite ANIM_TARGET, 66, relative_to=1, x=-25, y=0
	delay 1
	create_dragon_rage_fire_plume_sprite ANIM_TARGET, 2, relative_to=1, x=30, y=30
	delay 1
	create_dragon_rage_fire_plume_sprite ANIM_TARGET, 2, relative_to=1, x=-27, y=25
	delay 1
	create_dragon_rage_fire_plume_sprite ANIM_TARGET, 66, relative_to=1, x=0, y=8
	return
G4_CreateFireSpiral:
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_TARGET, 2, 0, 9, RGB_RED
	createsprite gFireSpiralInwardSpriteTemplate, ANIM_TARGET, 1, 0
	createsprite gFireSpiralInwardSpriteTemplate, ANIM_TARGET, 1, 64
	createsprite gFireSpiralInwardSpriteTemplate, ANIM_TARGET, 1, 128
	createsprite gFireSpiralInwardSpriteTemplate, ANIM_TARGET, 1, 196
	return
G4_CreateIceSpiral:
	playsewithpan SE_M_STRING_SHOT, SOUND_PAN_TARGET
	createsprite gIceCrystalSpiralInwardSmall, ANIM_ATTACKER, 2, 0
	createsprite gIceCrystalSpiralInwardSmall, ANIM_ATTACKER, 2, 64
	createsprite gIceCrystalSpiralInwardSmall, ANIM_ATTACKER, 2, 128
	createsprite gIceCrystalSpiralInwardSmall, ANIM_ATTACKER, 2, 192
	delay 5
	createsprite gIceCrystalSpiralInwardLarge, ANIM_ATTACKER, 2, 32
	createsprite gIceCrystalSpiralInwardLarge, ANIM_ATTACKER, 2, 96
	createsprite gIceCrystalSpiralInwardLarge, ANIM_ATTACKER, 2, 160
	createsprite gIceCrystalSpiralInwardLarge, ANIM_ATTACKER, 2, 224
	return
G4_CreateMagentaHearts:
	createvisualtask AnimTask_RockMonBackAndForth, 5, ANIM_ATTACKER, 2, 0
	createsprite gMagentaHeartSpriteTemplate, ANIM_ATTACKER, 3, 0, 20
	playsewithpan SE_M_CHARM, SOUND_PAN_ATTACKER
	delay 15
	createsprite gMagentaHeartSpriteTemplate, ANIM_ATTACKER, 3, -20, 20
	playsewithpan SE_M_CHARM, SOUND_PAN_ATTACKER
	delay 15
	createsprite gMagentaHeartSpriteTemplate, ANIM_ATTACKER, 3, 20, 20
	return
G4_CreateTailwindCrescents:
	createsprite gFlyingSandCrescentSpriteTemplate, ANIM_ATTACKER, 40, 10, 2304, 96, 1
	delay 12
	playsewithpan SE_M_DOUBLE_TEAM, SOUND_PAN_ATTACKER
	createsprite gFlyingSandCrescentSpriteTemplate, ANIM_ATTACKER, 40, 90, 2048, 96, 1
	delay 12
	playsewithpan SE_M_DOUBLE_TEAM, SOUND_PAN_ATTACKER
	createsprite gFlyingSandCrescentSpriteTemplate, ANIM_ATTACKER, 40, 50, 2560, 96, 1
	delay 12
	playsewithpan SE_M_DOUBLE_TEAM, SOUND_PAN_ATTACKER
	createsprite gFlyingSandCrescentSpriteTemplate, ANIM_ATTACKER, 40, 20, 2304, 96, 1
	delay 12
	playsewithpan SE_M_DOUBLE_TEAM, SOUND_PAN_ATTACKER
	createsprite gFlyingSandCrescentSpriteTemplate, ANIM_ATTACKER, 40, 70, 1984, 96, 1
	delay 12
	createsprite gFlyingSandCrescentSpriteTemplate, ANIM_ATTACKER, 40, 0, 2816, 96, 1
	delay 10
	createsprite gFlyingSandCrescentSpriteTemplate, ANIM_ATTACKER, 40, 60, 2560, 96, 1
	return
G4_DiveAttackWaterDroplets:
	createsprite gSprayWaterDropletSpriteTemplate, ANIM_TARGET, 5, 0, 1
	createsprite gSprayWaterDropletSpriteTemplate, ANIM_TARGET, 5, 1, 1
	return
G4_FireSpreadEffect:
	createsprite gFireSpreadSpriteTemplate, ANIM_TARGET, 1, 0, 10, 192, 176, 40
	createsprite gFireSpreadSpriteTemplate, ANIM_TARGET, 1, 0, 10, -192, 240, 40
	createsprite gFireSpreadSpriteTemplate, ANIM_TARGET, 1, 0, 10, 192, -160, 40
	createsprite gFireSpreadSpriteTemplate, ANIM_TARGET, 1, 0, 10, -192, -112, 40
	createsprite gFireSpreadSpriteTemplate, ANIM_TARGET, 1, 0, 10, 160, 48, 40
	createsprite gFireSpreadSpriteTemplate, ANIM_TARGET, 1, 0, 10, -224, -32, 40
	createsprite gFireSpreadSpriteTemplate, ANIM_TARGET, 1, 0, 10, 112, -128, 40
	return
G4_GigaDrainAbsorbEffect:
	playsewithpan SE_M_GIGA_DRAIN, SOUND_PAN_TARGET
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=0, y=5, wave_amplitude=8, wave_period=26
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=5, y=-18, wave_amplitude=-40, wave_period=35
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=-10, y=20, wave_amplitude=20, wave_period=39
	delay 4
	playsewithpan SE_M_GIGA_DRAIN, SOUND_PAN_TARGET
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=0, y=5, wave_amplitude=28, wave_period=26
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=10, y=-5, wave_amplitude=-8, wave_period=26
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=-10, y=20, wave_amplitude=40, wave_period=39
	delay 4
	playsewithpan SE_M_GIGA_DRAIN, SOUND_PAN_TARGET
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=10, y=-5, wave_amplitude=-8, wave_period=26
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=-5, y=15, wave_amplitude=16, wave_period=33
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=10, y=-5, wave_amplitude=-32, wave_period=26
	delay 4
	playsewithpan SE_M_GIGA_DRAIN, SOUND_PAN_TARGET
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=0, y=-15, wave_amplitude=-16, wave_period=36
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=0, y=5, wave_amplitude=8, wave_period=26
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=10, y=-5, wave_amplitude=-8, wave_period=26
	delay 4
	playsewithpan SE_M_GIGA_DRAIN, SOUND_PAN_TARGET
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=-5, y=15, wave_amplitude=16, wave_period=33
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=0, y=-15, wave_amplitude=-16, wave_period=36
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=0, y=5, wave_amplitude=8, wave_period=26
	delay 4
	playsewithpan SE_M_GIGA_DRAIN, SOUND_PAN_TARGET
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=0, y=5, wave_amplitude=8, wave_period=26
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=-5, y=15, wave_amplitude=16, wave_period=33
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=10, y=-5, wave_amplitude=-40, wave_period=26
	delay 4
	playsewithpan SE_M_GIGA_DRAIN, SOUND_PAN_TARGET
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=-5, y=15, wave_amplitude=36, wave_period=33
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=10, y=-5, wave_amplitude=-8, wave_period=26
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=-10, y=20, wave_amplitude=20, wave_period=39
	delay 4
	playsewithpan SE_M_GIGA_DRAIN, SOUND_PAN_TARGET
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=0, y=5, wave_amplitude=8, wave_period=26
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=0, y=5, wave_amplitude=8, wave_period=26
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=5, y=-18, wave_amplitude=-20, wave_period=35
	delay 4
	return
G4_GrantingStarsEffect:
	createsprite gGrantingStarsSpriteTemplate, ANIM_ATTACKER, 2, -15, 0, 0, 0, 32, 60
	delay 8
	createsprite gGrantingStarsSpriteTemplate, ANIM_ATTACKER, 2, 12, -5, 0, 0, 32, 60
	delay 8
	return
G4_HealingEffect:
	playsewithpan SE_M_ABSORB_2, SOUND_PAN_ATTACKER
	createsprite gHealingBlueStarSpriteTemplate, ANIM_ATTACKER, 2, 0, -5, 0, 0
	delay 7
	createsprite gHealingBlueStarSpriteTemplate, ANIM_ATTACKER, 2, -15, 10, 0, 0
	delay 7
	createsprite gHealingBlueStarSpriteTemplate, ANIM_ATTACKER, 2, -15, -15, 0, 0
	delay 7
	createsprite gHealingBlueStarSpriteTemplate, ANIM_ATTACKER, 2, 10, -5, 0, 0
	delay 7
	return
G4_IceCrystalEffectShort:
	createsprite gIceCrystalHitLargeSpriteTemplate, ANIM_TARGET, 2, -10, -10, 0
	playsewithpan SE_M_ICY_WIND, SOUND_PAN_TARGET
	delay 4
	createsprite gIceCrystalHitSmallSpriteTemplate, ANIM_TARGET, 2, 10, 20, 0
	playsewithpan SE_M_ICY_WIND, SOUND_PAN_TARGET
	delay 4
	createsprite gIceCrystalHitLargeSpriteTemplate, ANIM_TARGET, 2, -5, 10, 0
	playsewithpan SE_M_ICY_WIND, SOUND_PAN_TARGET
	delay 4
	createsprite gIceCrystalHitSmallSpriteTemplate, ANIM_TARGET, 2, 17, -12, 0
	playsewithpan SE_M_ICY_WIND, SOUND_PAN_TARGET
	delay 4
	createsprite gIceCrystalHitSmallSpriteTemplate, ANIM_TARGET, 2, -15, 15, 0
	playsewithpan SE_M_ICY_WIND, SOUND_PAN_TARGET
	delay 4
	createsprite gIceCrystalHitSmallSpriteTemplate, ANIM_TARGET, 2, 0, 0, 0
	playsewithpan SE_M_ICY_WIND, SOUND_PAN_TARGET
	delay 4
	createsprite gIceCrystalHitLargeSpriteTemplate, ANIM_TARGET, 2, 20, 2, 0
	playsewithpan SE_M_ICY_WIND, SOUND_PAN_TARGET
	return
G4_InitRoomAnimation:
	setalpha 8, 8
	createvisualtask AnimTask_ScaleMonAndRestore, 5, -6, -6, 15, ANIM_ATTACKER, 1
	return
G4_MindReaderEyeSpikeEffect:
	createsprite gTealAlertSpriteTemplate, ANIM_ATTACKER, 4, 70, 0, 6
	createsprite gTealAlertSpriteTemplate, ANIM_ATTACKER, 4, 40, 40, 6
	createsprite gTealAlertSpriteTemplate, ANIM_ATTACKER, 4, 10, -60, 6
	createsprite gTealAlertSpriteTemplate, ANIM_ATTACKER, 4, -50, -40, 6
	createsprite gTealAlertSpriteTemplate, ANIM_ATTACKER, 4, -40, 40, 6
	createsprite gTealAlertSpriteTemplate, ANIM_ATTACKER, 4, 50, -50, 6
	delay 2
	createsprite gTealAlertSpriteTemplate, ANIM_ATTACKER, 4, 50, -30, 6
	createsprite gTealAlertSpriteTemplate, ANIM_ATTACKER, 4, 60, 10, 6
	createsprite gTealAlertSpriteTemplate, ANIM_ATTACKER, 4, 0, 60, 6
	createsprite gTealAlertSpriteTemplate, ANIM_ATTACKER, 4, 0, -40, 6
	createsprite gTealAlertSpriteTemplate, ANIM_ATTACKER, 4, -60, 20, 6
	createsprite gTealAlertSpriteTemplate, ANIM_ATTACKER, 4, -60, -30, 6
	delay 2
	createsprite gTealAlertSpriteTemplate, ANIM_ATTACKER, 4, -50, 50, 6
	createsprite gTealAlertSpriteTemplate, ANIM_ATTACKER, 4, -60, 20, 6
	createsprite gTealAlertSpriteTemplate, ANIM_ATTACKER, 4, -40, -40, 6
	createsprite gTealAlertSpriteTemplate, ANIM_ATTACKER, 4, 20, -60, 6
	createsprite gTealAlertSpriteTemplate, ANIM_ATTACKER, 4, 50, -50, 6
	createsprite gTealAlertSpriteTemplate, ANIM_ATTACKER, 4, 35, 40, 6
	delay 2
	return
G4_PoisonBubblesEffect:
	createsprite gPoisonBubbleSpriteTemplate, ANIM_TARGET, 2, 10, 10, 0
	playsewithpan SE_M_TOXIC, SOUND_PAN_TARGET
	delay 6
	createsprite gPoisonBubbleSpriteTemplate, ANIM_TARGET, 2, 20, -20, 0
	playsewithpan SE_M_TOXIC, SOUND_PAN_TARGET
	delay 6
	createsprite gPoisonBubbleSpriteTemplate, ANIM_TARGET, 2, -20, 15, 0
	playsewithpan SE_M_TOXIC, SOUND_PAN_TARGET
	delay 6
	createsprite gPoisonBubbleSpriteTemplate, ANIM_TARGET, 2, 0, 0, 0
	playsewithpan SE_M_TOXIC, SOUND_PAN_TARGET
	delay 6
	createsprite gPoisonBubbleSpriteTemplate, ANIM_TARGET, 2, -20, -20, 0
	playsewithpan SE_M_TOXIC, SOUND_PAN_TARGET
	delay 6
	createsprite gPoisonBubbleSpriteTemplate, ANIM_TARGET, 2, 16, -8, 0
	playsewithpan SE_M_TOXIC, SOUND_PAN_TARGET
	return
G4_RisingWaterHitEffect:
	playsewithpan SE_M_WATERFALL, SOUND_PAN_TARGET
	createvisualtask AnimTask_ShakeMon2, 5, ANIM_TARGET, 4, 0, 17, 1
	createsprite gWaterHitSplatSpriteTemplate, ANIM_ATTACKER, 3, 0, 20, ANIM_TARGET, 1
	createsprite gSmallDriftingBubblesSpriteTemplate, ANIM_ATTACKER, 4, 0, 20
	createsprite gSmallDriftingBubblesSpriteTemplate, ANIM_ATTACKER, 4, 0, 20
	delay 2
	createsprite gWaterHitSplatSpriteTemplate, ANIM_ATTACKER, 3, 0, 15, ANIM_TARGET, 1
	createsprite gSmallDriftingBubblesSpriteTemplate, ANIM_ATTACKER, 4, 0, 15
	createsprite gSmallDriftingBubblesSpriteTemplate, ANIM_ATTACKER, 4, 0, 15
	delay 2
	createsprite gWaterHitSplatSpriteTemplate, ANIM_ATTACKER, 3, 0, 10, ANIM_TARGET, 1
	createsprite gSmallDriftingBubblesSpriteTemplate, ANIM_ATTACKER, 4, 0, 10
	createsprite gSmallDriftingBubblesSpriteTemplate, ANIM_ATTACKER, 4, 0, 10
	delay 2
	createsprite gWaterHitSplatSpriteTemplate, ANIM_ATTACKER, 3, 0, 5, ANIM_TARGET, 1
	createsprite gSmallDriftingBubblesSpriteTemplate, ANIM_ATTACKER, 4, 0, 5
	createsprite gSmallDriftingBubblesSpriteTemplate, ANIM_ATTACKER, 4, 0, 5
	delay 2
	createsprite gWaterHitSplatSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, ANIM_TARGET, 1
	createsprite gSmallDriftingBubblesSpriteTemplate, ANIM_ATTACKER, 4, 0, 0
	createsprite gSmallDriftingBubblesSpriteTemplate, ANIM_ATTACKER, 4, 0, 0
	delay 2
	createsprite gWaterHitSplatSpriteTemplate, ANIM_ATTACKER, 3, 0, -5, ANIM_TARGET, 1
	createsprite gSmallDriftingBubblesSpriteTemplate, ANIM_ATTACKER, 4, 0, -5
	createsprite gSmallDriftingBubblesSpriteTemplate, ANIM_ATTACKER, 4, 0, -5
	delay 2
	createsprite gWaterHitSplatSpriteTemplate, ANIM_ATTACKER, 3, 0, -10, ANIM_TARGET, 1
	createsprite gSmallDriftingBubblesSpriteTemplate, ANIM_ATTACKER, 4, 0, -10
	createsprite gSmallDriftingBubblesSpriteTemplate, ANIM_ATTACKER, 4, 0, -10
	delay 2
	createsprite gWaterHitSplatSpriteTemplate, ANIM_ATTACKER, 3, 0, -15, ANIM_TARGET, 1
	createsprite gSmallDriftingBubblesSpriteTemplate, ANIM_ATTACKER, 4, 0, -15
	createsprite gSmallDriftingBubblesSpriteTemplate, ANIM_ATTACKER, 4, 0, -15
	delay 2
	createsprite gWaterHitSplatSpriteTemplate, ANIM_ATTACKER, 3, 0, -20, ANIM_TARGET, 1
	createsprite gSmallDriftingBubblesSpriteTemplate, ANIM_ATTACKER, 4, 0, -20
	createsprite gSmallDriftingBubblesSpriteTemplate, ANIM_ATTACKER, 4, 0, -20
	return
G4_SetBugBg:
	createvisualtask AnimTask_BlendBattleAnimPalExclude, 10, ANIM_TARGET, 0, 0, 4, RGB_BLACK
	createvisualtask AnimTask_GetAttackerSide, 2
	jumprettrue G4_SetBugBgPlayer
	fadetobg BG_BUG_OPPONENT
	waitbgfadeout
	createvisualtask AnimTask_StartSlidingBg, 5, 1536, 0, 0, -1
	goto G4_SetBugBgFade
G4_SetBugBgFade:
	delay 0
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_BG, 0, 4, 4, RGB_BLACK
	waitbgfadein
	return
G4_SetBugBgPlayer:
	fadetobg BG_BUG_PLAYER
	waitbgfadeout
	createvisualtask AnimTask_StartSlidingBg, 5, -1536, 0, 0, -1
	goto G4_SetBugBgFade
G4_SetHighSpeedBg:
	createvisualtask AnimTask_GetAttackerSide, 2
	jumprettrue G4_SetHighSpeedBgPlayer
	fadetobg BG_HIGHSPEED_OPPONENT
	goto G4_SetHighSpeedBgFade
G4_SetHighSpeedBgFade:
	waitbgfadeout
	createvisualtask AnimTask_StartSlidingBg, 5, -2304, 0, 1, -1
	waitbgfadein
	return
G4_SetHighSpeedBgPlayer:
	fadetobg BG_HIGHSPEED_PLAYER
	goto G4_SetHighSpeedBgFade
G4_SetImpactBackground:
	delay 2
	createvisualtask AnimTask_IsContest, 2
	jumprettrue G4_SetImpactContestsBG
	createvisualtask AnimTask_IsTargetPlayerSide, 2
	jumpretfalse G4_SetImpactOpponentBG
	jumprettrue G4_SetImpactPlayerBG
G4_SetImpactBackgroundRet:
	return
G4_SetImpactContestsBG:
	changebg BG_IMPACT_CONTESTS
	goto G4_SetImpactBackgroundRet
G4_SetImpactOpponentBG:
	changebg BG_IMPACT_OPPONENT
	goto G4_SetImpactBackgroundRet
G4_SetImpactPlayerBG:
	changebg BG_IMPACT_PLAYER
	goto G4_SetImpactBackgroundRet
G4_SetPsychicBackground:
	fadetobg BG_PSYCHIC
	waitbgfadeout
	createvisualtask AnimTask_SetPsychicBackground, 5
	waitbgfadein
	return
G4_SetSkyBg:
	jumpifcontest G4_SetSkyBgContest
	fadetobg BG_SKY
	waitbgfadeout
	createvisualtask AnimTask_StartSlidingBg, 5, -2304, 768, 1, -1
	goto G4_SetSkyBgContinue
G4_SetSkyBgContest:
	fadetobg BG_SKY_CONTESTS
	waitbgfadeout
	createvisualtask AnimTask_StartSlidingBg, 5, 2304, 768, 0, -1
	goto G4_SetSkyBgContinue
G4_SetSkyBgContinue:
	waitbgfadein
	return
G4_StockpileAbsorb:
	create_stockpile_absorption_orb_sprite ANIM_ATTACKER, 2, x=55, y=55, duration=13
	delay 1
	create_stockpile_absorption_orb_sprite ANIM_ATTACKER, 2, x=-55, y=-55, duration=13
	delay 1
	create_stockpile_absorption_orb_sprite ANIM_ATTACKER, 2, x=0, y=55, duration=13
	delay 1
	create_stockpile_absorption_orb_sprite ANIM_ATTACKER, 2, x=0, y=-55, duration=13
	delay 1
	create_stockpile_absorption_orb_sprite ANIM_ATTACKER, 2, x=55, y=-34, duration=13
	delay 1
	create_stockpile_absorption_orb_sprite ANIM_ATTACKER, 2, x=55, y=34, duration=13
	delay 1
	create_stockpile_absorption_orb_sprite ANIM_ATTACKER, 2, x=-55, y=-34, duration=13
	delay 1
	create_stockpile_absorption_orb_sprite ANIM_ATTACKER, 2, x=-55, y=34, duration=13
	delay 1
	return
G4_UnsetBackground:
	restorebg
	waitbgfadeout
	setarg 7, -1
	waitbgfadein
	return
G4_UnsetBugBg:
	restorebg
	waitbgfadeout
	createvisualtask AnimTask_BlendBattleAnimPalExclude, 10, ANIM_TARGET, 0, 4, 0, RGB_BLACK
	setarg 7, -1
	waitbgfadein
	return

