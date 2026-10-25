@ Gen 5-8 move animations (Move_<MOVE>). A move without its own borrows a Gen 3 one of its type
@ (tools/moemon_gen58/gen_moves.py adds the stubs; tools/moemon_gen4/gen_anims.py ports expansion's).

Move_HONE_CLAWS:
	loadspritegfx ANIM_TAG_SWIPE
	call G58_HoneClawsAnim
	createvisualtask AnimTask_BlendParticle, 5, ANIM_TAG_SWIPE, 0, 6, 6, RGB_BLACK
	call G58_HoneClawsAnim
	createvisualtask AnimTask_BlendParticle, 5, ANIM_TAG_SWIPE, 0, 10, 10, RGB_BLACK
	call G58_HoneClawsAnim
	end

Move_WIDE_GUARD:
	goto Move_ROCK_TOMB

Move_GUARD_SPLIT:
	goto Move_CALM_MIND

Move_POWER_SPLIT:
	goto Move_CALM_MIND

Move_WONDER_ROOM:
	call G58_InitRoomAnimation

Move_PSYSHOCK:
	goto Move_PSYCHIC

Move_VENOSHOCK:
	loadspritegfx ANIM_TAG_TOXIC_BUBBLE
	createsprite gToxicBubbleSpriteTemplate, ANIM_TARGET, 2, -24, 16, 1, 1
	playsewithpan SE_M_TOXIC, SOUND_PAN_TARGET
	delay 5
	createsprite gToxicBubbleSpriteTemplate, ANIM_TARGET, 2, -16, 16, 1, 1
	playsewithpan SE_M_TOXIC, SOUND_PAN_TARGET
	delay 5
	createsprite gToxicBubbleSpriteTemplate, ANIM_TARGET, 2, -8, 16, 1, 1
	playsewithpan SE_M_TOXIC, SOUND_PAN_TARGET
	delay 5
	createsprite gToxicBubbleSpriteTemplate, ANIM_TARGET, 2, 0, 16, 1, 1
	playsewithpan SE_M_TOXIC, SOUND_PAN_TARGET
	delay 5
	createsprite gToxicBubbleSpriteTemplate, ANIM_TARGET, 2, 8, 16, 1, 1
	playsewithpan SE_M_TOXIC, SOUND_PAN_TARGET
	delay 5
	createsprite gToxicBubbleSpriteTemplate, ANIM_TARGET, 2, 16, 16, 1, 1
	playsewithpan SE_M_TOXIC, SOUND_PAN_TARGET
	delay 5
	createsprite gToxicBubbleSpriteTemplate, ANIM_TARGET, 2, 24, 16, 1, 1
	playsewithpan SE_M_TOXIC, SOUND_PAN_TARGET
	delay 5
	waitforvisualfinish
	delay 15
	end

Move_AUTOTOMIZE:
	goto Move_IRON_DEFENSE

Move_RAGE_POWDER:
	goto Move_SPIDER_WEB

Move_TELEKINESIS:
	loadspritegfx ANIM_TAG_SPARKLE_4
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_BG, 1, 0, 9, RGB_BLACK
	waitforvisualfinish
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_ATTACKER, 1, 0, 9, RGB_WHITE
	delay 18
	playsewithpan SE_M_DETECT, SOUND_PAN_ATTACKER
	createsprite gSpinningSparkleSpriteTemplate, ANIM_ATTACKER, 13, 20, -20
	waitforvisualfinish
	loopsewithpan SE_M_SUPERSONIC, SOUND_PAN_TARGET, 10, 3
	createvisualtask AnimTask_SwayMon, 5, 1, 24, 208, 1, ANIM_TARGET
	waitforvisualfinish
	delay 10
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_BG, 1, 9, 0, RGB_BLACK
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_ATTACKER, 2, 9, 0, RGB_WHITE
	waitforvisualfinish
	end

Move_MAGIC_ROOM:
	call G58_InitRoomAnimation

Move_SMACK_DOWN:
	loadspritegfx ANIM_TAG_IMPACT
	loadspritegfx ANIM_TAG_ROCKS
	playsewithpan SE_M_DOUBLE_TEAM, SOUND_PAN_ATTACKER
	createsprite gRockBlastRockSpriteTemplate, ANIM_TARGET, 2, 20, 0, 0, 0, 21, -25
	waitforvisualfinish
	createvisualtask AnimTask_SmokescreenImpact, 8, 1024, 6402
	fadetobg BG_IN_AIR
	waitbgfadeout
	createvisualtask AnimTask_SeismicTossBgAccelerateDownAtEnd, 3
	goto G58_SeismicTossWeak

Move_STORM_THROW:
	call G58_StormThorwAnim
	call G58_StormThorwAnim
	call G58_StormThorwAnim
	createsprite gSlideMonToOffsetSpriteTemplate, ANIM_ATTACKER, 2, ANIM_ATTACKER, 20, 0, 0, 4
	playsewithpan SE_M_SWAGGER, SOUND_PAN_TARGET
	delay 1
	createvisualtask AnimTask_RotateMonSpriteToSide, 2, 10, 3072, ANIM_TARGET, 2
	playsewithpan SE_M_VITAL_THROW2, SOUND_PAN_TARGET
	delay 3
	createsprite gSlideMonToOriginalPosSpriteTemplate, ANIM_ATTACKER, 2, 0, 0, 7
	waitforvisualfinish
	createsprite gSlideMonToOriginalPosSpriteTemplate, ANIM_ATTACKER, 2, 0, 1, 4
	waitforvisualfinish
	end

Move_FLAME_BURST:
	loadspritegfx ANIM_TAG_EXPLOSION
	loadspritegfx ANIM_TAG_SMALL_EMBER
	playsewithpan SE_M_FLAME_WHEEL, SOUND_PAN_ATTACKER
	create_dragon_breath_fire_sprite ANIM_TARGET, 2, initial_x=0, initial_y=0, target_x=0, target_y=0, duration=20
	delay 18
	createvisualtask AnimTask_ShakeMon2, 2, ANIM_TARGET, 4, 0, 16, 1
	createsprite gExplosionSpriteTemplate, ANIM_TARGET, 4, 0, 0, 1, 0
	call G58_FlameBurstSpread
	playsewithpan SE_M_SELF_DESTRUCT, SOUND_PAN_TARGET
	delay 3
	createsprite gExplosionSpriteTemplate, ANIM_TARGET, 4, 14, -14, 1, 0
	playsewithpan SE_M_SELF_DESTRUCT, SOUND_PAN_TARGET
	waitforvisualfinish
	end

Move_SLUDGE_WAVE:
	goto Move_SLUDGE_BOMB

Move_QUIVER_DANCE:
	loadspritegfx ANIM_TAG_HOLLOW_ORB
	monbg ANIM_ATTACKER
	call G58_SetBugBg
	createvisualtask AnimTask_DragonDanceWaver, 5
	playsewithpan SE_M_TELEPORT, SOUND_PAN_ATTACKER
	delay 8
	createvisualtask AnimTask_BlendPalInAndOutByTag, 5, ANIM_TAG_HOLLOW_ORB, RGB_GREEN, 14, 0, 3
	call G58_CreateDragonDanceOrbs
	delay 30
	playsewithpan SE_M_TELEPORT, SOUND_PAN_ATTACKER
	delay 30
	playsewithpan SE_M_TELEPORT, SOUND_PAN_ATTACKER
	waitforvisualfinish
	clearmonbg ANIM_ATTACKER
	call G58_UnsetBugBg
	end

Move_HEAVY_SLAM:
	loadspritegfx ANIM_TAG_IMPACT
	loopsewithpan SE_M_HARDEN, SOUND_PAN_ATTACKER, 28, 2
	metallic_shine permanent=0
	waitforvisualfinish
	monbg ANIM_TARGET
	setalpha 12, 8
	playsewithpan SE_M_TAKE_DOWN, SOUND_PAN_ATTACKER
	createsprite gVerticalDipSpriteTemplate, ANIM_ATTACKER, 2, 6, 1, ANIM_ATTACKER
	waitforvisualfinish
	delay 11
	createsprite gSlideMonToOffsetSpriteTemplate, ANIM_ATTACKER, 2, ANIM_ATTACKER, 26, 0, 0, 5
	delay 6
	create_basic_hitsplat_sprite ANIM_ATTACKER, 4, x=-10, y=0, relative_to=ANIM_TARGET, animation=0
	loopsewithpan SE_M_MEGA_KICK2, SOUND_PAN_TARGET, 10, 2
	delay 1
	createsprite gSlideMonToOffsetSpriteTemplate, ANIM_ATTACKER, 2, ANIM_TARGET, -28, 0, 0, 3
	waitforvisualfinish
	createvisualtask AnimTask_ShakeMonInPlace, 2, ANIM_TARGET, 4, 0, 12, 1
	waitforvisualfinish
	delay 10
	createsprite gSlideMonToOriginalPosSpriteTemplate, ANIM_ATTACKER, 2, 0, 0, 6
	delay 5
	createsprite gSlideMonToOriginalPosSpriteTemplate, ANIM_ATTACKER, 2, 1, 0, 6
	waitforvisualfinish
	clearmonbg ANIM_TARGET
	blendoff
	end

Move_SYNCHRONOISE:
	goto Move_PSYCHIC

Move_ELECTRO_BALL:
	goto Move_THUNDERBOLT

Move_SOAK:
	loadspritegfx ANIM_TAG_SMALL_BUBBLES
	loadspritegfx ANIM_TAG_WATER_IMPACT
	monbg ANIM_TARGET
	splitbgprio ANIM_TARGET
	setalpha 12, 8
	createsprite gWaterGunProjectileSpriteTemplate, ANIM_ATTACKER, 2, 20, 0, 0, 0, 40, -25
	playsewithpan SE_M_BUBBLE, SOUND_PAN_ATTACKER
	delay 2
	createsprite gWaterGunProjectileSpriteTemplate, ANIM_ATTACKER, 2, 20, 0, 5, 0, 40, -25
	playsewithpan SE_M_BUBBLE, SOUND_PAN_ATTACKER
	delay 2
	createsprite gWaterGunProjectileSpriteTemplate, ANIM_ATTACKER, 2, 20, 0, -5, 0, 40, -25
	playsewithpan SE_M_BUBBLE, SOUND_PAN_ATTACKER
	delay 30
	playsewithpan SE_M_WATERFALL, SOUND_PAN_TARGET
	createvisualtask AnimTask_ShakeMon2, 5, ANIM_TARGET, 4, 0, 17, 1
	blend_color_cycle selector=F_PAL_TARGET, delay=1, num_blends=2, initial_blend_y=0, target_blend_y=12, color=RGB_BLUE
	createsprite gWaterHitSplatSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, 1, 1
	createsprite gSmallDriftingBubblesSpriteTemplate, ANIM_ATTACKER, 4, 0, 0
	createsprite gSmallDriftingBubblesSpriteTemplate, ANIM_ATTACKER, 4, 0, 0
	delay 2
	createsprite gWaterHitSplatSpriteTemplate, ANIM_ATTACKER, 3, 5, 0, 1, 1
	createsprite gSmallDriftingBubblesSpriteTemplate, ANIM_ATTACKER, 4, 5, 0
	createsprite gSmallDriftingBubblesSpriteTemplate, ANIM_ATTACKER, 4, 5, 0
	delay 2
	createsprite gWaterHitSplatSpriteTemplate, ANIM_ATTACKER, 3, -5, 0, 1, 1
	createsprite gSmallDriftingBubblesSpriteTemplate, ANIM_ATTACKER, 4, -5, 0
	createsprite gSmallDriftingBubblesSpriteTemplate, ANIM_ATTACKER, 4, -5, 0
	delay 2
	waitforvisualfinish
	clearmonbg ANIM_TARGET
	blendoff
	end

Move_FLAME_CHARGE:
	goto Move_FLAMETHROWER

Move_COIL:
	loadspritegfx ANIM_TAG_TENDRILS
	call G58_CreateConstrictBinding
	createvisualtask AnimTask_ShakeMon2, 131, ANIM_TARGET, 4, 0, 5, 1
	createvisualtask AnimTask_StretchTargetUp, 3
	playsewithpan SE_M_BIND, SOUND_PAN_TARGET
	setarg 7, -1
	waitforvisualfinish
	end

Move_LOW_SWEEP:
	loadspritegfx ANIM_TAG_HANDS_AND_FEET
	loadspritegfx ANIM_TAG_IMPACT
	createsprite gSlideMonToOffsetSpriteTemplate, ANIM_ATTACKER, 2, ANIM_ATTACKER, 20, 0, 0, 4
	createsprite gSlidingKickSpriteTemplate, ANIM_TARGET, 2, -24, 14, 40, 8, 160, 0
	delay 4
	create_basic_hitsplat_sprite ANIM_TARGET, 2, x=-8, y=8, relative_to=ANIM_TARGET, animation=2
	playsewithpan SE_M_VITAL_THROW2, SOUND_PAN_TARGET
	waitforvisualfinish
	createsprite gSlideMonToOriginalPosSpriteTemplate, ANIM_ATTACKER, 2, 0, 1, 4
	end

Move_ACID_SPRAY:
	loadspritegfx ANIM_TAG_POISON_BUBBLE
	monbg ANIM_TARGET
	createsprite gAcidPoisonBubbleSpriteTemplate, ANIM_TARGET, 2, 20, 0, 40, 1, 0, -20, FALSE
	playsewithpan SE_M_BUBBLE3, SOUND_PAN_ATTACKER
	delay 33
	createvisualtask AnimTask_ShakeMon2, 5, ANIM_TARGET, 2, 0, 10, 1
	blend_color_cycle selector=F_PAL_TARGET, delay=2, num_blends=2, initial_blend_y=0, target_blend_y=12, color=RGB(30, 0, 31)
	playsewithpan SE_M_DIVE, SOUND_PAN_ATTACKER
	createsprite gAcidPoisonDropletSpriteTemplate, ANIM_TARGET, 2, 0, -20, 10, 15, 55, FALSE
	createsprite gAcidPoisonDropletSpriteTemplate, ANIM_TARGET, 2, 0, -20, -20, 15, 55, FALSE
	delay 3
	createsprite gAcidPoisonDropletSpriteTemplate, ANIM_TARGET, 2, 0, -20, 5, 15, 55, FALSE
	createsprite gAcidPoisonDropletSpriteTemplate, ANIM_TARGET, 2, 0, -20, -10, 15, 55, FALSE
	delay 3
	createsprite gAcidPoisonDropletSpriteTemplate, ANIM_TARGET, 2, 0, -20, 30, 15, 55, FALSE
	createsprite gAcidPoisonDropletSpriteTemplate, ANIM_TARGET, 2, 0, -20, -30, 15, 55, FALSE
	delay 3
	createsprite gAcidPoisonDropletSpriteTemplate, ANIM_TARGET, 2, 0, -20, 2, 15, 55, FALSE
	createsprite gAcidPoisonDropletSpriteTemplate, ANIM_TARGET, 2, 0, -20, -5, 15, 55, FALSE
	delay 3
	createsprite gAcidPoisonDropletSpriteTemplate, ANIM_TARGET, 2, 0, -20, 20, 15, 55, FALSE
	waitforvisualfinish
	clearmonbg ANIM_TARGET
	end

Move_FOUL_PLAY:
	goto Move_CRUNCH

Move_SIMPLE_BEAM:
	goto Move_GROWL

Move_ENTRAINMENT:
	createvisualtask AnimTask_TeeterDanceMovement, 5
	playsewithpan SE_M_TEETER_DANCE, SOUND_PAN_ATTACKER
	delay 24
	playsewithpan SE_M_TEETER_DANCE, SOUND_PAN_ATTACKER
	delay 24
	playsewithpan SE_M_TEETER_DANCE, SOUND_PAN_ATTACKER
	delay 24
	playsewithpan SE_M_TEETER_DANCE, SOUND_PAN_ATTACKER
	delay 24
	playsewithpan SE_M_TEETER_DANCE, SOUND_PAN_ATTACKER
	createvisualtask AnimTask_ShakeMon, 2, ANIM_TARGET, 3, 0, 31, 1
	waitforvisualfinish
	end

Move_AFTER_YOU:
	goto Move_GROWL

Move_ROUND:
	loadspritegfx ANIM_TAG_MUSIC_NOTES
	createvisualtask AnimTask_MusicNotesRainbowBlend, 2
	waitforvisualfinish
	monbg ANIM_TARGET
	createvisualtask SoundTask_PlayCryHighPitch, 2, ANIM_ATTACKER, 255
	createvisualtask AnimTask_UproarDistortion, 2, 0
	createsprite gWavyMusicNotesSpriteTemplate, ANIM_TARGET, 2, 7, 0, 12
	delay 5
	createsprite gWavyMusicNotesSpriteTemplate, ANIM_TARGET, 2, 6, 1, 12
	delay 5
	createsprite gWavyMusicNotesSpriteTemplate, ANIM_TARGET, 2, 1, 2, 12
	delay 5
	createsprite gWavyMusicNotesSpriteTemplate, ANIM_TARGET, 2, 2, 3, 12
	delay 20
	createvisualtask AnimTask_ShakeMon2, 2, ANIM_TARGET, 1, 0, 13, 1
	waitforvisualfinish
	clearmonbg ANIM_TARGET
	createvisualtask AnimTask_MusicNotesClearRainbowBlend, 2
	waitforvisualfinish
	end

Move_ECHOED_VOICE:
	loadspritegfx ANIM_TAG_METAL_SOUND_WAVES
	monbg ANIM_DEF_PARTNER
	splitbgprio_foes ANIM_TARGET
	createvisualtask AnimTask_ShakeMon2, 2, ANIM_ATTACKER, 2, 0, 8, 1
	call G58_MetalSoundPlayNote
	call G58_MetalSoundPlayNote
	call G58_MetalSoundPlayNote
	call G58_MetalSoundPlayNote
	waitforvisualfinish
	clearmonbg ANIM_DEF_PARTNER
	delay 0
	waitforvisualfinish
	end

Move_CHIP_AWAY:
	loadspritegfx ANIM_TAG_IMPACT
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_ATTACKER, 2, 0, 9, RGB_WHITE
	waitforvisualfinish
	monbg ANIM_ATK_PARTNER
	setalpha 12, 8
	createvisualtask AnimTask_TranslateMonEllipticalRespectSide, 2, ANIM_ATTACKER, 24, 6, 1, 5
	delay 4
	createvisualtask AnimTask_ShakeMon, 2, ANIM_TARGET, 5, 0, 6, 1
	create_basic_hitsplat_sprite ANIM_TARGET, 4, x=0, y=0, relative_to=ANIM_TARGET, animation=1
	playsewithpan SE_M_VITAL_THROW2 SOUND_PAN_TARGET
	waitforvisualfinish
	clearmonbg ANIM_ATK_PARTNER
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_ATTACKER, 2, 9, 0, RGB_WHITE
	waitforvisualfinish
	blendoff
	end

Move_CLEAR_SMOG:
	loadspritegfx ANIM_TAG_MIST_CLOUD
	monbg ANIM_TARGET
	splitbgprio_all
	setalpha 12, 8
	createvisualtask AnimTask_BlendParticle, 5, ANIM_TAG_MIST_CLOUD, 0, 6, 6, RGB(26, 0, 26)
	loopsewithpan SE_M_MIST, SOUND_PAN_TARGET, 17, 10
	call G58_ClearSmogCloud
	call G58_ClearSmogCloud
	call G58_ClearSmogCloud
	call G58_ClearSmogCloud
	call G58_ClearSmogCloud
	call G58_ClearSmogCloud
	call G58_ClearSmogCloud
	delay 120
	loopsewithpan SE_M_TOXIC, SOUND_PAN_TARGET, 18, 2
	blend_color_cycle selector=F_PAL_TARGET, delay=2, num_blends=2, initial_blend_y=0, target_blend_y=12, color=RGB_WHITE
	delay 10
	createvisualtask AnimTask_ShakeMon2, 2, ANIM_TARGET, 2, 0, 15, 1
	waitforvisualfinish
	clearmonbg ANIM_TARGET
	blendoff
	end

Move_STORED_POWER:
	loadspritegfx ANIM_TAG_IMPACT
	loadspritegfx ANIM_TAG_RED_ORB_2
	call G58_SetPsychicBackground
	playsewithpan SE_M_TAKE_DOWN, SOUND_PAN_ATTACKER
	createvisualtask AnimTask_SpitUpDeformMon, 5
	createvisualtask AnimTask_ShakeMon2, 2, ANIM_ATTACKER, 1, 0, 8, 2
	delay 45
	playsewithpan SE_M_SPIT_UP, SOUND_PAN_ATTACKER
	delay 3
	createsprite gSpitUpOrbSpriteTemplate, ANIM_ATTACKER, 2, 0, 12
	createsprite gSpitUpOrbSpriteTemplate, ANIM_ATTACKER, 2, 32, 12
	createsprite gSpitUpOrbSpriteTemplate, ANIM_ATTACKER, 2, 64, 12
	createsprite gSpitUpOrbSpriteTemplate, ANIM_ATTACKER, 2, 96, 12
	createsprite gSpitUpOrbSpriteTemplate, ANIM_ATTACKER, 2, 128, 12
	createsprite gSpitUpOrbSpriteTemplate, ANIM_ATTACKER, 2, 160, 12
	createsprite gSpitUpOrbSpriteTemplate, ANIM_ATTACKER, 2, 192, 12
	createsprite gSpitUpOrbSpriteTemplate, ANIM_ATTACKER, 2, 224, 12
	delay 10
	createvisualtask AnimTask_ShakeTargetBasedOnMovePowerOrDmg, 2, 0, 1, 8, 1, 0
	playsewithpan SE_M_DOUBLE_SLAP, SOUND_PAN_TARGET
	create_flashing_hitsplat_sprite ANIM_TARGET, 3, x=-12, y=10, relative_to=1, animation=1
	delay 5
	playsewithpan SE_M_DOUBLE_SLAP, SOUND_PAN_TARGET
	create_flashing_hitsplat_sprite ANIM_TARGET, 3, x=12, y=-10, relative_to=1, animation=1
	waitforvisualfinish
	call G58_UnsetBackground
	waitforvisualfinish
	end

Move_QUICK_GUARD:
	goto Move_BULK_UP

Move_ALLY_SWITCH:
	goto Move_CALM_MIND

Move_SCALD:
	loadspritegfx ANIM_TAG_WATER_IMPACT
	loadspritegfx ANIM_TAG_WATER_ORB
	monbg ANIM_DEF_PARTNER
	splitbgprio ANIM_TARGET
	setalpha 12, 8
	createvisualtask AnimTask_ShakeMon, 5, ANIM_ATTACKER, 0, 2, 40, 1
	delay 6
	panse SE_M_HYDRO_PUMP, SOUND_PAN_ATTACKER, SOUND_PAN_TARGET, +2, 0
	createvisualtask AnimTask_StartSinAnimTimer, 5, 100
	call G58_ScaldBeams
	call G58_ScaldBeams
	call G58_ScaldBeams
	blend_color_cycle selector=F_PAL_TARGET, delay=2, num_blends=2, initial_blend_y=0, target_blend_y=11, color=RGB_RED
	createvisualtask AnimTask_ShakeMon, 5, ANIM_TARGET, 3, 0, 37, 1
	call G58_ScaldHitSplats
	call G58_ScaldBeams
	call G58_ScaldBeams
	call G58_ScaldHitSplats
	call G58_ScaldBeams
	call G58_ScaldBeams
	call G58_ScaldHitSplats
	call G58_ScaldBeams
	call G58_ScaldBeams
	call G58_ScaldHitSplats
	call G58_ScaldBeams
	call G58_ScaldBeams
	call G58_ScaldHitSplats
	delay 2
	call G58_ScaldHitSplats
	waitforvisualfinish
	clearmonbg ANIM_DEF_PARTNER
	blendoff
	end

Move_SHELL_SMASH:
	goto Move_GROWL

Move_HEAL_PULSE:
	loadspritegfx ANIM_TAG_BLUE_STAR
	loadspritegfx ANIM_TAG_GREEN_SPARKLE
	loadspritegfx ANIM_TAG_THIN_RING
	playsewithpan SE_M_MORNING_SUN, SOUND_PAN_TARGET
	createsprite gUproarRingSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, 0, 0, RGB(13, 31, 12), 8
	create_present_heal_particle_sprite ANIM_TARGET, 4, initial_x=-16, initial_y=32, velocity_y=-3
	delay 3
	create_present_heal_particle_sprite ANIM_TARGET, 4, initial_x=16, initial_y=32, velocity_y=-3, unused3=-1
	delay 3
	createsprite gUproarRingSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, 0, 0, RGB(13, 31, 12), 8
	create_present_heal_particle_sprite ANIM_TARGET, 4, initial_x=32, initial_y=32, velocity_y=-3
	delay 3
	create_present_heal_particle_sprite ANIM_TARGET, 4, initial_x=-32, initial_y=32, velocity_y=-3
	delay 3
	createsprite gUproarRingSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, 0, 0, RGB(13, 31, 12), 8
	create_present_heal_particle_sprite ANIM_TARGET, 4, initial_x=0, initial_y=32, velocity_y=-3
	delay 3
	create_present_heal_particle_sprite ANIM_TARGET, 4, initial_x=-8, initial_y=32, velocity_y=-3
	delay 3
	createsprite gUproarRingSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, 0, 0, RGB(13, 31, 12), 8
	create_present_heal_particle_sprite ANIM_TARGET, 4, initial_x=-8, initial_y=32, velocity_y=-3
	delay 3
	create_present_heal_particle_sprite ANIM_TARGET, 4, initial_x=24, initial_y=32, velocity_y=-3
	delay 3
	create_present_heal_particle_sprite ANIM_TARGET, 4, initial_x=-24, initial_y=32, velocity_y=-3
	waitforvisualfinish
	waitsound
	call G58_HealingEffect2
	end

Move_HEX:
	goto Move_SHADOW_BALL

Move_SKY_DROP:
	goto Move_WING_ATTACK

Move_SHIFT_GEAR:
	goto Move_IRON_DEFENSE

Move_CIRCLE_THROW:
	goto Move_KARATE_CHOP

Move_INCINERATE:
	goto Move_FLAMETHROWER

Move_QUASH:
	goto Move_TORMENT

Move_ACROBATICS:
	goto Move_WING_ATTACK

Move_REFLECT_TYPE:
	goto Move_GROWL

Move_RETALIATE:
	loadspritegfx ANIM_TAG_CUT
	monbg ANIM_DEF_PARTNER
	setalpha 9, 8
	createvisualtask AnimTask_AttackerPunchWithTrace, 2, 28968, 10
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_BG, 1, 0, 16, RGB_BLACK
	waitforvisualfinish
	playsewithpan SE_M_CUT, SOUND_PAN_TARGET
	createsprite gGuillotineSpriteTemplate, ANIM_ATTACKER, 2, 0
	createsprite gGuillotineSpriteTemplate, ANIM_ATTACKER, 2, 1
	delay 56
	playsewithpan SE_M_DOUBLE_SLAP, SOUND_PAN_TARGET
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_BG, 1, 16, 16, RGB(29, 1, 1)
	waitforvisualfinish
	createvisualtask AnimTask_ShakeMon, 2, ANIM_TARGET, 0, 3, 15, 1
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_BG, 1, 16, 0, RGB(29, 1, 1)
	waitforvisualfinish
	clearmonbg ANIM_DEF_PARTNER
	blendoff
	end

Move_FINAL_GAMBIT:
	goto Move_KARATE_CHOP

Move_BESTOW:
	goto Move_GROWL

Move_INFERNO:
	loadspritegfx ANIM_TAG_SMALL_EMBER
	call G58_SetImpactBackground
	loopsewithpan SE_M_SACRED_FIRE2, SOUND_PAN_TARGET, 40, 4
	createvisualtask AnimTask_ShakeMon, 5, ANIM_TARGET, 0, 2, 94, 1
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_TARGET, 1, 0, 9, RGB_RED
	call G58_InfernoAnim
	call G58_InfernoAnim
	call G58_InfernoAnim
	call G58_InfernoAnim
	call G58_InfernoAnim
	call G58_InfernoAnim
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_TARGET, 1, 9, 0, RGB_RED
	restorebg
	waitbgfadein
	waitforvisualfinish
	end

Move_WATER_PLEDGE:
	goto Move_SURF

Move_FIRE_PLEDGE:
	goto Move_FLAMETHROWER

Move_GRASS_PLEDGE:
	goto Move_RAZOR_LEAF

Move_VOLT_SWITCH:
	goto Move_THUNDERBOLT

Move_STRUGGLE_BUG:
	goto Move_SIGNAL_BEAM

Move_BULLDOZE:
	goto Move_EARTHQUAKE

Move_FROST_BREATH:
	goto Move_ICE_BEAM

Move_DRAGON_TAIL:
	loadspritegfx ANIM_TAG_IMPACT
	loadspritegfx ANIM_TAG_ROCKS
	loadspritegfx ANIM_TAG_SLAM_HIT
	monbg ANIM_TARGET
	createvisualtask AnimTask_BlendParticle, 5, ANIM_TAG_SLAM_HIT, 2, 13, 13, RGB(14, 5, 23)
	waitforvisualfinish
	setalpha 12, 8
	playsewithpan SE_M_COMET_PUNCH, SOUND_PAN_ATTACKER
	createsprite gSlideMonToOffsetSpriteTemplate, ANIM_ATTACKER, 2, ANIM_ATTACKER, 20, 3, 0, 4
	delay 1
	createsprite gSlamHitSpriteTemplate, ANIM_ATTACKER, 2, 0, 0
	delay 3
	playsewithpan SE_M_MEGA_KICK2, SOUND_PAN_TARGET
	create_basic_hitsplat_sprite ANIM_ATTACKER, 3, x=0, y=0, relative_to=ANIM_TARGET, animation=1
	createsprite gSlideMonToOffsetSpriteTemplate, ANIM_ATTACKER, 2, ANIM_TARGET, -12, 10, 0, 3
	createsprite gSlideMonToOriginalPosSpriteTemplate, ANIM_ATTACKER, 2, ANIM_ATTACKER, 0, 5
	call G58_TailWhackDown
	delay 3
	createvisualtask AnimTask_ShakeMonInPlace, 2, ANIM_TARGET, 0, 3, 6, 1
	waitforvisualfinish
	delay 5
	createsprite gSlideMonToOriginalPosSpriteTemplate, ANIM_ATTACKER, 2, ANIM_TARGET, 0, 6
	waitforvisualfinish
	clearmonbg ANIM_TARGET
	blendoff
	end

Move_WORK_UP:
	loadspritegfx ANIM_TAG_FOCUS_ENERGY
	playsewithpan SE_M_DRAGON_RAGE, SOUND_PAN_ATTACKER
	createsprite gEndureEnergySpriteTemplate, ANIM_ATTACKER, 2, 0, -28, 26, 2
	createsprite gEndureEnergySpriteTemplate, ANIM_ATTACKER, 2, 0, 28, 26, 2
	delay 5
	createsprite gEndureEnergySpriteTemplate, ANIM_ATTACKER, 2, 0, -23, 10, 2
	createsprite gEndureEnergySpriteTemplate, ANIM_ATTACKER, 2, 0, 23, 10, 2
	delay 5
	createsprite gEndureEnergySpriteTemplate, ANIM_ATTACKER, 2, 0, -18, 26, 3
	createsprite gEndureEnergySpriteTemplate, ANIM_ATTACKER, 2, 0, 18, 26, 3
	delay 14
	blend_color_cycle selector=F_PAL_ATTACKER, delay=2, num_blends=2, initial_blend_y=0, target_blend_y=11, color=RGB(31, 2, 4)
	createvisualtask AnimTask_ShakeMon2, 2, ANIM_ATTACKER, 1, 0, 32, 1
	createsprite gEndureEnergySpriteTemplate, ANIM_ATTACKER, 2, 0, -13, 26, 2
	createsprite gEndureEnergySpriteTemplate, ANIM_ATTACKER, 2, 0, 13, 26, 2
	delay 5
	createsprite gEndureEnergySpriteTemplate, ANIM_ATTACKER, 2, 0, -8, 10, 2
	createsprite gEndureEnergySpriteTemplate, ANIM_ATTACKER, 2, 0, 8, 10, 2
	delay 5
	createsprite gEndureEnergySpriteTemplate, ANIM_ATTACKER, 2, 0, -3, 26, 3
	createsprite gEndureEnergySpriteTemplate, ANIM_ATTACKER, 2, 0, 3, 26, 3
	delay 14
	createsprite gEndureEnergySpriteTemplate, ANIM_ATTACKER, 2, 0, -2, 26, 2
	createsprite gEndureEnergySpriteTemplate, ANIM_ATTACKER, 2, 0, 2, 26, 2
	delay 5
	createsprite gEndureEnergySpriteTemplate, ANIM_ATTACKER, 2, 0, -7, 10, 2
	createsprite gEndureEnergySpriteTemplate, ANIM_ATTACKER, 2, 0, 7, 10, 2
	delay 5
	createsprite gEndureEnergySpriteTemplate, ANIM_ATTACKER, 2, 0, -12, 26, 3
	createsprite gEndureEnergySpriteTemplate, ANIM_ATTACKER, 2, 0, 12, 26, 3
	waitforvisualfinish
	end

Move_ELECTROWEB:
	goto Move_THUNDERBOLT

Move_WILD_CHARGE:
	loadspritegfx ANIM_TAG_CIRCLE_OF_LIGHT
	loadspritegfx ANIM_TAG_IMPACT
	loadspritegfx ANIM_TAG_SPARK_2
	monbg ANIM_TARGET
	setalpha 12, 8
	playsewithpan SE_M_CHARGE, SOUND_PAN_ATTACKER
	createsprite gGrowingShockWaveOrbSpriteTemplate, ANIM_ATTACKER, 2
	delay 30
	waitforvisualfinish
	loopsewithpan SE_M_HARDEN, SOUND_PAN_ATTACKER, 28, 2
	call G58_ScreenFlash
	waitforvisualfinish
	call G58_ScreenFlash
	waitforvisualfinish
	playsewithpan SE_M_TAKE_DOWN, SOUND_PAN_ATTACKER
	createsprite gVerticalDipSpriteTemplate, ANIM_ATTACKER, 2, 6, 1, 0
	waitforvisualfinish
	delay 11
	createsprite gSlideMonToOffsetSpriteTemplate, ANIM_ATTACKER, 2, ANIM_ATTACKER, 26, 0, 0, 5
	delay 6
	create_basic_hitsplat_sprite ANIM_ATTACKER, 4, x=-10, y=0, relative_to=ANIM_TARGET, animation=0
	createvisualtask AnimTask_ShakeMon, 2, ANIM_TARGET, 0, 3, 16, 1
	playsewithpan SE_M_MEGA_KICK2, SOUND_PAN_TARGET
	call G58_ElectricityEffect
	waitforvisualfinish
	createsprite gSlideMonToOriginalPosSpriteTemplate, ANIM_ATTACKER, 2, ANIM_ATTACKER, 0, 5
	waitforvisualfinish
	clearmonbg ANIM_TARGET
	blendoff
	end

Move_DRILL_RUN:
	loadspritegfx ANIM_TAG_HORN_HIT
	loadspritegfx ANIM_TAG_IMPACT
	setalpha 12, 8
	fadetobg BG_FISSURE
	waitbgfadeout
	createvisualtask AnimTask_PositionFissureBgOnBattler, 5, ANIM_TARGET, 5, -1
	call G58_CreateSharpDrill
	waitforvisualfinish
	call G58_UnsetBackground
	end

Move_DUAL_CHOP:
	goto Move_DRAGON_CLAW

Move_HEART_STAMP:
	goto Move_PSYCHIC

Move_HORN_LEECH:
	goto Move_RAZOR_LEAF

Move_SACRED_SWORD:
	goto Move_KARATE_CHOP

Move_RAZOR_SHELL:
	goto Move_SURF

Move_HEAT_CRASH:
	goto Move_FLAMETHROWER

Move_LEAF_TORNADO:
	goto Move_RAZOR_LEAF

Move_STEAMROLLER:
	loadspritegfx ANIM_TAG_IMPACT
	monbg ANIM_TARGET
	setalpha 12, 8
	createsprite gSlideMonToOffsetSpriteTemplate, ANIM_ATTACKER, 2, ANIM_ATTACKER, 26, 0, 0, 5
	delay 6
	create_basic_hitsplat_sprite ANIM_ATTACKER, 4, x=-10, y=0, relative_to=ANIM_TARGET, animation=0
	loopsewithpan SE_M_MEGA_KICK2, SOUND_PAN_TARGET, 6, 3
	delay 1
	createsprite gSlideMonToOffsetSpriteTemplate, ANIM_ATTACKER, 2, ANIM_TARGET, -28, 0, 0, 3
	create_basic_hitsplat_sprite ANIM_ATTACKER, 4, x=0, y=3, relative_to=ANIM_TARGET, animation=0
	delay 1
	create_basic_hitsplat_sprite ANIM_ATTACKER, 4, x=10, y=6, relative_to=ANIM_TARGET, animation=0
	delay 1
	waitforvisualfinish
	createvisualtask AnimTask_ShakeMonInPlace, 2, ANIM_TARGET, 4, 0, 12, 1
	waitforvisualfinish
	delay 10
	createsprite gSlideMonToOriginalPosSpriteTemplate, ANIM_ATTACKER, 2, 0, 0, 6
	delay 5
	createsprite gSlideMonToOriginalPosSpriteTemplate, ANIM_ATTACKER, 2, 1, 0, 6
	waitforvisualfinish
	clearmonbg ANIM_TARGET
	blendoff
	end

Move_COTTON_GUARD:
	goto Move_INGRAIN

Move_NIGHT_DAZE:
	goto Move_CRUNCH

Move_PSYSTRIKE:
	monbg ANIM_TARGET
	setalpha 8, 8
	createvisualtask AnimTask_ShakeMon2, 2, ANIM_ATTACKER, 1, 0, 10, 1
	blend_color_cycle selector=F_PAL_ATTACKER, delay=0, num_blends=2, initial_blend_y=0, target_blend_y=8, color=RGB_RED
	waitforvisualfinish
	invert_screen_color scenery=1, attacker=1, target=1
	loopsewithpan SE_M_SUPERSONIC, SOUND_PAN_TARGET, 10, 3
	createvisualtask AnimTask_ShakeMon, 2, ANIM_TARGET, 5, 0, 15, 1
	createvisualtask AnimTask_ScaleMonAndRestore, 5, -9, -9, 15, ANIM_TARGET, 1
	waitforvisualfinish
	invert_screen_color scenery=1, attacker=1, target=1
	waitforvisualfinish
	clearmonbg ANIM_TARGET
	blendoff
	end

Move_TAIL_SLAP:
	goto Move_TAKE_DOWN

Move_HURRICANE:
	goto Move_WING_ATTACK

Move_HEAD_CHARGE:
	goto Move_TAKE_DOWN

Move_GEAR_GRIND:
	goto Move_IRON_TAIL

Move_SEARING_SHOT:
	goto Move_FLAMETHROWER

Move_TECHNO_BLAST:
	goto Move_TAKE_DOWN

Move_RELIC_SONG:
	loadspritegfx ANIM_TAG_IMPACT
	loadspritegfx ANIM_TAG_JAGGED_MUSIC_NOTE
	loadspritegfx ANIM_TAG_MUSIC_NOTES
	loadspritegfx ANIM_TAG_THIN_RING
	monbg ANIM_DEF_PARTNER
	createvisualtask AnimTask_MusicNotesRainbowBlend, 2
	waitforvisualfinish
	createvisualtask SoundTask_PlayCryWithEcho, 2, FALSE
	createvisualtask AnimTask_UproarDistortion, 2, 0
	createsprite gUproarRingSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, 0, 0, RGB_RED, 8
	createsprite gJaggedMusicNoteSpriteTemplate, ANIM_ATTACKER, 2, 0, 29, -12, 0
	createsprite gJaggedMusicNoteSpriteTemplate, ANIM_ATTACKER, 2, 0, -12, -29, 1
	createsprite gWavyMusicNotesSpriteTemplate, ANIM_TARGET, 2, 7, 0, 12
	delay 5
	createsprite gWavyMusicNotesSpriteTemplate, ANIM_TARGET, 2, 6, 1, 12
	delay 5
	createvisualtask AnimTask_UproarDistortion, 2, 0
	createsprite gUproarRingSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, 0, 0, RGB_RED, 8
	createsprite gJaggedMusicNoteSpriteTemplate, ANIM_ATTACKER, 2, 0, 12, -29, 1
	createsprite gJaggedMusicNoteSpriteTemplate, ANIM_ATTACKER, 2, 0, -29, -12, 0
	createsprite gWavyMusicNotesSpriteTemplate, ANIM_TARGET, 2, 1, 2, 12
	delay 5
	createsprite gWavyMusicNotesSpriteTemplate, ANIM_TARGET, 2, 2, 3, 12
	delay 5
	createvisualtask AnimTask_UproarDistortion, 2, 0
	createsprite gUproarRingSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, 0, 0, RGB_RED, 8
	createsprite gJaggedMusicNoteSpriteTemplate, ANIM_ATTACKER, 2, 0, 24, -24, 1
	createsprite gJaggedMusicNoteSpriteTemplate, ANIM_ATTACKER, 2, 0, -24, -24, 0
	createsprite gWavyMusicNotesSpriteTemplate, ANIM_TARGET, 2, 3, 0, 12
	delay 5
	createsprite gWavyMusicNotesSpriteTemplate, ANIM_TARGET, 2, 2, 1, 12
	delay 5
	createvisualtask AnimTask_UproarDistortion, 2, 0
	createsprite gUproarRingSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, 0, 0, RGB_RED, 8
	createsprite gJaggedMusicNoteSpriteTemplate, ANIM_ATTACKER, 2, 0, 29, -12, 0
	createsprite gJaggedMusicNoteSpriteTemplate, ANIM_ATTACKER, 2, 0, -12, -29, 1
	createsprite gWavyMusicNotesSpriteTemplate, ANIM_TARGET, 2, 5, 2, 12
	delay 5
	createsprite gWavyMusicNotesSpriteTemplate, ANIM_TARGET, 2, 6, 3, 12
	delay 5
	createvisualtask AnimTask_UproarDistortion, 2, 0
	createsprite gUproarRingSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, 0, 0, RGB_RED, 8
	createsprite gJaggedMusicNoteSpriteTemplate, ANIM_ATTACKER, 2, 0, 12, -29, 1
	createsprite gJaggedMusicNoteSpriteTemplate, ANIM_ATTACKER, 2, 0, -29, -12, 0
	createsprite gWavyMusicNotesSpriteTemplate, ANIM_TARGET, 2, 2, 0, 12
	delay 5
	createsprite gWavyMusicNotesSpriteTemplate, ANIM_TARGET, 2, 2, 1, 12
	delay 5
	createvisualtask AnimTask_UproarDistortion, 2, 0
	createsprite gUproarRingSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, 0, 0, RGB_RED, 8
	createsprite gJaggedMusicNoteSpriteTemplate, ANIM_ATTACKER, 2, 0, 24, -24, 1
	createsprite gJaggedMusicNoteSpriteTemplate, ANIM_ATTACKER, 2, 0, -24, -24, 0
	createsprite gWavyMusicNotesSpriteTemplate, ANIM_TARGET, 2, 1, 2, 12
	delay 5
	createsprite gWavyMusicNotesSpriteTemplate, ANIM_TARGET, 2, 5, 3, 12
	delay 5
	createvisualtask AnimTask_UproarDistortion, 2, 0
	createsprite gUproarRingSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, 0, 0, RGB_RED, 8
	createsprite gJaggedMusicNoteSpriteTemplate, ANIM_ATTACKER, 2, 0, 29, -12, 0
	createsprite gJaggedMusicNoteSpriteTemplate, ANIM_ATTACKER, 2, 0, -12, -29, 1
	waitforvisualfinish
	create_basic_hitsplat_sprite ANIM_TARGET, 3, x=-32, y=-16, relative_to=ANIM_TARGET, animation=3
	playsewithpan SE_M_COMET_PUNCH, SOUND_PAN_TARGET
	createvisualtask AnimTask_ShakeMonInPlace, 2, ANIM_TARGET, 3, 0, 12, 1
	createvisualtask AnimTask_ShakeMonInPlace, 2, ANIM_DEF_PARTNER, 3, 0, 12, 1
	delay 5
	create_random_pos_hitsplat_sprite ANIM_TARGET, 3, relative_to=ANIM_TARGET, animation=3
	playsewithpan SE_M_COMET_PUNCH, SOUND_PAN_TARGET
	delay 5
	create_random_pos_hitsplat_sprite ANIM_TARGET, 3, relative_to=ANIM_TARGET, animation=3
	playsewithpan SE_M_COMET_PUNCH, SOUND_PAN_TARGET
	delay 5
	create_basic_hitsplat_sprite ANIM_TARGET, 3, x=32, y=20, relative_to=ANIM_TARGET, animation=3
	playsewithpan SE_M_COMET_PUNCH, SOUND_PAN_TARGET
	waitforvisualfinish
	clearmonbg ANIM_DEF_PARTNER
	createvisualtask AnimTask_MusicNotesClearRainbowBlend, 2
	waitforvisualfinish
	end

Move_SECRET_SWORD:
	goto Move_KARATE_CHOP

Move_GLACIATE:
	goto Move_ICE_BEAM

Move_BOLT_STRIKE:
	goto Move_THUNDERBOLT

Move_BLUE_FLARE:
	goto Move_FLAMETHROWER

Move_FIERY_DANCE:
	loadspritegfx ANIM_TAG_SMALL_EMBER
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_BG, 1, 0, 12, RGB(30, 5, 5)
	waitforvisualfinish
	playsewithpan SE_M_SACRED_FIRE2, SOUND_PAN_TARGET
	createvisualtask AnimTask_ShakeMon, 5, ANIM_TARGET, 0, 2, 64, 1
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_TARGET, 1, 0, 9, RGB_RED
	call G58_FireSpinEffect
	call G58_FireSpinEffect
	call G58_FireSpinEffect
	playsewithpan SE_M_SACRED_FIRE2, SOUND_PAN_TARGET
	call G58_FireSpinEffect
	call G58_FireSpinEffect
	call G58_FireSpinEffect
	waitforvisualfinish
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_TARGET, 1, 9, 0, RGB_RED
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_BG, 1, 12, 0, RGB(30, 5, 5)
	waitforvisualfinish
	end

Move_FREEZE_SHOCK:
	goto Move_ICE_BEAM

Move_ICE_BURN:
	goto Move_ICE_BEAM

Move_SNARL:
	loadspritegfx ANIM_TAG_IMPACT
	loadspritegfx ANIM_TAG_JAGGED_MUSIC_NOTE
	loadspritegfx ANIM_TAG_THIN_RING
	monbg ANIM_DEF_PARTNER
	fadetobg BG_DARK
	waitbgfadeout
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
	create_basic_hitsplat_sprite ANIM_TARGET, 3, x=-32, y=-16, relative_to=ANIM_TARGET, animation=3
	playsewithpan SE_M_COMET_PUNCH, SOUND_PAN_TARGET
	createvisualtask AnimTask_ShakeMonInPlace, 2, ANIM_TARGET, 3, 0, 12, 1
	createvisualtask AnimTask_ShakeMonInPlace, 2, ANIM_DEF_PARTNER, 3, 0, 12, 1
	delay 4
	create_random_pos_hitsplat_sprite ANIM_TARGET, 3, relative_to=ANIM_TARGET, animation=3
	playsewithpan SE_M_COMET_PUNCH, SOUND_PAN_TARGET
	delay 4
	create_random_pos_hitsplat_sprite ANIM_TARGET, 3, relative_to=ANIM_TARGET, animation=3
	playsewithpan SE_M_COMET_PUNCH, SOUND_PAN_TARGET
	delay 4
	create_basic_hitsplat_sprite ANIM_TARGET, 3, x=32, y=20, relative_to=ANIM_TARGET, animation=3
	playsewithpan SE_M_COMET_PUNCH, SOUND_PAN_TARGET
	waitforvisualfinish
	call G58_UnsetBackground
	clearmonbg ANIM_DEF_PARTNER
	blendoff
	end

Move_ICICLE_CRASH:
	goto Move_ICE_BEAM

Move_V_CREATE:
	goto Move_FLAMETHROWER

Move_FUSION_FLARE:
	goto Move_FLAMETHROWER

Move_FUSION_BOLT:
	goto Move_THUNDERBOLT

Move_FLYING_PRESS:
	goto Move_KARATE_CHOP

Move_MAT_BLOCK:
	goto Move_BULK_UP

Move_BELCH:
	goto Move_SLUDGE_BOMB

Move_ROTOTILLER:
	loadspritegfx ANIM_TAG_DIRT_MOUND
	loadspritegfx ANIM_TAG_MUD_SAND
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_BG, 1, 0, 16, RGB(12, 9, 4)
	waitforvisualfinish
	createsprite gDirtMoundSpriteTemplate, ANIM_ATTACKER, 1, 0, 0, 180
	createsprite gDirtMoundSpriteTemplate, ANIM_ATTACKER, 1, 0, 1, 180
	createvisualtask AnimTask_TranslateMonEllipticalRespectSide, 2, ANIM_ATTACKER, 6, 4, 2, 4
	call G58_DigThrowDirt
	call G58_DigThrowDirt
	createvisualtask AnimTask_TranslateMonEllipticalRespectSide, 2, ANIM_ATTACKER, 6, 4, 2, 4
	call G58_DigThrowDirt
	call G58_DigThrowDirt
	waitforvisualfinish
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_BG, 1, 16, 0, RGB(12, 9, 4)
	waitforvisualfinish
	end

Move_STICKY_WEB:
	loadspritegfx ANIM_TAG_SPIDER_WEB
	loadspritegfx ANIM_TAG_WEB_THREAD
	monbg ANIM_DEF_PARTNER
	delay 0
	simple_palette_blend unused_subpriority_offset=5, selector=F_PAL_BG, delay=2, initial_blend_y=0, target_blend_y=9, color=RGB_BLACK
	waitforvisualfinish
	splitbgprio ANIM_TARGET
	loopsewithpan SE_M_STRING_SHOT, SOUND_PAN_ATTACKER, 9, 6
	call G58_StringShotThread
	call G58_StringShotThread
	call G58_StringShotThread
	call G58_StringShotThread
	call G58_StringShotThread
	call G58_StringShotThread
	call G58_StringShotThread
	call G58_StringShotThread
	call G58_StringShotThread
	call G58_StringShotThread
	call G58_StringShotThread
	call G58_StringShotThread
	call G58_StringShotThread
	call G58_StringShotThread
	waitforvisualfinish
	playsewithpan SE_M_STRING_SHOT2, SOUND_PAN_TARGET
	createsprite gSpiderWebSpriteTemplate, ANIM_ATTACKER, 2, 0, 0, TRUE
	delay 50
	playsewithpan SE_M_STRING_SHOT2, SOUND_PAN_TARGET
	createsprite gSpiderWebSpriteTemplate, ANIM_ATTACKER, 2, -30, 30, TRUE
	delay 50
	playsewithpan SE_M_STRING_SHOT2, SOUND_PAN_TARGET
	createsprite gSpiderWebSpriteTemplate, ANIM_ATTACKER, 2, 30, -30, TRUE
	waitforvisualfinish
	clearmonbg ANIM_DEF_PARTNER
	delay 1
	simple_palette_blend unused_subpriority_offset=5, selector=F_PAL_BG, delay=2, initial_blend_y=9, target_blend_y=0, color=RGB_BLACK
	end

Move_FELL_STINGER:
	loadspritegfx ANIM_TAG_IMPACT
	loadspritegfx ANIM_TAG_NEEDLE
	monbg ANIM_TARGET
	splitbgprio ANIM_TARGET
	fadetobg BG_DARK
	waitbgfadein
	delay 0
	playsewithpan SE_M_RAZOR_WIND2, SOUND_PAN_ATTACKER
	create_linear_stinger_sprite ANIM_TARGET, 2, initial_x=20, initial_y=0, target_x=-8, target_y=0, duration=20
	waitforvisualfinish
	create_basic_hitsplat_sprite ANIM_ATTACKER, 3, x=0, y=0, relative_to=ANIM_TARGET, animation=2
	createvisualtask AnimTask_ShakeMon2, 2, ANIM_TARGET, 2, 0, 5, 1
	playsewithpan SE_M_HORN_ATTACK, SOUND_PAN_TARGET
	waitforvisualfinish
	delay 1
	restorebg
	waitbgfadein
	clearmonbg ANIM_TARGET
	end

Move_PHANTOM_FORCE:
	goto Move_SHADOW_BALL

Move_TRICK_OR_TREAT:
	goto Move_CONFUSE_RAY

Move_NOBLE_ROAR:
	loadspritegfx ANIM_TAG_NOISE_LINE
	monbg ANIM_ATTACKER
	splitbgprio ANIM_ATTACKER
	setalpha 8, 8
	simple_palette_blend selector=F_PAL_BG, delay=1, initial_blend_y=0, target_blend_y=8, color=RGB(4, 0, 0)
	waitforvisualfinish
	createvisualtask SoundTask_PlayCryHighPitch, 2, ANIM_ATTACKER, 2
	createvisualtask AnimTask_ScaleMonAndRestore, 5, -5, -5, 10, ANIM_ATTACKER, 1
	call G58_RoarEffect
	delay 10
	createvisualtask AnimTask_ShakeMon2, 2, ANIM_TARGET, 1, 0, 9, 1
	delay 10
	simple_palette_blend selector=F_PAL_BG, delay=1, initial_blend_y=8, target_blend_y=0, color=RGB(4, 0, 0)
	waitforvisualfinish
	clearmonbg ANIM_ATTACKER
	blendoff
	waitforvisualfinish
	delay 20
	end

Move_ION_DELUGE:
	goto Move_CHARGE

Move_PARABOLIC_CHARGE:
	goto Move_THUNDERBOLT

Move_FORESTS_CURSE:
	goto Move_INGRAIN

Move_PETAL_BLIZZARD:
	goto Move_RAZOR_LEAF

Move_FREEZE_DRY:
	loadspritegfx ANIM_TAG_ICE_CRYSTALS
	monbg ANIM_DEF_PARTNER
	setalpha 12, 8
	simple_palette_blend selector=F_PAL_BG, delay=1, initial_blend_y=0, target_blend_y=7, color=RGB_BLACK
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_TARGET, 2, 0, 9, RGB(12, 26, 31)
	delay 20
	call G58_CreateIceSpiral
	delay 17
	createvisualtask AnimTask_ShakeMon, 5, ANIM_TARGET, 0, 5, 3, 1
	waitforvisualfinish
	delay 15
	call G58_IceCrystalEffectShort
	delay 5
	createvisualtask AnimTask_FrozenIceCube, 2
	waitplaysewithpan SE_M_HAIL, SOUND_PAN_TARGET, 17
	waitforvisualfinish
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_TARGET, 2, 9, 0, RGB(12, 26, 31)
	waitforvisualfinish
	simple_palette_blend selector=F_PAL_BG, delay=0, initial_blend_y=7, target_blend_y=0, color=RGB_BLACK
	waitforvisualfinish
	clearmonbg ANIM_DEF_PARTNER
	blendoff
	end

Move_DISARMING_VOICE:
	loadspritegfx ANIM_TAG_NOISE_LINE
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_BG, 1, 0, 8, RGB(29, 19, 27)
	waitforvisualfinish
	createvisualtask SoundTask_PlayDoubleCry, 2, ANIM_ATTACKER, DOUBLE_CRY_GROWL
	call G58_RoarEffect
	delay 10
	createvisualtask AnimTask_ShakeMon2, 2, ANIM_TARGET, 1, 0, 26, 1
	createvisualtask AnimTask_ShakeMon2, 2, ANIM_DEF_PARTNER, 1, 0, 26, 1
	waitforvisualfinish
	createvisualtask SoundTask_WaitForCry, 5
	waitforvisualfinish
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_BG, 1, 8, 0, RGB(29, 19, 27)
	waitforvisualfinish
	end

Move_PARTING_SHOT:
	loadspritegfx ANIM_TAG_NOISE_LINE
	fadetobg BG_DARK
	waitbgfadein
	delay 0
	createvisualtask SoundTask_PlayDoubleCry, 2, ANIM_ATTACKER, DOUBLE_CRY_GROWL
	call G58_RoarEffect
	delay 10
	createvisualtask AnimTask_ShakeMon2, 2, ANIM_TARGET, 1, 0, 9, 1
	createvisualtask AnimTask_ShakeMon2, 2, ANIM_DEF_PARTNER, 1, 0, 9, 1
	waitforvisualfinish
	createvisualtask SoundTask_WaitForCry, 5
	waitforvisualfinish
	delay 1
	restorebg
	waitbgfadein
	end

Move_TOPSY_TURVY:
	goto Move_TORMENT

Move_DRAINING_KISS:
	loadspritegfx ANIM_TAG_BLUE_STAR
	loadspritegfx ANIM_TAG_IMPACT
	loadspritegfx ANIM_TAG_ORBS
	loadspritegfx ANIM_TAG_RED_HEART
	loopsewithpan SE_M_CHARM, SOUND_PAN_ATTACKER, 12, 3
	call G58_CreateRedHeart
	waitforvisualfinish
	playsewithpan SE_M_ATTRACT, SOUND_PAN_TARGET
	call G58_CreateRedHeartBurst
	waitforvisualfinish
	create_basic_hitsplat_sprite ANIM_ATTACKER, 2, x=0, y=0, relative_to=ANIM_TARGET, animation=2
	delay 5
	createvisualtask AnimTask_ShakeMon, 5, ANIM_TARGET, 0, 5, 5, 1
	waitforvisualfinish
	delay 3
	call G58_MegaDrainAbsorbEffect
	waitforvisualfinish
	delay 15
	call G58_HealingEffect
	waitforvisualfinish
	end

Move_CRAFTY_SHIELD:
	goto Move_MOONLIGHT

Move_FLOWER_SHIELD:
	loadspritegfx ANIM_TAG_FLOWER
	setalpha 12, 8
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_BG, 1, 0, 8, RGB(29, 19, 27)
	waitforvisualfinish
	playsewithpan SE_M_PETAL_DANCE, SOUND_PAN_ATTACKER
	call G58_CreatePetals
	waitforvisualfinish
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_BG, 1, 8, 0, RGB(29, 19, 27)
	waitforvisualfinish
	blendoff
	end

Move_GRASSY_TERRAIN:
	goto Move_INGRAIN

Move_MISTY_TERRAIN:
	goto Move_MOONLIGHT

Move_ELECTRIFY:
	goto Move_CHARGE

Move_PLAY_ROUGH:
	goto Move_SWEET_KISS

Move_FAIRY_WIND:
	goto Move_SWEET_KISS

Move_MOONBLAST:
	loadspritegfx ANIM_TAG_MOON
	loadspritegfx ANIM_TAG_SMALL_BUBBLES
	setalpha 0, 16
	simple_palette_blend selector=F_PAL_BG, delay=1, initial_blend_y=0, target_blend_y=16, color=RGB_BLACK
	waitforvisualfinish
	createsprite gMoonSpriteTemplate, ANIM_ATTACKER, 2, 120, 56
	createvisualtask AnimTask_AlphaFadeIn, 3, 0, 16, 16, 0, 1
	panse SE_M_BARRIER, SOUND_PAN_ATTACKER, SOUND_PAN_TARGET, 3, 0
	delay 40
	playsewithpan SE_M_STRING_SHOT, SOUND_PAN_ATTACKER
	createsprite gMistBallSpriteTemplate, ANIM_ATTACKER, 5, 0, 0, 0, 0, 30, 0
	delay 30
	loopsewithpan SE_M_SUPERSONIC, SOUND_PAN_TARGET, 5, 4
	createvisualtask AnimTask_ShakeMon, 2, ANIM_TARGET, 5, 0, 10, 0
	delay 4
	createvisualtask AnimTask_MoonlightEndFade, 2
	waitforvisualfinish
	blendoff
	end

Move_BOOMBURST:
	goto Move_TAKE_DOWN

Move_FAIRY_LOCK:
	setalpha 8, 8
	monbg ANIM_ATK_PARTNER
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_BG, 1, 0, 8, RGB(31, 24, 26)
	waitforvisualfinish
	loopsewithpan SE_M_SCRATCH, SOUND_PAN_TARGET, 6, 9
	createvisualtask AnimTask_VoltTackleBolt, 5, 7
	createvisualtask AnimTask_VoltTackleBolt, 5, 51
	createvisualtask AnimTask_VoltTackleBolt, 5, 2
	waitforvisualfinish
	createvisualtask AnimTask_VoltTackleBolt, 5, 50
	createvisualtask AnimTask_VoltTackleBolt, 5, 6
	createvisualtask AnimTask_VoltTackleBolt, 5, 53
	waitforvisualfinish
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_BG, 1, 8, 0, RGB(31, 24, 26)
	waitforvisualfinish
	clearmonbg ANIM_ATK_PARTNER
	blendoff
	end

Move_KINGS_SHIELD:
	goto Move_IRON_DEFENSE

Move_PLAY_NICE:
	loadspritegfx ANIM_TAG_RED_HEART
	loopsewithpan SE_M_SANDSTORM, SOUND_PAN_ATTACKER, 12, 3
	call G58_CreateRedHeart
	end

Move_CONFIDE:
	goto Move_GROWL

Move_DIAMOND_STORM:
	goto Move_ROCK_SLIDE

Move_STEAM_ERUPTION:
	goto Move_SURF

Move_HYPERSPACE_HOLE:
	goto Move_PSYCHIC

Move_WATER_SHURIKEN:
	goto Move_SURF

Move_MYSTICAL_FIRE:
	loadspritegfx ANIM_TAG_FIRE
	call G58_SetPsychicBackground
	setalpha 8, 8
	playse 177
	createvisualtask AnimTask_ShakeMon2, 2, ANIM_ATTACKER, 1, 0, 10, 1
	blend_color_cycle selector=F_PAL_ATTACKER, delay=0, num_blends=2, initial_blend_y=0, target_blend_y=8, color=RGB_WHITE
	waitforvisualfinish
	waitsound
	blendoff
	createsprite gLargeFlameScatterSpriteTemplate, ANIM_TARGET, 2, 0, 0, 30, 30, -1, 0
	delay 1
	createsprite gLargeFlameScatterSpriteTemplate, ANIM_TARGET, 2, 0, 0, 30, 30, 0, 1
	delay 1
	createsprite gLargeFlameScatterSpriteTemplate, ANIM_TARGET, 2, 0, 0, 30, 30, -1, -1
	delay 1
	createsprite gLargeFlameScatterSpriteTemplate, ANIM_TARGET, 2, 0, 0, 30, 30, 2, 1
	delay 1
	createsprite gLargeFlameScatterSpriteTemplate, ANIM_TARGET, 2, 0, 0, 30, 30, 1, -1
	delay 1
	createsprite gLargeFlameScatterSpriteTemplate, ANIM_TARGET, 2, 0, 0, 30, 30, -1, 1
	delay 1
	createsprite gLargeFlameScatterSpriteTemplate, ANIM_TARGET, 2, 0, 0, 30, 30, 1, -2
	delay 1
	createsprite gLargeFlameScatterSpriteTemplate, ANIM_TARGET, 2, 0, 0, 30, 30, 3, 1
	playse 140
	waitforvisualfinish
	call G58_UnsetBackground
	end

Move_SPIKY_SHIELD:
	loadspritegfx ANIM_TAG_GREEN_SPIKE
	loadspritegfx ANIM_TAG_PROTECT
	monbg ANIM_ATK_PARTNER
	splitbgprio ANIM_ATTACKER
	waitplaysewithpan SE_M_REFLECT, SOUND_PAN_ATTACKER, 16
	createsprite gProtectSpriteTemplate, ANIM_ATTACKER, 2, 24, 0, 90
	delay 15
	createsprite gNeedleArmSpikeSpriteTemplate, ANIM_TARGET, 2, 0, 1, 0, -32, 16
	delay 2
	createsprite gNeedleArmSpikeSpriteTemplate, ANIM_TARGET, 2, 0, 1, 22, -22, 16
	delay 2
	createsprite gNeedleArmSpikeSpriteTemplate, ANIM_TARGET, 2, 0, 1, 30, 0, 16
	delay 2
	createsprite gNeedleArmSpikeSpriteTemplate, ANIM_TARGET, 2, 0, 1, 20, 20, 16
	delay 2
	createsprite gNeedleArmSpikeSpriteTemplate, ANIM_TARGET, 2, 0, 1, 0, 28, 16
	delay 2
	createsprite gNeedleArmSpikeSpriteTemplate, ANIM_TARGET, 2, 0, 1, -19, 19, 16
	delay 2
	createsprite gNeedleArmSpikeSpriteTemplate, ANIM_TARGET, 2, 0, 1, -27, 0, 16
	delay 2
	createsprite gNeedleArmSpikeSpriteTemplate, ANIM_TARGET, 2, 0, 1, -18, -18, 16
	delay 2
	createsprite gNeedleArmSpikeSpriteTemplate, ANIM_TARGET, 2, 0, 1, 0, -25, 16
	delay 2
	createsprite gNeedleArmSpikeSpriteTemplate, ANIM_TARGET, 2, 0, 1, 17, -17, 16
	delay 2
	createsprite gNeedleArmSpikeSpriteTemplate, ANIM_TARGET, 2, 0, 1, 23, 0, 16
	delay 2
	createsprite gNeedleArmSpikeSpriteTemplate, ANIM_TARGET, 2, 0, 1, 16, 16, 16
	waitforvisualfinish
	clearmonbg ANIM_ATK_PARTNER
	end

Move_AROMATIC_MIST:
	loadspritegfx ANIM_TAG_PINK_PETAL
	playsewithpan SE_M_SWEET_SCENT, SOUND_PAN_ATTACKER
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_BG, 1, 0, 8, RGB(29, 19, 27)
	createsprite gSweetScentPetalSpriteTemplate, ANIM_ATTACKER, 2, 100, 0, 100
	delay 25
	setpan 0
	call G58_SweetScentEffect
	createsprite gSweetScentPetalSpriteTemplate, ANIM_ATTACKER, 2, 55, 0
	setpan SOUND_PAN_TARGET
	blend_color_cycle selector=F_PAL_DEF_SIDE, delay=1, num_blends=5, initial_blend_y=5, target_blend_y=13, color=RGB(31, 21, 21)
	call G58_SweetScentEffect
	waitforvisualfinish
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_BG, 1, 8, 0, RGB(29, 19, 27)
	waitforvisualfinish
	end

Move_EERIE_IMPULSE:
	goto Move_CHARGE

Move_VENOM_DRENCH:
	goto Move_TOXIC

Move_POWDER:
	goto Move_SPIDER_WEB

Move_GEOMANCY:
	goto Move_MOONLIGHT

Move_MAGNETIC_FLUX:
	goto Move_CHARGE

Move_HAPPY_HOUR:
	goto Move_GROWL

Move_ELECTRIC_TERRAIN:
	goto Move_CHARGE

Move_DAZZLING_GLEAM:
	loadspritegfx ANIM_TAG_IMPACT
	loadspritegfx ANIM_TAG_SPARKLE_2
	simple_palette_blend selector=F_PAL_BG, delay=2, initial_blend_y=0, target_blend_y=13, color=RGB(31, 25, 25)
	loopsewithpan SE_M_HEAL_BELL, SOUND_PAN_ATTACKER, 16, 3
	createsprite gGrantingStarsSpriteTemplate, ANIM_ATTACKER, 2, 12, -5, 0, 0, 32, 60
	waitforvisualfinish
	blend_color_cycle selector=F_PAL_ATTACKER, delay=0, num_blends=3, initial_blend_y=0, target_blend_y=16, color=RGB_WHITE
	createvisualtask AnimTask_BlendParticle, 5, ANIM_TAG_IMPACT, 0, 12, 12, RGB(31, 28, 28)
	createvisualtask AnimTask_ShakeMon, 2, ANIM_TARGET, 0, 3, 21, 1
	createvisualtask AnimTask_ShakeMon, 2, ANIM_DEF_PARTNER, 0, 3, 21, 1
	create_random_pos_hitsplat_sprite ANIM_TARGET, 3, relative_to=ANIM_TARGET, animation=2
	createvisualtask SoundTask_PlaySE1WithPanning, 5, SE_M_HYPER_BEAM, SOUND_PAN_TARGET
	delay 3
	create_random_pos_hitsplat_sprite ANIM_TARGET, 3, relative_to=ANIM_TARGET, animation=2
	createvisualtask SoundTask_PlaySE1WithPanning, 5, SE_M_HYPER_BEAM, SOUND_PAN_TARGET
	delay 3
	create_random_pos_hitsplat_sprite ANIM_TARGET, 3, relative_to=ANIM_TARGET, animation=2
	createvisualtask SoundTask_PlaySE1WithPanning, 5, SE_M_HYPER_BEAM, SOUND_PAN_TARGET
	delay 3
	create_random_pos_hitsplat_sprite ANIM_TARGET, 3, relative_to=ANIM_TARGET, animation=2
	createvisualtask SoundTask_PlaySE1WithPanning, 5, SE_M_HYPER_BEAM, SOUND_PAN_TARGET
	delay 3
	create_random_pos_hitsplat_sprite ANIM_TARGET, 3, relative_to=ANIM_TARGET, animation=2
	createvisualtask SoundTask_PlaySE1WithPanning, 5, SE_M_HYPER_BEAM, SOUND_PAN_TARGET
	delay 3
	create_random_pos_hitsplat_sprite ANIM_TARGET, 3, relative_to=ANIM_TARGET, animation=2
	createvisualtask SoundTask_PlaySE1WithPanning, 5, SE_M_HYPER_BEAM, SOUND_PAN_TARGET
	waitforvisualfinish
	simple_palette_blend selector=F_PAL_BG, delay=0, initial_blend_y=13, target_blend_y=0, color=RGB(31, 25, 25)
	waitforvisualfinish
	clearmonbg 0
	blendoff
	end

Move_CELEBRATE:
	goto Move_GROWL

Move_HOLD_HANDS:
	goto Move_GROWL

Move_BABY_DOLL_EYES:
	loadspritegfx ANIM_TAG_OPENING_EYE
	setalpha 8, 8
	monbg ANIM_DEF_PARTNER
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_BG, 0, 0, 10, RGB_WHITE
	waitforvisualfinish
	createsprite gOpeningEyeSpriteTemplate, ANIM_ATTACKER, 5, 0, 0, 1, 0
	delay 32
	playsewithpan SE_M_LEER, SOUND_PAN_ATTACKER
	createvisualtask AnimTask_ScaleMonAndRestore, 5, -5, -5, 10, ANIM_ATTACKER, 1
	waitforvisualfinish
	delay 10
	createvisualtask AnimTask_ShakeMon2, 2, ANIM_TARGET, 1, 0, 9, 1
	createvisualtask AnimTask_ShakeMon2, 2, ANIM_DEF_PARTNER, 1, 0, 9, 1
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_BG, 0, 10, 0, RGB_WHITE
	waitforvisualfinish
	clearmonbg ANIM_DEF_PARTNER
	blendoff
	end

Move_NUZZLE:
	loadspritegfx ANIM_TAG_IMPACT
	loadspritegfx ANIM_TAG_MAGENTA_HEART
	loadspritegfx ANIM_TAG_SPARK_2
	call G58_CreateMagentaHearts
	playsewithpan SE_M_CHARM, SOUND_PAN_ATTACKER
	waitforvisualfinish
	playsewithpan SE_M_THUNDERBOLT2, SOUND_PAN_TARGET
	create_basic_hitsplat_sprite ANIM_ATTACKER, 2, x=0, y=0, relative_to=ANIM_TARGET, animation=2
	call G58_ElectricityEffectNoSound
	waitforvisualfinish
	end

Move_HOLD_BACK:
	goto Move_TAKE_DOWN

Move_INFESTATION:
	goto Move_SIGNAL_BEAM

Move_POWER_UP_PUNCH:
	loadspritegfx ANIM_TAG_IMPACT
	monbg ANIM_TARGET
	setalpha 12, 8
	createvisualtask AnimTask_GrowAndShrink, 2
	playsewithpan SE_M_METRONOME, SOUND_PAN_ATTACKER
	waitforvisualfinish
	playsewithpan SE_M_DOUBLE_SLAP, SOUND_PAN_TARGET
	create_basic_hitsplat_sprite ANIM_TARGET, 2, x=0, y=0, relative_to=ANIM_TARGET, animation=1
	createvisualtask AnimTask_ShakeMon, 2, ANIM_TARGET, 3, 0, 6, 1
	waitforvisualfinish
	clearmonbg ANIM_TARGET
	blendoff
	end

Move_OBLIVION_WING:
	goto Move_WING_ATTACK

Move_THOUSAND_ARROWS:
	goto Move_EARTHQUAKE

Move_THOUSAND_WAVES:
	goto Move_EARTHQUAKE

Move_LANDS_WRATH:
	goto Move_EARTHQUAKE

Move_LIGHT_OF_RUIN:
	goto Move_SWEET_KISS

Move_ORIGIN_PULSE:
	goto Move_SURF

Move_PRECIPICE_BLADES:
	goto Move_EARTHQUAKE

Move_DRAGON_ASCENT:
	goto Move_WING_ATTACK

Move_HYPERSPACE_FURY:
	goto Move_CRUNCH

Move_SHORE_UP:
	goto Move_SAND_ATTACK

Move_FIRST_IMPRESSION:
	goto Move_SIGNAL_BEAM

Move_BANEFUL_BUNKER:
	goto Move_TOXIC

Move_SPIRIT_SHACKLE:
	goto Move_SHADOW_BALL

Move_DARKEST_LARIAT:
	goto Move_CRUNCH

Move_SPARKLING_ARIA:
	goto Move_SURF

Move_ICE_HAMMER:
	goto Move_ICE_BEAM

Move_FLORAL_HEALING:
	goto Move_MOONLIGHT

Move_HIGH_HORSEPOWER:
	goto Move_EARTHQUAKE

Move_STRENGTH_SAP:
	goto Move_INGRAIN

Move_SOLAR_BLADE:
	goto Move_RAZOR_LEAF

Move_LEAFAGE:
	goto Move_RAZOR_LEAF

Move_SPOTLIGHT:
	loadspritegfx ANIM_TAG_SPOTLIGHT
	createvisualtask AnimTask_CreateSpotlight, 2
	createvisualtask AnimTask_HardwarePaletteFade, 2, 248, 3, 0, 10, 0
	waitforvisualfinish
	playsewithpan SE_CONTEST_ICON_CHANGE, SOUND_PAN_ATTACKER
	createsprite gSpotlightSpriteTemplate, ANIM_TARGET, 2, 0, -8
	delay 64
	createvisualtask AnimTask_HardwarePaletteFade, 2, 248, 3, 10, 0, 1
	waitforvisualfinish
	createvisualtask AnimTask_RemoveSpotlight, 2
	end

Move_TOXIC_THREAD:
	goto Move_TOXIC

Move_LASER_FOCUS:
	goto Move_GROWL

Move_GEAR_UP:
	goto Move_IRON_DEFENSE

Move_THROAT_CHOP:
	goto Move_CRUNCH

Move_POLLEN_PUFF:
	goto Move_SIGNAL_BEAM

Move_ANCHOR_SHOT:
	goto Move_IRON_TAIL

Move_PSYCHIC_TERRAIN:
	goto Move_CALM_MIND

Move_LUNGE:
	goto Move_SIGNAL_BEAM

Move_FIRE_LASH:
	loadspritegfx ANIM_TAG_SLASH
	loadspritegfx ANIM_TAG_SMALL_EMBER
	monbg ANIM_DEF_PARTNER
	splitbgprio_foes ANIM_TARGET
	createsprite gFireSpiralOutwardSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, 56, 0
	playsewithpan SE_M_FLAME_WHEEL, SOUND_PAN_ATTACKER
	delay 2
	createsprite gFireSpiralOutwardSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, 56, 4
	playsewithpan SE_M_FLAME_WHEEL, SOUND_PAN_ATTACKER
	delay 2
	createsprite gFireSpiralOutwardSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, 56, 8
	playsewithpan SE_M_FLAME_WHEEL, SOUND_PAN_ATTACKER
	delay 2
	createsprite gFireSpiralOutwardSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, 56, 12
	playsewithpan SE_M_FLAME_WHEEL, SOUND_PAN_ATTACKER
	delay 2
	createsprite gFireSpiralOutwardSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, 56, 16
	playsewithpan SE_M_FLAME_WHEEL, SOUND_PAN_ATTACKER
	delay 2
	createsprite gFireSpiralOutwardSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, 56, 20
	playsewithpan SE_M_FLAME_WHEEL, SOUND_PAN_ATTACKER
	delay 2
	createsprite gFireSpiralOutwardSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, 56, 24
	playsewithpan SE_M_FLAME_WHEEL, SOUND_PAN_ATTACKER
	waitforvisualfinish
	createsprite gSlideMonToOffsetSpriteTemplate, ANIM_ATTACKER, 2, ANIM_ATTACKER, 24, 0, 0, 6
	delay 4
	createvisualtask AnimTask_ShakeMon, 2, ANIM_TARGET, 5, 0, 8, 1
	createvisualtask AnimTask_BlendMonInAndOut, 3, ANIM_TARGET, RGB_RED, 12, 1, 1
	playsewithpan SE_M_FLAME_WHEEL2, SOUND_PAN_TARGET
	createsprite gSlashSliceSpriteTemplate, ANIM_TARGET, 2, 1, -8, 0
	playsewithpan SE_M_RAZOR_WIND, SOUND_PAN_TARGET
	delay 4
	createsprite gSlashSliceSpriteTemplate, ANIM_TARGET, 2, 1, 8, 0
	createvisualtask AnimTask_ShakeMon2, 2, ANIM_TARGET, 4, 0, 18, 1
	playsewithpan SE_M_RAZOR_WIND, SOUND_PAN_TARGET
	delay 7
	createsprite gSlideMonToOriginalPosSpriteTemplate, ANIM_ATTACKER, 2, ANIM_ATTACKER, 0, 9
	waitforvisualfinish
	clearmonbg ANIM_DEF_PARTNER
	end

Move_POWER_TRIP:
	goto Move_CRUNCH

Move_BURN_UP:
	goto Move_FLAMETHROWER

Move_SPEED_SWAP:
	goto Move_CALM_MIND

Move_SMART_STRIKE:
	goto Move_IRON_TAIL

Move_PURIFY:
	goto Move_TOXIC

Move_REVELATION_DANCE:
	goto Move_TAKE_DOWN

Move_CORE_ENFORCER:
	goto Move_DRAGON_CLAW

Move_TROP_KICK:
	goto Move_RAZOR_LEAF

Move_INSTRUCT:
	loadspritegfx ANIM_TAG_FINGER
	loadspritegfx ANIM_TAG_SPOTLIGHT
	setalpha 12, 8
	createsprite gMetronomeFingerSpriteTemplate, ANIM_ATTACKER, 12, 0
	delay 24
	loopsewithpan SE_M_TAIL_WHIP, SOUND_PAN_ATTACKER, 22, 3
	waitforvisualfinish
	createvisualtask AnimTask_HardwarePaletteFade, 2, 248, 3, 0, 10, 0
	waitforvisualfinish
	createsprite gSpotlightSpriteTemplate, ANIM_TARGET, 2, 0, -8
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_TARGET, 2, 0, 8, RGB(25, 13, 31)
	delay 4
	monbg ANIM_TARGET
	createvisualtask AnimTask_BlendMonInAndOut, 5, ANIM_ATTACKER, RGB(27, 27, 0), 12, 1, 1
	createvisualtask AnimTask_ShakeMon, 2, ANIM_TARGET, 3, 0, 41, 1
	playsewithpan SE_M_BIND, SOUND_PAN_TARGET
	delay 26
	createvisualtask AnimTask_BlendMonInAndOut, 5, ANIM_ATTACKER, RGB(27, 27, 0), 12, 1, 1
	playsewithpan SE_M_BIND, SOUND_PAN_TARGET
	delay 26
	createvisualtask AnimTask_BlendMonInAndOut, 5, ANIM_ATTACKER, RGB(27, 27, 0), 12, 1, 1
	playsewithpan SE_M_BIND, SOUND_PAN_TARGET
	delay 29
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_TARGET, 2, 8, 0, RGB(25, 13, 31)
	waitforvisualfinish
	clearmonbg ANIM_TARGET
	blendoff
	end

Move_BEAK_BLAST:
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_ATTACKER, 2, 0, 9, RGB_RED
	waitforvisualfinish
	createsprite gBowMonSpriteTemplate, ANIM_ATTACKER, 2, 0
	playsewithpan SE_M_HEADBUTT, SOUND_PAN_ATTACKER
	waitforvisualfinish
	delay 2
	createsprite gBowMonSpriteTemplate, ANIM_ATTACKER, 2, 1
	delay 2
	loopsewithpan SE_M_HORN_ATTACK, SOUND_PAN_TARGET 4 8
	createvisualtask AnimTask_DrillPeckHitSplats, 5
	createvisualtask AnimTask_ShakeMon2, 2, ANIM_TARGET, 4, 0, 18, 1
	waitforvisualfinish
	createsprite gBowMonSpriteTemplate, ANIM_ATTACKER, 2, 2
	waitforvisualfinish
	createsprite gSlideMonToOriginalPosSpriteTemplate, ANIM_ATTACKER, 2, ANIM_ATTACKER, 0, 6
	waitforvisualfinish
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_ATTACKER, 2, 9, 0, RGB_RED
	waitforvisualfinish
	end

Move_CLANGING_SCALES:
	goto Move_DRAGON_CLAW

Move_DRAGON_HAMMER:
	goto Move_DRAGON_CLAW

Move_BRUTAL_SWING:
	goto Move_CRUNCH

Move_AURORA_VEIL:
	goto Move_HAIL

Move_SHELL_TRAP:
	goto Move_FLAMETHROWER

Move_FLEUR_CANNON:
	goto Move_SWEET_KISS

Move_PSYCHIC_FANGS:
	monbg ANIM_TARGET
	call G58_SetPsychicBackground
	setalpha 12, 8
	choosetwoturnanim G58_PsychicFangsNormal, G58_PsychicFangsShatteredWall

Move_STOMPING_TANTRUM:
	goto Move_EARTHQUAKE

Move_SHADOW_BONE:
	goto Move_SHADOW_BALL

Move_ACCELEROCK:
	loadspritegfx ANIM_TAG_IMPACT
	loadspritegfx ANIM_TAG_ROCKS
	monbg ANIM_ATK_PARTNER
	setalpha 12, 8
	createvisualtask AnimTask_TranslateMonEllipticalRespectSide, 2, ANIM_ATTACKER, 24, 6, 1, 5
	createvisualtask AnimTask_TraceMonBlended, 2, 0, 4, 7, 3
	playsewithpan SE_M_JUMP_KICK, SOUND_PAN_ATTACKER
	delay 4
	createvisualtask AnimTask_ShakeMon, 2, ANIM_TARGET, 5, 0, 6, 1
	create_basic_hitsplat_sprite ANIM_TARGET, 4, x=0, y=0, relative_to=ANIM_TARGET, animation=1
	playsewithpan SE_M_VITAL_THROW2, SOUND_PAN_TARGET
	createsprite gHorizontalLungeSpriteTemplate, ANIM_ATTACKER, 2, 1, 3, 0, 5, 1
	createsprite gRockFragmentSpriteTemplate, ANIM_TARGET, 2, 5, 0, -20, 24, 14, 1
	createsprite gRockFragmentSpriteTemplate, ANIM_TARGET, 2, 0, 5, 20, -24, 14, 2
	createsprite gRockFragmentSpriteTemplate, ANIM_TARGET, 2, -5, 0, -20, -24, 14, 2
	createsprite gHorizontalLungeSpriteTemplate, ANIM_ATTACKER, 2, 1, 3, 0, 5, 1
	waitforvisualfinish
	clearmonbg ANIM_ATK_PARTNER
	blendoff
	waitforvisualfinish
	end

Move_LIQUIDATION:
	loadspritegfx ANIM_TAG_IMPACT
	loadspritegfx ANIM_TAG_SMALL_BUBBLES
	loadspritegfx ANIM_TAG_WATER_IMPACT
	monbg ANIM_TARGET
	setalpha 12, 8
	createsprite gHorizontalLungeSpriteTemplate, ANIM_ATTACKER, 2, 4, 4
	delay 6
	create_basic_hitsplat_sprite ANIM_ATTACKER, 2, x=0, y=0, relative_to=ANIM_TARGET, animation=1
	createvisualtask AnimTask_ShakeMon, 2, ANIM_TARGET, 3, 0, 6, 1
	playsewithpan SE_M_DOUBLE_TEAM, SOUND_PAN_TARGET
	waitforvisualfinish
	call G58_RisingWaterHitEffect
	waitforvisualfinish
	clearmonbg ANIM_DEF_PARTNER
	blendoff
	end

Move_PRISMATIC_LASER:
	goto Move_PSYCHIC

Move_SPECTRAL_THIEF:
	goto Move_SHADOW_BALL

Move_SUNSTEEL_STRIKE:
	goto Move_IRON_TAIL

Move_MOONGEIST_BEAM:
	goto Move_SHADOW_BALL

Move_TEARFUL_LOOK:
	loadspritegfx ANIM_TAG_OPENING_EYE
	loadspritegfx ANIM_TAG_SMALL_BUBBLES
	monbg ANIM_DEF_PARTNER
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_BG, 1, 0, 5, RGB(0, 15, 31)
	waitforvisualfinish
	playsewithpan SE_M_CONFUSE_RAY, SOUND_PAN_TARGET
	createsprite gOpeningEyeSpriteTemplate, ANIM_ATTACKER, 5, 0, 0, 1, 0
	delay 53
	loopsewithpan SE_M_TAIL_WHIP, SOUND_PAN_ATTACKER, 12, 2
	delay 8
	create_tear_drop_sprite ANIM_ATTACKER, 2, relative_to=ANIM_TARGET, type=0
	create_tear_drop_sprite ANIM_ATTACKER, 2, relative_to=ANIM_TARGET, type=1
	create_tear_drop_sprite ANIM_ATTACKER, 2, relative_to=ANIM_TARGET, type=2
	create_tear_drop_sprite ANIM_ATTACKER, 2, relative_to=ANIM_TARGET, type=3
	delay 8
	create_tear_drop_sprite ANIM_ATTACKER, 2, relative_to=ANIM_TARGET, type=0
	create_tear_drop_sprite ANIM_ATTACKER, 2, relative_to=ANIM_TARGET, type=1
	create_tear_drop_sprite ANIM_ATTACKER, 2, relative_to=ANIM_TARGET, type=2
	create_tear_drop_sprite ANIM_ATTACKER, 2, relative_to=ANIM_TARGET, type=3
	waitforvisualfinish
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_BG, 1, 5, 0, RGB(0, 15, 31)
	waitforvisualfinish
	clearmonbg ANIM_DEF_PARTNER
	end

Move_ZING_ZAP:
	goto Move_THUNDERBOLT

Move_NATURES_MADNESS:
	goto Move_SWEET_KISS

Move_MULTI_ATTACK:
	loadspritegfx ANIM_TAG_CUT
	loadspritegfx ANIM_TAG_FOCUS_ENERGY
	monbg ANIM_TARGET
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_BG, 1, 0, 12, RGB_BLACK
	waitforvisualfinish
	playsewithpan SE_M_DRAGON_RAGE, SOUND_PAN_ATTACKER
	call G58_MultiAttackBuff
	delay 8
	blend_color_cycle selector=F_PAL_ATTACKER, delay=2, num_blends=2, initial_blend_y=0, target_blend_y=11, color=RGB_WHITE
	createvisualtask AnimTask_ShakeMon2, 2, ANIM_ATTACKER, 1, 0, 32, 1
	call G58_MultiAttackBuff
	delay 8
	call G58_MultiAttackBuff
	waitforvisualfinish
	delay 15
	playsewithpan SE_M_RAZOR_WIND, SOUND_PAN_TARGET
	createsprite gCuttingSliceSpriteTemplate, ANIM_ATTACKER, 2, 40, -32, 0
	createsprite gCuttingSliceSpriteTemplate, ANIM_ATTACKER, 2, 40, -48, 0
	createvisualtask AnimTask_ShakeMon, 2, ANIM_TARGET, 0, 3, 15, 1
	waitforvisualfinish
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_BG, 1, 12, 0, RGB_BLACK
	waitforvisualfinish
	clearmonbg ANIM_TARGET
	end

Move_MIND_BLOWN:
	goto Move_FLAMETHROWER

Move_PLASMA_FISTS:
	goto Move_THUNDERBOLT

Move_PHOTON_GEYSER:
	goto Move_PSYCHIC

Move_ZIPPY_ZAP:
	loadspritegfx ANIM_TAG_CIRCLE_OF_LIGHT
	loadspritegfx ANIM_TAG_IMPACT
	loadspritegfx ANIM_TAG_SPARK_2
	monbg ANIM_ATK_PARTNER
	setalpha 12, 8
	playsewithpan SE_M_CHARGE, SOUND_PAN_ATTACKER
	createsprite gGrowingShockWaveOrbSpriteTemplate, ANIM_ATTACKER, 2  @electric circle
	waitforvisualfinish
	createvisualtask AnimTask_TranslateMonEllipticalRespectSide, 2, ANIM_ATTACKER, 24, 6, 1, 5
	createvisualtask AnimTask_TraceMonBlended, 2, 0, 4, 7, 3
	playsewithpan SE_M_JUMP_KICK, SOUND_PAN_ATTACKER
	delay 4
	createvisualtask AnimTask_ShakeMon, 2, ANIM_TARGET, 5, 0, 6, 1
	create_basic_hitsplat_sprite ANIM_TARGET, 4, x=0, y=0, relative_to=ANIM_TARGET, animation=1
	call G58_ElectricityEffect
	playsewithpan SE_M_VITAL_THROW2, SOUND_PAN_TARGET
	waitforvisualfinish
	clearmonbg ANIM_ATK_PARTNER
	blendoff
	waitforvisualfinish
	end

Move_SPLISHY_SPLASH:
	loadspritegfx ANIM_TAG_SPARK_2
	create_surf_wave palette=ANIM_SURF_PAL_SURF
	delay 24
	panse SE_M_SURF, SOUND_PAN_ATTACKER, SOUND_PAN_TARGET, +2, 0
	waitforvisualfinish
	createvisualtask AnimTask_ShakeMon2, 2, ANIM_TARGET, 1, 0, 10, 1
	call G58_ElectricityEffect
	end

Move_FLOATY_FALL:
	loadspritegfx ANIM_TAG_IMPACT
	loadspritegfx ANIM_TAG_ROUND_SHADOW
	createvisualtask AnimTask_BlendParticle, 5, ANIM_TAG_ROUND_SHADOW, 0, 12, 12, RGB_YELLOW
	monbg ANIM_DEF_PARTNER
	setalpha 12, 8
	playsewithpan SE_M_FLY, SOUND_PAN_ATTACKER
	createsprite gFlyBallUpSpriteTemplate, ANIM_ATTACKER, 2, 0, 0, 13, 336
	waitforvisualfinish
	playsewithpan SE_M_DOUBLE_TEAM, SOUND_PAN_ATTACKER
	createsprite gFlyBallAttackSpriteTemplate, ANIM_ATTACKER, 2, 20, FALSE
	delay 20
	create_basic_hitsplat_sprite ANIM_ATTACKER, 2, x=0, y=0, relative_to=ANIM_TARGET, animation=0
	createvisualtask AnimTask_ShakeMon, 5, ANIM_TARGET, 6, 0, 8, 1
	playsewithpan SE_M_RAZOR_WIND, SOUND_PAN_TARGET
	waitforvisualfinish
	clearmonbg ANIM_DEF_PARTNER
	blendoff
	waitforvisualfinish
	end

Move_PIKA_PAPOW:
	goto Move_THUNDERBOLT

Move_BOUNCY_BUBBLE:
	loadspritegfx ANIM_TAG_BLUE_STAR
	loadspritegfx ANIM_TAG_IMPACT
	loadspritegfx ANIM_TAG_ORBS
	loadspritegfx ANIM_TAG_SMALL_BUBBLES
	monbg ANIM_TARGET
	splitbgprio ANIM_TARGET
	setalpha 12, 8
	delay 1
	simple_palette_blend selector=F_PAL_BG, delay=1, initial_blend_y=0, target_blend_y=12, color=RGB(13, 12, 31)
	waitforvisualfinish
	playsewithpan SE_M_ABSORB, SOUND_PAN_TARGET
	create_basic_hitsplat_sprite ANIM_ATTACKER, 2, x=0, y=0, relative_to=ANIM_TARGET, animation=0
	delay 2
	createvisualtask AnimTask_ShakeMon, 5, ANIM_TARGET, 0, 5, 5, 1
	waitforvisualfinish
	call G58_WaterBubblesEffectLong
	call G58_GigaDrainAbsorbEffect
	waitforvisualfinish
	delay 15
	call G58_HealingEffect
	waitforvisualfinish
	simple_palette_blend selector=F_PAL_BG, delay=1, initial_blend_y=12, target_blend_y=0, color=RGB(13, 12, 31)
	waitforvisualfinish
	clearmonbg ANIM_TARGET
	blendoff
	end

Move_BUZZY_BUZZ:
	goto Move_THUNDERBOLT

Move_SIZZLY_SLIDE:
	loadspritegfx ANIM_TAG_IMPACT
	loadspritegfx ANIM_TAG_SMALL_EMBER
	monbg ANIM_ATK_PARTNER
	setalpha 12, 8
	createsprite gFireSpiralOutwardSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, 56, 0
	playsewithpan SE_M_FLAME_WHEEL, SOUND_PAN_ATTACKER
	delay 2
	createsprite gFireSpiralOutwardSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, 56, 4
	playsewithpan SE_M_FLAME_WHEEL, SOUND_PAN_ATTACKER
	delay 2
	createsprite gFireSpiralOutwardSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, 56, 8
	playsewithpan SE_M_FLAME_WHEEL, SOUND_PAN_ATTACKER
	delay 2
	createsprite gFireSpiralOutwardSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, 56, 12
	playsewithpan SE_M_FLAME_WHEEL, SOUND_PAN_ATTACKER
	delay 2
	createsprite gFireSpiralOutwardSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, 56, 16
	playsewithpan SE_M_FLAME_WHEEL, SOUND_PAN_ATTACKER
	delay 2
	createsprite gFireSpiralOutwardSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, 56, 20
	playsewithpan SE_M_FLAME_WHEEL, SOUND_PAN_ATTACKER
	delay 2
	createsprite gFireSpiralOutwardSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, 56, 24
	playsewithpan SE_M_FLAME_WHEEL, SOUND_PAN_ATTACKER
	waitforvisualfinish
	createvisualtask AnimTask_TranslateMonEllipticalRespectSide, 2, ANIM_ATTACKER, 24, 6, 1, 5
	createvisualtask AnimTask_TraceMonBlended, 2, 0, 4, 7, 3
	playsewithpan SE_M_JUMP_KICK, SOUND_PAN_ATTACKER
	delay 4
	createvisualtask AnimTask_ShakeMon, 2, ANIM_TARGET, 5, 0, 6, 1
	create_basic_hitsplat_sprite ANIM_TARGET, 4, x=0, y=0, relative_to=ANIM_TARGET, animation=1
	createvisualtask AnimTask_BlendMonInAndOut, 3, ANIM_TARGET, RGB_RED, 12, 1, 1
	playsewithpan SE_M_FLAME_WHEEL2, SOUND_PAN_TARGET
	call G58_FireSpreadEffect
	delay 7
	createsprite gSlideMonToOriginalPosSpriteTemplate, ANIM_ATTACKER, 2, 0, 0, 9
	waitforvisualfinish
	clearmonbg ANIM_ATK_PARTNER
	end

Move_GLITZY_GLOW:
	goto Move_PSYCHIC

Move_BADDY_BAD:
	goto Move_CRUNCH

Move_SAPPY_SEED:
	goto Move_RAZOR_LEAF

Move_FREEZY_FROST:
	goto Move_ICE_BEAM

Move_SPARKLY_SWIRL:
	goto Move_SWEET_KISS

Move_VEEVEE_VOLLEY:
	goto Move_TAKE_DOWN

Move_DOUBLE_IRON_BASH:
	goto Move_IRON_TAIL

Move_DYNAMAX_CANNON:
	goto Move_DRAGON_CLAW

Move_SNIPE_SHOT:
	goto Move_SURF

Move_JAW_LOCK:
	goto Move_CRUNCH

Move_STUFF_CHEEKS:
	goto Move_GROWL

Move_NO_RETREAT:
	goto Move_BULK_UP

Move_TAR_SHOT:
	goto Move_ROCK_TOMB

Move_MAGIC_POWDER:
	goto Move_CALM_MIND

Move_DRAGON_DARTS:
	goto Move_DRAGON_CLAW

Move_TEATIME:
	goto Move_GROWL

Move_OCTOLOCK:
	loadspritegfx ANIM_TAG_TENDRILS
	createvisualtask AnimTask_BlendParticle, 5, ANIM_TAG_TENDRILS, 0, 12, 12, RGB(24, 12, 15)    @Pinkish Red
	call G58_CreateConstrictBinding
	blend_color_cycle selector=F_PAL_TARGET, delay=2, num_blends=2, initial_blend_y=0, target_blend_y=8, color=RGB_RED
	playsewithpan SE_M_BIND, SOUND_PAN_TARGET
	setarg 7, -1
	waitforvisualfinish
	end

Move_BOLT_BEAK:
	goto Move_THUNDERBOLT

Move_FISHIOUS_REND:
	goto Move_SURF

Move_COURT_CHANGE:
	goto Move_GROWL

Move_CLANGOROUS_SOUL:
	goto Move_DRAGON_DANCE

Move_BODY_PRESS:
	goto Move_KARATE_CHOP

Move_DECORATE:
	goto Move_MOONLIGHT

Move_DRUM_BEATING:
	loadspritegfx ANIM_TAG_IMPACT
	loadspritegfx ANIM_TAG_MUSIC_NOTES
	loadspritegfx ANIM_TAG_PURPLE_HAND_OUTLINE
	loadspritegfx ANIM_TAG_ROOTS
	createvisualtask AnimTask_BlendParticle, 5, ANIM_TAG_MUSIC_NOTES, 0, 13, 13, RGB(12, 29, 0)   @Green
	monbg ANIM_TARGET
	splitbgprio ANIM_TARGET
	setalpha 12, 8
	call G58_BellyDrumRight
	createsprite gSlowFlyingMusicNotesSpriteTemplate, ANIM_ATTACKER, 2, 0, 0, 0, 0
	playsewithpan SE_M_BELLY_DRUM, SOUND_PAN_ATTACKER
	delay 15
	call G58_BellyDrumLeft
	createsprite gSlowFlyingMusicNotesSpriteTemplate, ANIM_ATTACKER, 2, 1, 1, 1, 0
	playsewithpan SE_M_BELLY_DRUM, SOUND_PAN_ATTACKER
	delay 15
	call G58_BellyDrumRight
	createsprite gSlowFlyingMusicNotesSpriteTemplate, ANIM_ATTACKER, 2, 0, 3, 3, 128
	playsewithpan SE_M_BELLY_DRUM, SOUND_PAN_ATTACKER
	delay 7
	call G58_BellyDrumLeft
	createsprite gSlowFlyingMusicNotesSpriteTemplate, ANIM_ATTACKER, 2, 1, 2, 0, 128
	playsewithpan SE_M_BELLY_DRUM, SOUND_PAN_ATTACKER
	delay 7
	call G58_BellyDrumRight
	createsprite gSlowFlyingMusicNotesSpriteTemplate, ANIM_ATTACKER, 2, 0, 1, 1, 0
	playsewithpan SE_M_BELLY_DRUM, SOUND_PAN_ATTACKER
	delay 7
	call G58_BellyDrumLeft
	createsprite gSlowFlyingMusicNotesSpriteTemplate, ANIM_ATTACKER, 2, 1, 0, 3, 0
	playsewithpan SE_M_BELLY_DRUM, SOUND_PAN_ATTACKER
	create_frenzy_plant_root_sprite ANIM_ATTACKER, 2, interpolate_percent=85, offset_x=16, offset_y=6, subpriority=0, animation=3, duration=80
	playsewithpan SE_M_SCRATCH, SOUND_PAN_TARGET
	delay 5
	create_frenzy_plant_root_sprite ANIM_ATTACKER, 2, interpolate_percent=85, offset_x=-16, offset_y=-6, subpriority=0, animation=2, duration=75
	playsewithpan SE_M_SCRATCH, SOUND_PAN_TARGET
	delay 5
	create_basic_hitsplat_sprite ANIM_ATTACKER, 2, x=-10, y=-10, relative_to=ANIM_TARGET, animation=3
	playsewithpan SE_M_DOUBLE_SLAP, SOUND_PAN_TARGET
	createvisualtask AnimTask_ShakeMon, 3, ANIM_TARGET, 8, 0, 20, 1
	delay 3
	create_basic_hitsplat_sprite ANIM_ATTACKER, 2, x=10, y=8, relative_to=ANIM_TARGET, animation=3
	playsewithpan SE_M_DOUBLE_SLAP, SOUND_PAN_TARGET
	delay 3
	create_basic_hitsplat_sprite ANIM_ATTACKER, 2, x=10, y=-3, relative_to=ANIM_TARGET, animation=2
	playsewithpan SE_M_DOUBLE_SLAP, SOUND_PAN_TARGET
	delay 3
	create_basic_hitsplat_sprite ANIM_ATTACKER, 2, x=-3, y=1, relative_to=ANIM_TARGET, animation=2
	playsewithpan SE_M_DOUBLE_SLAP, SOUND_PAN_TARGET
	delay 2
	create_basic_hitsplat_sprite ANIM_ATTACKER, 2, x=-10, y=1, relative_to=ANIM_TARGET, animation=1
	playsewithpan SE_M_DOUBLE_SLAP, SOUND_PAN_TARGET
	delay 2
	create_basic_hitsplat_sprite ANIM_ATTACKER, 2, x=0, y=10, relative_to=ANIM_TARGET, animation=1
	playsewithpan SE_M_DOUBLE_SLAP, SOUND_PAN_TARGET
	waitforvisualfinish
	clearmonbg ANIM_TARGET
	blendoff
	end

Move_SNAP_TRAP:
	goto Move_RAZOR_LEAF

Move_PYRO_BALL:
	goto Move_FLAMETHROWER

Move_BEHEMOTH_BLADE:
	goto Move_IRON_TAIL

Move_BEHEMOTH_BASH:
	goto Move_IRON_TAIL

Move_AURA_WHEEL:
	goto Move_THUNDERBOLT

Move_BREAKING_SWIPE:
	goto Move_DRAGON_CLAW

Move_BRANCH_POKE:
	goto Move_RAZOR_LEAF

Move_OVERDRIVE:
	goto Move_THUNDERBOLT

Move_APPLE_ACID:
	goto Move_RAZOR_LEAF

Move_GRAV_APPLE:
	goto Move_RAZOR_LEAF

Move_SPIRIT_BREAK:
	goto Move_SWEET_KISS

Move_STRANGE_STEAM:
	goto Move_SWEET_KISS

Move_LIFE_DEW:
	goto Move_WATER_SPORT

Move_OBSTRUCT:
	goto Move_TORMENT

Move_FALSE_SURRENDER:
	goto Move_CRUNCH

Move_METEOR_ASSAULT:
	goto Move_KARATE_CHOP

Move_ETERNABEAM:
	goto Move_DRAGON_CLAW

Move_STEEL_BEAM:
	goto Move_IRON_TAIL

Move_EXPANDING_FORCE:
	goto Move_PSYCHIC

Move_STEEL_ROLLER:
	goto Move_IRON_TAIL

Move_SCALE_SHOT:
	goto Move_DRAGON_CLAW

Move_METEOR_BEAM:
	goto Move_ROCK_SLIDE

Move_SHELL_SIDE_ARM:
	goto Move_SLUDGE_BOMB

Move_MISTY_EXPLOSION:
	loadspritegfx ANIM_TAG_EXPLOSION
	complex_palette_blend selector=F_PAL_BG, delay=8, num_blends=9, color1=RGB(27, 18, 30), blend_y1=8, color2=RGB_BLACK, blend_y2=8
	createvisualtask AnimTask_ShakeMon2, 5, ANIM_PLAYER_LEFT, 8, 0, 40, 1
	createvisualtask AnimTask_ShakeMon2, 5, ANIM_PLAYER_RIGHT, 8, 0, 40, 1
	createvisualtask AnimTask_ShakeMon2, 5, ANIM_OPPONENT_LEFT, 8, 0, 40, 1
	createvisualtask AnimTask_ShakeMon2, 5, ANIM_OPPONENT_RIGHT, 8, 0, 40, 1
	createvisualtask AnimTask_ShakeMon2, 5, ANIM_ATTACKER_FORCE, 8, 0, 40, 1
	call G58_Explosion1
	call G58_Explosion1
	waitforvisualfinish
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_BG, 1, 16, 16, RGB(31, 27, 31) @;White Pink
	delay 50
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_BG, 3, 16, 0, RGB(31, 27, 31) @;White Pink
	end

Move_GRASSY_GLIDE:
	goto Move_RAZOR_LEAF

Move_RISING_VOLTAGE:
	goto Move_THUNDERBOLT

Move_TERRAIN_PULSE:
	goto Move_TAKE_DOWN

Move_SKITTER_SMACK:
	goto Move_SIGNAL_BEAM

Move_BURNING_JEALOUSY:
	goto Move_FLAMETHROWER

Move_LASH_OUT:
	goto Move_CRUNCH

Move_POLTERGEIST:
	goto Move_SHADOW_BALL

Move_CORROSIVE_GAS:
	loadspritegfx ANIM_TAG_PINK_CLOUD
	createvisualtask AnimTask_BlendParticle, 5, ANIM_TAG_PINK_CLOUD, 0, 14, 14, RGB(15, 15, 6) @;Garbage green
	monbg ANIM_ATTACKER
	loopsewithpan SE_M_FLAME_WHEEL2, SOUND_PAN_ATTACKER, 8, 3
	delay 0
	blend_color_cycle selector=F_PAL_ATTACKER, delay=1, num_blends=2, initial_blend_y=0, target_blend_y=15, color=RGB(15, 15, 6)@;Garbage green
	createsprite gSmokeBallEscapeCloudSpriteTemplate, ANIM_ATTACKER, 122, 3, -14, 18, 46
	delay 0
	createsprite gSmokeBallEscapeCloudSpriteTemplate, ANIM_ATTACKER, 121, 3, 14, -14, 46
	delay 0
	createsprite gSmokeBallEscapeCloudSpriteTemplate, ANIM_ATTACKER, 120, 3, -12, -10, 46
	delay 0
	createsprite gSmokeBallEscapeCloudSpriteTemplate, ANIM_ATTACKER, 119, 3, 14, 14, 46
	delay 0
	createsprite gSmokeBallEscapeCloudSpriteTemplate, ANIM_ATTACKER, 118, 3, 0, 0, 46
	delay 0
	createsprite gSmokeBallEscapeCloudSpriteTemplate, ANIM_ATTACKER, 123, 3, 4, 4, 30
	waitforvisualfinish
	playsewithpan SE_M_TOXIC, SOUND_PAN_TARGET
	blend_color_cycle selector=(F_PAL_BG | F_PAL_TARGET | F_PAL_ATK_PARTNER | F_PAL_DEF_PARTNER), delay=1, num_blends=2, initial_blend_y=0, target_blend_y=14, color=RGB(15, 15, 6)@;Garbage green
	waitforvisualfinish
	clearmonbg ANIM_ATTACKER
	end

Move_COACHING:
	playsewithpan SE_M_TAIL_WHIP, SOUND_PAN_ATTACKER
	createvisualtask AnimTask_Splash, 2, ANIM_ATTACKER, 1
	waitforvisualfinish
	playsewithpan SE_M_TAIL_WHIP, SOUND_PAN_TARGET
	blend_color_cycle selector=F_PAL_TARGET, delay=1, num_blends=2, initial_blend_y=0, target_blend_y=10, color=RGB(29, 18, 7)
	createvisualtask AnimTask_Splash, 2, ANIM_TARGET, 1
	waitforvisualfinish
	end

Move_FLIP_TURN:
	goto Move_SURF

Move_TRIPLE_AXEL:
	goto Move_ICE_BEAM

Move_DUAL_WINGBEAT:
	goto Move_WING_ATTACK

Move_SCORCHING_SANDS:
	loadspritegfx ANIM_TAG_MUD_SAND
	loadspritegfx ANIM_TAG_SMALL_EMBER
	monbg ANIM_ATK_PARTNER
	splitbgprio ANIM_ATTACKER
	setalpha 12, 8
	playsewithpan SE_M_SAND_ATTACK, SOUND_PAN_ATTACKER
	createsprite gSlideMonToOffsetSpriteTemplate, ANIM_ATTACKER, 2, ANIM_ATTACKER, -10, 0, 0, 3
	waitforvisualfinish
	createsprite gSlideMonToOriginalPosSpriteTemplate, ANIM_ATTACKER, 2, 0, 0, 2
	call G58_SandAttackDirt
	call G58_SandAttackDirt
	call G58_SandAttackDirt
	call G58_SandAttackDirt
	createvisualtask AnimTask_ShakeMon, 2, ANIM_TARGET, 2, 0, 25, 1
	call G58_SandAttackDirt
	call G58_SandAttackDirt
	playsewithpan SE_M_FIRE_PUNCH, SOUND_PAN_TARGET
	call G58_FireSpreadEffect
	waitforvisualfinish
	clearmonbg ANIM_ATK_PARTNER
	blendoff
	end

Move_JUNGLE_HEALING:
	goto Move_INGRAIN

Move_WICKED_BLOW:
	goto Move_CRUNCH

Move_SURGING_STRIKES:
	goto Move_SURF

Move_THUNDER_CAGE:
	loadspritegfx ANIM_TAG_SHOCK_3
	loadspritegfx ANIM_TAG_SPARK_H
	monbg ANIM_TARGET
	splitbgprio ANIM_TARGET
	loopsewithpan SE_M_THUNDERBOLT2, SOUND_PAN_TARGET, 10, 9
	createsprite gThunderboltOrbSpriteTemplate, ANIM_TARGET, 3, 100, -25, -30, 0
	createsprite gThunderboltOrbSpriteTemplate, ANIM_TARGET, 3, 100, 0, -30, 0
	createsprite gThunderboltOrbSpriteTemplate, ANIM_TARGET, 3, 100, 25, -30, 0
	call G58_CreateThunderWaves
	createvisualtask AnimTask_ShakeMon, 5, ANIM_TARGET, 3, 0, 50, 1
	call G58_ThunderCageBolts
	delay 4
	call G58_CreateThunderWaves
	call G58_ThunderCageBolts
	waitforvisualfinish
	clearmonbg ANIM_TARGET
	end

Move_DRAGON_ENERGY:
	goto Move_DRAGON_CLAW

Move_FREEZING_GLARE:
	loadspritegfx ANIM_TAG_EYE_SPARKLE
	loadspritegfx ANIM_TAG_ICE_CRYSTALS
	call G58_SetPsychicBackground
	createsprite gEyeSparkleSpriteTemplate, ANIM_ATTACKER, 0, -16, -8
	createsprite gEyeSparkleSpriteTemplate, ANIM_ATTACKER, 0, 16, -8
	createvisualtask AnimTask_GlareEyeDots, 5, 0
	playsewithpan SE_M_PSYBEAM2, SOUND_PAN_ATTACKER
	waitforvisualfinish
	blend_color_cycle selector=F_PAL_TARGET, delay=2, num_blends=2, initial_blend_y=0, target_blend_y=12, color=RGB(12, 26, 31)@;Ice blue
	createvisualtask AnimTask_ShakeMon, 5, ANIM_TARGET, 0, 4, 28, 1
	call G58_IceCrystalEffectShort
	waitforvisualfinish
	call G58_UnsetBackground
	end

Move_FIERY_WRATH:
	goto Move_CRUNCH

Move_THUNDEROUS_KICK:
	goto Move_KARATE_CHOP

Move_GLACIAL_LANCE:
	goto Move_ICE_BEAM

Move_ASTRAL_BARRAGE:
	goto Move_SHADOW_BALL

Move_EERIE_SPELL:
	goto Move_PSYCHIC

@ ---- routines shared by the animations above (from pokeemerald-expansion, prefixed)
G58_BellyDrumLeft:
	createsprite gBellyDrumHandSpriteTemplate, ANIM_ATTACKER, 3, 0
	createvisualtask AnimTask_ShakeMon, 2, ANIM_ATTACKER, 0, 8, 2, 1
	return
G58_BellyDrumRight:
	createsprite gBellyDrumHandSpriteTemplate, ANIM_ATTACKER, 3, 1
	createvisualtask AnimTask_ShakeMon, 2, ANIM_ATTACKER, 0, 8, 2, 1
	return
G58_BreakScreens:
	createsprite gBrickBreakWallShardSpriteTemplate, ANIM_ATTACKER, 2, ANIM_TARGET, 0, -8, -12
	createsprite gBrickBreakWallShardSpriteTemplate, ANIM_ATTACKER, 2, ANIM_TARGET, 1, 8, -12
	createsprite gBrickBreakWallShardSpriteTemplate, ANIM_ATTACKER, 2, ANIM_TARGET, 2, -8, 12
	createsprite gBrickBreakWallShardSpriteTemplate, ANIM_ATTACKER, 2, ANIM_TARGET, 3, 8, 12
	playsewithpan SE_M_BRICK_BREAK, SOUND_PAN_TARGET
	return
G58_ClearSmogCloud:
	createsprite gMistCloudSpriteTemplate, ANIM_ATTACKER, 2, 0, -24, 48, 240, 1, 0
	delay 7
	return
G58_CreateConstrictBinding:
	loopsewithpan SE_M_SCRATCH, SOUND_PAN_TARGET, 6, 4
	create_constrict_binding_sprite ANIM_TARGET, 4, initial_x=0, initial_y=16, affine_animation=0, squeezes=2
	delay 7
	create_constrict_binding_sprite ANIM_TARGET, 3, initial_x=0, initial_y=0, affine_animation=0, squeezes=2
	create_constrict_binding_sprite ANIM_TARGET, 2, initial_x=0, initial_y=8, affine_animation=1, squeezes=2
	delay 7
	create_constrict_binding_sprite ANIM_TARGET, 3, initial_x=0, initial_y=-8, affine_animation=1, squeezes=2
	delay 8
	createvisualtask AnimTask_ShakeMon2, 2, ANIM_TARGET, 3, 0, 6, 1
	delay 20
	return
G58_CreateCrunch1:
	playsewithpan SE_M_BITE, SOUND_PAN_TARGET
	create_sharp_teeth_sprite ANIM_ATTACKER, 2, x=-32, y=-32, animation=1, x_velocity=819/256, y_velocity=819/256, half_duration=10
	create_sharp_teeth_sprite ANIM_ATTACKER, 2, x=32, y=32, animation=5, x_velocity=-819/256, y_velocity=-819/256, half_duration=10
	delay 10
	return
G58_CreateCrunch2:
	playsewithpan SE_M_BITE, SOUND_PAN_TARGET
	create_sharp_teeth_sprite ANIM_ATTACKER, 2, x=32, y=-32, animation=7, x_velocity=-819/256, y_velocity=819/256, half_duration=10
	create_sharp_teeth_sprite ANIM_ATTACKER, 2, x=-32, y=32, animation=3, x_velocity=819/256, y_velocity=-819/256, half_duration=10
	delay 10
	return
G58_CreateDragonDanceOrbs:
	create_dragon_dance_orb_sprite ANIM_ATTACKER, 2, angle=0/256
	create_dragon_dance_orb_sprite ANIM_ATTACKER, 2, angle=43/256
	create_dragon_dance_orb_sprite ANIM_ATTACKER, 2, angle=85/256
	create_dragon_dance_orb_sprite ANIM_ATTACKER, 2, angle=128/256
	create_dragon_dance_orb_sprite ANIM_ATTACKER, 2, angle=170/256
	create_dragon_dance_orb_sprite ANIM_ATTACKER, 2, angle=213/256
	return
G58_CreateIceSpiral:
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
G58_CreateMagentaHearts:
	createvisualtask AnimTask_RockMonBackAndForth, 5, ANIM_ATTACKER, 2, 0
	createsprite gMagentaHeartSpriteTemplate, ANIM_ATTACKER, 3, 0, 20
	playsewithpan SE_M_CHARM, SOUND_PAN_ATTACKER
	delay 15
	createsprite gMagentaHeartSpriteTemplate, ANIM_ATTACKER, 3, -20, 20
	playsewithpan SE_M_CHARM, SOUND_PAN_ATTACKER
	delay 15
	createsprite gMagentaHeartSpriteTemplate, ANIM_ATTACKER, 3, 20, 20
	return
G58_CreatePetals:
	create_petal_dance_big_flower_sprite ANIM_ATTACKER, 2, initial_x=0, initial_y=-24, target_y=8, duration=140
	create_petal_dance_small_flower_sprite ANIM_ATTACKER, 2, initial_x=16, initial_y=-24, target_y=8, duration=100
	create_petal_dance_small_flower_sprite ANIM_ATTACKER, 2, initial_x=-16, initial_y=-24, target_y=8, duration=100
	delay 15
	create_petal_dance_big_flower_sprite ANIM_ATTACKER, 2, initial_x=0, initial_y=-24, target_y=8, duration=140
	create_petal_dance_small_flower_sprite ANIM_ATTACKER, 2, initial_x=32, initial_y=-24, target_y=8, duration=100
	create_petal_dance_small_flower_sprite ANIM_ATTACKER, 2, initial_x=-32, initial_y=-24, target_y=8, duration=100
	delay 15
	create_petal_dance_big_flower_sprite ANIM_ATTACKER, 2, initial_x=0, initial_y=-24, target_y=8, duration=140
	create_petal_dance_small_flower_sprite ANIM_ATTACKER, 2, initial_x=24, initial_y=-24, target_y=8, duration=100
	create_petal_dance_small_flower_sprite ANIM_ATTACKER, 2, initial_x=-24, initial_y=-24, target_y=8, duration=100
	delay 30
	create_petal_dance_small_flower_sprite ANIM_ATTACKER, 2, initial_x=16, initial_y=-24, target_y=0, duration=100
	create_petal_dance_small_flower_sprite ANIM_ATTACKER, 2, initial_x=-16, initial_y=-24, target_y=0, duration=100
	delay 30
	create_petal_dance_small_flower_sprite ANIM_ATTACKER, 2, initial_x=20, initial_y=-16, target_y=14, duration=80
	create_petal_dance_small_flower_sprite ANIM_ATTACKER, 2, initial_x=-20, initial_y=-14, target_y=16, duration=80
	return
G58_CreateRedHeart:
	createvisualtask AnimTask_SwayMon, 5, 0, 12, 4096, 4, ANIM_ATTACKER
	delay 15
	createsprite gRedHeartProjectileSpriteTemplate, ANIM_TARGET, 3, 20, -8
	return
G58_CreateRedHeartBurst:
	createsprite gRedHeartBurstSpriteTemplate, ANIM_TARGET, 3, 160, -32
	createsprite gRedHeartBurstSpriteTemplate, ANIM_TARGET, 3, -256, -40
	createsprite gRedHeartBurstSpriteTemplate, ANIM_TARGET, 3, 128, -16
	createsprite gRedHeartBurstSpriteTemplate, ANIM_TARGET, 3, 416, -38
	createsprite gRedHeartBurstSpriteTemplate, ANIM_TARGET, 3, -128, -22
	createsprite gRedHeartBurstSpriteTemplate, ANIM_TARGET, 3, -384, -31
	return
G58_CreateSharpDrill:
	createsprite gBowMonSpriteTemplate, ANIM_ATTACKER, 2, 0
	playsewithpan SE_M_HEADBUTT, SOUND_PAN_ATTACKER
	waitforvisualfinish
	delay 2
	createsprite gBowMonSpriteTemplate, ANIM_ATTACKER, 2, 1
	createsprite gHornHitSpriteTemplate, ANIM_TARGET, 4, 0, 0, 12
	waitforvisualfinish
	playse SE_BANG
	createvisualtask AnimTask_ShakeMonInPlace, 2, ANIM_ATTACKER, 2, 0, 40, 1
	createvisualtask AnimTask_ShakeMonInPlace, 2, ANIM_TARGET, 10, 0, 40, 1
	create_flashing_hitsplat_sprite ANIM_TARGET, 3, x=0, y=0, relative_to=ANIM_TARGET, animation=3
	playsewithpan SE_M_HORN_ATTACK, SOUND_PAN_TARGET
	delay 4
	create_flashing_hitsplat_sprite ANIM_TARGET, 3, x=0, y=2, relative_to=ANIM_TARGET, animation=3
	playsewithpan SE_M_HORN_ATTACK, SOUND_PAN_TARGET
	delay 4
	create_flashing_hitsplat_sprite ANIM_TARGET, 3, x=-4, y=3, relative_to=ANIM_TARGET, animation=3
	playsewithpan SE_M_HORN_ATTACK, SOUND_PAN_TARGET
	delay 4
	create_flashing_hitsplat_sprite ANIM_TARGET, 3, x=-8, y=-5, relative_to=ANIM_TARGET, animation=3
	playsewithpan SE_M_HORN_ATTACK, SOUND_PAN_TARGET
	delay 4
	create_flashing_hitsplat_sprite ANIM_TARGET, 3, x=4, y=-12, relative_to=ANIM_TARGET, animation=3
	playsewithpan SE_M_HORN_ATTACK, SOUND_PAN_TARGET
	delay 4
	create_flashing_hitsplat_sprite ANIM_TARGET, 3, x=16, y=0, relative_to=ANIM_TARGET, animation=3
	playsewithpan SE_M_HORN_ATTACK, SOUND_PAN_TARGET
	delay 4
	create_flashing_hitsplat_sprite ANIM_TARGET, 3, x=5, y=18, relative_to=ANIM_TARGET, animation=3
	playsewithpan SE_M_HORN_ATTACK, SOUND_PAN_TARGET
	delay 4
	create_flashing_hitsplat_sprite ANIM_TARGET, 3, x=-17, y=12, relative_to=ANIM_TARGET, animation=2
	playsewithpan SE_M_HORN_ATTACK, SOUND_PAN_TARGET
	delay 4
	create_flashing_hitsplat_sprite ANIM_TARGET, 3, x=-21, y=-15, relative_to=ANIM_TARGET, animation=2
	playsewithpan SE_M_HORN_ATTACK, SOUND_PAN_TARGET
	delay 4
	create_flashing_hitsplat_sprite ANIM_TARGET, 3, x=8, y=-27, relative_to=ANIM_TARGET, animation=2
	playsewithpan SE_M_HORN_ATTACK, SOUND_PAN_TARGET
	delay 4
	create_flashing_hitsplat_sprite ANIM_TARGET, 3, x=32, y=0, relative_to=ANIM_TARGET, animation=2
	playsewithpan SE_M_HORN_ATTACK, SOUND_PAN_TARGET
	delay 4
	createsprite gBowMonSpriteTemplate, ANIM_ATTACKER, 2, 2
	return
G58_CreateThunderWaves:
	createsprite gThunderWaveSpriteTemplate, ANIM_TARGET, 2, -16, -16
	delay 4
	createsprite gThunderWaveSpriteTemplate, ANIM_TARGET, 2, -16, 0
	delay 4
	createsprite gThunderWaveSpriteTemplate, ANIM_TARGET, 2, -16, 16
	return
G58_DigThrowDirt:
	createsprite gDirtPlumeSpriteTemplate, ANIM_ATTACKER, 2, ANIM_ATTACKER, 0, 12, 4, -16, 18
	createsprite gDirtPlumeSpriteTemplate, ANIM_ATTACKER, 2, ANIM_ATTACKER, 0, 16, 4, -10, 18
	createsprite gDirtPlumeSpriteTemplate, ANIM_ATTACKER, 2, ANIM_ATTACKER, 1, 14, 4, -18, 18
	createsprite gDirtPlumeSpriteTemplate, ANIM_ATTACKER, 2, ANIM_ATTACKER, 1, 12, 4, -16, 18
	playsewithpan SE_M_DIG, SOUND_PAN_ATTACKER
	delay 32
	return
G58_ElectricityEffect:
	playsewithpan SE_M_THUNDERBOLT2, SOUND_PAN_TARGET
	goto G58_ElectricityEffectNoSound
G58_ElectricityEffectNoSound:
	createsprite gElectricitySpriteTemplate, ANIM_TARGET, 2, 5, 0, 5, 0, ANIM_TARGET
	delay 2
	createsprite gElectricitySpriteTemplate, ANIM_TARGET, 2, -5, 10, 5, 1, ANIM_TARGET
	delay 2
	createsprite gElectricitySpriteTemplate, ANIM_TARGET, 2, 15, 20, 5, 2, ANIM_TARGET
	delay 2
	createsprite gElectricitySpriteTemplate, ANIM_TARGET, 2, -15, -10, 5, 0, ANIM_TARGET
	delay 2
	createsprite gElectricitySpriteTemplate, ANIM_TARGET, 2, 25, 0, 5, 1, ANIM_TARGET
	delay 2
	createsprite gElectricitySpriteTemplate, ANIM_TARGET, 2, -8, 8, 5, 2, ANIM_TARGET
	delay 2
	createsprite gElectricitySpriteTemplate, ANIM_TARGET, 2, 2, -8, 5, 0, ANIM_TARGET
	delay 2
	createsprite gElectricitySpriteTemplate, ANIM_TARGET, 2, -20, 15, 5, 1, ANIM_TARGET
	return
G58_Explosion1:
	playsewithpan SE_M_EXPLOSION, SOUND_PAN_ATTACKER
	createsprite gExplosionSpriteTemplate, ANIM_ATTACKER, 3, 0, 0, 0, 1
	delay 6
	playsewithpan SE_M_EXPLOSION, SOUND_PAN_ATTACKER
	createsprite gExplosionSpriteTemplate, ANIM_ATTACKER, 3, 24, -24, 0, 1
	delay 6
	playsewithpan SE_M_EXPLOSION, SOUND_PAN_ATTACKER
	createsprite gExplosionSpriteTemplate, ANIM_ATTACKER, 3, -16, 16, 0, 1
	delay 6
	playsewithpan SE_M_EXPLOSION, SOUND_PAN_ATTACKER
	createsprite gExplosionSpriteTemplate, ANIM_ATTACKER, 3, -24, -12, 0, 1
	delay 6
	playsewithpan SE_M_EXPLOSION, SOUND_PAN_ATTACKER
	createsprite gExplosionSpriteTemplate, ANIM_ATTACKER, 3, 16, 16, 0, 1
	delay 6
	return
G58_FireSpinEffect:
	createsprite gFireSpinSpriteTemplate, ANIM_TARGET, 2, 0, 28, 528, 30, 13, 50, ANIM_TARGET
	delay 2
	createsprite gFireSpinSpriteTemplate, ANIM_TARGET, 2, 0, 32, 480, 20, 16, -46, ANIM_TARGET
	delay 2
	createsprite gFireSpinSpriteTemplate, ANIM_TARGET, 2, 0, 33, 576, 20, 8, 42, ANIM_TARGET
	delay 2
	createsprite gFireSpinSpriteTemplate, ANIM_TARGET, 2, 0, 31, 400, 25, 11, -42, ANIM_TARGET
	delay 2
	createsprite gFireSpinSpriteTemplate, ANIM_TARGET, 2, 0, 28, 512, 25, 16, 46, ANIM_TARGET
	delay 2
	createsprite gFireSpinSpriteTemplate, ANIM_TARGET, 2, 0, 33, 464, 30, 15, -50, ANIM_TARGET
	delay 2
	return
G58_FireSpreadEffect:
	createsprite gFireSpreadSpriteTemplate, ANIM_TARGET, 1, 0, 10, 192, 176, 40
	createsprite gFireSpreadSpriteTemplate, ANIM_TARGET, 1, 0, 10, -192, 240, 40
	createsprite gFireSpreadSpriteTemplate, ANIM_TARGET, 1, 0, 10, 192, -160, 40
	createsprite gFireSpreadSpriteTemplate, ANIM_TARGET, 1, 0, 10, -192, -112, 40
	createsprite gFireSpreadSpriteTemplate, ANIM_TARGET, 1, 0, 10, 160, 48, 40
	createsprite gFireSpreadSpriteTemplate, ANIM_TARGET, 1, 0, 10, -224, -32, 40
	createsprite gFireSpreadSpriteTemplate, ANIM_TARGET, 1, 0, 10, 112, -128, 40
	return
G58_FlameBurstSpread:
	createsprite gFireSpreadSpriteTemplate, ANIM_TARGET, 1, 0, 0, 192, 76, 70
	createsprite gFireSpreadSpriteTemplate, ANIM_TARGET, 1, 0, 0, -192, 76, 70
	createsprite gFireSpreadSpriteTemplate, ANIM_TARGET, 1, 0, 0, 20, 140, 70
	createsprite gFireSpreadSpriteTemplate, ANIM_TARGET, 1, 0, 0, -20, 140, 70
	createsprite gFireSpreadSpriteTemplate, ANIM_TARGET, 1, 0, 0, 192, -160, 70
	createsprite gFireSpreadSpriteTemplate, ANIM_TARGET, 1, 0, 0, -192, -160, 70
	createsprite gFireSpreadSpriteTemplate, ANIM_TARGET, 1, 0, 0, 82, -112, 70
	createsprite gFireSpreadSpriteTemplate, ANIM_TARGET, 1, 0, 0, -82, -112, 70
	createsprite gFireSpreadSpriteTemplate, ANIM_TARGET, 1, 0, 0, 160, -52, 70
	createsprite gFireSpreadSpriteTemplate, ANIM_TARGET, 1, 0, 0, -160, -52, 70
	createsprite gFireSpreadSpriteTemplate, ANIM_TARGET, 1, 0, 0, 224, -32, 70
	createsprite gFireSpreadSpriteTemplate, ANIM_TARGET, 1, 0, 0, -224, -32, 70
	createsprite gFireSpreadSpriteTemplate, ANIM_TARGET, 1, 0, 0, 112, -128, 70
	createsprite gFireSpreadSpriteTemplate, ANIM_TARGET, 1, 0, 0, -112, -128, 70
	return
G58_GigaDrainAbsorbEffect:
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
G58_HealingEffect:
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
G58_HealingEffect2:
	playsewithpan SE_M_ABSORB_2, SOUND_PAN_TARGET
	createsprite gHealingBlueStarSpriteTemplate, ANIM_TARGET, 2, 0, -5, 1, 0
	delay 7
	createsprite gHealingBlueStarSpriteTemplate, ANIM_TARGET, 2, -15, 10, 1, 0
	delay 7
	createsprite gHealingBlueStarSpriteTemplate, ANIM_TARGET, 2, -15, -15, 1, 0
	delay 7
	createsprite gHealingBlueStarSpriteTemplate, ANIM_TARGET, 2, 10, -5, 1, 0
	delay 7
	return
G58_HoneClawsAnim:
	createsprite gHorizontalLungeSpriteTemplate, ANIM_ATTACKER, 2, 2, 2
	delay 4
	playsewithpan SE_M_SCRATCH, SOUND_PAN_TARGET
	createsprite gFurySwipesSpriteTemplate, ANIM_TARGET, 2, 16, 0, 1
	delay 10
	createsprite gHorizontalLungeSpriteTemplate, ANIM_TARGET, 2, 2, 2
	delay 4
	playsewithpan SE_M_SCRATCH, SOUND_PAN_TARGET
	createsprite gFurySwipesSpriteTemplate, ANIM_TARGET, 2, -16, 0, 0
	delay 10
	return
G58_IceCrystalEffectShort:
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
G58_InfernoAnim:
	createsprite gFireSpinSpriteTemplate, ANIM_TARGET, 2, 0, 28, 528, 30, 13, 50, 1
	delay 1
	createsprite gFireSpinSpriteTemplate, ANIM_TARGET, 2, 0, 33, 464, 30, 15, -25, 1
	delay 1
	createsprite gFireSpinSpriteTemplate, ANIM_TARGET, 2, 0, 32, 480, 20, 16, -46, 1
	delay 1
	createsprite gFireSpinSpriteTemplate, ANIM_TARGET, 2, 0, 28, 512, 25, 16, 23, 1
	delay 1
	createsprite gFireSpinSpriteTemplate, ANIM_TARGET, 2, 0, 33, 576, 20, 8, 42, 1
	delay 1
	createsprite gFireSpinSpriteTemplate, ANIM_TARGET, 2, 0, 31, 400, 25, 11, -21, 1
	delay 1
	createsprite gFireSpinSpriteTemplate, ANIM_TARGET, 2, 0, 31, 400, 25, 11, -42, 1
	delay 1
	createsprite gFireSpinSpriteTemplate, ANIM_TARGET, 2, 0, 33, 576, 20, 8, 21, 1
	delay 1
	createsprite gFireSpinSpriteTemplate, ANIM_TARGET, 2, 0, 28, 512, 25, 16, 46, 1
	delay 1
	createsprite gFireSpinSpriteTemplate, ANIM_TARGET, 2, 0, 32, 480, 20, 16, -23, 1
	delay 1
	createsprite gFireSpinSpriteTemplate, ANIM_TARGET, 2, 0, 33, 464, 30, 15, -50, 1
	delay 1
	createsprite gFireSpinSpriteTemplate, ANIM_TARGET, 2, 0, 28, 528, 30, 13, 25, 1
	delay 1
	return
G58_InitRoomAnimation:
	setalpha 8, 8
	createvisualtask AnimTask_ScaleMonAndRestore, 5, -6, -6, 15, ANIM_ATTACKER, 1
	return
G58_MegaDrainAbsorbEffect:
	playsewithpan SE_M_BUBBLE3, SOUND_PAN_TARGET
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=0, y=5, wave_amplitude=8, wave_period=26
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=5, y=-18, wave_amplitude=-20, wave_period=35
	delay 4
	playsewithpan SE_M_BUBBLE3, SOUND_PAN_TARGET
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=10, y=-5, wave_amplitude=-8, wave_period=26
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=-10, y=20, wave_amplitude=20, wave_period=39
	delay 4
	playsewithpan SE_M_BUBBLE3, SOUND_PAN_TARGET
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=-5, y=15, wave_amplitude=16, wave_period=33
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=10, y=-5, wave_amplitude=-8, wave_period=26
	delay 4
	playsewithpan SE_M_BUBBLE3, SOUND_PAN_TARGET
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=0, y=-15, wave_amplitude=-16, wave_period=36
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=0, y=5, wave_amplitude=8, wave_period=26
	delay 4
	playsewithpan SE_M_BUBBLE3, SOUND_PAN_TARGET
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=0, y=5, wave_amplitude=8, wave_period=26
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=0, y=-15, wave_amplitude=-16, wave_period=36
	delay 4
	playsewithpan SE_M_BUBBLE3, SOUND_PAN_TARGET
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=10, y=-5, wave_amplitude=-8, wave_period=26
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=-5, y=15, wave_amplitude=16, wave_period=33
	delay 4
	playsewithpan SE_M_BUBBLE3, SOUND_PAN_TARGET
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=-10, y=20, wave_amplitude=20, wave_period=39
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=10, y=-5, wave_amplitude=-8, wave_period=26
	delay 4
	playsewithpan SE_M_BUBBLE3, SOUND_PAN_TARGET
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=0, y=5, wave_amplitude=8, wave_period=26
	create_absorption_orb_sprite ANIM_ATTACKER, 3, x=5, y=-18, wave_amplitude=-20, wave_period=35
	delay 4
	return
G58_MetalSoundPlayNote:
	panse SE_M_UPROAR, SOUND_PAN_ATTACKER, SOUND_PAN_TARGET, 2, 0
	createsprite gMetalSoundSpriteTemplate, ANIM_TARGET, 2, 16, 0, 0, 0, 30, 0
	delay 2
	return
G58_MultiAttackBuff:
	createsprite gEndureEnergySpriteTemplate, ANIM_ATTACKER, 2, 0, -24, 26, 2
	delay 4
	createsprite gEndureEnergySpriteTemplate, ANIM_ATTACKER, 2, 0, 14, 28, 1
	delay 4
	createsprite gEndureEnergySpriteTemplate, ANIM_ATTACKER, 2, 0, -5, 10, 2
	delay 4
	createsprite gEndureEnergySpriteTemplate, ANIM_ATTACKER, 2, 0, 28, 26, 3
	delay 4
	createsprite gEndureEnergySpriteTemplate, ANIM_ATTACKER, 2, 0, -12, 0, 1
	return
G58_PsychicFangsCommon:
	call G58_CreateCrunch1
	create_basic_hitsplat_sprite ANIM_ATTACKER, 2, x=-8, y=0, relative_to=ANIM_TARGET, animation=1
	createvisualtask AnimTask_ShakeMon, 5, ANIM_TARGET, 0, 7, 5, 2
	delay 16
	call G58_CreateCrunch2
	create_basic_hitsplat_sprite ANIM_ATTACKER, 2, x=8, y=0, relative_to=ANIM_TARGET, animation=1
	return
G58_PsychicFangsEnd:
	playsewithpan SE_M_SUPERSONIC, SOUND_PAN_TARGET
	waitforvisualfinish
	clearmonbg ANIM_TARGET
	blendoff
	call G58_UnsetBackground
	end
G58_PsychicFangsNormal:
	call G58_PsychicFangsCommon
	createvisualtask AnimTask_ShakeMon, 5, ANIM_TARGET, 0, 8, 4, 2
	goto G58_PsychicFangsEnd
G58_PsychicFangsShatteredWall:
	createsprite gBrickBreakWallSpriteTemplate, ANIM_ATTACKER, 3, ANIM_TARGET, 0, 0, 33, 10
	call G58_PsychicFangsCommon
	call G58_BreakScreens
	createvisualtask AnimTask_ShakeMon, 5, ANIM_TARGET, 0, 8, 4, 2
	delay 16
	goto G58_PsychicFangsEnd
G58_RisingWaterHitEffect:
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
G58_RoarEffect:
	createsprite gRoarNoiseLineSpriteTemplate, ANIM_ATTACKER, 2, 24, -8, 0
	createsprite gRoarNoiseLineSpriteTemplate, ANIM_ATTACKER, 2, 24, 0, 2
	createsprite gRoarNoiseLineSpriteTemplate, ANIM_ATTACKER, 2, 24, 8, 1
	delay 15
	createsprite gRoarNoiseLineSpriteTemplate, ANIM_ATTACKER, 2, 24, -8, 0
	createsprite gRoarNoiseLineSpriteTemplate, ANIM_ATTACKER, 2, 24, 0, 2
	createsprite gRoarNoiseLineSpriteTemplate, ANIM_ATTACKER, 2, 24, 8, 1
	return
G58_SandAttackDirt:
	createsprite gSandAttackDirtSpriteTemplate, ANIM_TARGET, 2, 15, 15, 20, 0, 0
	createsprite gSandAttackDirtSpriteTemplate, ANIM_TARGET, 2, 15, 15, 20, 10, 10
	createsprite gSandAttackDirtSpriteTemplate, ANIM_TARGET, 2, 15, 15, 20, -10, -10
	createsprite gSandAttackDirtSpriteTemplate, ANIM_TARGET, 2, 15, 15, 20, 20, 5
	createsprite gSandAttackDirtSpriteTemplate, ANIM_TARGET, 2, 15, 15, 20, -20, -5
	delay 2
	return
G58_ScaldBeams:
	createsprite gHydroPumpOrbSpriteTemplate, ANIM_ATTACKER, 3, 10, 10, 0, 0
	delay 1
	createsprite gHydroPumpOrbSpriteTemplate, ANIM_ATTACKER, 3, 10, 10, 0, 0
	delay 1
	return
G58_ScaldHitSplats:
	createsprite gWaterHitSplatSpriteTemplate, ANIM_ATTACKER, 4, 0, 0, ANIM_TARGET, 1
	return
G58_ScreenFlash:
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_BG, 1, 12, 0, RGB(30, 31, 1)
	return
G58_SeismicTossContinue:
	restorebg
	waitbgfadeout
	setarg 7, 4095
	waitbgfadein
	clearmonbg ANIM_DEF_PARTNER
	blendoff
	end
G58_SeismicTossRockScatter1:
	create_basic_hitsplat_sprite ANIM_TARGET, 3, x=-10, y=-8, relative_to=ANIM_TARGET, animation=1
	playsewithpan SE_M_STRENGTH, SOUND_PAN_TARGET
	createvisualtask AnimTask_ShakeMon, 2, ANIM_TARGET, 0, 3, 5, 1
	createsprite gRockScatterSpriteTemplate, ANIM_TARGET, 2, -12, 27, 2, 3
	createsprite gRockScatterSpriteTemplate, ANIM_TARGET, 2, 8, 28, 3, 4
	createsprite gRockScatterSpriteTemplate, ANIM_TARGET, 2, -4, 30, 2, 3
	createsprite gRockScatterSpriteTemplate, ANIM_TARGET, 2, 12, 25, 4, 4
	return
G58_SeismicTossRockScatter2:
	create_basic_hitsplat_sprite ANIM_TARGET, 3, x=10, y=-8, relative_to=ANIM_TARGET, animation=1
	playsewithpan SE_M_ROCK_THROW, SOUND_PAN_TARGET
	createvisualtask AnimTask_ShakeMon, 2, ANIM_TARGET, 0, 3, 5, 1
	createsprite gRockScatterSpriteTemplate, ANIM_TARGET, 2, -12, 32, 3, 4
	createsprite gRockScatterSpriteTemplate, ANIM_TARGET, 2, 8, 31, 2, 2
	createsprite gRockScatterSpriteTemplate, ANIM_TARGET, 2, -4, 28, 2, 3
	createsprite gRockScatterSpriteTemplate, ANIM_TARGET, 2, 12, 30, 4, 3
	return
G58_SeismicTossWeak:
	call G58_SeismicTossRockScatter1
	delay 16
	call G58_SeismicTossRockScatter2
	goto G58_SeismicTossContinue
G58_SetBugBg:
	createvisualtask AnimTask_BlendBattleAnimPalExclude, 10, ANIM_TARGET, 0, 0, 4, RGB_BLACK
	createvisualtask AnimTask_GetAttackerSide, 2
	jumprettrue G58_SetBugBgPlayer
	fadetobg BG_BUG_OPPONENT
	waitbgfadeout
	createvisualtask AnimTask_StartSlidingBg, 5, 1536, 0, 0, -1
	goto G58_SetBugBgFade
G58_SetBugBgFade:
	delay 0
	createvisualtask AnimTask_BlendBattleAnimPal, 10, F_PAL_BG, 0, 4, 4, RGB_BLACK
	waitbgfadein
	return
G58_SetBugBgPlayer:
	fadetobg BG_BUG_PLAYER
	waitbgfadeout
	createvisualtask AnimTask_StartSlidingBg, 5, -1536, 0, 0, -1
	goto G58_SetBugBgFade
G58_SetImpactBackground:
	delay 2
	createvisualtask AnimTask_IsContest, 2
	jumprettrue G58_SetImpactContestsBG
	createvisualtask AnimTask_IsTargetPlayerSide, 2
	jumpretfalse G58_SetImpactOpponentBG
	jumprettrue G58_SetImpactPlayerBG
G58_SetImpactBackgroundRet:
	return
G58_SetImpactContestsBG:
	changebg BG_IMPACT_CONTESTS
	goto G58_SetImpactBackgroundRet
G58_SetImpactOpponentBG:
	changebg BG_IMPACT_OPPONENT
	goto G58_SetImpactBackgroundRet
G58_SetImpactPlayerBG:
	changebg BG_IMPACT_PLAYER
	goto G58_SetImpactBackgroundRet
G58_SetPsychicBackground:
	fadetobg BG_PSYCHIC
	waitbgfadeout
	createvisualtask AnimTask_SetPsychicBackground, 5
	waitbgfadein
	return
G58_StormThorwAnim:
	createsprite gSlideMonToOffsetSpriteTemplate, ANIM_ATTACKER, 2, ANIM_ATTACKER, 20, 0, 0, 4
	playsewithpan SE_M_SWAGGER, SOUND_PAN_TARGET
	delay 6
	createsprite gSlideMonToOriginalPosSpriteTemplate, ANIM_ATTACKER, 2, 0, 0, 7
	delay 8
	return
G58_StringShotThread:
	create_web_thread_sprite ANIM_TARGET, 2, x=20, y=0, unk2=512, amplitude=20, targets_both=TRUE
	delay 1
	return
G58_SweetScentEffect:
	createsprite gSweetScentPetalSpriteTemplate, ANIM_ATTACKER, 2, 70, 1, 64
	delay 2
	createsprite gSweetScentPetalSpriteTemplate, ANIM_ATTACKER, 2, 60, 0, 64
	delay 5
	createsprite gSweetScentPetalSpriteTemplate, ANIM_ATTACKER, 2, 80, 1, 64
	delay 2
	createsprite gSweetScentPetalSpriteTemplate, ANIM_ATTACKER, 2, 58, 0, 120
	delay 2
	createsprite gSweetScentPetalSpriteTemplate, ANIM_ATTACKER, 2, 100, 0, 120
	delay 2
	createsprite gSweetScentPetalSpriteTemplate, ANIM_ATTACKER, 2, 90, 0, 64
	delay 2
	createsprite gSweetScentPetalSpriteTemplate, ANIM_ATTACKER, 2, 48, 0, 64
	delay 2
	createsprite gSweetScentPetalSpriteTemplate, ANIM_ATTACKER, 2, 95, 1, 80
	delay 2
	createsprite gSweetScentPetalSpriteTemplate, ANIM_ATTACKER, 2, 100, 0, 120
	delay 2
	createsprite gSweetScentPetalSpriteTemplate, ANIM_ATTACKER, 2, 75, 1, 64
	delay 2
	createsprite gSweetScentPetalSpriteTemplate, ANIM_ATTACKER, 2, 85, 0, 120
	delay 2
	return
G58_TailWhackDown:
	create_basic_hitsplat_sprite ANIM_TARGET, 3, x=-10, y=-8, relative_to=ANIM_TARGET, animation=1
	playsewithpan SE_M_STRENGTH, SOUND_PAN_TARGET
	createsprite gRockScatterSpriteTemplate, ANIM_TARGET, 2, -12, 27, 2, 3
	createsprite gRockScatterSpriteTemplate, ANIM_TARGET, 2, 8, 28, 3, 4
	createsprite gRockScatterSpriteTemplate, ANIM_TARGET, 2, -4, 30, 2, 3
	createsprite gRockScatterSpriteTemplate, ANIM_TARGET, 2, 12, 25, 4, 4
	return
G58_ThunderCageBolts:
	createvisualtask AnimTask_ElectricBolt, 2, 25, -40, 1
	delay 9
	createvisualtask AnimTask_ElectricBolt, 2, -25, -40, 1
	delay 9
	createvisualtask AnimTask_ElectricBolt, 2, 0, -40, 1
	delay 9
	return
G58_UnsetBackground:
	restorebg
	waitbgfadeout
	setarg 7, -1
	waitbgfadein
	return
G58_UnsetBugBg:
	restorebg
	waitbgfadeout
	createvisualtask AnimTask_BlendBattleAnimPalExclude, 10, ANIM_TARGET, 0, 4, 0, RGB_BLACK
	setarg 7, -1
	waitbgfadein
	return
G58_WaterBubblesEffectLong:
	createsprite gWaterBubbleSpriteTemplate, ANIM_ATTACKER, 2, 10, 10, 1
	playsewithpan SE_M_BUBBLE3, SOUND_PAN_TARGET
	delay 6
	createsprite gWaterBubbleSpriteTemplate, ANIM_ATTACKER, 2, -28, -10, 1
	playsewithpan SE_M_BUBBLE3, SOUND_PAN_TARGET
	delay 6
	createsprite gWaterBubbleSpriteTemplate, ANIM_ATTACKER, 2, 20, -20, 1
	playsewithpan SE_M_BUBBLE3, SOUND_PAN_TARGET
	delay 6
	createsprite gWaterBubbleSpriteTemplate, ANIM_ATTACKER, 2, -20, 15, 1
	playsewithpan SE_M_BUBBLE3, SOUND_PAN_TARGET
	delay 6
	createsprite gWaterBubbleSpriteTemplate, ANIM_ATTACKER, 2, 0, 0, 1
	playsewithpan SE_M_BUBBLE3, SOUND_PAN_TARGET
	delay 6
	createsprite gWaterBubbleSpriteTemplate, ANIM_ATTACKER, 2, 27, 8, 1
	playsewithpan SE_M_BUBBLE3, SOUND_PAN_TARGET
	delay 6
	createsprite gWaterBubbleSpriteTemplate, ANIM_ATTACKER, 2, -20, -20, 1
	playsewithpan SE_M_BUBBLE3, SOUND_PAN_TARGET
	delay 6
	createsprite gWaterBubbleSpriteTemplate, ANIM_ATTACKER, 2, 16, -8, 1
	playsewithpan SE_M_BUBBLE3, SOUND_PAN_TARGET
	return

