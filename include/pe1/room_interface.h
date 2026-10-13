#ifndef PE1_ROOM_INTERFACE_H
#define PE1_ROOM_INTERFACE_H

/* The room interface: globals every room or scene overlay defines in its own
 * data (or its own copy of a room_lib unit) and the shared sources
 * (src/overlays/room_lib, include/pe1) name with one spelling. The overlays
 * link one at a time into the same load window, so the retail names could
 * repeat; a single executable cannot hold two definitions of one name.
 *
 * The overlay compile rule passes -DPE_OVERLAY=<overlay name> (Makefile,
 * overlay-build), and each name below then becomes <name>__<overlay> in the
 * object file, for example g_RoomScript__room_m256_museum. The source keeps
 * writing g_RoomScript; the overlay's symbol file names the data with the
 * suffixed name (docs/CONVENTIONS.md, Overlays). Without PE_OVERLAY (main,
 * tools that compile one unit on its own) the names stay as written. */

#ifdef PE_OVERLAY
#define PE_OVERLAY_PASTE_(name, overlay) name##__##overlay
#define PE_OVERLAY_PASTE(name, overlay) PE_OVERLAY_PASTE_(name, overlay)
#define PE_OVERLAY_SYM(name) PE_OVERLAY_PASTE(name, PE_OVERLAY)
#else
#define PE_OVERLAY_SYM(name) name
#endif

/* pe1/room_module.h */
#define g_RoomClassTable PE_OVERLAY_SYM(g_RoomClassTable)
#define g_RoomScript PE_OVERLAY_SYM(g_RoomScript)
#define g_RoomDrawList PE_OVERLAY_SYM(g_RoomDrawList)
#define g_RoomUpdateList PE_OVERLAY_SYM(g_RoomUpdateList)
#define g_RoomInitList PE_OVERLAY_SYM(g_RoomInitList)
#define g_RoomSpawnLayout PE_OVERLAY_SYM(g_RoomSpawnLayout)
#define g_RoomScriptArgs PE_OVERLAY_SYM(g_RoomScriptArgs)
#define g_RoomScriptArg PE_OVERLAY_SYM(g_RoomScriptArg)
/* pe1/room_script_record.h */
#define g_RoomScriptRecord PE_OVERLAY_SYM(g_RoomScriptRecord)
/* room_lib/room_lib.h */
#define RoomLib_SlotTable PE_OVERLAY_SYM(RoomLib_SlotTable)
/* pe1/room_flare.h, pe1/room_grab_attack.h */
#define RoomLib_PairB PE_OVERLAY_SYM(RoomLib_PairB)
/* pe1/room_grab_attack.h */
#define g_RoomGrabSweepSide PE_OVERLAY_SYM(g_RoomGrabSweepSide)
#define g_RoomGrabTrailTrack PE_OVERLAY_SYM(g_RoomGrabTrailTrack)
/* pe1/scene_grab_head.h: each grab scene's own approach check */
#define RoomLib_GrabOnApproach PE_OVERLAY_SYM(RoomLib_GrabOnApproach)
/* pe1/room_drop_field.h, pe1/room_drop_field_class.h */
#define g_RoomDropFieldDrawList PE_OVERLAY_SYM(g_RoomDropFieldDrawList)
#define g_RoomDropFieldParticleFrames PE_OVERLAY_SYM(g_RoomDropFieldParticleFrames)
/* pe1/room_ground_eruption.h */
#define g_RoomEruptionSpawnLayout PE_OVERLAY_SYM(g_RoomEruptionSpawnLayout)
/* pe1/room_glow_orb.h */
#define g_RoomGlowOrbScript PE_OVERLAY_SYM(g_RoomGlowOrbScript)
/* pe1/room_motion_init.h */
#define g_RoomMotionTrailA PE_OVERLAY_SYM(g_RoomMotionTrailA)
/* pe1/room_paired_glow.h */
#define g_RoomPairedGlowColorTable PE_OVERLAY_SYM(g_RoomPairedGlowColorTable)
#define g_RoomPairedGlowPacket PE_OVERLAY_SYM(g_RoomPairedGlowPacket)
/* pe1/room_trail_burst.h */
#define g_RoomScatterBurstLayer PE_OVERLAY_SYM(g_RoomScatterBurstLayer)
/* pe1/room_spark_ring_burst.h */
#define g_RoomSparkSlot PE_OVERLAY_SYM(g_RoomSparkSlot)
/* pe1/room_spark_attacks.h */
#define g_RoomCaptureFrameWord PE_OVERLAY_SYM(g_RoomCaptureFrameWord)

