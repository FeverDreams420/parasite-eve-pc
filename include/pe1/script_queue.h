#ifndef PE1_SCRIPT_QUEUE_H
#define PE1_SCRIPT_QUEUE_H

#include "pe1/game_state_types.h"

#include "common.h"
#include "pe1/field_actor.h"
#include "pe1/script_node.h"
#include "pe1/script_state.h"

extern u8 *D_8009D2F0[];
extern struct BattleEntity *D_8009D254;
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

/* Resident callbacks use the interpreter's decoded operand-pointer array. */
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
