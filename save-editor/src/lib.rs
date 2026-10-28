pub mod charset;
pub mod cli;
pub mod crc;
pub mod disc;
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
        // The Item Capacity stat gives at most 46 slots; the game resets anything above it.
        assert_eq!(reparsed.inventory_slot_count, 46);
        assert!(reparsed.stats_levels[6] >= crate::save::MAX_BAG_ITEM_CAPACITY_STAT);
        assert_eq!(reparsed.category_counts[0], 999);
    }

    #[test]
    fn test_new_gear_uses_the_games_icon_and_kind() {
        let sample_path =
            PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("samples/Parasite_Eve_PS1_Sample_Card.srm");
        let mut container = MemoryCardContainer::open(&sample_path).expect("open sample .srm card");
        let slot = container.slots[0].as_mut().unwrap();

        // P90 (machine gun) and a handgun: icon 4 / kind 3 and icon 1 / kind 1 in the game's records.
        for (item_id, icon, kind) in [(0x78u8, 4u8, 3u8), (0x44, 1, 1)] {
            let val = slot.allocate_equipment_from_item(item_id).unwrap();
            let rec = &slot.equipment[(val - 0x100) as usize];
            assert_eq!((rec.item_id, rec.icon_id, rec.kind), (item_id, icon, kind));
        }
    }

    #[test]
    fn test_edits_keep_mod_data_in_the_free_tail() {
        // Mods keep their own blocks after the checksum word (0x1C90..0x2000), inside the CRC.
        let sample_path =
            PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("samples/Parasite_Eve_PS1_Sample_Card.srm");
        let mut container = MemoryCardContainer::open(&sample_path).expect("open sample .srm card");
        let slot = container.slots[0].as_mut().unwrap();
        let tail: Vec<u8> = (0..0x370u32).map(|i| (i * 7 + 3) as u8).collect();
        slot.raw_bytes[0x1C90..0x2000].copy_from_slice(&tail);

        slot.apply_cheat_max_aya();
        slot.add_item_to_inventory(0x78).unwrap();
        let bytes = slot.to_bytes();
        let reparsed = crate::save::PeSaveSlot::from_bytes(&bytes, 0, &slot.filename).unwrap();
        assert!(reparsed.is_crc_valid());
        assert_eq!(&bytes[0x1C90..0x2000], &tail[..]);
    }

    #[test]
    fn test_bag_size_follows_item_capacity_stat_without_disc() {
        let sample_path =
            PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("samples/Parasite_Eve_PS1_Sample_Card.srm");
        let mut container = MemoryCardContainer::open(&sample_path).expect("open sample .srm card");
        let slot = container.slots[0].as_mut().unwrap();
        slot.stats_levels[6] = 0;
        let _ = slot.to_bytes();

        slot.inventory_slot_count = 50;
        let bytes = slot.to_bytes();
        let reparsed = crate::save::PeSaveSlot::from_bytes(&bytes, 0, &slot.filename).unwrap();
        assert_eq!(reparsed.inventory_slot_count, 46);
        assert_eq!(reparsed.stats_levels[6], crate::save::MAX_BAG_ITEM_CAPACITY_STAT);
    }
}
