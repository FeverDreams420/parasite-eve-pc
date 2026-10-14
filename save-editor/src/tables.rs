use serde::{Deserialize, Serialize};

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum ItemCategory {
    Empty,
    Ammo,
    Consumable,
    StatUpgrade,
    Tool,
    Crate,
    MeleeWeapon,
    Handgun,
    Shotgun,
    MachineGun,
    Rifle,
    GrenadeLauncher,
    RocketLauncher,
    Armor,
    KeyItem,
    TradingCard,
    ParasiteEnergy,
}

impl ItemCategory {
    pub fn label(self) -> &'static str {
        match self {
            Self::Empty => "Empty",
            Self::Ammo => "Ammo",
            Self::Consumable => "Medicine / Cure",
            Self::StatUpgrade => "Stat Upgrade (+N)",
            Self::Tool => "Tool / Permit",
            Self::Crate => "Ammo Crate",
            Self::MeleeWeapon => "Club (Melee)",
            Self::Handgun => "Handgun",
            Self::Shotgun => "Shotgun",
            Self::MachineGun => "Machine Gun",
            Self::Rifle => "Rifle",
            Self::GrenadeLauncher => "Grenade Launcher",
            Self::RocketLauncher => "Rocket Launcher",
            Self::Armor => "Armor",
            Self::KeyItem => "Key Item / Charm",
            Self::TradingCard => "Trading Card",
            Self::ParasiteEnergy => "Parasite Energy Tech",
        }
    }

    pub fn is_weapon(self) -> bool {
        matches!(
            self,
            Self::MeleeWeapon
                | Self::Handgun
                | Self::Shotgun
                | Self::MachineGun
                | Self::Rifle
                | Self::GrenadeLauncher
                | Self::RocketLauncher
        )
    }

    pub fn is_armor(self) -> bool {
        matches!(self, Self::Armor)
    }

    pub fn is_equipment(self) -> bool {
        self.is_weapon() || self.is_armor()
    }
}

#[derive(Debug, Clone, Copy)]
pub struct ItemMetadata {
    pub id: u8,
    pub code_name: &'static str,
    pub display_name: &'static str,
    pub description: &'static str,
    pub category: ItemCategory,
    pub default_icon_id: u8,
    pub default_kind: u8,
    pub default_stats: [u8; 3],
    pub default_slots: (u8, u8), // (tail_count, tail_capacity)
}

#[derive(Debug, Clone, Copy)]
pub struct ModMetadata {
    pub id: u8,
    pub code_name: &'static str,
    pub name: &'static str,
    pub description: &'static str,
    pub is_armor_mod: bool,
    pub default_group_bits: u8,
}

#[derive(Debug, Clone, Copy)]
pub struct SpellMetadata {
    pub bit_index: u8,
    pub name: &'static str,
    pub category: &'static str,
    pub pe_cost_note: &'static str,
}

pub const PARASITE_SPELLS: [SpellMetadata; 20] = [
    SpellMetadata { bit_index: 0, name: "Heal 1", category: "Defense / Heal", pe_cost_note: "Restores +30 HP (0xEC)" },
    SpellMetadata { bit_index: 1, name: "Heal 2", category: "Defense / Heal", pe_cost_note: "Restores +60 HP (0xED)" },
    SpellMetadata { bit_index: 2, name: "Heal 3", category: "Defense / Heal", pe_cost_note: "Restores +280 HP (0xEE)" },
    SpellMetadata { bit_index: 3, name: "Detox", category: "Assist / Cure", pe_cost_note: "Cures Poison (0xEF)" },
    SpellMetadata { bit_index: 4, name: "Medic", category: "Assist / Cure", pe_cost_note: "Cures all status ailments (0xF0)" },
    SpellMetadata { bit_index: 5, name: "Barrier", category: "Defense / Buff", pe_cost_note: "Uses PE to Absorb Damage (0xF1)" },
    SpellMetadata { bit_index: 6, name: "Energy Shot", category: "Offense / Buff", pe_cost_note: "Boosts bullet attack power (0xF2)" },
    SpellMetadata { bit_index: 7, name: "Scan", category: "Assist", pe_cost_note: "Analyzes enemy HP & status (0xF3)" },
    SpellMetadata { bit_index: 8, name: "Slow", category: "Offense / Debuff", pe_cost_note: "Slows enemy AT recharge (0xF4)" },
    SpellMetadata { bit_index: 9, name: "Haste", category: "Assist / Buff", pe_cost_note: "Speeds Up Movement and AT Recharge (0xF5)" },
    SpellMetadata { bit_index: 10, name: "Confuse", category: "Offense / Debuff", pe_cost_note: "Confuses enemy target (0xF6)" },
    SpellMetadata { bit_index: 11, name: "Gene Heal", category: "Defense / Regen", pe_cost_note: "Uses PE to Slowly Recover HP (0xF7)" },
    SpellMetadata { bit_index: 12, name: "Preraise", category: "Defense / Revive", pe_cost_note: "Revives When HP Reaches 0 (0xF8)" },
    SpellMetadata { bit_index: 13, name: "Mito14", category: "Cut / Dummy", pe_cost_note: "ITEM_MITO14 (0xF9) — no case in Battle_ApplySpellEffect" },
    SpellMetadata { bit_index: 14, name: "Mito15", category: "Cut / Dummy", pe_cost_note: "ITEM_MITO15 (0xFA) — no case in Battle_ApplySpellEffect" },
    SpellMetadata { bit_index: 15, name: "Mito16", category: "Cut / Dummy", pe_cost_note: "ITEM_MITO16 (0xFB) — no case in Battle_ApplySpellEffect" },
    SpellMetadata { bit_index: 16, name: "Mito17", category: "Cut / Dummy", pe_cost_note: "ITEM_MITO17 (0xFC) — no case in Battle_ApplySpellEffect" },
    SpellMetadata { bit_index: 17, name: "Mito18", category: "Cut / Dummy", pe_cost_note: "ITEM_MITO18 (0xFD) — no case in Battle_ApplySpellEffect" },
    SpellMetadata { bit_index: 18, name: "Full Recover", category: "Defense / Heal", pe_cost_note: "Full HP & cures all ailments (0xFE)" },
    SpellMetadata { bit_index: 19, name: "Liberate", category: "Ultimate", pe_cost_note: "Liberate transformation attack (0xFF)" },
];

pub const STAT_GROWTH_NAMES: [&str; 7] = [
    "0: HP Growth",
    "1: Offense (Attack)",
    "2: Defense",
    "3: P.Energy Power",
    "4: Status Recovery",
    "5: Active Time (ATB)",
    "6: Item Capacity",
];

pub const WEAPON_MOD_TABLE: [ModMetadata; 21] = [
    ModMetadata { id: 0x00, code_name: "MOD_EMPTY_SLOT", name: "Empty Slot", description: "No modification installed", is_armor_mod: false, default_group_bits: 0x00 },
    ModMetadata { id: 0x01, code_name: "WMOD_ROF_2", name: "Rate of Fire x2", description: "Fires 2 rounds per attack command", is_armor_mod: false, default_group_bits: 0x20 },
    ModMetadata { id: 0x02, code_name: "WMOD_ROF_3", name: "Rate of Fire x3", description: "Fires 3 rounds per attack command", is_armor_mod: false, default_group_bits: 0x20 },
    ModMetadata { id: 0x03, code_name: "WMOD_ROF_5", name: "Rate of Fire x5", description: "Fires 5 rounds per attack command", is_armor_mod: false, default_group_bits: 0x20 },
    ModMetadata { id: 0x04, code_name: "WMOD_ROF_7", name: "Rate of Fire x7", description: "Fires 7 rounds per attack command", is_armor_mod: false, default_group_bits: 0x20 },
    ModMetadata { id: 0x05, code_name: "WMOD_ROF_10", name: "Rate of Fire x10", description: "Fires 10 rounds per attack command", is_armor_mod: false, default_group_bits: 0x20 },
    ModMetadata { id: 0x06, code_name: "WMOD_RANDOM_ROF", name: "Random RoF (1.5x)", description: "Random rate of fire multiplier", is_armor_mod: false, default_group_bits: 0x00 },
    ModMetadata { id: 0x07, code_name: "WMOD_BURST", name: "Burst (Wide Spread)", description: "Sprays bullets across multiple targets", is_armor_mod: false, default_group_bits: 0x00 },
    ModMetadata { id: 0x08, code_name: "WMOD_RANDOM_TARGET", name: "Random Target (All)", description: "Attacks all enemies randomly with RoF", is_armor_mod: false, default_group_bits: 0x00 },
    ModMetadata { id: 0x09, code_name: "WMOD_HEAT", name: "Heat Effect", description: "Adds fire elemental damage to bullets", is_armor_mod: false, default_group_bits: 0x40 },
    ModMetadata { id: 0x0A, code_name: "WMOD_FROST", name: "Frost Effect", description: "Adds ice elemental damage to bullets", is_armor_mod: false, default_group_bits: 0x40 },
    ModMetadata { id: 0x0B, code_name: "WMOD_CYANIDE", name: "Cyanide (Poison)", description: "Adds poison status effect to bullets", is_armor_mod: false, default_group_bits: 0x60 },
    ModMetadata { id: 0x0C, code_name: "WMOD_ACID", name: "Acid Effect", description: "Adds acid DOT damage to bullets", is_armor_mod: false, default_group_bits: 0x40 },
    ModMetadata { id: 0x0D, code_name: "WMOD_TRANQUILIZER", name: "Tranquilizer", description: "Adds stiffness/slow status to bullets", is_armor_mod: false, default_group_bits: 0x60 },
    ModMetadata { id: 0x0E, code_name: "WMOD_COUNTER", name: "Counterattack", description: "Automatically counterattacks when hit", is_armor_mod: false, default_group_bits: 0x00 },
    ModMetadata { id: 0x0F, code_name: "WMOD_FIRST_ATTACK", name: "First Attack (Quickdraw)", description: "Starts battle with full ATB gauge", is_armor_mod: false, default_group_bits: 0x00 },
    ModMetadata { id: 0x10, code_name: "WMOD_STEAL_HALF", name: "Steal (Half Atk)", description: "Steals items on hit, halves attack power", is_armor_mod: false, default_group_bits: 0x80 },
    ModMetadata { id: 0x11, code_name: "WMOD_STEAL_FULL", name: "Steal (Full Atk)", description: "Steals items during attack without penalty", is_armor_mod: false, default_group_bits: 0x80 },
    ModMetadata { id: 0x12, code_name: "WMOD_CRIT_UP", name: "Critical Up", description: "Increases critical hit probability", is_armor_mod: false, default_group_bits: 0x00 },
    ModMetadata { id: 0x13, code_name: "WMOD_COMMAND_2", name: "Command x2", description: "Allows entering 2 commands per turn", is_armor_mod: false, default_group_bits: 0xA0 },
    ModMetadata { id: 0x14, code_name: "WMOD_COMMAND_3", name: "Command x3", description: "Allows entering 3 commands per turn", is_armor_mod: false, default_group_bits: 0xA0 },
];

