#include "pe1/geom_state.h"
#include "pe1/gte.h"
#include "pe1/scene_e20_hover_orb.h"

/* Original charge, beam and fade states share the native task engine. */
int SceneE20_HoverOrbController(int mode, void *memory, intptr_t argument) {
    SceneE20TrailEffect *effect = memory;
    SceneEffectSlot *command = (SceneEffectSlot *)argument;
    s32 initialMode;
    SceneE20Color *initColor0;
    SceneE20Color *initColor1;
    u16 headPositionZ;
    u16 headPositionY;
    u16 headOffsetX;
    u16 headPositionX;
    SceneE20Vec point;
    SceneE20Vec drawRotation;
    SceneE20Color beamColor;
    SceneE20Color edgeColor;
    SceneE20Color flashA;
    SceneE20Color flashB;
    SceneE20Matrix matrix;
    GteVector scale = {0};
    SceneE20Color *oddColor;
    SceneE20Color *evenColor;
    SceneE20Color *flashColor;
    SceneE20Particle *temp_v0_10;
    SceneE20Particle *temp_v0_12;
    SceneE20Vec *temp_s0_2;
    SceneE20Vec *var_a1;
    s16 temp_v0_14;
    s16 temp_v0_2;
    s16 temp_v0_5;
    s16 temp_v0_7;
    s16 temp_v0_9;
    s16 temp_v1;
    s16 temp_v1_2;
    s32 var_s0;
    s32 modelScale;
    u32 assetKey;
    s32 paletteX;
    s32 paletteY;
    s32 translationZ;
    s32 drawAngle;
    u32 particleZ, effectZ;
    u16 endpointX;
    s32 temp_ret;
    s32 temp_s0;
    s32 temp_s0_3;
    s32 temp_s3;
    s32 temp_v0_13;
    s32 temp_v0_4;
    s32 var_a0;
    s32 var_a1_2;
    s32 particleIndex;
    s32 var_v0;
    u16 temp_a1;
    u16 texturePage;
    u16 *pageIndex;
    int pageArg0;
    int pageArg1;
    int pageArg2;
    u32 textureIndex;
    u16 temp_v0_3;
    void *temp_v0;

    beamColor = g_SceneE20OrbBeamColor;
    edgeColor = g_SceneE20OrbEdgeColor;
    flashA = g_SceneE20OrbFlashColorOdd;
    flashB = g_SceneE20OrbFlashColorEven;
    initColor0 = &beamColor; initColor1 = &edgeColor;
    oddColor = &flashA;
    initialMode = 1;
    if (mode == initialMode) { evenColor = &flashB; goto update; }
    evenColor = &flashB;
    if (mode < 2) {
        if (mode == 0) goto initialize;
        return 0;
    }
    if (mode == 2) goto render;
    return 0;
initialize:
            effect->phase = 0;
            effect->timer = 0;
            effect->alpha = 0x80;
            command->pending = 0;
            effect->target.x = (s16) (u16) g_FieldFxOwner->actor->render_object.target_x;
            effect->target.y = (s16) (u16) g_FieldFxOwner->actor->render_object.target_y;
            effect->target.z = (s16) (u16) g_FieldFxOwner->actor->render_object.target_z;
            assetKey = 0xC5540000U;
            effect->previousPosition.x = (s16) (u16) effect->target.x;
            effect->previousPosition.y = (s16) (u16) effect->target.y;
            effect->previousPosition.z = (s16) (u16) effect->target.z;
            effect->position.x = (s16) (u16) effect->target.x;
            effect->position.y = (s16) (u16) effect->target.y;
            effect->position.z = (s16) (u16) effect->target.z;

            temp_v0 = Asset_FindTable08ByU32Key(g_LoadedSceneAssetBlock[0], assetKey | 0x3704U);
            g_SceneE20HoverOrbModel = temp_v0;
            FieldFx_ModelScrollUv(temp_v0, 0, 0);
            FieldFx_DrawHistoryTrail(&effect->position, &effect->position, 0x3EE, (unsigned char *)initColor0, (unsigned char *)initColor1, 0x80, (struct FieldTrailPair *)effect->history, initialMode);
            return FieldFx_InitCallbackList((char *)g_FieldFxActiveTaskSlot->end, 0x14, 0x20,
                                 SceneE20_FlareParticle);
update: {
    temp_v1_2 = effect->phase;
    switch (temp_v1_2) {                            /* switch 1 */
    case 0:                                         /* switch 1 */
        command->pending = 0;
        FieldFx_GetActorPosition((char *)g_PlayerEntity, 0, (s16 *)&point);
        point.x = (rand() & 0x7FF) - 0x400;
        point.y = ((u16) g_RoomFloorY->y - 0x258) - (rand() & 0x1FF);
        point.z = (rand() & 0x7FF) - 0x400;
        effect->target.x = (s16) (u16) point.x;
        effect->target.y = (s16) (u16) point.y;
        effect->target.z = (s16) (u16) point.z;
        FieldFx_GetActorPosition((char *)g_PlayerEntity, 1, (s16 *)&effect->endpoint);
        effect->duration = (rand() & 0xF) + 6;
        effect->timer = 0;
        effect->phase = 1;
        break;
    case 1:                                         /* switch 1 */
        temp_v0_5 = (u16) effect->timer + 1;
        effect->timer = temp_v0_5;
        if ((temp_v0_5 == 1) && (rand() & 1)) {
            Asset_Find08w(0x5FF, FieldFx_GetOwnerProgress(), effect->position.x, effect->position.y, (s32) effect->position.z);
        }
        temp_s3 = rsin((effect->timer * 1024) / effect->duration);
        LoadAverageShort12(&effect->previousPosition, &effect->target, 0x1000 - temp_s3, temp_s3, &effect->position);
        FieldFx_GetActorPosition((char *)g_PlayerEntity, 1, (s16 *)&effect->endpoint);
        if (effect->timer >= effect->duration) {
            effect->phase = 2;
            effect->timer = 0;
        }
        break;
    case 2:                                         /* switch 1 */
        effect->timer = (u16) effect->timer + 1;
        FieldFx_GetActorPosition((char *)g_PlayerEntity, 1, (s16 *)&effect->endpoint);
        if (effect->timer >= 4) {
            effect->phase = 0;
            effect->timer = 0;
            effect->previousPosition.x = (s16) (u16) effect->position.x;
            effect->previousPosition.y = (s16) (u16) effect->position.y;
            effect->previousPosition.z = (s16) (u16) effect->position.z;
            if (command->pending != 0) {
                command->pending = 0;
                effect->phase = 3;
                endpointX = (u16) effect->endpoint.x;
                effect->target.x = (s16) (u16) command->target.x;
                effect->target.y = (s16) (u16) command->target.y;
                effect->target.z = (s16) (u16) command->target.z;
                temp_v1 = (u16) effect->endpoint.z;
                effect->commandedEndpoint.x = (s16) (u16) command->position.x;
                effect->commandedEndpoint.y = (s16) (u16) command->position.y;
                effect->commandedEndpoint.z = (s16) (u16) command->position.z;
                                var_a1_2 = command->duration;
                effect->previousEndpoint.x = (s16) endpointX;
                effect->previousEndpoint.z = temp_v1;
                effect->previousEndpoint.y = (s16) (u16) effect->endpoint.y;
                effect->duration = (s16) var_a1_2;
            }
        }
        break;
    case 3:                                         /* switch 1 */
        temp_v0_7 = (u16) effect->timer + 1;
        effect->timer = temp_v0_7;
        if (temp_v0_7 == 1) {
            Asset_Find08w(0x5FF, FieldFx_GetOwnerProgress(), effect->position.x, effect->position.y, (s32) effect->position.z);
        }
        temp_s3 = rsin((effect->timer * 1024) / effect->duration);
        temp_s0_3 = 0x1000 - temp_s3;
        LoadAverageShort12(&effect->previousPosition, &effect->target, temp_s0_3, temp_s3, &effect->position);
        LoadAverageShort12(&effect->previousEndpoint, &effect->commandedEndpoint, temp_s0_3, temp_s3, &effect->endpoint);
        if (effect->timer >= effect->duration) {
            effect->phase = 4;
            effect->timer = 0;
            effect->previousPosition.x = (s16) (u16) effect->position.x;
            effect->previousPosition.y = (s16) (u16) effect->position.y;
            effect->previousPosition.z = (s16) (u16) effect->position.z;
        }
        break;
    case 4:                                         /* switch 1 */
        temp_v0_9 = (u16) effect->timer + 1;
        effect->timer = temp_v0_9;
        if (temp_v0_9 == 1) {
            temp_v0_10 = FieldFx_AcquireCallbackSlot((char *)g_FieldFxActiveTaskSlot->end);
            if (temp_v0_10 != NULL) {
                temp_v0_10->velocity.x = (s16) (u16) effect->rotation.x;
                temp_v0_10->velocity.y = (s16) (u16) effect->rotation.y;
                temp_v0_10->velocity.z = (s16) (u16) effect->rotation.z;
                FieldFx_AnglesToVector(&effect->rotation, 0x8C, &temp_v0_10->position);
                temp_v0_10->position.x = (u16) temp_v0_10->position.x + (u16) effect->position.x;
                temp_v0_10->position.y = (u16) temp_v0_10->position.y + (u16) effect->position.y;
                particleZ = (u16) temp_v0_10->position.z;
                effectZ = (u16) effect->position.z;
                temp_v0_10->kind = 0;
                temp_v0_10->timer = 0;
                temp_v0_10->position.z = particleZ + effectZ;
            }
            Asset_Find08w(0x600, 0x80, effect->position.x, effect->position.y, (s32) effect->position.z);
        }
        if (effect->timer >= 4) {
            effect->phase = 5;
            effect->timer = 0;
            temp_v0_10 = FieldFx_AcquireCallbackSlot((char *)g_FieldFxActiveTaskSlot->end);
            if (temp_v0_10 != NULL) {
                temp_v0_10->position.x = (s16) (u16) effect->endpoint.x;
                temp_v0_10->position.y = (s16) (u16) effect->endpoint.y;
                temp_v0_10->position.z = (s16) (u16) effect->endpoint.z;
                temp_v0_10->velocity.x = (s16) (u16) effect->rotation.x;
                temp_v0_10->velocity.y = (s16) (u16) effect->rotation.y;
                temp_v0_14 = effect->rotation.z;
                temp_v0_10->kind = 1;
                temp_v0_10->timer = 0;
                temp_v0_10->velocity.z = temp_v0_14;
            }
            for (particleIndex = 0; particleIndex < 12; ++particleIndex) {
            temp_v0_12 = FieldFx_AcquireCallbackSlot((char *)g_FieldFxActiveTaskSlot->end);
            if (temp_v0_12 != NULL) {
                temp_v0_12->position.x = (s16) (u16) effect->endpoint.x;
                temp_v0_12->position.y = (s16) (u16) effect->endpoint.y;
                temp_v0_12->position.z = (s16) (u16) effect->endpoint.z;
                temp_v0_12->velocity.x = (rand() % 70) - 0x23;
                temp_ret = rand();
                temp_v0_12->velocity.y = ((temp_ret / 70) * 0x46) - temp_ret;
                temp_v0_13 = rand();
                temp_v0_12->kind = 2;
                temp_v0_12->timer = 0;
                temp_v0_12->velocity.z = (temp_v0_13 % 70) - 0x23;
            }
            }
        }
        break;
    case 5:                                         /* switch 1 */
        effect->timer = (u16) effect->timer + 1;
        if ((FieldFx_IsPlayerWithinRadius(&effect->endpoint, 0x42) != 0) && (g_SceneE20HoverOrbHitCooldown == 0)) {
            EnemyCombatant *owner = (EnemyCombatant *)g_FieldFxOwner->actor->state;
            unsigned int selected = (owner->coreFlags >> 21) & 7;
            if (selected < 6) owner->records[selected].state = 2;
            if (g_FieldFxTaskContext->flags != 0 &&
                (owner->coreFlags & 0x3F000000) == 0x01000000) {
                g_PlayerEntity->state->flags |= 0x4000;
                owner->coreFlags = (owner->coreFlags & 0xC0FFFFFF) | 0xAF000000;
            }
            g_SceneE20HoverOrbHitCooldown = 0x74;
        }
        if (effect->timer >= 8) {
            effect->phase = 0;
            effect->timer = 0;
        }
        break;
    case 6:                                         /* switch 1 */
        temp_v0_14 = (u16) effect->timer + 1;
        effect->timer = temp_v0_14;
        effect->alpha = 0x60 - (temp_v0_14 * 3);
        if (effect->timer >= 0x20) {
            return 1;
        }
        break;
    default: break;
    }
        if (effect->phase != 6) {
            effect->alpha = (rsin((u32)g_FieldFxCurrentAge << 5) / 128) + 0x80;
        }
        temp_s0_2 = &effect->rotation;
        FieldFx_CalculateLookAngles(&effect->position, &effect->endpoint, temp_s0_2);
        FieldFx_AnglesToVector(temp_s0_2, 0x8C, &effect->trailHead);
        headOffsetX = (u16)effect->trailHead.x;
        headPositionX = (u16)effect->position.x;
        headPositionY = (u16)effect->position.y;
        headPositionZ = (u16)effect->position.z;
        effect->trailHead.x = headOffsetX + headPositionX;
        effect->trailHead.y = (u16) effect->trailHead.y + headPositionY;
        effect->trailHead.z = (u16) effect->trailHead.z + headPositionZ;
        FieldFx_AnglesToVector(temp_s0_2, -0x8C, &effect->trailTail);
        effect->trailTail.x = (u16) effect->trailTail.x + (u16) effect->position.x;
        effect->trailTail.y = (u16) effect->trailTail.y + (u16) effect->position.y;
        effect->trailTail.z = (u16) effect->trailTail.z + (u16) effect->position.z;
        if ((g_FieldFxOwner->actor->state->control10.command_value <= 0x7A11F) && (effect->phase != 6)) {
            effect->phase = 6;
            effect->timer = 0;
        }
        if (g_SceneE20HoverOrbHitCooldown > 0) {
            g_SceneE20HoverOrbHitCooldown -= 1;
            return 0;
        }
        g_SceneE20HoverOrbHitCooldown = 0;
        return 0;
    }
render: {
        GteMatrix **matrixAddress = &D_800BCFA4.value;
        {
            u32 *matrixWords;
            (matrixWords = (u32 *)*matrixAddress);
            gte_ldrotmatrix((const GteMatrixWords *)matrixWords);
            gte_ldtransmatrix((const GteMatrixWords *)matrixWords);
        }
        textureIndex = g_FieldFxTpageSlot3;
        g_FieldFxSpriteParams.parameter00 = 0x20;
        g_FieldFxSpriteParams.parameter02 = 2;
        g_FieldFxSpriteParams.extent_x = 0x20;
        g_FieldFxSpriteParams.extent_y = 0x20;
        texturePage = g_FieldFxTpageTable[textureIndex];

        g_FieldFxSpriteParams.palette = 3;
        g_FieldFxSpriteParams.parameter06 = 0;
        g_FieldFxSpriteParams.parameter0A = 0;
        g_FieldFxSpriteParams.depth = 4;
        g_FieldFxSpriteParams.tpage = texturePage;
        {
            u32 *matrixWords;
            (matrixWords = (u32 *)*matrixAddress);
            gte_ldrotmatrix((const GteMatrixWords *)matrixWords);
            gte_ldtransmatrix((const GteMatrixWords *)matrixWords);
        }
        flashColor = oddColor;
        if (!(g_FieldFxCurrentAge & 1)) flashColor = evenColor;
        temp_v1 = effect->phase;
        switch (temp_v1) {                          /* switch 2; irregular */
        case 4:                                     /* switch 2 */
            temp_s3 = effect->timer * 1024;
            LoadAverageShort12(&effect->trailHead, &effect->endpoint, 0x1000 - temp_s3, temp_s3, &point);
            var_a1 = &point;
block_63:
            FieldFx_DrawGlowLine(&effect->trailHead, var_a1, (u8 *)flashColor, (u8 *)flashColor, 128, 128, 1);
            break;
        case 5:                                     /* switch 2 */
            var_s0 = rcos(effect->timer << 7) / 32;
            FieldFx_DrawGlowLine(&effect->trailHead, &effect->endpoint, (u8 *)flashColor, (u8 *)flashColor,
                         var_s0 / 2, var_s0, 1);
            break;
        }
        temp_v0_2 = effect->alpha;
        if (temp_v0_2 != 0) {
            temp_s3 = 1;
            if (temp_v0_2 >= 0x60) {
                temp_s3 = 0xFF;
            }
            pageArg0 = 0; pageArg1 = temp_s3; pageArg2 = 0;
                        pageIndex = &g_FieldFxTpageSlot11;
            texturePage = g_FieldFxTpageTable[*pageIndex];

            g_FieldFxSpriteParams.palette = 3;
            g_FieldFxSpriteParams.parameter06 = 1;
            g_FieldFxSpriteParams.tpage = texturePage;
            temp_s0 = (g_FieldFxTpageTable[*pageIndex] | GetTPage(pageArg0, pageArg1, pageArg2, 0)) & 0xFFFF;
            modelScale = 0x400;
            temp_a1 = g_FieldFxPaletteTable[D_800F336C];
            if ((D_800F336C == 4) && (g_FieldFxAltPaletteFlag != 0)) {
                var_a1_2 = temp_a1 + 6;
            } else {
                var_a1_2 = temp_a1 + 2;
            }
            FieldFx_ModelSetTexture(temp_s0, GetClut(0, var_a1_2) & 0xFFFF);
            if (temp_s3 != 0xFF) FieldFx_ModelSetSemiTrans(1);
            else FieldFx_ModelSetSemiTrans(0);
            drawAngle = g_FieldFxCurrentAge * 0x60;
            temp_v0_3 = (u16) effect->rotation.x;
            drawRotation.x = temp_v0_3;
            drawRotation.y = (u16) effect->rotation.y;
            drawRotation.z = (u16) effect->rotation.z;
            drawRotation.x = temp_v0_3 + 0x800;
            var_v0 = rsin(drawAngle);
            if (var_v0 < 0) {
                var_v0 += 7;
            }
            drawRotation.z = (var_v0 >> 3) + 0x800;
            RotMatrixYXZ(&effect->rotation, &matrix);
            matrix.t[0] = (s32) effect->position.x;
            matrix.t[1] = (s32) effect->position.y;
            translationZ = effect->position.z;
            scale.x = modelScale;
            scale.y = modelScale;
            scale.z = modelScale;
            matrix.t[2] = translationZ;
            ScaleMatrix(&matrix, &scale);
            FieldFx_ModelSaveColors(g_SceneE20HoverOrbModel);
            FieldFx_ModelScaleColors(g_SceneE20HoverOrbModel, (u16) effect->alpha);
            FieldFx_ModelDraw(g_SceneE20HoverOrbModel, (GteMatrix *)&matrix);
            FieldFx_ModelRestoreColors(g_SceneE20HoverOrbModel);
            FieldFx_DrawHistoryTrail(&effect->trailHead, &effect->trailTail, 6, (unsigned char *)&beamColor, (unsigned char *)&edgeColor, (s32) effect->alpha, (struct FieldTrailPair *)effect->history, 1);
            g_FieldFxSpriteParams.depth = 0x33;
            var_s0 = effect->alpha;
            if (g_FieldFxCurrentAge & 1) {
                temp_v0_4 = var_s0 * 3;
                var_s0 = temp_v0_4 >> 2;
                if (temp_v0_4 < 0) {
                    var_s0 = (temp_v0_4 + 3) >> 2;
                }
            }
            paletteX = 0;
            paletteY = g_FieldFxPaletteEntry3;

            texturePage = g_FieldFxTpageTable[g_FieldFxTpageSlot11];

            g_FieldFxSpriteParams.palette = 3;
            g_FieldFxSpriteParams.parameter06 = 1;
            g_FieldFxSpriteParams.tpage = texturePage;
            FieldFx_DrawShapeQuads(&effect->position, NULL, 0x2000, 0x2000, 6, GetClut(paletteX, paletteY + 5) & 0xFFFF, 3, var_s0, (struct RenderColor *)&beamColor);
        }
        g_FieldFxSpriteParams.depth = 0xB;
        return 0;
    }
}

