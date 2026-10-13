# -G boundary analysis (cc1/maspsx small-data markers vs. module merges)

Measured on `main` @ 57511d717 (2026-10-12). The repo was only read. All
compiles ran on scratch copies through the repo's real `tools/scripts/cc.sh`,
with `PE_STOCK_CC1=scratchpad/linuxcc1/cc1-hybrid.sh` so float-literal units
use the Linux cc1. The reference for every lev is the unit's retail-matching
object in `build/USA/src/main/<unit>.c.o`. **lev** is the Levenshtein distance
over instruction words.

Relocations are resolved through `build/USA/main.map`. For example,
`g_GameState+0xE` and `D_800B0CE6` count as the same word, because they link to
the same bytes.

Scripts and raw data live in `scratchpad/reorg/gflags/`:

| file | what it does or holds |
|---|---|
| `bounds_g.py` → `gbounds.json` | the 43 boundaries |
| `gprobe.py` → `gprobe.json` | full neighbour-set swap |
| `gonly.py` → `gonly.json` | small-data-only swap |
| `gfinal.py` → `gfinal.json` | unified FORCE_G0 sets |
| `evidence.py` → `evidence.json` | adjacency evidence |
| `merge.py` + `trials.json` → `merge.json`, `merged/*.c` | merged-TU trials |
| `apply_clone.py`, `apply_mod.py` | real `make check` in the scratch clone `scratchpad/gclone` |

## Headline

1. **Most -G "conflicts" do not come from the original source.** Almost
   every boundary compiles to retail under one canonical small-data set,
   which the repo already documents for other units:

   ```
   /* CC1_FLAGS: -G8 */          (or -G0 for an all-default neighbourhood)
   /* MASPSX_FLAGS: --use-comm-section -G8 */
   /* MASPSX_FORCE_G0: 1 */
   ```

   Each word that retail reaches through `$gp` gets a tentative (COMMON)
   definition such as `TaskNode *g_TaskNodePool;`. MASPSX then emits `$gp`
   only for COMMON-known words, and every other extern stays absolute, as
   retail has it. A unit that has `-G8` without FORCE_G0, or the default
   `-G0`, differs from a neighbour only in which words are `$gp`. That
   difference is a property of declarations, not of the compiler flag.
2. **30 of the 38 compilable boundaries reach lev 0 on both sides** under
   one shared set (`gfinal.json`). They need no C change, only the COMMON
   lines.
3. **Three more reach lev 0 with one C-shape change each:** #14 (which
   also lets #13 use the same K8 set), #19 and #29.
4. **The remaining boundaries are not -G problems:**
   - #15, #37 and #40 are `-fno-strength-reduce` / `-fno-schedule-insns`
     boundaries. Their small-data part alone is already compatible.
   - #5–#7 are `-g3`.
   - #41 and #42 are GNU-as units.
   - Only #0 (`script_expr`) and #20 (`Gpu_InitPipeline`) are real -G
     shape problems. Both are the "address of a small word kept in a
     register" pattern and are open; hypotheses are below.
5. **Verified in a real build.**
   - All 10 top candidates were merged into single TUs in a scratch clone:
     24 files became 10, and yaml `c` entries were collapsed. `make check`
     gives `build/USA/main.exe: OK`.
   - Separately, 9 whole planned modules became one file each under one
     marker set, with all 9 merges applied together: 76 files became 9, and
     `make check` is OK. The modules are `script_system_ops` (11),
     `battle_setup` (14), `menu_item` (5), `menu_main` (4), `menu_list`
     (10), `item_use` (6), `save_data` (10), `scene_load` (5) and
     `menu_status` (11). This covers every non-G boundary inside them,
     including the `-g3` units `Inv_ItemActions` and `Save_MetadataWindow`.
     They match when the whole module takes `-g3`.
6. **Probe false negatives.** The REORG probe flagged #21, #24–#28, #30 and
   #31 as incompatible only because cc1 `-G8` emits an extra
   *unreferenced* `U <word>` symbol (`D_800C0E20`, `g_DrawBlendColor`). That
   symbol is link-neutral. `probe.py`'s fingerprint should ignore
   unreferenced `U` symbols and COMMON↔U changes, or it will keep reporting
   false blocks.