pub const ARMOR_MOD_TABLE: [ModMetadata; 18] = [
    ModMetadata { id: 0x00, code_name: "AMOD_EMPTY_SLOT", name: "Empty Slot", description: "No modification installed", is_armor_mod: true, default_group_bits: 0x00 },
    ModMetadata { id: 0x01, code_name: "AMOD_RESIST_POISON", name: "Resist Poison", description: "Prevents Poison status ailment", is_armor_mod: true, default_group_bits: 0x00 },
    ModMetadata { id: 0x02, code_name: "AMOD_RESIST_STIFFNESS", name: "Resist Stiffness", description: "Prevents Stiffness/Paralysis status", is_armor_mod: true, default_group_bits: 0x00 },
    ModMetadata { id: 0x03, code_name: "AMOD_RESIST_DARKNESS", name: "Resist Darkness", description: "Prevents Darkness/Blind status", is_armor_mod: true, default_group_bits: 0x00 },
    ModMetadata { id: 0x04, code_name: "AMOD_ANTI_THEFT", name: "Anti-Theft (Resist Steal)", description: "Decreases chance of items being stolen", is_armor_mod: true, default_group_bits: 0x00 },
    ModMetadata { id: 0x05, code_name: "AMOD_RESIST_CONFUSION", name: "Resist Confusion", description: "Prevents Confusion status", is_armor_mod: true, default_group_bits: 0x00 },
    ModMetadata { id: 0x06, code_name: "AMOD_AUTO_MEDICINE", name: "Auto-Heal (Medicine)", description: "Automatically uses Medicine when HP is low", is_armor_mod: true, default_group_bits: 0x00 },
    ModMetadata { id: 0x07, code_name: "AMOD_AUTO_CURE", name: "Auto-Cure", description: "Automatically uses Cures on status ailment", is_armor_mod: true, default_group_bits: 0x00 },
    ModMetadata { id: 0x08, code_name: "AMOD_ITEM_CAP_1", name: "Item Capacity +1", description: "Increases inventory capacity by +1 slot", is_armor_mod: true, default_group_bits: 0xA0 },
    ModMetadata { id: 0x09, code_name: "AMOD_ITEM_CAP_2", name: "Item Capacity +2", description: "Increases inventory capacity by +2 slots", is_armor_mod: true, default_group_bits: 0xA0 },
    ModMetadata { id: 0x0A, code_name: "AMOD_ITEM_CAP_4", name: "Item Capacity +4", description: "Increases inventory capacity by +4 slots", is_armor_mod: true, default_group_bits: 0xA0 },
    ModMetadata { id: 0x0B, code_name: "AMOD_HP_MAX_UP", name: "HP Max Up", description: "Increases maximum HP while equipped", is_armor_mod: true, default_group_bits: 0x00 },
    ModMetadata { id: 0x0C, code_name: "AMOD_OFFENSE_UP", name: "Attack Power Up", description: "Increases Offense stat while equipped", is_armor_mod: true, default_group_bits: 0x00 },
    ModMetadata { id: 0x0D, code_name: "AMOD_CRIT_UP", name: "Critical Up (Armor)", description: "Increases critical rate while equipped", is_armor_mod: true, default_group_bits: 0x00 },
    ModMetadata { id: 0x0E, code_name: "AMOD_PE_LIGHT", name: "PE Light", description: "Reduces Parasite Energy cost by 1/3", is_armor_mod: true, default_group_bits: 0x00 },
    ModMetadata { id: 0x0F, code_name: "AMOD_PE_DEF_UP", name: "PE Defense Up", description: "Increases Parasite Energy defense", is_armor_mod: true, default_group_bits: 0x00 },
    ModMetadata { id: 0x10, code_name: "AMOD_ATB_UP", name: "Active Time (ATB) Up", description: "Increases ATB charge speed", is_armor_mod: true, default_group_bits: 0x00 },
    ModMetadata { id: 0x11, code_name: "AMOD_RECOVERY_UP", name: "Status Recovery Up", description: "Increases status recovery rate", is_armor_mod: true, default_group_bits: 0x00 },
];

pub fn mod_metadata(mod_byte: u8, is_armor: bool) -> &'static ModMetadata {
    let idx = (mod_byte & 0x1F) as usize;
    if is_armor {
        ARMOR_MOD_TABLE.get(idx).unwrap_or(&ARMOR_MOD_TABLE[0])
    } else {
        WEAPON_MOD_TABLE.get(idx).unwrap_or(&WEAPON_MOD_TABLE[0])
    }
}

pub fn encode_mod_byte(mod_id: u8, is_armor: bool) -> u8 {
    let meta = mod_metadata(mod_id, is_armor);
    (meta.id & 0x1F) | meta.default_group_bits
}

const fn item(
    id: u8,
    code_name: &'static str,
    display_name: &'static str,
    description: &'static str,
    category: ItemCategory,
    default_icon_id: u8,
    default_kind: u8,
    default_stats: [u8; 3],
    default_slots: (u8, u8),
) -> ItemMetadata {
    ItemMetadata {
        id,
        code_name,
        display_name,
        description,
        category,
        default_icon_id,
        default_kind,
        default_stats,
        default_slots,
    }
}