/* Authored USA M430 constants and complete task descriptor. */
RenderColor g_SceneE20FlareStreakColor = {128,128,128,0};
RenderColor g_SceneE20FlareGlowColor = {200,50,10,0};
RenderColor g_SceneE20OrbBeamColor = {70,200,100,0};
RenderColor g_SceneE20OrbEdgeColor = {0,50,100,0};
RenderColor g_SceneE20OrbFlashColorOdd = {200,50,0,0};
RenderColor g_SceneE20OrbFlashColorEven = {120,10,170,0};
u8 g_SceneE20FlareColorTrack[] = {0,0,0,0,0,0,0,0,200,240,240,8,0,0,0,0,200,0,0,8,0,0,0,0,0,0,0,0,0,0,0,0};
s32 g_SceneE20HoverOrbHitCooldown;
void *g_SceneE20HoverOrbModel;
/* USA M430 reserves 80190860..801908bf for four 24-byte commands. */
SceneEffectSlot g_SceneE20EffectSlots[4];
static u16 s_SceneE20HoverOrbScript[] = {5,0,0,1,6,103,4,2,0,2,0,0,0,0,0,0};
FieldFxTaskProgram g_SceneE20HoverOrbProgram = {
    {{SceneE20_HoverOrbController}, {sizeof(SceneE20TrailEffect)}},
    SceneE20_CommandEffectSlot, s_SceneE20HoverOrbScript
};
