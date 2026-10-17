#ifndef PE1_SCRIPT_QUEUE_H
#define PE1_SCRIPT_QUEUE_H

#include "pe1/game_state_types.h"

#include "common.h"
#include "pe1/field_actor.h"
#include "pe1/script_node.h"
#include "pe1/script_state.h"

extern u8 *D_8009D2F0[];
#include "pe1/player_entity.h"
/* The 0x200 script variables (0x800 bytes), cleared with the entity pools
 * and saved in the save tail; operand mode 2 indexes them. */
extern u32 g_ScriptVariableBank[];

/* Script variables with a fixed role outside the scripts: the room word
 * (Boot_Shutdown saves the dispatch token there, the effect sequences set
 * 999 and pick their room by it), the scene flags word and the area word. */
#define SCRIPT_VAR_ROOM 1
#define SCRIPT_VAR_FLAGS 3
#define SCRIPT_VAR_AREA 0x4A
extern int (*g_ScriptQueueOpcodeHandlers[])(int **);
int Entity_SetFieldByTypeId(int **args);
int Entity_CopyAttachmentData(int **args);
int Entity_GetField(int **args);
int Entity_SetField(int **args);
int Entity_FindByTypeId(int **args);
int Script_Random(int **args);
int Script_InitEntityMoveState(int **unused);
int Script_SetEntityAnim12Args(int **args);
int Script_SetSceneEntryAnim(int **args);
int Script_SetRenderEntryEnabled(int **args);
int Script_SetMenuSlotEntry(int **args);
int Script_CountItemsByValue(int **args);
int Script_SetBattleStateMode(int **unused);
int Script_ComputeAudioTimer(int **args);
int Script_GetCombatModeFlag(int **args);
int Script_SetCombatModeFlag(int **unused);
int GameTime_ConvertSecondsToHMS(int **args);
int Script_SetEntityAnim5Args(int **args);
int Script_SetBattleEntryCoords(int **args);
int Script_GetFloorNumber(int **args);
int Script_IsBattlePhase4(int **args);
int Entity_MarkNodeAnimPendingById(int **args);
int Entity_AdvanceNodeById(int **args);
int Script_SetEntityParentLink(int **args);
int Script_Noop(int **unused);
int Script_GetSqrt(int **args);
int Script_SetEntityShadowFlag(int **unused);
int Script_ClearEntityShadowFlag(int **unused);
int Script_SignalSceneExit(int **unused);
int Script_GetEntityEffect(int **args);
int Script_RunEntityCommand(int **args);
int Script_SetEntityActionAndWait(int **args);
int Entity_TestCurrentFlags(int **args);
int Entity_SetCurrentFlag100(int **unused);
int Entity_ClearCurrentFlag100(int **unused);
int Script_ClearEntityFlag1(int **unused);
int Script_SetEntityFlag1(int **unused);
int Script_SetEntityFlag400(int **unused);
int Script_ClearEntityFlag400(int **unused);
int Script_WaitEntityFlags8(int **unused);
int Pm_ScriptSendCommand0(int **args);
int Pm_ScriptSendCommand1(int **args);
int Pm_ScriptSetState(int **args);
int Pm_ScriptGetState(int **args);
int Pm_ScriptStopCurrentEntity(int **args);
int Script_SetEntityTarget(int **args);
int Script_ResetRenderState(int **unused);
int Script_CondBranch(int **args);
int Script_GetMenuChoice(int **args);
int Script_SetCurrentEntityField20(int **args);
int Script_SetEntityField1C(int **args);
int Script_SetEntityField1E6(int **args);
int Render_CameraTrackEntityAngle(int **args);
int Script_SetEntityField224(int **args);
int Script_GetFloorTableValue(int **args);
int Boot_CheckModeSwitch(int **args);
int Script_GiveItem(int **args);
int Entity_InitWithTimeout(int **args);
int Script_CopyEntityRenderObject(int **args);
int Script_DrawSaveOverlay(int **args);
int Script_ResetEntityAnimSlot(int **args);
int Entity_MarkAnimPending(int **unused);
int Entity_AdvancePendingAnim(int **unused);
int Script_ResetPlayerCameraPos(int **unused);
int Script_ShowReceivedItem(int **args);
int Scene_TestGlobalFlags(int **args);
int Scene_SetGlobalFlags(int **args);
int Scene_ClearGlobalFlags(int **args);
int Boot_MemCardStepInit(int **args);
int Menu_SetModeState5(int **unused);
int Menu_SetModeState6(int **unused);
int Menu_ClearModeState(int **unused);
int Menu_SetModeState8(int **unused);
int Entity_ClearGlobalFlag1(int **unused);
int Entity_SetGlobalFlag1(int **unused);
int Entity_SetCurrentFlag40(int **unused);
int Entity_ClearCurrentFlag40(int **unused);
int Script_ClearEntityFlag40(int **unused);
int Script_Noop1(int **unused);
int Script_Noop2(int **unused);
int Script_Noop3(int **unused);
int Script_Noop4(int **unused);
int Script_WaitFrameOrDraw(int **args);
int Script_EncodeBarcode(int **args);
int Script_StopEntityAnim(int **args);
int Script_YieldWhileTableEntrySet(int **args);
int Script_ClearTableEntry(int **args);
int Script_PointInPoly(int **args);
int Script_OpenSaveLoadMenu(int **args);
int Script_BeginSceneTransition(int **args);
int Script_RunInvCommand(int **args);
int Script_OpenMemCardDialog(int **unused);
int Script_SetMoveFlag(int **unused);
int Script_ClearMoveFlag(int **unused);
int Script_ClampMenuRange(int **args);
int Script_SetGteMatrix(int **args);
int Script_SpawnEntityAt(int **args);
int Script_DispatchScriptCmd(int **args);
int Entity_SetCurrentActionMode(int **args);
int Entity_ClearCurrentFlag20(int **unused);
int Entity_SetCurrentFlag20(int **unused);
int Script_SetObjEntryFlag80(int **args);
int Script_ClearObjEntryFlag80(int **args);
int Script_LoadGeomState(int **args);
int Script_SetFogParams(int **args);
int Script_SetFogParamsAlt(int **args);
int Entity_ClearMotionVectors(int **unused);
int Entity_ComputeVelocity(int **unused);
int Render_CameraSnapToEntity(int **args);
int Render_CameraTrackEntityZoom(int **args);
int Script_SetEntityAnimSpeed(int **args);
int Script_SetScreenScrollPos(int **args);
int Script_SetCdRomSeekPos(int **args);
int Script_SetFogColor(int **args);
int Script_SetCdRomScreenPos(int **args);
int Script_ScaleAnimValue(int **args);
int Entity_YieldOnLoading(int **unused);
int Script_SetFlagSlot(int **args);
int Script_SetObjEntryLimit(int **args);
int Script_SetObjEntryField8(int **args);
int Script_GetObjEntryField6(int **args);
int Script_SetEntryScrolled(int **args);
int Script_SetEntryMirrored(int **args);
int Script_ClearBattleFlags(int **unused);
int Script_SetBattleFlags(int **unused);
extern u8 g_ScriptCameraBytes[3];
int Entity_YieldOnDrawBusy(int **unused);