pub const ITEM_TABLE: [ItemMetadata; 256] = [
    item(0x00, "ITEM_EMPTY_SLOT", "Empty", "Empty slot", ItemCategory::Empty, 0, 0, [0, 0, 0], (0, 0)),
    item(0x01, "ITEM_AMMO_6", "Ammo +6", "Bullets (+6)", ItemCategory::Ammo, 11, 0, [0, 0, 0], (0, 0)),
    item(0x02, "ITEM_AMMO_15", "Ammo +15", "Bullets (+15)", ItemCategory::Ammo, 11, 0, [0, 0, 0], (0, 0)),
    item(0x03, "ITEM_AMMO_30", "Ammo +30", "Bullets (+30)", ItemCategory::Ammo, 11, 0, [0, 0, 0], (0, 0)),
    item(0x04, "ITEM_DNA_BULLETS", "DNA Bullets", "Maeda's Bullets", ItemCategory::Ammo, 11, 0, [0, 0, 0], (0, 0)),
    item(0x05, "ITEM_ROCKET", "Rocket Ammo", "For Rocket Launcher", ItemCategory::Ammo, 11, 0, [0, 0, 0], (0, 0)),
    item(0x06, "ITEM_MEDICINE_1", "Medicine 1", "Restores 45 HP", ItemCategory::Consumable, 12, 0, [0, 0, 0], (0, 0)),
    item(0x07, "ITEM_MEDICINE_2", "Medicine 2", "Restores 90 HP", ItemCategory::Consumable, 12, 0, [0, 0, 0], (0, 0)),
    item(0x08, "ITEM_MEDICINE_3", "Medicine 3", "Restores 180 HP", ItemCategory::Consumable, 12, 0, [0, 0, 0], (0, 0)),
    item(0x09, "ITEM_MEDICINE_4", "Medicine 4", "Restores 400 HP", ItemCategory::Consumable, 12, 0, [0, 0, 0], (0, 0)),
    item(0x0A, "ITEM_FULL_RECOVER", "Full Recover", "Restores All HP", ItemCategory::Consumable, 12, 0, [0, 0, 0], (0, 0)),
    item(0x0B, "ITEM_SUPER_JUNK", "Super Junk", "Helps Item (Restores PE)", ItemCategory::Consumable, 15, 0, [0, 0, 0], (0, 0)),
    item(0x0C, "ITEM_DUPER_JUNK", "Duper Junk", "Helps Item (Restores PE)", ItemCategory::Consumable, 15, 0, [0, 0, 0], (0, 0)),
    item(0x0D, "ITEM_CURE_P", "Cure-P", "Cures & Prevents Poison", ItemCategory::Consumable, 13, 0, [0, 0, 0], (0, 0)),
    item(0x0E, "ITEM_CURE_D", "Cure-D", "Cures & Prevents Darkness", ItemCategory::Consumable, 13, 0, [0, 0, 0], (0, 0)),
    item(0x0F, "ITEM_CURE_C", "Cure-C", "Cures & Prevents Confusion", ItemCategory::Consumable, 13, 0, [0, 0, 0], (0, 0)),
    item(0x10, "ITEM_CURE_M", "Cure-M", "Cures & Prevents Stiffness", ItemCategory::Consumable, 13, 0, [0, 0, 0], (0, 0)),
    item(0x11, "ITEM_FULL_CURE", "Full Cure", "Cures All Negative Status", ItemCategory::Consumable, 13, 0, [0, 0, 0], (0, 0)),
    item(0x12, "ITEM_REVIVE", "Revive", "Restores Life When You Die", ItemCategory::Consumable, 14, 0, [0, 0, 0], (0, 0)),
    item(0x13, "ITEM_DEFENSE_1", "Defense +1 (13h)", "Increases Defense Power by 1", ItemCategory::StatUpgrade, 16, 0, [0, 0, 0], (0, 0)),
    item(0x14, "ITEM_JUNK", "Junk", "Broken Item (Give 300 to Wayne)", ItemCategory::KeyItem, 15, 0, [0, 0, 0], (0, 0)),
    item(0x15, "ITEM_TRADING_CARD", "Trading Card", "Normal Trading Card", ItemCategory::TradingCard, 17, 0, [0, 0, 0], (0, 0)),
    item(0x16, "ITEM_TOOL", "Tool", "Moves Equipped Parameter or Effect", ItemCategory::Tool, 18, 0, [0, 0, 0], (0, 0)),
    item(0x17, "ITEM_SUPER_TOOL", "Super Tool", "Moves Parameter Without Destroying Host", ItemCategory::Tool, 18, 0, [0, 0, 0], (0, 0)),
    item(0x18, "ITEM_ITEM24", "Item 24", "Helps Item", ItemCategory::Consumable, 15, 0, [0, 0, 0], (0, 0)),
    item(0x19, "ITEM_ITEM25", "Item 25", "Helps Item", ItemCategory::Consumable, 15, 0, [0, 0, 0], (0, 0)),
    item(0x1A, "ITEM_AMMO_CRATE", "Ammo Crate", "Bullet Crate", ItemCategory::Crate, 11, 0, [0, 0, 0], (0, 0)),
    item(0x1B, "ITEM_ROCKET_CRATE", "Rocket Crate", "Rocket Crate", ItemCategory::Crate, 11, 0, [0, 0, 0], (0, 0)),
    item(0x1C, "ITEM_MAEDA_CRATE", "Maeda's Bullet Crate", "Maeda's Bullet Crate", ItemCategory::Crate, 11, 0, [0, 0, 0], (0, 0)),
    item(0x1D, "ITEM_OFFENSE_1", "Offense +1", "Increases Attack Power by 1", ItemCategory::StatUpgrade, 16, 0, [0, 0, 0], (0, 0)),
    item(0x1E, "ITEM_OFFENSE_2", "Offense +2", "Increases Attack Power by 2", ItemCategory::StatUpgrade, 16, 0, [0, 0, 0], (0, 0)),
    item(0x1F, "ITEM_OFFENSE_3", "Offense +3", "Increases Attack Power by 3", ItemCategory::StatUpgrade, 16, 0, [0, 0, 0], (0, 0)),
    item(0x20, "ITEM_OFFENSE_4", "Offense +4", "Increases Attack Power by 4", ItemCategory::StatUpgrade, 16, 0, [0, 0, 0], (0, 0)),
    item(0x21, "ITEM_RANGE_1", "Range +1", "Increases Attack Range by 1", ItemCategory::StatUpgrade, 16, 0, [0, 0, 0], (0, 0)),
    item(0x22, "ITEM_RANGE_2", "Range +2", "Increases Attack Range by 2", ItemCategory::StatUpgrade, 16, 0, [0, 0, 0], (0, 0)),
    item(0x23, "ITEM_RANGE_3", "Range +3", "Increases Attack Range by 3", ItemCategory::StatUpgrade, 16, 0, [0, 0, 0], (0, 0)),
    item(0x24, "ITEM_RANGE_4", "Range +4", "Increases Attack Range by 4", ItemCategory::StatUpgrade, 16, 0, [0, 0, 0], (0, 0)),
    item(0x25, "ITEM_BULLET_CAP_1", "Bullet Cap +1", "Increases Bullet Capacity by 1", ItemCategory::StatUpgrade, 16, 0, [0, 0, 0], (0, 0)),
    item(0x26, "ITEM_BULLET_CAP_2", "Bullet Cap +2", "Increases Bullet Capacity by 2", ItemCategory::StatUpgrade, 16, 0, [0, 0, 0], (0, 0)),
    item(0x27, "ITEM_BULLET_CAP_3", "Bullet Cap +3", "Increases Bullet Capacity by 3", ItemCategory::StatUpgrade, 16, 0, [0, 0, 0], (0, 0)),
    item(0x28, "ITEM_BULLET_CAP_4", "Bullet Cap +4", "Increases Bullet Capacity by 4", ItemCategory::StatUpgrade, 16, 0, [0, 0, 0], (0, 0)),
    item(0x29, "ITEM_DEFENSE_1_29", "Defense +1", "Increases Defense Power by 1", ItemCategory::StatUpgrade, 16, 0, [0, 0, 0], (0, 0)),
    item(0x2A, "ITEM_DEFENSE_2", "Defense +2", "Increases Defense Power by 2", ItemCategory::StatUpgrade, 16, 0, [0, 0, 0], (0, 0)),
    item(0x2B, "ITEM_DEFENSE_3", "Defense +3", "Increases Defense Power by 3", ItemCategory::StatUpgrade, 16, 0, [0, 0, 0], (0, 0)),
    item(0x2C, "ITEM_DEFENSE_4", "Defense +4", "Increases Defense Power by 4", ItemCategory::StatUpgrade, 16, 0, [0, 0, 0], (0, 0)),
    item(0x2D, "ITEM_CR_EVADE_1", "Cr Evade +1", "Increases Critical Evasion % by 1", ItemCategory::StatUpgrade, 16, 0, [0, 0, 0], (0, 0)),
    item(0x2E, "ITEM_CR_EVADE_2", "Cr Evade +2", "Increases Critical Evasion % by 2", ItemCategory::StatUpgrade, 16, 0, [0, 0, 0], (0, 0)),
    item(0x2F, "ITEM_CR_EVADE_3", "Cr Evade +3", "Increases Critical Evasion % by 3", ItemCategory::StatUpgrade, 16, 0, [0, 0, 0], (0, 0)),
    item(0x30, "ITEM_CR_EVADE_4", "Cr Evade +4", "Increases Critical Evasion % by 4", ItemCategory::StatUpgrade, 16, 0, [0, 0, 0], (0, 0)),
    item(0x31, "ITEM_PE_1", "PE +1", "Increases PE Defense by 1", ItemCategory::StatUpgrade, 16, 0, [0, 0, 0], (0, 0)),
    item(0x32, "ITEM_PE_2", "PE +2", "Increases PE Defense by 2", ItemCategory::StatUpgrade, 16, 0, [0, 0, 0], (0, 0)),
    item(0x33, "ITEM_PE_3", "PE +3", "Increases PE Defense by 3", ItemCategory::StatUpgrade, 16, 0, [0, 0, 0], (0, 0)),
    item(0x34, "ITEM_PE_4", "PE +4", "Increases PE Defense by 4", ItemCategory::StatUpgrade, 16, 0, [0, 0, 0], (0, 0)),
    item(0x35, "ITEM_TOOL_KIT", "Tool Kit", "Unlimited Parameter Transfer Tool", ItemCategory::Tool, 18, 0, [0, 0, 0], (0, 0)),
    item(0x36, "ITEM_SUPER_TOOL_KIT", "Super Tool Kit", "Unlimited Super Parameter Transfer Tool", ItemCategory::Tool, 18, 0, [0, 0, 0], (0, 0)),
    item(0x37, "ITEM_MOD_PERMIT", "Mod Permit", "Permit from Baker", ItemCategory::Tool, 19, 0, [0, 0, 0], (0, 0)),
    item(0x38, "ITEM_CHRYSLER_KEY1", "Chrysler Key 1", "Elevator Key to 10th Fl. CB", ItemCategory::KeyItem, 20, 0, [0, 0, 0], (0, 0)),
    item(0x39, "ITEM_CHRYSLER_KEY2", "Chrysler Key 2", "Elevator Key to 20th Fl. CB", ItemCategory::KeyItem, 20, 0, [0, 0, 0], (0, 0)),
    item(0x3A, "ITEM_CHRYSLER_KEY3", "Chrysler Key 3", "Elevator Key to 30th Fl. CB", ItemCategory::KeyItem, 20, 0, [0, 0, 0], (0, 0)),
    item(0x3B, "ITEM_CHRYSLER_KEY4", "Chrysler Key 4", "Elevator Key to 40th Fl. CB", ItemCategory::KeyItem, 20, 0, [0, 0, 0], (0, 0)),
    item(0x3C, "ITEM_CHRYSLER_KEY5", "Chrysler Key 5", "Elevator Key to 50th Fl. CB", ItemCategory::KeyItem, 20, 0, [0, 0, 0], (0, 0)),
    item(0x3D, "ITEM_CHRYSLER_KEY6", "Chrysler Key 6", "Elevator Key to 60th Fl. CB", ItemCategory::KeyItem, 20, 0, [0, 0, 0], (0, 0)),
    item(0x3E, "ITEM_CHRYSLER_KEY7", "Chrysler Key 7", "Elevator Key to 70th Fl. CB", ItemCategory::KeyItem, 20, 0, [0, 0, 0], (0, 0)),
    // Melee Weapons (0x3F..=0x43)
    item(0x3F, "ITEM_CLUB_1", "Club 1", "Police Nightstick (Melee)", ItemCategory::MeleeWeapon, 1, 8, [8, 2, 0], (1, 4)),
    item(0x40, "ITEM_CLUB_2", "Club 2", "Reinforced Nightstick (Melee)", ItemCategory::MeleeWeapon, 1, 8, [18, 3, 0], (2, 5)),
    item(0x41, "ITEM_CLUB_3", "Club 3", "Tactical Baton (Melee)", ItemCategory::MeleeWeapon, 1, 8, [32, 4, 0], (3, 6)),
    item(0x42, "ITEM_CLUB_4", "Club 4", "High-Impact Baton (Melee)", ItemCategory::MeleeWeapon, 1, 8, [48, 5, 0], (4, 8)),
    item(0x43, "ITEM_CLUB_5", "Club 5", "Ultimate Baton (Melee)", ItemCategory::MeleeWeapon, 1, 8, [68, 6, 0], (5, 10)),
    // Handguns & Special Guns (0x44..=0x67)
    item(0x44, "ITEM_M84F", "M84F", "Beretta M84F Handgun", ItemCategory::Handgun, 2, 1, [11, 20, 10], (1, 4)),
    item(0x45, "ITEM_M9", "M9", "Beretta M9 Handgun", ItemCategory::Handgun, 2, 1, [14, 24, 15], (2, 5)),
    item(0x46, "ITEM_M9_2", "M9-2", "Custom Beretta M9-2 Handgun", ItemCategory::Handgun, 2, 1, [20, 26, 15], (3, 6)),
    item(0x47, "ITEM_M9_3", "M9-3", "Custom Beretta M9-3 Handgun", ItemCategory::Handgun, 2, 1, [28, 28, 15], (4, 7)),
    item(0x48, "ITEM_M8000", "M8000", "Beretta M8000 Cougar Handgun", ItemCategory::Handgun, 2, 1, [35, 30, 12], (3, 6)),
    item(0x49, "ITEM_M96", "M96", "Beretta M96 Handgun", ItemCategory::Handgun, 2, 1, [42, 32, 11], (4, 7)),
    item(0x4A, "ITEM_M96R", "M96R", "Beretta M96R Burst Handgun", ItemCategory::Handgun, 2, 1, [52, 35, 15], (5, 8)),
    item(0x4B, "ITEM_P220", "P220", "SIG Sauer P220 Handgun", ItemCategory::Handgun, 2, 1, [16, 22, 9], (2, 5)),
    item(0x4C, "ITEM_P220_2", "P220-2", "SIG Sauer P220-2 Handgun", ItemCategory::Handgun, 2, 1, [24, 25, 9], (3, 6)),
    item(0x4D, "ITEM_P228", "P228", "SIG Sauer P228 Handgun", ItemCategory::Handgun, 2, 1, [32, 28, 13], (3, 6)),
    item(0x4E, "ITEM_P226", "P226", "SIG Sauer P226 Handgun", ItemCategory::Handgun, 2, 1, [45, 32, 15], (4, 7)),
    item(0x4F, "ITEM_P229", "P229", "SIG Sauer P229 Handgun", ItemCategory::Handgun, 2, 1, [58, 36, 12], (5, 8)),
    item(0x50, "ITEM_M1911A1", "M1911A1", "Colt M1911A1 Handgun", ItemCategory::Handgun, 2, 1, [22, 24, 7], (2, 5)),
    item(0x51, "ITEM_M1911A2", "M1911A2", "Colt M1911A2 Handgun", ItemCategory::Handgun, 2, 1, [32, 28, 7], (3, 6)),
    item(0x52, "ITEM_M1911A3", "M1911A3", "Colt M1911A3 Handgun", ItemCategory::Handgun, 2, 1, [44, 32, 8], (4, 7)),
    item(0x53, "ITEM_M1911A4", "M1911A4", "Colt M1911A4 Handgun", ItemCategory::Handgun, 2, 1, [56, 36, 8], (5, 8)),
    item(0x54, "ITEM_M1911A5", "M1911A5", "Colt M1911A5 Custom Handgun", ItemCategory::Handgun, 2, 1, [72, 40, 10], (6, 9)),
    item(0x55, "ITEM_P8", "P8", "H&K P8 Handgun", ItemCategory::Handgun, 2, 1, [26, 26, 15], (3, 6)),
    item(0x56, "ITEM_USP", "USP", "H&K USP Handgun", ItemCategory::Handgun, 2, 1, [38, 30, 12], (4, 7)),
    item(0x57, "ITEM_USP_2", "USP-2", "H&K USP-2 Handgun", ItemCategory::Handgun, 2, 1, [50, 34, 13], (5, 8)),
    item(0x58, "ITEM_USP_3", "USP-3", "H&K USP-3 Handgun", ItemCategory::Handgun, 2, 1, [64, 38, 15], (6, 9)),
    item(0x59, "ITEM_MARK_23", "Mark 23", "H&K Mark 23 SOCOM Handgun", ItemCategory::Handgun, 2, 1, [78, 45, 12], (6, 9)),
    item(0x5A, "ITEM_G19", "G19", "Glock 19 Handgun", ItemCategory::Handgun, 2, 1, [25, 22, 15], (3, 6)),
    item(0x5B, "ITEM_G23", "G23", "Glock 23 Handgun", ItemCategory::Handgun, 2, 1, [38, 26, 13], (4, 7)),
    item(0x5C, "ITEM_G22", "G22", "Glock 22 Handgun", ItemCategory::Handgun, 2, 1, [52, 30, 15], (5, 8)),
    item(0x5D, "ITEM_G20", "G20", "Glock 20 10mm Handgun", ItemCategory::Handgun, 2, 1, [68, 35, 15], (6, 9)),
    item(0x5E, "ITEM_M712", "M712", "Mauser M712 Broomhandle", ItemCategory::Handgun, 2, 1, [75, 42, 10], (5, 8)),
    item(0x5F, "ITEM_PPK", "PPK", "Walther PPK Handgun", ItemCategory::Handgun, 2, 1, [48, 28, 7], (4, 7)),
    item(0x60, "ITEM_AM44", "AM44", "AutoMag .44 Handgun", ItemCategory::Handgun, 2, 1, [92, 50, 8], (5, 8)),
    item(0x61, "ITEM_MAEDAS_GUN", "Maeda's Gun", "Maeda's Special Handgun", ItemCategory::Handgun, 2, 1, [99, 60, 9], (6, 10)),
    item(0x62, "ITEM_PPSH41", "PPSh41", "PPSh-41 Submachine Gun", ItemCategory::MachineGun, 5, 4, [82, 40, 71], (5, 8)),
    item(0x63, "ITEM_SP1C", "SP1C", "Spectre M4 Submachine Handgun", ItemCategory::Handgun, 2, 1, [85, 45, 30], (5, 8)),
    item(0x64, "ITEM_USP_TU", "USP-TU", "H&K USP Tactical Unlimited", ItemCategory::Handgun, 2, 1, [88, 48, 18], (6, 10)),
    item(0x65, "ITEM_AK_47", "AK-47", "Kalashnikov AK-47 Assault Rifle", ItemCategory::Rifle, 6, 5, [95, 70, 30], (6, 9)),
    item(0x66, "ITEM_DE50AE7", "DE50AE7", "Desert Eagle .50AE Handgun", ItemCategory::Handgun, 2, 1, [110, 60, 7], (6, 10)),
    item(0x67, "ITEM_DEBUGSMG", "Debug SMG", "Developer Debug Machine Gun", ItemCategory::MachineGun, 5, 4, [120, 80, 99], (8, 10)),
    // Shotguns (0x68..=0x6E)
    item(0x68, "ITEM_M870", "M870", "Remington M870 Shotgun", ItemCategory::Shotgun, 4, 3, [30, 18, 7], (2, 5)),
    item(0x69, "ITEM_M870_2", "M870-2", "Remington M870-2 Shotgun", ItemCategory::Shotgun, 4, 3, [46, 22, 8], (3, 6)),
    item(0x6A, "ITEM_M500", "M500", "Mossberg M500 Shotgun", ItemCategory::Shotgun, 4, 3, [58, 25, 6], (4, 7)),
    item(0x6B, "ITEM_M500_2", "M500-2", "Mossberg M500-2 Shotgun", ItemCategory::Shotgun, 4, 3, [72, 28, 8], (5, 8)),
    item(0x6C, "ITEM_MAVERICK", "Maverick", "Maverick 88 Shotgun", ItemCategory::Shotgun, 4, 3, [64, 26, 6], (4, 7)),
    item(0x6D, "ITEM_S12", "S12", "SPAS-12 Combat Shotgun", ItemCategory::Shotgun, 4, 3, [85, 32, 8], (5, 9)),
    item(0x6E, "ITEM_M10B", "M10B", "Ithaca Mag-10 Roadblocker Shotgun", ItemCategory::Shotgun, 4, 3, [98, 36, 5], (6, 9)),
    // Machine Guns (0x6F..=0x78)
    item(0x6F, "ITEM_M11", "M11", "MAC-11 Submachine Gun", ItemCategory::MachineGun, 5, 4, [24, 18, 32], (2, 5)),
    item(0x70, "ITEM_M10", "M10", "MAC-10 Submachine Gun", ItemCategory::MachineGun, 5, 4, [38, 22, 30], (3, 6)),
    item(0x71, "ITEM_MP5K", "MP5K", "H&K MP5K Submachine Gun", ItemCategory::MachineGun, 5, 4, [45, 25, 15], (4, 7)),
    item(0x72, "ITEM_MP5PDW", "MP5PDW", "H&K MP5K-PDW Submachine Gun", ItemCategory::MachineGun, 5, 4, [58, 30, 30], (5, 8)),
    item(0x73, "ITEM_MP5A5", "MP5A5", "H&K MP5A5 Submachine Gun", ItemCategory::MachineGun, 5, 4, [72, 35, 30], (6, 9)),
    item(0x74, "ITEM_MP5SD6", "MP5SD6", "H&K MP5SD6 Suppressed SMG", ItemCategory::MachineGun, 5, 4, [84, 38, 30], (6, 9)),
    item(0x75, "ITEM_MICRO_UZ", "Micro UZ", "Micro Uzi Submachine Gun", ItemCategory::MachineGun, 5, 4, [34, 20, 20], (3, 6)),
    item(0x76, "ITEM_MINI_UZ", "Mini UZ", "Mini Uzi Submachine Gun", ItemCategory::MachineGun, 5, 4, [50, 24, 25], (4, 7)),
    item(0x77, "ITEM_FULL_UZ", "Full UZ", "Full-Size Uzi Submachine Gun", ItemCategory::MachineGun, 5, 4, [66, 28, 32], (5, 8)),
    item(0x78, "ITEM_P90", "P90", "FN P90 Personal Defense Weapon", ItemCategory::MachineGun, 5, 4, [92, 42, 50], (7, 10)),
    // Rifles (0x79..=0x82)
    item(0x79, "ITEM_M16A1", "M16A1", "Colt M16A1 Assault Rifle", ItemCategory::Rifle, 6, 5, [48, 55, 20], (4, 7)),
    item(0x7A, "ITEM_M16A2", "M16A2", "Colt M16A2 Assault Rifle", ItemCategory::Rifle, 6, 5, [65, 65, 30], (5, 8)),
    item(0x7B, "ITEM_SG550", "SG550", "SIG SG 550 Assault Rifle", ItemCategory::Rifle, 6, 5, [76, 70, 30], (5, 8)),
    item(0x7C, "ITEM_SAR", "SAR", "Galil SAR Assault Rifle", ItemCategory::Rifle, 6, 5, [70, 62, 35], (5, 8)),
    item(0x7D, "ITEM_G3A3", "G3A3", "H&K G3A3 Battle Rifle", ItemCategory::Rifle, 6, 5, [86, 75, 20], (5, 9)),
    item(0x7E, "ITEM_TYPE64", "Type64", "Howa Type 64 Battle Rifle", ItemCategory::Rifle, 6, 5, [80, 68, 20], (5, 8)),
    item(0x7F, "ITEM_XM177E2", "XM177E2", "Colt XM177E2 Commando Carbine", ItemCategory::Rifle, 6, 5, [72, 58, 30], (6, 9)),
    item(0x80, "ITEM_PSG_1", "PSG-1", "H&K PSG-1 Sniper Rifle", ItemCategory::Rifle, 6, 5, [110, 95, 20], (6, 9)),
    item(0x81, "ITEM_FA_MAS", "FA-MAS", "FAMAS F1 Bullpup Assault Rifle", ItemCategory::Rifle, 6, 5, [88, 66, 25], (6, 9)),
    item(0x82, "ITEM_MAG", "MAG", "FN MAG General Purpose Machine Gun", ItemCategory::Rifle, 6, 5, [105, 80, 50], (6, 10)),
    // Grenade Launchers (0x83..=0x8F)
    item(0x83, "ITEM_M203", "M203", "M203 Grenade Launcher", ItemCategory::GrenadeLauncher, 7, 6, [55, 35, 1], (2, 4)),
    item(0x84, "ITEM_M203_2", "M203-2", "M203-2 Grenade Launcher", ItemCategory::GrenadeLauncher, 7, 6, [68, 38, 2], (3, 5)),
    item(0x85, "ITEM_M203_3", "M203-3", "M203-3 Grenade Launcher", ItemCategory::GrenadeLauncher, 7, 6, [78, 42, 2], (3, 6)),
    item(0x86, "ITEM_M203_4", "M203-4", "M203-4 Grenade Launcher", ItemCategory::GrenadeLauncher, 7, 6, [88, 45, 3], (4, 6)),
    item(0x87, "ITEM_M203_5", "M203-5", "M203-5 Grenade Launcher", ItemCategory::GrenadeLauncher, 7, 6, [98, 48, 3], (4, 7)),
    item(0x88, "ITEM_M203_6", "M203-6", "M203-6 Grenade Launcher", ItemCategory::GrenadeLauncher, 7, 6, [110, 52, 4], (5, 8)),
    item(0x89, "ITEM_M79", "M79", "M79 Blooper Grenade Launcher", ItemCategory::GrenadeLauncher, 7, 6, [62, 40, 1], (2, 5)),
    item(0x8A, "ITEM_M79_2", "M79-2", "M79-2 Grenade Launcher", ItemCategory::GrenadeLauncher, 7, 6, [75, 44, 2], (3, 6)),
    item(0x8B, "ITEM_M79_3", "M79-3", "M79-3 Grenade Launcher", ItemCategory::GrenadeLauncher, 7, 6, [86, 48, 2], (4, 7)),
    item(0x8C, "ITEM_M79_4", "M79-4", "M79-4 Grenade Launcher", ItemCategory::GrenadeLauncher, 7, 6, [96, 52, 3], (4, 7)),
    item(0x8D, "ITEM_M79_5", "M79-5", "M79-5 Grenade Launcher", ItemCategory::GrenadeLauncher, 7, 6, [108, 56, 4], (5, 8)),
    item(0x8E, "ITEM_M79_6", "M79-6", "M79-6 Grenade Launcher", ItemCategory::GrenadeLauncher, 7, 6, [120, 60, 5], (6, 9)),
    item(0x8F, "ITEM_HK40", "HK40", "H&K HK69A1 40mm Grenade Launcher", ItemCategory::GrenadeLauncher, 7, 6, [125, 65, 6], (6, 9)),
    // Rocket Launchers & Final Weapons (0x90..=0x94)
    item(0x90, "ITEM_AT4", "AT4", "AT4 Anti-Tank Rocket Launcher", ItemCategory::RocketLauncher, 8, 7, [130, 80, 1], (3, 5)),
    item(0x91, "ITEM_AT4_1", "AT4-1", "AT4-1 Custom Rocket Launcher", ItemCategory::RocketLauncher, 8, 7, [145, 85, 2], (4, 7)),
    item(0x92, "ITEM_LAW80", "LAW80", "LAW 80 Heavy Rocket Launcher", ItemCategory::RocketLauncher, 8, 7, [160, 90, 3], (5, 8)),
    item(0x93, "ITEM_MAEDAS_GUN_93", "Maeda's Gun (Loaded)", "Maeda's Gun With DNA Bullets", ItemCategory::Handgun, 2, 1, [99, 75, 9], (6, 10)),
    item(0x94, "ITEM_M92F", "M92F", "Aya's Starting Beretta M92F Handgun", ItemCategory::Handgun, 2, 1, [12, 22, 15], (1, 4)),
    // Armor (0x95..=0xC7)
    item(0x95, "ITEM_DRESS", "Dress", "Aya's Opera Evening Dress", ItemCategory::Armor, 9, 9, [4, 4, 2], (0, 2)),
    item(0x96, "ITEM_N_VEST", "N Vest", "Normal Vest Armor", ItemCategory::Armor, 9, 9, [10, 8, 5], (1, 3)),
    item(0x97, "ITEM_N_PROTECTOR", "N Protector", "Normal Protector Armor", ItemCategory::Armor, 9, 9, [16, 14, 10], (2, 4)),
    item(0x98, "ITEM_N_JACKET", "N Jacket", "Normal Jacket Armor", ItemCategory::Armor, 9, 9, [24, 20, 14], (2, 5)),
    item(0x99, "ITEM_N_SUIT", "N Suit", "Normal Suit Armor", ItemCategory::Armor, 9, 9, [36, 30, 20], (3, 6)),
    item(0x9A, "ITEM_N_ARMOR", "N Armor", "Normal Heavy Armor", ItemCategory::Armor, 9, 9, [48, 40, 26], (4, 7)),
    item(0x9B, "ITEM_KV_VEST_1", "KV Vest 1", "Kevlar Light & Durable Vest", ItemCategory::Armor, 9, 9, [14, 10, 8], (2, 4)),
    item(0x9C, "ITEM_KV_PROTECTOR_1", "KV Protector 1", "Kevlar Protector", ItemCategory::Armor, 9, 9, [22, 16, 14], (2, 5)),
    item(0x9D, "ITEM_KV_JACKET_1", "KV Jacket 1", "Kevlar Jacket", ItemCategory::Armor, 9, 9, [32, 24, 18], (3, 6)),
    item(0x9E, "ITEM_KV_SUIT_1", "KV Suit 1", "Kevlar Tactical Suit", ItemCategory::Armor, 9, 9, [44, 34, 24], (4, 7)),
    item(0x9F, "ITEM_KV_ARMOR_1", "KV Armor 1", "Kevlar Full Body Armor", ItemCategory::Armor, 9, 9, [58, 44, 30], (5, 8)),
    item(0xA0, "ITEM_SP_VEST_1", "SP Vest 1", "Spectra Light & Very Durable Vest", ItemCategory::Armor, 9, 9, [18, 16, 12], (2, 5)),
    item(0xA1, "ITEM_SP_VEST_2", "SP Vest 2", "Spectra Vest Mk II", ItemCategory::Armor, 9, 9, [26, 22, 16], (3, 6)),
    item(0xA2, "ITEM_SP_PROTECTOR", "SP Protector", "Spectra Protector", ItemCategory::Armor, 9, 9, [34, 28, 20], (3, 6)),
    item(0xA3, "ITEM_SP_JACKET", "SP Jacket", "Spectra Jacket", ItemCategory::Armor, 9, 9, [42, 36, 26], (4, 7)),
    item(0xA4, "ITEM_SP_SUIT_1", "SP Suit 1", "Spectra Suit Mk I", ItemCategory::Armor, 9, 9, [52, 44, 32], (4, 7)),
    item(0xA5, "ITEM_SP_SUIT_2", "SP Suit 2", "Spectra Suit Mk II", ItemCategory::Armor, 9, 9, [62, 52, 38], (5, 8)),
    item(0xA6, "ITEM_SP_ARMOR_1", "SP Armor 1", "Spectra Armor Mk I", ItemCategory::Armor, 9, 9, [72, 60, 44], (5, 8)),
    item(0xA7, "ITEM_SP_ARMOR_2", "SP Armor 2", "Spectra Armor Mk II", ItemCategory::Armor, 9, 9, [82, 68, 50], (6, 9)),
    item(0xA8, "ITEM_SV_VEST_1", "SV Vest 1", "Survival Vest 1 (Extra Pockets)", ItemCategory::Armor, 9, 9, [16, 14, 10], (3, 5)),
    item(0xA9, "ITEM_SV_VEST_2", "SV Vest 2", "Survival Vest 2 (Extra Pockets)", ItemCategory::Armor, 9, 9, [24, 20, 15], (3, 6)),
    item(0xAA, "ITEM_SV_PROTECTOR", "SV Protector", "Survival Protector (Extra Pockets)", ItemCategory::Armor, 9, 9, [32, 26, 18], (4, 6)),
    item(0xAB, "ITEM_SV_JACKET", "SV Jacket", "Survival Jacket (Extra Pockets)", ItemCategory::Armor, 9, 9, [40, 34, 24], (4, 7)),
    item(0xAC, "ITEM_SV_SUIT_1", "SV Suit 1", "Survival Suit 1 (Extra Pockets)", ItemCategory::Armor, 9, 9, [50, 42, 30], (5, 8)),
    item(0xAD, "ITEM_SV_SUIT_2", "SV Suit 2", "Survival Suit 2 (Extra Pockets)", ItemCategory::Armor, 9, 9, [60, 50, 36], (5, 8)),
    item(0xAE, "ITEM_SV_ARMOR_1", "SV Armor 1", "Survival Armor 1 (Extra Pockets)", ItemCategory::Armor, 9, 9, [70, 58, 42], (6, 9)),
    item(0xAF, "ITEM_SV_ARMOR_2", "SV Armor 2", "Survival Armor 2 (Extra Pockets)", ItemCategory::Armor, 9, 9, [80, 66, 48], (6, 9)),
    item(0xB0, "ITEM_CR_VEST_1", "Cr Vest 1", "Critical-Evade Light & Tough Vest 1", ItemCategory::Armor, 9, 9, [20, 18, 22], (2, 5)),
    item(0xB1, "ITEM_CR_VEST_2", "Cr Vest 2", "Critical-Evade Light & Tough Vest 2", ItemCategory::Armor, 9, 9, [28, 24, 28], (3, 6)),
    item(0xB2, "ITEM_CR_PROTECTOR", "Cr Protector", "Critical-Evade Protector", ItemCategory::Armor, 9, 9, [36, 30, 34], (3, 6)),
    item(0xB3, "ITEM_CR_JACKET", "Cr Jacket", "Critical-Evade Jacket", ItemCategory::Armor, 9, 9, [46, 38, 42], (4, 7)),
    item(0xB4, "ITEM_CR_SUIT_1", "Cr Suit 1", "Critical-Evade Suit 1", ItemCategory::Armor, 9, 9, [56, 46, 50], (5, 8)),
    item(0xB5, "ITEM_CR_SUIT_2", "Cr Suit 2", "Critical-Evade Suit 2", ItemCategory::Armor, 9, 9, [66, 54, 58], (5, 8)),
    item(0xB6, "ITEM_CR_ARMOR_1", "Cr Armor 1", "Critical-Evade Armor 1", ItemCategory::Armor, 9, 9, [76, 62, 66], (6, 9)),
    item(0xB7, "ITEM_CR_ARMOR_2", "Cr Armor 2", "Critical-Evade Armor 2", ItemCategory::Armor, 9, 9, [88, 72, 75], (6, 10)),
    item(0xB8, "ITEM_B_VEST_1", "B Vest 1", "Bio Vest 1 (Strong vs Status)", ItemCategory::Armor, 9, 9, [18, 26, 12], (3, 5)),
    item(0xB9, "ITEM_B_VEST_2", "B Vest 2", "Bio Vest 2 (Strong vs Status)", ItemCategory::Armor, 9, 9, [26, 34, 18], (3, 6)),
    item(0xBA, "ITEM_B_PROTECTOR", "B Protector", "Bio Protector (Strong vs Status)", ItemCategory::Armor, 9, 9, [34, 42, 24], (4, 7)),
    item(0xBB, "ITEM_B_JACKET_1", "B Jacket 1", "Bio Jacket 1 (Strong vs Status)", ItemCategory::Armor, 9, 9, [44, 52, 30], (4, 7)),
    item(0xBC, "ITEM_B_JACKET_2", "B Jacket 2", "Bio Jacket 2 (Strong vs Status)", ItemCategory::Armor, 9, 9, [54, 62, 36], (5, 8)),
    item(0xBD, "ITEM_B_SUIT_1", "B Suit 1", "Bio Suit 1 (Strong vs Status)", ItemCategory::Armor, 9, 9, [64, 72, 42], (5, 8)),
    item(0xBE, "ITEM_B_SUIT_2", "B Suit 2", "Bio Suit 2 (Strong vs Status)", ItemCategory::Armor, 9, 9, [74, 82, 48], (6, 9)),
    item(0xBF, "ITEM_B_ARMOR", "B Armor", "Bio Full Armor (Strong vs Status)", ItemCategory::Armor, 9, 9, [86, 92, 56], (6, 10)),
    item(0xC0, "ITEM_CM_VEST_1", "Cm Vest 1", "Custom Vest 1 (High Def & Status)", ItemCategory::Armor, 9, 9, [28, 28, 20], (3, 6)),
    item(0xC1, "ITEM_CM_VEST_2", "Cm Vest 2", "Custom Vest 2 (High Def & Status)", ItemCategory::Armor, 9, 9, [38, 38, 28], (4, 7)),
    item(0xC2, "ITEM_CM_PROTECTOR", "Cm Protector", "Custom Protector (High Def & Status)", ItemCategory::Armor, 9, 9, [48, 48, 36], (4, 7)),
    item(0xC3, "ITEM_CM_JACKET", "Cm Jacket", "Custom Jacket (High Def & Status)", ItemCategory::Armor, 9, 9, [60, 60, 44], (5, 8)),
    item(0xC4, "ITEM_CM_SUIT_1", "Cm Suit 1", "Custom Suit 1 (High Def & Status)", ItemCategory::Armor, 9, 9, [72, 72, 52], (6, 9)),
    item(0xC5, "ITEM_CM_ARMOR_1", "Cm Armor 1", "Custom Armor 1 (High Def & Status)", ItemCategory::Armor, 9, 9, [84, 84, 60], (6, 9)),
    item(0xC6, "ITEM_CM_ARMOR_2", "Cm Armor 2", "Custom Armor 2 (High Def & Status)", ItemCategory::Armor, 9, 9, [96, 96, 70], (7, 10)),
    item(0xC7, "ITEM_CM_SUIT_2", "Cm Suit 2", "Ultimate Custom Suit (Chrysler Bldg)", ItemCategory::Armor, 9, 9, [105, 105, 80], (8, 10)),
    // Key Items & Trading Cards (0xC8..=0xEB)
    item(0xC8, "ITEM_THEATER_KEY", "Theater Key", "Dressing Room Key (Carnegie Hall)", ItemCategory::KeyItem, 20, 0, [0, 0, 0], (0, 0)),
    item(0xC9, "ITEM_REHEARSE_KEY", "Rehearse Key", "Rehearsal Room Key (Carnegie Hall)", ItemCategory::KeyItem, 20, 0, [0, 0, 0], (0, 0)),
    item(0xCA, "ITEM_ZOO_KEY", "Zoo Key", "Central Park Zoo Key", ItemCategory::KeyItem, 20, 0, [0, 0, 0], (0, 0)),
    item(0xCB, "ITEM_NARITA", "Narita", "Good Luck Charm (From Maeda)", ItemCategory::KeyItem, 21, 0, [0, 0, 0], (0, 0)),
    item(0xCC, "ITEM_MAYOKE", "Mayoke", "Helpful Good Luck Charm (From Maeda)", ItemCategory::KeyItem, 21, 0, [0, 0, 0], (0, 0)),
    item(0xCD, "ITEM_HAMAYA", "Hamaya", "Special Good Luck Charm (From Maeda)", ItemCategory::KeyItem, 21, 0, [0, 0, 0], (0, 0)),
    item(0xCE, "ITEM_FUSE_1", "Fuse 1", "Hospital Fuse 1", ItemCategory::KeyItem, 22, 0, [0, 0, 0], (0, 0)),
    item(0xCF, "ITEM_FUSE_2", "Fuse 2", "Hospital Fuse 2", ItemCategory::KeyItem, 22, 0, [0, 0, 0], (0, 0)),
    item(0xD0, "ITEM_FUSE_3", "Fuse 3", "Hospital Fuse 3", ItemCategory::KeyItem, 22, 0, [0, 0, 0], (0, 0)),
    item(0xD1, "ITEM_AUTOPSY_KEY", "Autopsy Key", "St. Francis Hospital Autopsy Key", ItemCategory::KeyItem, 20, 0, [0, 0, 0], (0, 0)),
    item(0xD2, "ITEM_BLUE_CARDKEY", "Blue Cardkey", "Blue Magnetic Cardkey (Hospital)", ItemCategory::KeyItem, 23, 0, [0, 0, 0], (0, 0)),
    item(0xD3, "ITEM_GREEN_CARDKEY", "Green Cardkey", "Green Magnetic Cardkey (Hospital)", ItemCategory::KeyItem, 23, 0, [0, 0, 0], (0, 0)),
    item(0xD4, "ITEM_PUMP_KEY", "Pump Key", "Chinatown Pumping Station Key", ItemCategory::KeyItem, 20, 0, [0, 0, 0], (0, 0)),
    item(0xD5, "ITEM_KLAMP_KEY", "Klamp Key", "Dr. Klamp's Office Key (Museum)", ItemCategory::KeyItem, 20, 0, [0, 0, 0], (0, 0)),
    item(0xD6, "ITEM_STORAGE_KEY", "Storage Key", "NYPD Weapons Storage Key", ItemCategory::KeyItem, 20, 0, [0, 0, 0], (0, 0)),
    item(0xD7, "ITEM_LOCKER_KEY", "Locker Key", "NYPD Locker Key", ItemCategory::KeyItem, 20, 0, [0, 0, 0], (0, 0)),
    item(0xD8, "ITEM_WEAPON_MEMO", "Weapon Memo", "Weapon Memo", ItemCategory::KeyItem, 19, 0, [0, 0, 0], (0, 0)),
    item(0xD9, "ITEM_MEDAL", "Medal", "Shiny Medal", ItemCategory::KeyItem, 21, 0, [0, 0, 0], (0, 0)),
    item(0xDA, "ITEM_ELEVATOR_KEY", "Elevator Key", "Elevator Key", ItemCategory::KeyItem, 20, 0, [0, 0, 0], (0, 0)),
    item(0xDB, "ITEM_GATE_KEY", "Gate Key", "Subway Gate Key", ItemCategory::KeyItem, 20, 0, [0, 0, 0], (0, 0)),
    item(0xDC, "ITEM_WAREHOUSE_KEY", "Warehouse Key", "Port Storage Key", ItemCategory::KeyItem, 20, 0, [0, 0, 0], (0, 0)),
    item(0xDD, "ITEM_GSP_T_CARD", "GSP T-Card", "Walther GSP Rare Trading Card", ItemCategory::TradingCard, 17, 0, [0, 0, 0], (0, 0)),
    item(0xDE, "ITEM_P38_T_CARD", "P38 T-Card", "Walther P38 Rare Trading Card", ItemCategory::TradingCard, 17, 0, [0, 0, 0], (0, 0)),
    item(0xDF, "ITEM_BHAWK_T_CARD", "Bhawk T-Card", "Ruger Blackhawk Rare Trading Card", ItemCategory::TradingCard, 17, 0, [0, 0, 0], (0, 0)),
    item(0xE0, "ITEM_KASUL_T_CARD", "Kasul T-Card", "Casull .454 Rare Trading Card", ItemCategory::TradingCard, 17, 0, [0, 0, 0], (0, 0)),
    item(0xE1, "ITEM_PPKS_T_CARD", "PPKS T-Card", "Walther PPK/S Rare Trading Card", ItemCategory::TradingCard, 17, 0, [0, 0, 0], (0, 0)),
    item(0xE2, "ITEM_M1_T_CARD", "M1 T-Card", "M1 Garand Rare Trading Card", ItemCategory::TradingCard, 17, 0, [0, 0, 0], (0, 0)),
    item(0xE3, "ITEM_MK5_T_CARD", "MK5 T-Card", "Sterling Mk5 Rare Trading Card", ItemCategory::TradingCard, 17, 0, [0, 0, 0], (0, 0)),
    item(0xE4, "ITEM_MP44_T_CARD", "MP44 T-Card", "StG 44 / MP44 Rare Trading Card", ItemCategory::TradingCard, 17, 0, [0, 0, 0], (0, 0)),
    item(0xE5, "ITEM_BAR_T_CARD", "BAR T-Card", "Browning BAR Rare Trading Card", ItemCategory::TradingCard, 17, 0, [0, 0, 0], (0, 0)),
    item(0xE6, "ITEM_MG42_T_CARD", "MG42 T-Card", "MG42 Rare Trading Card", ItemCategory::TradingCard, 17, 0, [0, 0, 0], (0, 0)),
    item(0xE7, "ITEM_M29_T_CARD", "M29 T-Card", "S&W M29 .44 Magnum Rare Trading Card", ItemCategory::TradingCard, 17, 0, [0, 0, 0], (0, 0)),
    item(0xE8, "ITEM_M73_T_CARD", "M73 T-Card", "Colt M1873 Rare Trading Card", ItemCategory::TradingCard, 17, 0, [0, 0, 0], (0, 0)),
    item(0xE9, "ITEM_TYPE38_T_CARD", "Type38 T-Card", "Arisaka Type 38 Rare Trading Card", ItemCategory::TradingCard, 17, 0, [0, 0, 0], (0, 0)),
    item(0xEA, "ITEM_TYPE3_T_CARD", "Type3 T-Card", "Type 3 Heavy MG Rare Trading Card", ItemCategory::TradingCard, 17, 0, [0, 0, 0], (0, 0)),
    item(0xEB, "ITEM_EAGLE_T_CARD", "Eagle T-Card", "Desert Eagle Rare Trading Card", ItemCategory::TradingCard, 17, 0, [0, 0, 0], (0, 0)),
    // Parasite Energy Techs (0xEC..=0xFF)
    item(0xEC, "ITEM_HEAL_1", "PE: Heal 1", "Parasite Energy Tech: Heal 1", ItemCategory::ParasiteEnergy, 24, 0, [0, 0, 0], (0, 0)),
    item(0xED, "ITEM_HEAL_2", "PE: Heal 2", "Parasite Energy Tech: Heal 2", ItemCategory::ParasiteEnergy, 24, 0, [0, 0, 0], (0, 0)),
    item(0xEE, "ITEM_HEAL_3", "PE: Heal 3", "Parasite Energy Tech: Heal 3", ItemCategory::ParasiteEnergy, 24, 0, [0, 0, 0], (0, 0)),
    item(0xEF, "ITEM_DETOX", "PE: Detox", "Parasite Energy Tech: Detox", ItemCategory::ParasiteEnergy, 24, 0, [0, 0, 0], (0, 0)),
    item(0xF0, "ITEM_MEDIC", "PE: Medic", "Parasite Energy Tech: Medic", ItemCategory::ParasiteEnergy, 24, 0, [0, 0, 0], (0, 0)),
    item(0xF1, "ITEM_BARRIER", "PE: Barrier", "Parasite Energy Tech: Barrier", ItemCategory::ParasiteEnergy, 24, 0, [0, 0, 0], (0, 0)),
    item(0xF2, "ITEM_ENERGY_SHOT", "PE: Energy Shot", "Parasite Energy Tech: Energy Shot", ItemCategory::ParasiteEnergy, 24, 0, [0, 0, 0], (0, 0)),
    item(0xF3, "ITEM_SCAN", "PE: Scan", "Parasite Energy Tech: Scan", ItemCategory::ParasiteEnergy, 24, 0, [0, 0, 0], (0, 0)),
    item(0xF4, "ITEM_SLOW", "PE: Slow", "Parasite Energy Tech: Slow", ItemCategory::ParasiteEnergy, 24, 0, [0, 0, 0], (0, 0)),
    item(0xF5, "ITEM_HASTE", "PE: Haste", "Parasite Energy Tech: Haste", ItemCategory::ParasiteEnergy, 24, 0, [0, 0, 0], (0, 0)),
    item(0xF6, "ITEM_CONFUSE", "PE: Confuse", "Parasite Energy Tech: Confuse", ItemCategory::ParasiteEnergy, 24, 0, [0, 0, 0], (0, 0)),
    item(0xF7, "ITEM_GENE_HEAL", "PE: Gene Heal", "Parasite Energy Tech: Gene Heal", ItemCategory::ParasiteEnergy, 24, 0, [0, 0, 0], (0, 0)),
    item(0xF8, "ITEM_PRERAISE", "PE: Preraise", "Parasite Energy Tech: Preraise", ItemCategory::ParasiteEnergy, 24, 0, [0, 0, 0], (0, 0)),
    item(0xF9, "ITEM_MITO14", "PE: Mito14", "Parasite Energy tech dummy (0xF9)", ItemCategory::ParasiteEnergy, 24, 0, [0, 0, 0], (0, 0)),
    item(0xFA, "ITEM_MITO15", "PE: Mito15", "Parasite Energy tech dummy (0xFA)", ItemCategory::ParasiteEnergy, 24, 0, [0, 0, 0], (0, 0)),
    item(0xFB, "ITEM_MITO16", "PE: Mito16", "Parasite Energy tech dummy (0xFB)", ItemCategory::ParasiteEnergy, 24, 0, [0, 0, 0], (0, 0)),
    item(0xFC, "ITEM_MITO17", "PE: Mito17", "Parasite Energy tech dummy (0xFC)", ItemCategory::ParasiteEnergy, 24, 0, [0, 0, 0], (0, 0)),
    item(0xFD, "ITEM_MITO18", "PE: Mito18", "Parasite Energy tech dummy (0xFD)", ItemCategory::ParasiteEnergy, 24, 0, [0, 0, 0], (0, 0)),
    item(0xFE, "ITEM_FULL_RECOVER_FE", "PE: Full Recover", "Parasite Energy Tech: Full Recover", ItemCategory::ParasiteEnergy, 24, 0, [0, 0, 0], (0, 0)),
    item(0xFF, "ITEM_LIBERATE", "PE: Liberate", "Parasite Energy Tech: Liberate", ItemCategory::ParasiteEnergy, 24, 0, [0, 0, 0], (0, 0)),
];

