import pathlib
import re
import tempfile
import unittest

from tools.scripts import source_quality
from tools.tests.csource import find_definition, function_text


class SourceQualityTests(unittest.TestCase):
    def classify(self, text):
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / "unit.c"
            path.write_text(text)
            return source_quality.classify(path)

    def assert_symbol_is_semantic(self, name):
        # Locate the defining unit by symbol: units are merged and moved.
        self.assertEqual(source_quality.classify(find_definition(name)), "semantic_c")

    def test_plain_c_is_semantic(self):
        self.assertEqual(self.classify("int f(void) { return 1; }"), "semantic_c")

    def test_repaired_engine_load_and_distance_sources_are_semantic(self):
        for name in ("PointSprite_UpdateRisingStreak", "FieldFx_IsActorWithinRadius", "FieldFx_CirclesOverlapXZ",
                     "FieldFx_IsPlayerWithinRadius", "FieldFx_IsPlayerInQuadXZ", "FieldFx_FreeAllSlots",
                     "FieldFx_SetSpriteCellSize", "FieldFx_PlaySoundAtOwner", "PointSprite_DrawOrientedFlash",
                     "FieldFx_UpdateStripCollision", "PointSprite_DrawGroundRing", "PointSprite_DrawBillboardFlash",
                     "BurstEffectC_DrawTimedSpark", "BurstEffectB_DrawSparkPoint", "BurstEffectB_DrawSparkCloud",
                     "GlowEffect_DrawParticle", "GlowEffect_DrawGroundGlow", "GlowEffect_DrawHaloGlow",
                     "GlowEffectB_DrawParticle", "GlowEffectC_DrawParticle", "FourLayerGlow_DrawParticle",
                     "FourLayerGlow_DrawHaloGlow"):
            with self.subTest(name=name):
                self.assert_symbol_is_semantic(name)

    def test_repaired_render_load_and_tint_sources_are_semantic(self):
        for name in ("Pm_StopAll", "Render_ApplyScreenTint", "Render_StepFade"):
            with self.subTest(name=name):
                self.assert_symbol_is_semantic(name)

    def test_font_step_modulo_is_semantic_c(self):
        root = pathlib.Path(__file__).resolve().parents[2]
        self.assertEqual(source_quality.classify(
            root / "src/main/render/font.c"), "semantic_c")

    def test_wayne_item_table_store_is_semantic_c(self):
        self.assert_symbol_is_semantic("Inv_LoadWayneItemsAsOverride")

    def test_object_texture_page_frame_is_semantic_c(self):
        self.assert_symbol_is_semantic("Render_SetObjectTexturePage")

    def test_equip_panel_alignment_is_semantic_c(self):
        root = pathlib.Path(__file__).resolve().parents[2]
        self.assertEqual(source_quality.classify(
            root / "src/main/menu/item_actions.c"), "semantic_c")

    def test_selected_equip_lookup_is_semantic_c(self):
        root = pathlib.Path(__file__).resolve().parents[2]
        self.assertEqual(source_quality.classify(
            root / "src/main/render/font.c"), "semantic_c")

    def test_max_level_inventory_is_semantic_c(self):
        root = pathlib.Path(__file__).resolve().parents[2]
        self.assertEqual(source_quality.classify(
            root / "src/main/save/game_data.c"), "semantic_c")

    def test_memcard_dialog_workflow_is_semantic_c(self):
        root = pathlib.Path(__file__).resolve().parents[2]
        self.assertEqual(source_quality.classify(
            root / "src/main/menu/save_screens.c"), "semantic_c")

    def test_two_line_dialog_width_picker_is_semantic_c(self):
        root = pathlib.Path(__file__).resolve().parents[2]
        self.assertEqual(source_quality.classify(
            root / "src/main/menu/save_screens.c"), "semantic_c")

    def test_akao_pitch_lfo_depth_is_semantic_c(self):
        self.assert_symbol_is_semantic("SeqOp_SetPitchLFO")

    def test_akao_pitch_slide_voice_search_is_semantic_c(self):
        self.assert_symbol_is_semantic("SeqOp_NoteOnWithPitchSlide")

    def test_audited_cpu_helpers_are_not_gte_exemptions(self):
        for name in ("gte_ldv0_short3", "gte_load_packed_short3",
                     "gte_store_ir123_packed_short3", "gte_store_third_output",
                     "gte_store_flag_bound", "gte_store_mac12_byte2",
                     "gte_store_mac123_byte3", "gte_stir123_matrix_column",
                     "gte_stsz3_s16"):
            with self.subTest(name=name):
                self.assertEqual(self.classify("void f(void) { %s(0); }" % name),
                                 "asm_constrained")

    def test_critical_section_syscalls_keep_bios_classification(self):
        root = pathlib.Path(__file__).resolve().parents[2]
        for name in ("a36.c", "a37.c"):
            self.assertEqual(source_quality.classify(
                root / "src/main/psyq/libapi" / name), "original_asm")
        self.assertEqual(self.classify(
            "int f(void) { PSYQ_BIOS_SYSCALL(result, selector); return result; }"),
            "original_asm")

    def test_cpu_helper_in_direct_template_is_quarantined(self):
        with tempfile.TemporaryDirectory() as directory:
            root = pathlib.Path(directory)
            (root / "body.inc").write_text("void f(void) { gte_ldv0_short3(0); }")
            source = root / "unit.c"
            source.write_text('#include "body.inc"')
            self.assertEqual(source_quality.classify(source), "asm_constrained")

    def test_room_header_cpu_helpers_are_quarantined(self):
        for name in ("ROOMLIB_LOAD_S16", "ROOMLIB_LOAD_PTR", "ROOMLIB_LOAD_U16",
                     "ROOMLIB_DIV_V0_A0_CHECKED", "ROOM_M089_LOAD_EFFECT_LOOKUP"):
            with self.subTest(name=name):
                self.assertEqual(self.classify("void f(void) { %s(a, b); }" % name),
                                 "asm_constrained")

    def test_room_helper_mentions_are_not_calls(self):
        text = '''
        /* ROOMLIB_LOAD_S16(a, b); */
        const char *s = "ROOMLIB_DIV_V0_A0_CHECKED(a, b)";
        int f(void) { return 1; }
        '''
        self.assertEqual(self.classify(text), "semantic_c")

    def test_retired_room_cpu_helpers_are_not_defined(self):
        root = pathlib.Path(__file__).resolve().parents[2]
        for header, names in (
            ("room_lib/room_lib.h", ("ROOMLIB_LOAD_S16", "ROOMLIB_LOAD_PTR",
                                    "ROOMLIB_LOAD_U16", "ROOMLIB_DIV_V0_A0_CHECKED")),
            ("room_m089_central_park/room_m089.h", ("ROOM_M089_LOAD_EFFECT_LOOKUP",)),
        ):
            text = (root / "src/overlays" / header).read_text()
            for name in names:
                with self.subTest(name=name):
                    self.assertNotIn(name, text)

    def test_arc_divisions_use_c_and_stock_expansion(self):
        # The arc steppers divide in C and rely on maspsx's stock div expansion
        # rather than the retired ROOMLIB_DIV_V0_A0_CHECKED CPU helper.
        for name in ("RoomLib_AdvanceArcToTarget", "RoomLib_AdvanceArcToTargetY"):
            with self.subTest(name=name):
                text = function_text(name)
                self.assertEqual(len(re.findall(r"\) / count;", text)), 2)
                self.assertNotIn("ROOMLIB_DIV_V0_A0_CHECKED", text)
                self.assertNotIn("--no-expand-div",
                                 find_definition(name).read_text())

    def test_cpu_helper_mentions_are_not_calls(self):
        text = '''
        /* gte_ldv0_short3(0); */
        const char *s = "gte_store_third_output(0)";
        int f(void) { return 1; }
        '''
        self.assertEqual(self.classify(text), "semantic_c")

    def test_pins_aliases_and_empty_barriers_remain_semantic(self):
        text = '''
        extern int value asm("D_80010000");
        int f(void) {
            register int result asm("$v0") = value;
            asm("");
            return result;
        }
        '''
        self.assertEqual(self.classify(text), "semantic_c")

    def test_global_scalar_link_alias_is_semantic(self):
        text = '''
        signed char target_mode asm("g_BattleTargetCycleMode");
        int f(void) { return target_mode; }
        '''
        self.assertEqual(self.classify(text), "semantic_c")

    def test_instruction_asm_is_constrained(self):
        self.assertEqual(self.classify('void f(void) { asm("nop"); }'),
                         "asm_constrained")

    def test_all_template_literals_are_checked(self):
        for template in ('"" "nop"', '"" /* comment */ "nop"',
                         '"" \\\n "nop"', '"label: nop"', '"sync"'):
            with self.subTest(template=template):
                self.assertEqual(self.classify('void f(void) { asm(%s); }' % template),
                                 "asm_constrained")

    def test_comment_markers_in_strings_do_not_hide_instructions(self):
        text = 'const char *url = "https://example.invalid"; void f(void) { asm("nop"); }'
        self.assertEqual(self.classify(text), "asm_constrained")

    def test_gte_mnemonic_in_operand_does_not_exempt_template(self):
        text = 'void f(int mtc2) { asm("nop" : : "r"(mtc2)); }'
        self.assertEqual(self.classify(text), "asm_constrained")

    def test_gte_address_arithmetic_is_not_automatically_exempt(self):
        text = r'void f(void *p) { asm("addiu $2,%0,32\n\t" "lwc2 $0,0($2)" : : "r"(p)); }'
        self.assertEqual(self.classify(text), "asm_constrained")

    def test_gte_transfer_only_concatenation_remains_supported(self):
        text = r'void f(void) { asm("mfc2 $2,$9\n\t" "nop"); }'
        self.assertEqual(self.classify(text), "semantic_c")

    def test_gte_transfer_does_not_hide_cpu_work(self):
        for suffix in (r'\naddu $2,$3,$4', r'; sh $2,0($4)',
                       r'\nsll $2,$2,16', r'\n.word 0',
                       r'\nlabel: nop', r'\nnop # mtc2'):
            with self.subTest(suffix=suffix):
                text = 'void f(void) { asm("mtc2 $2,$0%s"); }' % suffix
                self.assertEqual(self.classify(text), "asm_constrained")

    def test_extern_declaration_does_not_hide_file_scope_asm(self):
        text = 'extern int data __attribute__((unused)); asm(".word 0");'
        self.assertEqual(self.classify(text), "asm_constrained")

    def test_alternate_asm_keyword(self):
        self.assertEqual(self.classify('void f(void) { __asm("nop"); }'),
                         "asm_constrained")

    def test_anonymous_struct_and_function_aliases(self):
        text = '''
        extern struct { char padding[16]; } alias asm("real_symbol");
        int function(int x) asm("real_function");
        unsigned char array[3] asm("real_array");
        extern int scratch asm("0x1F800008");
        '''
        self.assertEqual(self.classify(text), "semantic_c")

    def test_macro_argument_does_not_hide_following_gte_literal(self):
        text = r'void f(void) { asm("addiu $2,$4," STR(OFFSET) "\n\tlwc2 $0,0($2)"); }'
        self.assertEqual(self.classify(text), "asm_constrained")

    def test_opaque_template_is_not_assumed_empty(self):
        self.assertEqual(self.classify('void f(void) { asm(IMPLEMENTATION); }'),
                         "asm_constrained")

    def test_extern_before_macro_does_not_hide_instructions(self):
        text = r'''
        extern char *state;
        #define LOAD(dst) \
            asm volatile("lui\t%0, %%hi(state)\n" \
                         "lw\t%0, %%lo(state)(%0)" : "=r"(dst))
        int f(void) { char *p; LOAD(p); return p != 0; }
        '''
        self.assertEqual(self.classify(text), "asm_constrained")

    def test_bios_trampoline_is_original_asm(self):
        self.assertEqual(self.classify("PSYQ_BIOS_TRAMPOLINE(open, 0x32, 0);"),
                         "original_asm")

    def test_psyq_assembler_object_is_original_asm(self):
        self.assertEqual(self.classify(
            '#include "pe1/psyq_asm.h"\n'
            'PSYQ_ASM_OBJECT(LIBGTE, MSC00)\n'
            'PSYQ_ASM_FUNCTION(InitGeom,\n'
            '    "    jr      $ra\\n"\n'
            '    "    nop\\n");\n'), "original_asm")

    def test_game_assembler_routine_is_original_asm(self):
        self.assertEqual(self.classify(
            '/* ASSEMBLER: GNU */\n'
            '#include "pe1/game_asm.h"\n'
            'GAME_ASM_FUNCTION(Math_FixedRoundToInt,\n'
            '    "    ori     $at, $zero, 0x8000\\n"\n'
            '    "    add     $v0, $a0, $at\\n"\n'
            '    "    jr      $ra\\n"\n'
            '    "    sra     $v0, $v0, 16\\n");\n'), "original_asm")

    def test_game_assembler_routine_cannot_hide_c_functions(self):
        self.assertEqual(self.classify(
            'GAME_ASM_FUNCTION(f, "    jr $ra\\n" "    nop\\n");\n'
            'int g(void) { return 1; }\n'), "asm_constrained")

    def test_game_assembler_name_in_a_comment_is_not_a_use(self):
        self.assertEqual(self.classify(
            '/* formerly GAME_ASM_FUNCTION(f, ...) */\n'
            'int f(void) { return 1; }\n'), "semantic_c")

    STACK_SWITCH = (
        '#include "pe1/boot_stack.h"\n'
        'void main(void) {\n'
        '    BOOT_CALL_ON_SCRATCHPAD_STACK(BOOT_SCRATCHPAD_STACK_TOP, f());\n'
        '}\n')

    def classify_unit(self, name, text, allowed=None):
        with tempfile.TemporaryDirectory() as directory:
            path = pathlib.Path(directory) / "src" / name
            path.parent.mkdir(parents=True)
            path.write_text(text)
            return source_quality.classify(path, allowed)

    def test_listed_stack_switch_is_semantic_c(self):
        self.assertEqual(self.classify_unit(
            "main/boot/main.c", self.STACK_SWITCH,
            {"main/boot/main.c"}), "semantic_c")

    def test_unlisted_stack_switch_is_asm_constrained(self):
        self.assertEqual(self.classify_unit(
            "main/boot/Other.c", self.STACK_SWITCH,
            {"main/boot/main.c"}), "asm_constrained")

    def test_stack_switch_does_not_excuse_other_asm(self):
        self.assertEqual(self.classify_unit(
            "main/boot/main.c",
            self.STACK_SWITCH + 'void g(void) { asm volatile("nop"); }\n',
            {"main/boot/main.c"}), "asm_constrained")

    def test_repository_stack_switch_user_is_semantic_c(self):
        root = pathlib.Path(__file__).resolve().parents[2]
        self.assertEqual(source_quality.classify(
            root / "src/main/boot/main.c"), "semantic_c")

    def test_psyq_assembler_object_cannot_hide_c_functions(self):
        self.assertEqual(self.classify(
            'PSYQ_ASM_FUNCTION(f, "    jr $ra\\n" "    nop\\n");\n'
            'int g(void) { return 1; }\n'), "asm_constrained")

    def test_sdk_assembler_sources_are_original_asm(self):
        root = pathlib.Path(__file__).resolve().parents[2] / "src/main/psyq"
        for name in ("libsn/snmain.c", "libgte/msc00.c", "libgte/msc01.c",
                     "libgte/msc02.c", "libgte/mtx_003.c", "libgte/mtx_006.c",
                     "libgte/patchgte.c", "libapi/patch_2.c",
                     "libapi/chclrpad.c", "libcard/patch.c",
                     "libcard/patch_3.c", "libcard/end.c"):
            with self.subTest(name=name):
                self.assertEqual(source_quality.classify(root / name),
                                 "original_asm")

    def test_text_resident_array_is_data(self):
        self.assertEqual(
            self.classify('int words[] __attribute__((section(".text"))) = { 0 };'),
            "text_data",
        )

    def test_function_with_text_resident_data_is_still_semantic_c(self):
        text = '''
        int f(void) { return 1; }
        int words[] __attribute__((section(".text"))) = { 0 };
        '''
        self.assertEqual(self.classify(text), "semantic_c")


if __name__ == "__main__":
    unittest.main()