/* Resident callbacks use the interpreter's decoded operand-pointer array. */
/* Movie opcode: 1 starts; -1 invalid input/workspace, -2 busy, -3 stream error. */
int Script_LoadSceneData(int **args);
int Script_MoveTowardPoint(int **args);
int Render_CameraTrackRelativeOffset(int **args);
int Render_CameraStepAngleFade(int **args);
int Script_TurnTowardPointStep(int **args);
int Script_PlayEntityAnim(int **args);
int Script_WaitAnimRange(int **unused);
int Script_WaitAnimFrameMatch(int **unused);
int Script_SetEntityAnimFrame(int **args);
int Entity_QueryField(int **args);
int Entity_GetPositionByType(int **args);
int Script_GetAngleToEntity(int **args);
int Entity_GetDistanceComponents(int **args);
int Entity_YieldWhileMoving(int **unused);
int Entity_ComputeVelocityToTarget(int **args);
int Script_DispatchCmd(int **args);
int Script_SetEntityVec3(int **args);
int Script_GetEntityVec3(int **args);
int Script_SetEntityMoveSpeed(int **args);
int Script_SetAnimInterpRate(int **args);
int Menu_GetModeState(int **args);
int Script_GetSin(int **args);
int Script_GetCos(int **args);
int Script_GetAtan2(int **args);
int Entity_PopCount(int **args);
int Script_SetMeshFadeLevel(int **args);
int Entity_CallAction(int **args);
int Script_SetRenderFlag8Params(int **args);
int Script_ClearRenderFlag8(int **unused);
int Script_SetRenderFlag10Value(int **args);
int Script_ClearRenderFlag10(int **unused);
int Script_FillObjEntrySlotValues(int **args);
int Script_SetObjEntrySlotValue(int **args);
int Script_SetObjEntryFlags(int **args);
int Script_ClearObjEntryFlags(int **args);
int Entity_PolarToPosition(int **args);
int Script_RotateEntityAngle(int **args);
int Script_GetAngleToPoint(int **args);
int Script_SnapPosToSpawnPos(int **unused);
int Entity_ClearParentLink(int **unused);
int Script_ClearEntityParentLink(int **unused);
int Script_SetNodeFlag80(int **unused);
int Script_ClearEntityRoamFlag(int **unused);
int Script_SetEntityRoamFlag(int **unused);
int Script_GetInventoryTotal(int **args);
int Script_SetInventorySlotPointer(int **args);
int Script_GetInventorySlotLimit(int **args);
int Script_SetEntityFlag1000000(int **unused);
int Script_SetAnimSlotPointer(int **args);
int Script_SetInputFlagBit2(int **unused);
int Script_ClearInputFlagBit2(int **unused);
int Script_EnableEntityGravity(int **unused);
int Script_DisableEntityGravity(int **unused);
int Script_SetSceneFlag2000(int **unused);
int Script_ClearSceneFlag2000(int **unused);
int Entity_SetCurrentFlag80(int **unused);
int Entity_ClearCurrentFlag80(int **unused);
int Entity_SetCurrentFlag4000(int **unused);
int Entity_ClearCurrentFlag4000(int **unused);

void Script_RunQueue(void);
int Script_JumpToEntityOffset(int **args);
int Entity_MarkCurrentDead(int **args);
int Entity_SetCurrentNodeActive(int **args);
int Entity_MarkCurrentNodeFree(int **args);
int Entity_MarkNodeFree(int **unused);
int Entity_FindNodeById(int **args);
int Script_AssignValue(int **args);
int Script_SpawnChildNode(int **args);
int Script_JumpIfZero(int **args);

#endif