pub fn item_metadata(item_id: u8) -> &'static ItemMetadata {
    &ITEM_TABLE[item_id as usize]
}

/// Returns the short save-header location name (matching `Text_GetMapLocationName` in `src/main/text/tables.c`).
pub fn save_header_location_name(map_number: i16, is_clear_data: bool) -> &'static str {
    if is_clear_data || map_number < 0 {
        return "Clear Data";
    }
    match map_location_id(map_number) {
        1 => "Theater",
        2 => "Sewer",
        3 => "N.Y.P.D.",
        4 => "Central Park",
        5 => "Soho",
        6 => "Hospital",
        7 => "Chinatown",
        8 => "Sewer2",
        9 => "Subway",
        10 => "Museum",
        11 => "Warehouse",
        12 => "Carrier",
        13 => "Liberty",
        14 => "Cruiser",
        15 => "Clear Data",
        _ => "",
    }
}

/// Returns a descriptive human-readable location label for any map number (`1..=437`),
/// including the post-game Chrysler Building (maps 288-318, 382-424, 434-437).
pub fn map_location_descriptive(map_number: i16) -> &'static str {
    if map_number < 0 {
        return "Clear Data (EX Round)";
    }
    match map_location_id(map_number) {
        1 => "Carnegie Hall (Theater)",
        2 => "Sewer (Under Theater)",
        3 => "NYPD 28th Precinct",
        4 => "Central Park",
        5 => "Soho",
        6 => "St. Francis Hospital",
        7 => "Chinatown",
        8 => "Chinatown Sewer (Sewer2)",
        9 => "Subway",
        10 => "American Museum of Natural History",
        11 => "Pier No. 4 Warehouse",
        12 => "Aircraft Carrier",
        13 => "Statue of Liberty",
        14 => "Cruiser",
        15 => "Clear Data",
        _ => {
            if matches!(map_number, 288..=318 | 382..=424 | 434..=437) {
                "Chrysler Building (EX Post-Game)"
            } else {
                "Cutscene / Special Map"
            }
        }
    }
}

