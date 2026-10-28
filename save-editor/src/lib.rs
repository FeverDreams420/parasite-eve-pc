pub mod charset;
pub mod cli;
pub mod crc;
pub mod gui;
pub mod memcard;
pub mod save;
pub mod tables;

#[cfg(test)]
mod tests {
    use std::path::PathBuf;

    use crate::memcard::MemoryCardContainer;

    #[test]
    fn test_real_ps1_sample_card_parsing_and_crc() {
        let sample_path =
            PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("samples/Parasite_Eve_PS1_Sample_Card.srm");
        let container = MemoryCardContainer::open(&sample_path).expect("open sample .srm card");

        let active = container.active_slot_indices();
        assert_eq!(active, vec![0, 1, 2, 3, 4, 5, 6]);

        let expected_crcs = [0x7231u16, 0x0A51, 0xE7FE, 0xF331, 0xE70C, 0xF2C1, 0x08DC];
        let expected_levels = [7u8, 14, 18, 24, 32, 34, 1];

        for (i, &exp_crc) in expected_crcs.iter().enumerate() {
            let slot = container.slots[i].as_ref().expect("slot exists");
            assert!(
                slot.is_crc_valid(),
                "Slot {} CRC mismatch: stored 0x{:04X} vs calc 0x{:04X}",
                i + 1,
                slot.stored_crc,
                slot.calculated_crc
            );
            assert_eq!(slot.stored_crc, exp_crc);
            assert_eq!(slot.display_level(), expected_levels[i]);
            assert_eq!(slot.aya_name, "Aya");
        }

        // Slot 7 is Clear Data
        let slot7 = container.slots[6].as_ref().unwrap();
        assert!(slot7.is_clear_data);
    }

    #[test]
    fn test_edit_and_roundtrip_serialization() {
        let sample_path =
            PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("samples/Parasite_Eve_PS1_Sample_Card.srm");
        let mut container = MemoryCardContainer::open(&sample_path).expect("open sample .srm card");

        let slot = container.slots[0].as_mut().unwrap();
        slot.apply_cheat_max_aya();
        slot.apply_cheat_unlock_all_pe();
        slot.apply_cheat_max_ammo();
        slot.apply_cheat_give_ultimate_loadout().unwrap();

        let bytes = slot.to_bytes();
        let reparsed =
            crate::save::PeSaveSlot::from_bytes(&bytes, 0, &slot.filename).expect("reparse slot");
        assert!(reparsed.is_crc_valid());
        assert_eq!(reparsed.display_level(), 99);
        assert_eq!(reparsed.current_hp, 999);
        assert_eq!(reparsed.max_hp, 999);
        assert_eq!(reparsed.bonus_points, 99_999);
        assert_eq!(reparsed.inventory_slot_count, 46);
        assert_eq!(reparsed.category_counts[0], 999);
    }
    #[test]
    fn factory_equipment_preserves_native_type_ammo_bonuses_and_effects() {
        use crate::save::EquipmentRecord;
        let p90 = EquipmentRecord::from_item_template(7, 0x78);
        assert_eq!((p90.index, p90.icon_id, p90.kind), (7, 4, 3));
        assert_eq!(p90.base_stats, [122, 51, 200]);
        assert_eq!(p90.ammo, 201);
        assert_eq!(p90.bonus_stats, [0, 2, 1]);
        assert_eq!((p90.tail_count, p90.tail_capacity), (5, 7));
        assert_eq!(&p90.tail_data[..2], &[37, 72]);
        let armor = EquipmentRecord::from_item_template(8, 0xB7);
        assert_eq!((armor.icon_id, armor.kind), (18, 9));
        assert_eq!(armor.base_stats, [106, 102, 48]);
        assert_eq!(armor.bonus_stats, [1, 2, 3]);
        assert_eq!((armor.tail_count, armor.tail_capacity), (7, 10));
        assert_eq!(armor.tail_data[0], 11);
        for id in 0x3F..=0xC7 {
            let item = EquipmentRecord::from_item_template(0, id);
            assert_eq!(item.item_id, id);
            assert!((1..=9).contains(&item.kind));
            assert!(item.tail_count <= 10 && item.tail_capacity <= 10);
            let meta = crate::tables::item_metadata(id);
            assert_eq!(meta.default_kind, item.kind);
            assert_eq!(meta.default_icon_id, item.icon_id);
        }
    }

    #[test]
    fn bag_capacity_edits_update_growth_and_preserve_other_stats() {
        let path = PathBuf::from(env!("CARGO_MANIFEST_DIR"))
            .join("samples/Parasite_Eve_PS1_Sample_Card.srm");
        let card = MemoryCardContainer::open(&path).unwrap();
        let original = card.slots[0].as_ref().unwrap();
        let mut native_test_data = Vec::new();
        for id in 0x3F..=0xC7 {
            let item = crate::save::EquipmentRecord::from_item_template(0, id);
            let mut bytes = [0; 32];
            item.write(&mut bytes);
            native_test_data.extend_from_slice(&bytes);
        }
        for capacity in 10..=46 {
            let mut slot = original.clone();
            slot.inventory_slot_count = capacity;
            let bytes = slot.to_bytes();
            let decoded = crate::save::PeSaveSlot::from_bytes(&bytes, 0, &slot.filename).unwrap();
            let growth = decoded.stats_levels[6];
            assert_eq!(crate::tables::BAG_CAPACITY_BY_LEVEL[(growth / 10).min(98) as usize], capacity);
            assert_eq!(&decoded.stats_levels[..6], &original.stats_levels[..6]);
            assert!(decoded.is_crc_valid());
            native_test_data.extend_from_slice(&(capacity as u16).to_le_bytes());
            native_test_data.extend_from_slice(&growth.to_le_bytes());
        }
        // Optional cross-language fixture: the native asset test compares every
        // editor-created record with the disc and recalculates all bag sizes.
        if let Some(path) = std::env::var_os("PE_TEST_EDITOR_DATA") {
            std::fs::write(path, native_test_data).unwrap();
        }
        let mut unchanged = original.clone();
        assert_eq!(unchanged.to_bytes(), original.raw_bytes);
    }

    #[test]
    fn repair_legacy_equipment_preserves_custom_upgrades() {
        let path = PathBuf::from(env!("CARGO_MANIFEST_DIR"))
            .join("samples/Parasite_Eve_PS1_Sample_Card.srm");
        let mut card = MemoryCardContainer::open(&path).unwrap();
        let save = card.slots[0].as_mut().unwrap();
        let mut p90 = crate::save::EquipmentRecord::from_item_template(7, 0x78);
        p90.icon_id = 5;
        p90.kind = 4;
        p90.bonus_stats = [900, 333, 222];
        p90.ammo = 700;
        p90.set_engraved(true);
        p90.set_mod_at(0, 20);
        save.equipment[7] = p90.clone();
        save.repair_equipment_types();
        let fixed = &save.equipment[7];
        let mut expected = [0; 32];
        p90.write(&mut expected);
        expected[0] = 4;
        expected[6] = 3;
        let mut actual = [0; 32];
        fixed.write(&mut actual);
        assert_eq!(actual, expected);
        assert_eq!(save.repair_equipment_types(), 0);
    }

}
