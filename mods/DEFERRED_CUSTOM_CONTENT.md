# Deferred custom content ideas

Recorded 2026-10-25. User instruction: keep these as notes only; do not implement them. These are candidates for future discussion, not approved features.

| Candidate | Original reason for dropping or restricting it | Possible native-app follow-up |
|---|---|---|
| Lucky Charm | Doubled rare-drop pity progress for 10 won battles. Item ID 0x19 was reassigned to Panacea; Lucky Find armor provided similar functionality. | Consider restoring it under a separate custom item identity, without replacing Panacea. Revisit overlap with Lucky Find before deciding. |
| Physical mod chips | Only two spare placeholder item IDs were available. The Catalog also avoided normal inventory clutter. | Expanded item definitions could support chips, but the inventory-clutter objection still applies. Keep the current Catalog unless explicitly reconsidered. |
| New guns and armor | Set aside because new pieces needed models and stats; this was not solely an item-ID limit. | Could be revisited with explicit equipment definitions, assets, balance and save support. No particular new piece has been approved. |
| Recycler armor effect | Leftover battle rewards would become Junk when the bag was full. Dropped by the user's design decision, not a technical limit. | Do not restore without a new design decision. |
| Stat-booster fusion | Proposed recipes lost permanent stat value and saved no battle turns. | Do not restore merely because the native app is more extensible; recipes need a useful design first. |

## Technical considerations for a future decision

The native app still retains original game formats: one-byte item IDs, five-bit effect numbers in equipment modifier bytes, and original save layouts. Native execution makes extending these possible, but does not remove the limits automatically.

A modular custom-item registry with explicit names, icons, use effects and versioned save support is a possible prerequisite. Evaluate migration of existing saves and compatibility with inventory, rewards, shops, storage and sorting before expanding IDs. This infrastructure is also deferred and is not authorized by this note.

Smoke Grenade and Panacea are already implemented; this note does not request changes to them.

## Historical sources

- D:/Recomp Games/psxrecomp-decomp-bridge/findings/pe-custom-items-design-2026-10-11.md (C6-C11, C14)
- D:/Recomp Games/psxrecomp-decomp-bridge/findings/pe-parasite-energy-rebalance-2026-10-15.md (Lucky Charm removed; item 0x19 becomes Panacea)
- D:/Recomp Games/psxrecomp-decomp-bridge/findings/pe-loot-overhaul-design-2026-10-11.md (section 6.2: physical mod-chip alternatives and inventory clutter)