/// Exact mapping from `docs/ROOMS.md` (`g_ResidentDataPack.directory.mapLocationIds`).
pub fn map_location_id(map_number: i16) -> u8 {
    match map_number {
        1..=25 => 1,
        26..=34 => 2,
        35..=40 => 0,
        41..=56 => 3,
        57..=89 => 4,
        90..=97 => 5,
        98..=124 => 3,
        125..=160 => 6,
        161..=165 => 7,
        166..=181 => 8,
        182..=190 => 9,
        191..=259 => 10,
        260..=270 => 11,
        271 => 12,
        272..=273 => 13,
        274..=287 => 14,
        288..=318 => 0,
        319..=332 => 6,
        333 => 11,
        334 => 2,
        335 => 0,
        336..=338 => 3,
        339 => 6,
        340..=346 => 10,
        347 => 0,
        348 => 2,
        349 => 14,
        350..=355 => 0,
        356..=357 => 3,
        358 => 4,
        359 => 1,
        360..=363 => 0,
        364 => 10,
        365..=366 => 0,
        367..=368 => 1,
        369..=370 => 3,
        371 => 10,
        372..=373 => 1,
        374..=375 => 4,
        376 => 3,
        377..=378 => 1,
        379 => 10,
        380..=381 => 6,
        382..=424 => 0,
        425..=430 => 14,
        431 => 1,
        432 => 13,
        433 => 8,
        434..=437 => 0,
        _ => 0,
    }
}