7. **Binary evidence for "one file".**
   - Every pair is address-contiguous, with no gap.
   - The *only* split evidence the evidence report records at any of the
     43 boundaries is `flags`, which is the marker difference itself. That
     evidence is circular, so once the markers unify, nothing argues for a
     split.
   - Positive join evidence exists at 21 of the 43 boundaries: private
     helpers or calls (e.g. #5, #7, #12, #17, #25, #26, #28, #29), shared
     small words (#8, #11, #13, #14 with 21 shared words, #15, #34), or
     order. Details are in the table.

## Mechanisms (the causes in column (c))

| key | mechanism | C-shape fix that works under the neighbour's set |
|---|---|---|
| **GP** | The `-G8` unit (no FORCE_G0) reaches its small words through `$gp`. Under `-G0`, or under FORCE_G0 without COMMON, they become `lui/lw` (lev = 1–2 per access). Conversely, a `-G0` unit compiled at `-G8` turns its absolute small words into `$gp`. | Use the FORCE_G0 set with tentative COMMON definitions of exactly the `$gp` words. Every other extern stays absolute whatever its size. This explains all lev-0 rows. |
| **REG** | Retail keeps a small word's *address* in a register for a read-modify-write or a reload (`lui a0; addiu a0; lw 0(a0) … sw 0(a0)`). cc1 does that only when the symbol is not small data, which means cc1 `-G0` or a declared size over 8 / unknown. Under cc1 `-G8` the word is a cheap constant and each access is folded to `lui/lw` + `lui/sw`. | Address the word through a non-small object: an incomplete-array declaration (`extern int g_RenderCounter[];`), or the containing record's field (`&g_GameState.story_day_flags`, `(char *)&D_800BEA40 + 6`). |
| **BIG** | The inverse case (`-G4096`): retail reads a word of a large record with direct `lui/lw`/`lui/sw`. Under `-G8` the record is expensive, so its address stays in a register. | Access the word through its scalar symbol (`D_800B0CD8`, already declared as `g_GameState.flags`). |
| **FLAG** | The difference is not -G: `--expand-div`, `-fno-strength-reduce`, `-fno-schedule-insns`, `-fno-expensive-optimizations`, `ASPSX_VERSION`, or `-g3`. | Take the union of the soft flags (`--expand-div`, `-fno-expensive-optimizations` and `-fno-cse-skip-blocks` were neutral wherever tried). Otherwise this is out of scope for -G. |

## Per-boundary data

The columns follow the brief:

- (a) units, flags and evidence.
- (b) lev when a unit is compiled with the neighbour's *full* marker set,
  then with only the small-data part swapped. The small-data part is cc1
  `-G`, as `-G`, `--use-comm-section` and FORCE_G0; the unit's own other
  flags are kept.
- The last column is the unified result: both units under one FORCE_G0 set
  with COMMON lines and the union of soft flags. K8 means cc1 `-G8`, K0 means
  cc1 `-G0`; each cell gives lev for A/B.
- Numbers are total lev, followed by the worst functions.

| # | module | A (flags) | B (flags) | evidence | A under B-set | B under A-set | small-data-only swap A / B | unified K8 ; K0 |
|---|---|---|---|---|---|---|---|---|
| 0 | script_expr.c | Task_ValueOps (as div) | Task_NodeActions (cc1 -G8, as div comm -G8, FG0) | 0x3050→0x39E8; 2 shared words; join private | 14 (Task_SetEntityVec3 14) | 254 (Camera_TrackEntityZoom 46, Camera_SnapToEntity 41, Task_SpawnChildNode 40, …) | 14 (Task_SetEntityVec3 14) / 254 (Camera_TrackEntityZoom 46, Camera_SnapToEntity 41, Task_SpawnChildNode 40, …) | 14/0 ; 0/32 |
| 1 | script_system_ops.c | Task_SystemCommandOps (default) | Task_DrawYield (cc1 -G8, as -G8) | 0x8D6C→0x97DC | 83 (Sys_ComputeAudioTimer 15, Entity_ComputeVelocity 14, Task_SetObjEntryFlag80 8, …) | 15 (Task_WaitFrameOrDraw 8, Entity_YieldOnDrawBusy 7) | 83 (Sys_ComputeAudioTimer 15, Entity_ComputeVelocity 14, Task_SetObjEntryFlag80 8, …) / 15 (Task_WaitFrameOrDraw 8, Entity_YieldOnDrawBusy 7) | 11/0 ; 0/0 |
| 2 | script_system_ops.c | Task_DrawYield (cc1 -G8, as -G8) | Entity_AttachmentAndControl (default) | 0x97DC→0x98AC | 15 (Task_WaitFrameOrDraw 8, Entity_YieldOnDrawBusy 7) | 54 (Entity_CopyAttachmentData 18, Task_SetAnimInterpRate 11, Task_SetEntityTarget 11, …) | 15 (Task_WaitFrameOrDraw 8, Entity_YieldOnDrawBusy 7) / 54 (Entity_CopyAttachmentData 18, Task_SetAnimInterpRate 11, Task_SetEntityTarget 11, …) | 0/0 ; 0/0 |
| 3 | script_system_ops.c | Task_RotateEntityAngle (default) | Task_SetNodeFlag80 (cc1 -G8, as -G8) | 0xA378→0xA3E4 | 6 (Task_RotateEntityAngle 6) | 2 (Task_SetNodeFlag80 2) | 6 (Task_RotateEntityAngle 6) / 2 (Task_SetNodeFlag80 2) | 0/0 ; 0/0 |
| 4 | script_system_ops.c | Task_SetNodeFlag80 (cc1 -G8, as -G8) | Task_SceneCommandOps (default) | 0xA3E4→0xA404 | 2 (Task_SetNodeFlag80 2) | 65 (Task_SetEntityParentLink 16, Task_ClearEntityParentLink 10, Entity_ClearParentLink 7, …) | 2 (Task_SetNodeFlag80 2) / 65 (Task_SetEntityParentLink 16, Task_ClearEntityParentLink 10, Entity_ClearParentLink 7, …) | 0/0 ; 0/0 |
| 5 | battle_actor.c | Menu_MainUpdate (cc1 -G1 -g3, as div comm -G1, FG0) | Entity_HitAndDropTable (cc1 -G8, as div comm -G8, FG0) | 0xDB40→0xFCD4; calls Entity_ApplyHitAndSetAnim,Entity_ResolveDropTable; join private+helper+call | 98 (Menu_MainUpdate 98) | 34 (Entity_ResolveDropTable 21, Entity_ApplyHitAndSetAnim 13) | n/a / 13 (Entity_ApplyHitAndSetAnim 13) | -g3 ; -g3 |
| 6 | battle_actor.c | Entity_HitAndDropTable (cc1 -G8, as div comm -G8, FG0) | Entity_UpdateTimers (cc1 -G1 -g3, as div -G1) | 0xFCD4→0x101C4; join private+helper+order | 34 (Entity_ResolveDropTable 21, Entity_ApplyHitAndSetAnim 13) | 64 (Entity_UpdateTimers 64) | 13 (Entity_ApplyHitAndSetAnim 13) / n/a | -g3 ; -g3 |
| 7 | battle_actor.c | Entity_UpdateTimers (cc1 -G1 -g3, as div -G1) | Battle_SubActionStep (default) | 0x101C4→0x109DC; calls Battle_SubActionStep; join private+helper+call+order | 70 (Entity_UpdateTimers 70) | 1 (Battle_SubActionStep 1) | n/a / 0 | -g3 ; -g3 |
| 8 | battle_setup.c | Battle_HaltOnPositiveX (cc1 -G1, as -G1) | Battle_InitEntityColors (cc1 -G8, as comm -G8, FG0) | 0x11550→0x115D0; 1 shared words; join private | 3 (Battle_HaltOnPositiveX 3) | 3 (Battle_InitEntityColors 3) | 3 (Battle_HaltOnPositiveX 3) / 3 (Battle_InitEntityColors 3) | 0/0 ; 0/0 |
| 9 | battle_setup.c | Battle_DispatchEntityEffect (cc1 -G8, as comm -G8, FG0) | Battle_CheckEvasion (as div) | 0x11928→0x11A78; 2 shared words | 15 (Battle_DispatchEntityEffect 15) | 42 (Battle_CheckEvasion 42) | 15 (Battle_DispatchEntityEffect 15) / 0 | 0/0 ; 0/0 |
| 10 | battle_setup.c | Battle_CheckEvasion (as div) | Battle_BuildTargetList (cc1 -G8, as comm -G8, FG0) | 0x11A78→0x11D6C; 1 shared words | 42 (Battle_CheckEvasion 42) | 22 (Battle_BuildTargetList 22) | 0 / 22 (Battle_BuildTargetList 22) | 0/0 ; 0/0 |
| 11 | battle_setup.c | Battle_FlushScriptSounds (cc1 -G8, as comm -G8, FG0) | Battle_StepScriptEntry (cc1 -G2, as -G2) | 0x1254C→0x125E0; 3 shared words | 5 (Battle_FlushScriptSounds 5) | 28 (Battle_StepScriptEntry 28) | 5 (Battle_FlushScriptSounds 5) / 28 (Battle_StepScriptEntry 28) | 0/0 ; 0/0 |
| 12 | battle_turn.c | Battle_ActionSlotAndEnemy (cc1 -G8, as comm -G8, FG0) | Battle_StepEnemyMovement (cc1 -G0, as div -G0) | 0x18208→0x18D74; calls Battle_StepEnemyMovement; 1 shared words; join private+helper+call | 30 (Battle_ProcessActionSlot 30) | 70 (Battle_StepEnemyMovement 70) | 30 (Battle_ProcessActionSlot 30) / 0 | 0/0 ; 0/0 |
| 13 | battle_main.c | Save_InitSystem (cc1 -G8, as -G8) | Battle_EncounterInitializationFlow (cc1 -G0, as comm -G8, FG0) | 0x19B88→0x19BF4; 2 shared words; join private | 4 (Save_InitSystem 4) | 72 (Battle_StartEncounter 36, Battle_ResetEnemyStats 19, Battle_SetupPlayerPalette 17) | 4 (Save_InitSystem 4) / 72 (Battle_StartEncounter 36, Battle_ResetEnemyStats 19, Battle_SetupPlayerPalette 17) | 0/8 ; 0/0 |
| 14 | battle_main.c | Battle_EncounterInitializationFlow (cc1 -G0, as comm -G8, FG0) | Battle_Update (cc1 -G8, as comm -G8, FG0) | 0x19BF4→0x1A1CC; 21 shared words; join private+helper | 8 (Battle_SetupPlayerPalette 8) | 6 (Battle_Update 6) | 8 (Battle_SetupPlayerPalette 8) / 6 (Battle_Update 6) | 8/0 ; 0/6 |
| 15 | entity.c | Entity_PoolInit (cc1 -fno-strength-reduce, as comm -G8) | Scene_LoadMap (cc1 -G8, as comm -G8) | 0x25710→0x25838; 6 shared words; join private | 3 (Entity_ResetAllPools 3) | 56 (Scene_LoadMap 56) | 0 / 44 (Scene_LoadMap 44) | 0/31 ; 0/31 |
| 16 | object.c | Render_VertexTransforms (cc1 -G8, as -G8) | Render_EntityAndTexturedQuadFlow (default) | 0x2A888→0x2B714; calls Render_TransformVertices; join private | 3 (Render_TransformSkinnedVertices 3) | 44 (Render_DrawEntity 22, Render_DrawTexturedQuads 22) | 3 (Render_TransformSkinnedVertices 3) / 44 (Render_DrawEntity 22, Render_DrawTexturedQuads 22) | 0/0 ; 0/0 |
| 17 | object.c | Render_EntityAndTexturedQuadFlow (default) | Render_DrawObjectAndClut (cc1 -G8, as -G8) | 0x2B714→0x2C17C; calls Render_DrawObject,Render_UpdateClutTable; join private+helper+call | 44 (Render_DrawEntity 22, Render_DrawTexturedQuads 22) | 14 (Render_DrawObject 14) | 44 (Render_DrawEntity 22, Render_DrawTexturedQuads 22) / 14 (Render_DrawObject 14) | 0/0 ; 0/0 |
| 18 | object.c | Render_DrawObjectAndClut (cc1 -G8, as -G8) | Render_DrawObjectAlt (default) | 0x2C17C→0x2C8B4; calls Render_UpdateClutTable; 1 shared words; join private+helper+call | 14 (Render_DrawObject 14) | 4 (Render_DrawObjectAlt 4) | 14 (Render_DrawObject 14) / 4 (Render_DrawObjectAlt 4) | 0/0 ; 0/0 |
| 19 | object.c | Render_DrawObjectVariant (cc1 -G8, as div) | Render_ObjectSetupAndAnim (default) | 0x2CAE0→0x2D6F8; calls Render_SetObjectTexturePage,Anim_SetInterpRate; 1 shared words; join helper | 67 (Render_ColorEntity 50, Anim_SetInterpRate 9, Render_TickObject 8) | 6 (Render_IncrementCounter 6) | 40 (Render_ColorEntity 32, Render_TickObject 8) / 6 (Render_IncrementCounter 6) | 0/6 ; 40/0 |
| 20 | boot.c | Gpu_InitPipeline (cc1 -G0, as comm -G8, FG0) | Boot_FrameLoop (cc1 -G8, as comm -G8, FG0) | 0x2F874→0x2FAFC; calls Gpu_InitPipeline; 2 shared words; join private+helper | 12 (Gpu_InitPipeline 12) | 13 (Boot_RunFrame 13) | 12 (Gpu_InitPipeline 12) / 13 (Boot_RunFrame 13) | 12/0 ; 0/13 |
| 21 | menu_status.c | Inv_FetchSelectedInventoryItem (default) | Menu_HandleInput (cc1 -G8, as -G8) | 0x3384C→0x338A0 | 0 | 27 (Menu_HandleInput 27) | 0 / 27 (Menu_HandleInput 27) | 0/0 ; 0/0 |
| 22 | menu_equip.c | Menu_ItemLabel (default) | Menu_ItemDetailPanel (cc1 -G8 -fno-expensive-optimizations, as -G8) | 0x355CC→0x35698; calls Menu_DrawItemLabel; 2 shared words; join private+helper | 3 (Menu_DrawItemLabel 3) | 66 (Menu_StepItemDetailPanel 44, Menu_ConfirmDialogHandler 12, Menu_OnItemDiscardConfirm 10) | 3 (Menu_DrawItemLabel 3) / 37 (Menu_StepItemDetailPanel 15, Menu_ConfirmDialogHandler 12, Menu_OnItemDiscardConfirm 10) | 0/0 ; 0/0 |
| 23 | menu_skill.c | Evt_DeferredExec (as comm -G8) | Menu_StepSkillList (cc1 -G8, as -G8) | 0x36B34→0x36B78 | 6 (Evt_DeferredExec 6) | 27 (Menu_StepSkillList 27) | 6 (Evt_DeferredExec 6) / 27 (Menu_StepSkillList 27) | 0/0 ; 0/0 |
| 24 | menu_skill.c | Menu_OnEquipConfirm (cc1 -G8, as -G8) | Menu_CreateSkillActionScreen (default) | 0x36D74→0x372BC | 53 (Menu_StepEquipConfirm 35, Menu_OnEquipConfirm 18) | 0 | 53 (Menu_StepEquipConfirm 35, Menu_OnEquipConfirm 18) / 0 | 0/0 ; 0/0 |
| 25 | menu_skill.c | Menu_CreateSkillActionScreen (default) | Menu_SkillActionHandler (cc1 -G8, as comm -G8) | 0x372BC→0x37358; calls Menu_SkillActionHandler; join helper+call | 0 | 18 (Menu_StepItemDetailPanel2 15, Menu_OnBattleCommandConfirm 3) | 0 / 18 (Menu_StepItemDetailPanel2 15, Menu_OnBattleCommandConfirm 3) | 0/0 ; 0/0 |
| 26 | menu_item.c | Menu_StepSaveLoadScreen (cc1 -G8, as -G8) | Menu_SaveLoadNavigate (default) | 0x38E54→0x39038; calls Menu_SaveLoadNavigate; join helper+call | 9 (Menu_StepSaveLoadScreen 9) | 0 | 9 (Menu_StepSaveLoadScreen 9) / 0 | 0/0 ; 0/0 |
| 27 | menu_item.c | Menu_SaveLoadNavigate (default) | Menu_InventoryAndBonusPoints (cc1 -G8, as comm -G8) | 0x39038→0x39118 | 0 | 500 (Menu_ItemListInputHandler 157, Menu_StepInventoryRoot 87, Spend_BonusPoints 83, …) | 0 / 500 (Menu_ItemListInputHandler 157, Menu_StepInventoryRoot 87, Spend_BonusPoints 83, …) | 0/0 ; 0/0 |
| 28 | menu_main.c | Menu_CreateMainMenuView (default) | Menu_VisualSettingsFlow (cc1 -G8, as comm -G8) | 0x3B59C→0x3B93C; calls Menu_OpenBlendColorScreen,Menu_OpenScreenAdjustView; join helper | 0 | 14 (Menu_StepColorSelect 5, Menu_ScreenAdjustInputHandler 5, Menu_OpenBlendColorScreen 2, …) | 0 / 14 (Menu_StepColorSelect 5, Menu_ScreenAdjustInputHandler 5, Menu_OpenBlendColorScreen 2, …) | 0/0 ; 0/0 |
| 29 | menu_save.c | Menu_OpenRenameScreen (cc1 -G8, as -G8) | Menu_NameEntryFlow (cc1 -G4096, as -G8) | 0x3E564→0x3E874; calls Menu_StepNameEntry,Menu_StepNameEntryAlt; 1 shared words; join private+helper | 12 (Menu_DrawSaveMetadataPreview 7, Menu_OpenRenameScreen 5) | 74 (Menu_StepNameEntryAlt 56, Menu_StepNameEntry 18) | 12 (Menu_DrawSaveMetadataPreview 7, Menu_OpenRenameScreen 5) / 74 (Menu_StepNameEntryAlt 56, Menu_StepNameEntry 18) | 0/74 ; 0/74 |
| 30 | menu_list.c | Menu_DrawSaveSlotList (as comm -G8) | Menu_ListCallbacks (cc1 -G8, as comm -G8) | 0x406EC→0x40730; 1 shared words | 0 | 4 (Menu_DrawItemIconList 2, Menu_DrawEquipItemDetailPanel 2) | 0 / 4 (Menu_DrawItemIconList 2, Menu_DrawEquipItemDetailPanel 2) | 0/0 ; 0/0 |
| 31 | menu_list.c | SaveWriteFlow (cc1 -G8, as comm -G8) | Menu_DrawListRows (as comm -G8) | 0x40CF4→0x40E18 | 6 (Menu_HandleMemCardWriteOrError 6) | 0 | 6 (Menu_HandleMemCardWriteOrError 6) / 0 | 0/0 ; 0/0 |
| 32 | item_use.c | Inv_EquipmentCapacityFlow (cc1 -G4, as comm -G4, FG0) | Inv_RollRandomItemType (cc1 -G8, as comm -G8) | 0x4564C→0x45BA4; 1 shared words | 8 (Inv_CheckFreeSlotCapacity 8) | 31 (Inv_RollRandomItemType 31) | 8 (Inv_CheckFreeSlotCapacity 8) / 31 (Inv_RollRandomItemType 31) | 0/0 ; 0/0 |
| 33 | save_data.c | Inv_FindItem (cc1 -G8, as comm -G8) | Save_BuildHeader (default) | 0x4C744→0x4CA5C | 63 (Inv_EquipItemById 40, Inventory_OpenAyaItemList 12, MemCard_SetDialogActive 9, …) | 2 (Save_BuildHeader 2) | 63 (Inv_EquipItemById 40, Inventory_OpenAyaItemList 12, MemCard_SetDialogActive 9, …) / 2 (Save_BuildHeader 2) | 0/0 ; 0/0 |
| 34 | save_data.c | Save_BuildHeader (default) | Save_RestoreHeader (cc1 -G8, as -G8) | 0x4CA5C→0x4CB74; 3 shared words; join private | 2 (Save_BuildHeader 2) | 3 (Save_RestoreHeader 3) | 2 (Save_BuildHeader 2) / 3 (Save_RestoreHeader 3) | 0/0 ; 0/0 |
| 35 | save_data.c | Save_GetCurrentMapNumber (default) | Menu_InitScrollDirection (cc1 -G8, as -G8) | 0x4E140→0x4E170 | 2 (Save_GetCurrentMapNumber 2) | 5 (Menu_InitScrollDirection 5) | 2 (Save_GetCurrentMapNumber 2) / 5 (Menu_InitScrollDirection 5) | 0/0 ; 0/0 |
| 36 | draw.c | Draw_BaseOffset (default) | Draw_TextPrimitives (cc1 -G8 -fno-cse-skip-blocks, as div comm -G8) | 0x4F050→0x4F094 | 4 (Draw_SetBaseOffsetPosition 2, Draw_GetBaseY 2) | 951 (Draw_AllocTexturedRectAlt 211, Draw_PrintTimeValue 81, Draw_AllocTexturedQuad 65, …) | 4 (Draw_SetBaseOffsetPosition 2, Draw_GetBaseY 2) / 907 (Draw_AllocTexturedRectAlt 211, Draw_PrintTimeValue 81, Draw_PrintCenteredTextInWidth 65, …) | 0/0 ; 0/0 |
| 37 | render.c | Render_MeshListAndFade (cc1 -fno-strength-reduce, as div) | Render_SortOt (cc1 -G8, as -G8) | 0x57A94→0x58F10; join helper | 407 (Render_DrawSpriteEntry 89, Geom_ScrollEntries 79, Geo_RenderMeshList 61, …) | 169 (Render_MoveGridCursor 169) | 263 (Geo_RenderMeshList 51, Render_StepFade 38, Render_DrawSpriteEntry 30, …) / 86 (Render_MoveGridCursor 86) | 97/135 ; 0/135 |
| 38 | render.c | Render_SortOt (cc1 -G8, as -G8) | Render_FrameUpdateAndScreenState (as div) | 0x58F10→0x59394; join helper | 86 (Render_MoveGridCursor 86) | 107 (Render_SetCDDCSlot 77, CdRom_InitScreenState 19, Render_PrepareFrame 11) | 86 (Render_MoveGridCursor 86) / 89 (Render_SetCDDCSlot 59, CdRom_InitScreenState 19, Render_PrepareFrame 11) | 0/0 ; 0/0 |
| 39 | scene_load.c | Scene_LoadRoom (cc1 -G1, as -G1) | Scene_LoadTextureAndPlayer (default) | 0x5BCF8→0x5C568; 1 shared words; join private | 63 (Scene_LoadRoom 63) | 143 (Scene_LoadEntityTextures 53, Scene_InitEntityPlayer 33, Scene_LoadEntityTexture 21, …) | 63 (Scene_LoadRoom 63) / 143 (Scene_LoadEntityTextures 53, Scene_InitEntityPlayer 33, Scene_LoadEntityTexture 21, …) | 0/0 ; 0/0 |
| 40 | prim_queue.c | Render_DrawWireframeModel (cc1 -G8, as -G8) | Task_InitGpuHwRegs (cc1 -fno-schedule-insns) | 0x60C1C→0x61510 | 214 (Render_DrawWireframeModel 214) | 4 (Task_InitGpuHwRegs 4) | 57 (Render_DrawWireframeModel 57) / 0 | 210/0 ; 213/0 |
| 41 | prim_queue.c | Task_GpuPackPrimColor (default) | Gpu_RenderFrame (cc1 -G8, as -G8) | 0x615D0→0x61654 | n/a | n/a | n/a / n/a | GNU-as ; GNU-as |
| 42 | prim_queue.c | Gpu_RenderFrame (cc1 -G8, as -G8) | Math_SqrtApprox3 (default) | 0x61654→0x617AC | n/a | n/a | n/a / n/a | GNU-as ; GNU-as |

## Cause and hypothesis per boundary (c)

The cause keys are defined in **Mechanisms** above. The **set** column names
the unified set that matches both units:

- **K8:** cc1 `-G8`, as `--use-comm-section -G8`, FORCE_G0.
- **K0:** the same, but cc1 `-G0`.

In both cases add the union of the soft flags and COMMON definitions of the
`$gp` words.

| # | cause | what differs | hypothesis | result |
|---|---|---|---|---|
| 0 | REG | `Task_SetEntityVec3` RMWs `g_RenderStateFlags` (0x800BCF88) through a register-held address. Under K0, `Task_SpawnChildNode` holds `&g_TaskNodeSeqCounter` in a register instead of using `$gp`. | The scroll state at 0x800BCF88 was a record in the original, so every RMW goes through a non-small object. `extern int g_RenderStateFlags[];` with `flags = g_RenderStateFlags` makes `Task_ValueOps` lev 0 under K8. However, the other users read it as a scalar, and `Render_MeshListAndFade` then costs lev 80. That needs a header-wide redesign of the 0x800BCF88 views (record declaration, `GeomScrollRecord` as the real type, volatile-scalar users moved to it). | open (best: K8 14 / NodeActions 0) |
| 1 | GP | `Task_DrawYield` reads `g_TaskNodePool` and `g_SceneDataTable0` via `$gp`. | K0 plus COMMON for both words. | **0/0** (verified) |
| 2 | GP | same unit | K0 or K8 | **0/0** (verified) |
| 3, 4 | GP | `Task_SetNodeFlag80`: `lw v1,%gp_rel(g_TaskNodePool)` becomes `lui/lw` (lev 2). | K0 plus `TaskNode *g_TaskNodePool;` | **0/0** (verified) |
| 5–7 | FLAG (-g3) | `Menu_MainUpdate` and `Entity_UpdateTimers` are `-g3 -G1`. `Entity_HitAndDropTable` under `-g3 -G1` scores lev 34; `Battle_SubActionStep` under `-g3` scores lev 1. | -g3 line notes, not -G. The small-data part matches: HitAndDropTable is 0 under K8 and SubActionStep is identical under `-G1`. Retry once the -g3 units are reshaped. | -g3 blocked |
| 8 | GP | `HaltOnPositiveX` (`-G1`) reads `D_8009CE30` through `$gp`. | K8; `D_8009CE30` is already COMMON in `InitEntityColors`. | **0/0** (verified) |
| 9, 10, 12 | GP + FLAG | Only `--expand-div` and the `$gp` words differ. | K8 + `--expand-div` (neutral) | **0/0** |
| 11 | GP | `StepScriptEntry` (`-G2`) reaches `D_8009CE3C`/`D_8009D1D4` through `$gp`. | K8; the words are COMMON in `FlushScriptSounds`. | **0/0** |
| 13, 14 | GP + REG | `Save_InitSystem` and `Battle_Update` are `-G8`, while `EncounterInit` is cc1 `-G0` only for `*flags \|= 2` on `D_800B0CE6` (u8 small word, register-held address, lev 8 under K8). | Use the record field: `flags = &g_GameState.story_day_flags;` | **0/0/0** under K8 (verified) |
| 15 | FLAG | Small-data part identical; `Scene_LoadMap` under `-fno-strength-reduce` scores lev 31. | not -G | – |
| 16–18 | GP | `VertexTransforms`/`DrawObjectAndClut` reach `D_8009CD9A`/`D_8009CDA0` through `$gp`. | K8 + COMMON. The 4-unit chain VertexTransforms…DrawObjectAlt was merged and linked: `make check` OK and **no GPREL overflow**. The CODE_ORGANIZATION.md note on #17 is stale for this recipe. | **0/0** (verified) |
| 19 | REG | `Render_IncrementCounter` (hand-written-asm-shaped, pinned `$a0/$a1`) keeps `&g_RenderCounter` in a register; under K8 it folds to `lui/lw` + `lui/sw` (lev 6). | `extern int g_RenderCounter[];` in `render_object.h` (only user) and `ptr = g_RenderCounter;`. The word sits in `.text`, so an unsized declaration is the natural original. | **0/0** (verified) |
| 20 | REG | `Gpu_InitPipeline` under K8 scores lev 12 on the light-matrix element stores (`&D_800BEA46`, `&D_800BEA4C`). Under K0, `Boot_RunFrame` scores lev 13. | Address the elements through the `D_800BEA40` record (`(char *)&D_800BEA40 + 6`). This gives lev 21, because CSE now shares the record base across the three blocks and the crutched function (LAUNDER, pins) needs reshaping. | open |
| 21 | GP | `Menu_HandleInput`'s 5 `$gp` words | K8 + COMMON | **0/0** (verified) |
| 22 | GP + FLAG | `Menu_ItemLabel` (default) vs `ItemDetailPanel` (`-G8 -fno-expensive-optimizations`) | K8 + `-fno-expensive-optimizations` (neutral for ItemLabel) + 6 COMMON | **0/0** (verified) |
| 23–28, 30–32 | GP | `$gp` word sets only; for #32, the `-G4` unit `Inv_CheckFreeSlotCapacity` also matches at `-G8` | K8 + COMMON | **0/0** |
| 29 | BIG | `Menu_NameEntryFlow` is cc1 `-G4096` only so that `g_GameState.flags &= …` stays direct `lui/lw`/`lui/sw`; under K8 the record address is kept in a register (lev 74). | `D_800B0CD8 &= …` (3 sites; already declared as `g_GameState.flags`) + COMMON `D_8009D004`, `g_MenuItemIconGlyphTable` | **0/0** (verified) |
| 33, 34 | GP | `Save_BuildHeader` (`-G0`) reads `g_PlayTimeFrameCounter` absolutely, while its neighbours' 7 words are `$gp`. | K8 + COMMON for the neighbours' words only | **0/0/0** (verified) |
| 35 | GP | `g_SceneDispatchToken` absolute vs `D_8009CDAC`/`g_MenuScrollStep` `$gp` | K8 + COMMON | **0/0** (verified) |
| 36 | GP + FLAG | `Draw_TextPrimitives` (`-G8 -fno-cse-skip-blocks`, 21 `$gp` words) | K8 + `-fno-cse-skip-blocks` + `--expand-div` | **0/0** |
| 37 | FLAG | `Render_SortOt` under `-fno-strength-reduce` scores lev 135 (`Render_MoveGridCursor`). | not -G | – |
| 38, 39 | GP | `Render_SortOt` grid-cursor words; `Scene_LoadRoom` (`-G1`) `D_8009CDC8` | K8 + COMMON | **0/0** |
| 40 | FLAG | `Task_InitGpuHwRegs` is identical under `-G8`; `-fno-schedule-insns` blocks `Render_DrawWireframeModel` (lev ~210). | not -G | – |
| 41, 42 | – | `Task_GpuPackPrimColor` and `Math_SqrtApprox3` are GNU-as units. | policy | – |

## Ranking (d)

The ranking is by the cheapest single-direction swap under the neighbour's
current *full* set. This is the input order the brief asked for. The
unified-set results above beat this ranking for most rows.

| rank | # | module | cheapest direction |
|---|---|---|---|
| 1 | 21 | menu/menu_status.c | item/Inv_FetchSelectedInventoryItem under neighbour set: lev 0 in 0 function(s) |
| 2 | 24 | menu/menu_skill.c | menu/Menu_CreateSkillActionScreen under neighbour set: lev 0 in 0 function(s) |
| 3 | 25 | menu/menu_skill.c | menu/Menu_CreateSkillActionScreen under neighbour set: lev 0 in 0 function(s) |
| 4 | 26 | menu/menu_item.c | menu/Menu_SaveLoadNavigate under neighbour set: lev 0 in 0 function(s) |
| 5 | 27 | menu/menu_item.c | menu/Menu_SaveLoadNavigate under neighbour set: lev 0 in 0 function(s) |
| 6 | 28 | menu/menu_main.c | menu/Menu_CreateMainMenuView under neighbour set: lev 0 in 0 function(s) |
| 7 | 30 | menu/menu_list.c | menu/Menu_DrawSaveSlotList under neighbour set: lev 0 in 0 function(s) |
| 8 | 31 | menu/menu_list.c | menu/Menu_DrawListRows under neighbour set: lev 0 in 0 function(s) |
| 9 | 7 | battle/battle_actor.c | battle/Battle_SubActionStep under neighbour set: lev 1 in 1 function(s) |
| 10 | 3 | script/script_system_ops.c | task/Task_SetNodeFlag80 under neighbour set: lev 2 in 1 function(s) |
| 11 | 4 | script/script_system_ops.c | task/Task_SetNodeFlag80 under neighbour set: lev 2 in 1 function(s) |
| 12 | 33 | save/save_data.c | save/Save_BuildHeader under neighbour set: lev 2 in 1 function(s) |
| 13 | 34 | save/save_data.c | save/Save_BuildHeader under neighbour set: lev 2 in 1 function(s) |
| 14 | 35 | save/save_data.c | save/Save_GetCurrentMapNumber under neighbour set: lev 2 in 1 function(s) |
| 15 | 8 | battle/battle_setup.c | battle/Battle_HaltOnPositiveX under neighbour set: lev 3 in 1 function(s) |
| 16 | 15 | entity/entity.c | entity/Entity_PoolInit under neighbour set: lev 3 in 1 function(s) |
| 17 | 16 | render/object.c | render/Render_VertexTransforms under neighbour set: lev 3 in 1 function(s) |
| 18 | 22 | menu/menu_equip.c | menu/Menu_ItemLabel under neighbour set: lev 3 in 1 function(s) |
| 19 | 13 | battle/battle_main.c | save/Save_InitSystem under neighbour set: lev 4 in 1 function(s) |
| 20 | 18 | render/object.c | render/Render_DrawObjectAlt under neighbour set: lev 4 in 1 function(s) |
| 21 | 40 | draw/prim_queue.c | task/Task_InitGpuHwRegs under neighbour set: lev 4 in 1 function(s) |
| 22 | 36 | draw/draw.c | gpu/Draw_BaseOffset under neighbour set: lev 4 in 2 function(s) |
| 23 | 11 | battle/battle_setup.c | battle/Battle_FlushScriptSounds under neighbour set: lev 5 in 1 function(s) |
| 24 | 14 | battle/battle_main.c | battle/Battle_Update under neighbour set: lev 6 in 1 function(s) |
| 25 | 19 | render/object.c | render/Render_ObjectSetupAndAnim under neighbour set: lev 6 in 1 function(s) |
| 26 | 23 | menu/menu_skill.c | event/Evt_DeferredExec under neighbour set: lev 6 in 1 function(s) |
| 27 | 32 | item/item_use.c | item/Inv_EquipmentCapacityFlow under neighbour set: lev 8 in 1 function(s) |
| 28 | 20 | boot/boot.c | boot/Gpu_InitPipeline under neighbour set: lev 12 in 1 function(s) |
| 29 | 29 | menu/menu_save.c | menu/Menu_OpenRenameScreen under neighbour set: lev 12 in 2 function(s) |
| 30 | 0 | script/script_expr.c | task/Task_ValueOps under neighbour set: lev 14 in 1 function(s) |
| 31 | 17 | render/object.c | render/Render_DrawObjectAndClut under neighbour set: lev 14 in 1 function(s) |
| 32 | 9 | battle/battle_setup.c | battle/Battle_DispatchEntityEffect under neighbour set: lev 15 in 1 function(s) |
| 33 | 1 | script/script_system_ops.c | task/Task_DrawYield under neighbour set: lev 15 in 2 function(s) |
| 34 | 2 | script/script_system_ops.c | task/Task_DrawYield under neighbour set: lev 15 in 2 function(s) |
| 35 | 10 | battle/battle_setup.c | main/Battle_BuildTargetList under neighbour set: lev 22 in 1 function(s) |
| 36 | 12 | battle/battle_turn.c | battle/Battle_ActionSlotAndEnemy under neighbour set: lev 30 in 1 function(s) |
| 37 | 5 | battle/battle_actor.c | entity/Entity_HitAndDropTable under neighbour set: lev 34 in 2 function(s) |
| 38 | 6 | battle/battle_actor.c | entity/Entity_HitAndDropTable under neighbour set: lev 34 in 2 function(s) |
| 39 | 39 | scene/scene_load.c | scene/Scene_LoadRoom under neighbour set: lev 63 in 1 function(s) |
| 40 | 38 | render/render.c | render/Render_SortOt under neighbour set: lev 86 in 1 function(s) |
| 41 | 37 | render/render.c | render/Render_SortOt under neighbour set: lev 169 in 1 function(s) |
| 42 | 41 | draw/prim_queue.c | GNU-as, not compilable |
| 43 | 42 | draw/prim_queue.c | GNU-as, not compilable |

## Top-10 trials: exact C changes, all lev 0, all verified by `make check`

**How the trials were run.** Each trial concatenated its units in manifest
order into the first unit's file, under one marker header, using
`merge.py`. The merged sources are in `scratchpad/reorg/gflags/merged/<trial>.c`.

- *Merged-TU lev:* every function was compared with its retail object. All
  10 trials score **lev 0**, with function order preserved.
- *Link test:* all 10 were applied together to the scratch clone
  (`scratchpad/gclone`), 24 files to 10. Each trial kept one `c` yaml entry and deleted the
  other files; there were no rodata subsegments to collapse. `make check`
  printed **`build/USA/main.exe: OK`**.

The changes per trial are listed below. Every unit's own marker lines are
dropped and replaced by the one header shown.

1. **#3+#4 script_system_ops: `Task_RotateEntityAngle` + `Task_SetNodeFlag80` + `Task_SceneCommandOps`**
   ```c
   /* CC1_FLAGS: -G0 */
   /* MASPSX_FLAGS: --use-comm-section -G8 */
   /* MASPSX_FORCE_G0: 1 */
   TaskNode *g_TaskNodePool;              /* COMMON: the one $gp word */
   ```
2. **#33+#34 save_data: `Inv_FindItem` + `Save_BuildHeader` + `Save_RestoreHeader`**
   ```c
   /* CC1_FLAGS: -G8 */  /* MASPSX_FLAGS: --use-comm-section -G8 */  /* MASPSX_FORCE_G0: 1 */
   s16 *g_InvItemPtr; s32 g_InvSelectionBitWords; u32 *g_InvSelectionBits; s32 g_InvSlotLimit;
   int g_MemCardDialogState; int g_MenuBattleEquipMode; int g_SaveMetadataWindowIndex;
   ```
   `g_PlayTimeFrameCounter` (absolute in `Save_BuildHeader`) gets no
   definition, so it stays absolute.
3. **#35 `Save_GetCurrentMapNumber` + `Menu_InitScrollDirection`**: K8 header plus `int D_8009CDAC; int g_MenuScrollStep;`
4. **#8 battle_setup: `Battle_HaltOnPositiveX` (`-G1`) + `Battle_InitEntityColors`**: K8 header only. `D_8009CE30` is already COMMON in `InitEntityColors`.
5. **#22 menu_equip: `Menu_ItemLabel` + `Menu_ItemDetailPanel`**
   ```c
   /* CC1_FLAGS: -G8 -fno-expensive-optimizations */  /* MASPSX_FLAGS: --use-comm-section -G8 */  /* MASPSX_FORCE_G0: 1 */
   int g_MenuActionTextBase; int g_MenuActiveItemList; int g_MenuActiveItemSlot;
   MenuConfirmCallback g_MenuConfirmCallback; int g_MenuEquipMode; int g_MenuItemPrimListHandles[2];
   ```
6. **#21 menu_status: `Inv_FetchSelectedInventoryItem` + `Menu_HandleInput`**: K8 header plus
   `MenuWidgetNode *g_MenuActiveWidget; int g_MenuItemDiscardMode; int g_MenuItemRenameMode; int g_MenuItemUseMode; int g_MenuLayoutLocked;`
7. **#13+#14 battle_main: `Save_InitSystem` + `Battle_EncounterInitializationFlow` + `Battle_Update`**: K8 header. `EncounterInit` already has its COMMON block. The one C change is in `Battle_SetupPlayerPalette`:
   ```diff
   -    flags = &D_800B0CE6;
   +    flags = &g_GameState.story_day_flags;
        *flags |= 2;
   ```
   The record is not small data, so cc1 `-G8` keeps its address in a
   register, as retail does. This retires the reason `EncounterInit` was
   cc1 `-G0`; its header comment needs updating.
8. **#29 menu_save: `Menu_OpenRenameScreen` + `Menu_NameEntryFlow`**: K8 header plus `ItemDataRecord *g_MenuRenameTargetRecord; int D_8009D004; u8 *g_MenuItemIconGlyphTable;`, and three statements in `Menu_NameEntryFlow`:
   ```diff
   -                g_GameState.flags &= ~0x8000;
   +                D_800B0CD8 &= ~0x8000;
   -                g_GameState.flags &= 0xFFFF7FFF;   /* x2 */
   +                D_800B0CD8 &= 0xFFFF7FFF;
   ```
   This retires the `-G4096` marker.
9. **#19 render/object: `Render_DrawObjectVariant` + `Render_ObjectSetupAndAnim`**
   ```c
   /* CC1_FLAGS: -G8 */  /* MASPSX_FLAGS: --expand-div --use-comm-section -G8 */  /* MASPSX_FORCE_G0: 1 */
   ```
   ```diff
   --- include/pe1/render_object.h
   -extern int g_RenderCounter;
   +extern int g_RenderCounter[];
   --- Render_IncrementCounter
   -    ptr = &g_RenderCounter;
   +    ptr = g_RenderCounter;
   ```
   The `$a0/$a1` pins are still required: without them it is lev 6 in
   `v0/v1`, which confirms the hand-asm shape.
10. **#1+#2 script_system_ops: `Task_SystemCommandOps` + `Task_DrawYield` + `Entity_AttachmentAndControl`**: K0 header (cc1 `-G0`) plus `int g_SceneDataTable0; TaskNode *g_TaskNodePool;`

### Whole-module merges verified in the same clone

Each module was merged into one file under one set. In total 80 files
became 10, and `make check` is OK.

| module | files | set |
|---|---|---|
| `script/script_system_ops` | 11 | K0 |
| `battle/battle_setup` | 14 | K8 + `--expand-div` + `-G1`/`-G2` units folded |
| `menu/menu_item` | 5 | K8 |
| `menu/menu_main` | 4 | K8 |
| `menu/menu_list` | 10 | K8 |
| `item/item_use` | 6 | K8 + `-g3` for the whole TU |
| `save/save_data` | 10 | K8 + `-g3` |
| `scene/scene_load` | 5 | K8 |
| `menu/menu_status` | 11 | K8; its two `.rodata` entries collapsed into one |
| render chain `VertexTransforms…DrawObjectAlt` | 4 | K8 |

Run `python3 merge.py <trial>` to see the exact flag union per module
(`trials.json`, keys `mod_*`).

Near misses among the merged-TU trials:

| module | lev | cause |
|---|---|---|
| `menu_status` | 1 | only the jump-table `.rodata` offset inside the merged object; the link is OK |
| `battle_turn` | 3 | same jump-table cause, but the foreign blob `rodata_001080` must be claimed first |
| `menu_skill` | 18 | `Menu_DrawWeaponModSlots` under the `aspsx 2.21` / `--dont-expand-li` union, not -G |
| `menu_save` | 73 | `Menu_DrawContextHelpText`, same aspsx union, plus #29 |
| `menu_equip` | 165 | non-G flags |
| `draw/draw`, `render/render`, `entity` | – | `-fno-strength-reduce` / `-fno-cse-skip-blocks` units |

Four modules did not compile as one TU because unit-local prototypes
conflict: `battle_actor` (`Entity_ResolveDropTable`), `battle_main`
(`Battle_SlotFree`), `render/object` (`Render_DrawEntity`) and `boot`
(`Gte_SetBackColor` / `Gte_SetLightColor`). This is header hygiene, not
codegen.

## Recommendations

1. Adopt the canonical set as the reorg's marker unification rule (Step 6
   of REORG_PLAN): FORCE_G0 + `--use-comm-section -G8`, cc1 `-G8`, or
   cc1 `-G0` in default-heavy neighbourhoods. COMMON definitions replace
   per-file `-G` values. This is the documented `CODE_ORGANIZATION.md`
   recipe applied module-wide.
2. Fix `probe.py`'s fingerprint so that unreferenced `U` symbols and U↔C
   changes do not count. 8 boundaries were false blocks.
3. Open -G items:
   - #0 needs one declaration for the 0x800BCF88 scroll-state record (all
     5 users).
   - #20 (`Gpu_InitPipeline`) needs a de-crutch pass. Both are REG-type.
4. Not -G, and out of this scope: #15 and #37 (`-fno-strength-reduce`),
   #40 (`-fno-schedule-insns`), #5–#7 (`-g3`) and #41–#42 (GNU-as).