/* Shared room_lib functions that read or write one of the names above (or
 * call such a function). Each overlay's copy works on that overlay's data,
 * so it is that overlay's function and takes the same suffix. */
#define RoomLib_PlantScript PE_OVERLAY_SYM(RoomLib_PlantScript)
#define RoomLib_RegisterDrawList PE_OVERLAY_SYM(RoomLib_RegisterDrawList)
#define RoomLib_RegisterDrawListActive PE_OVERLAY_SYM(RoomLib_RegisterDrawListActive)
#define RoomLib_RegisterPairedTables PE_OVERLAY_SYM(RoomLib_RegisterPairedTables)
#define RoomLib_RegisterPairedTablesActive PE_OVERLAY_SYM(RoomLib_RegisterPairedTablesActive)
#define RoomLib_RegisterPairedTablesGated PE_OVERLAY_SYM(RoomLib_RegisterPairedTablesGated)
#define RoomLib_SetArg1 PE_OVERLAY_SYM(RoomLib_SetArg1)
#define RoomLib_SetArgs3 PE_OVERLAY_SYM(RoomLib_SetArgs3)
#define RoomLib_SetTaskVar0 PE_OVERLAY_SYM(RoomLib_SetTaskVar0)
#define RoomLib_SlotSet PE_OVERLAY_SYM(RoomLib_SlotSet)
#define RoomLib_CaptureFrameSample PE_OVERLAY_SYM(RoomLib_CaptureFrameSample)
#define RoomLib_GrabAwaitStance PE_OVERLAY_SYM(RoomLib_GrabAwaitStance)
#define RoomLib_InitPairedGlowSprite PE_OVERLAY_SYM(RoomLib_InitPairedGlowSprite)
#define RoomLib_DrawPairedGlowSprite PE_OVERLAY_SYM(RoomLib_DrawPairedGlowSprite)
#define RoomLib_TransformPairedEmitter PE_OVERLAY_SYM(RoomLib_TransformPairedEmitter)
#define RoomLib_UpdatePairedEmitter PE_OVERLAY_SYM(RoomLib_UpdatePairedEmitter)
#define RoomEffect_ConfigureEffectSequence PE_OVERLAY_SYM(RoomEffect_ConfigureEffectSequence)
#define RoomEffect_DrawEightParticles PE_OVERLAY_SYM(RoomEffect_DrawEightParticles)
#define RoomEffect_DropFieldRegister PE_OVERLAY_SYM(RoomEffect_DropFieldRegister)
#define RoomEffect_DroppedFlareSetArgs PE_OVERLAY_SYM(RoomEffect_DroppedFlareSetArgs)
#define RoomEffect_EffectSequenceRegister PE_OVERLAY_SYM(RoomEffect_EffectSequenceRegister)
#define RoomEffect_EffectSequenceStart PE_OVERLAY_SYM(RoomEffect_EffectSequenceStart)
#define RoomEffect_GlowOrbSetup PE_OVERLAY_SYM(RoomEffect_GlowOrbSetup)
#define RoomEffect_GlowOrbSpawnerUpdate PE_OVERLAY_SYM(RoomEffect_GlowOrbSpawnerUpdate)
#define RoomEffect_GlowOrbUpdate PE_OVERLAY_SYM(RoomEffect_GlowOrbUpdate)
#define RoomEffect_GrabSetPair PE_OVERLAY_SYM(RoomEffect_GrabSetPair)
#define RoomEffect_GroundEruptionStart PE_OVERLAY_SYM(RoomEffect_GroundEruptionStart)
#define RoomEffect_GroundSweepController PE_OVERLAY_SYM(RoomEffect_GroundSweepController)
#define RoomEffect_InitParticleRenderPair PE_OVERLAY_SYM(RoomEffect_InitParticleRenderPair)
#define RoomEffect_MotionParticlesInit PE_OVERLAY_SYM(RoomEffect_MotionParticlesInit)
#define RoomEffect_OrbitTrailBurst PE_OVERLAY_SYM(RoomEffect_OrbitTrailBurst)
#define RoomEffect_OrbitTrailParticle PE_OVERLAY_SYM(RoomEffect_OrbitTrailParticle)
#define RoomEffect_ScatterBurstController PE_OVERLAY_SYM(RoomEffect_ScatterBurstController)
#define RoomEffect_SetSparkSlot PE_OVERLAY_SYM(RoomEffect_SetSparkSlot)
#define RoomEffect_UpdateFallingSprite PE_OVERLAY_SYM(RoomEffect_UpdateFallingSprite)

#endif