pub const MAP_TELEPORT_PRESETS: [(i16, i16, &str); 16] = [
    (5, 1, "Day 1 - Carnegie Hall (Theater Save Room, m005)"),
    (27, 1, "Day 1 - Theater Sewer (m027)"),
    (45, 2, "Day 2 - NYPD 17th Precinct (m045)"),
    (59, 2, "Day 2 - Central Park Entrance (m059)"),
    (90, 3, "Day 3 - Soho Gun Shop (m090)"),
    (100, 3, "Day 3 - NYPD Attack Revisit (m100)"),
    (126, 4, "Day 4 - St. Francis Hospital (m126)"),
    (162, 5, "Day 5 - Chinatown (m162)"),
    (167, 5, "Day 5 - Chinatown Sewer (m167)"),
    (186, 5, "Day 5 - Subway (m186)"),
    (197, 5, "Day 5 - Museum of Natural History (m197)"),
    (242, 5, "Day 5 - Museum T-Rex / Klamp Area (m242)"),
    (263, 5, "Day 5 - Pier No. 4 Warehouse (m263)"),
    (272, 6, "Day 6 - Statue of Liberty (m272)"),
    (275, 6, "Day 6 - Cruiser (m275)"),
    (291, 2, "EX Game - Chrysler Building Lobby (m291)"),
];
