# Roguelike Expansion & Design Document

This document contains every idea and technical note for expanding the Funcy roguelike into a richer dungeon crawler.

---

## 1. Dungeon & Map Variety

### Biomes (per-depth themes)
- **Catacombs**: narrow rooms, long corridors; skeletons; more traps.
- **Caves**: cellular rooms (organic), lakes; slimes, bats; fewer doors.
- **Ice Halls**: wide rooms, slippery floor (player slides until wall).
- **Fungal Groves**: spores (poison clouds), shrooms (heals/poisons).
- **Forge**: lava pockets (fire damage), golems; weapon loot up.

**Tech notes**
- Add `THEME` to level build; adjust monster tables + item bias per theme.
- Tile set per theme (wall/floor glyphs), e.g., `THEME_WALL`, `THEME_FLOOR`.

### Secret Doors & Hidden Rooms
- Some walls are “secret” and become doors after `search`.
- On generate: mark `grid[y][x] = '#'` + metadata `secret=true`.

**Gameplay**
- Command: `search` (1 turn). If adjacent secret: reveal as `+`.
- Optional: Perception stat increases chance to find without searching.

### Traps
- Types: spikes (hp loss), darts (ranged), gas (poison/blur), pits (fall 1 level).
- Visible only on `search` or with Perception check; disarm with a kit.

**Tech notes**
- `traps` array: `{x,y,type,armed:true}`; trigger on step if armed.
- Render: hidden unless detected; when visible show `^`.

### Multi-Floor Objectives
- Retrieve a relic on depth N, return to surface; or escort an NPC.
- Keep objective state in `game.fy` (`&objective = {...}`).

---

## 2. Player Progression

### Leveling (XP → level)
- XP from kills; thresholds: 10, 30, 60, 100, ... (Δ grows ~20–30%).
- On level-up: +HP, +ATK or choose a perk (see Skills).

**Data**
- `player.xp`, `player.level`.
- Function `gainXP(n)`, `onLevelUp()` with choice menu.

### Attributes
- STR (melee dmg), AGI (evasion/accuracy), INT (magic potency).
- Start: 1/1/1; gain +1 on level-up or via tomes.

### Skills/Perks (lightweight)
- **Cleave**: hit adjacent second enemy (cooldown).
- **Dash**: move 2 tiles ignoring ZoC (cooldown).
- **Second Wind**: once/floor heal small amount when <25% HP.
- **Treasure Sense**: reveals nearby items on enter.

**Tech notes**
- `player.skills = {name => level/cd}`.
- Simple cooldown tick after each player turn.

### UI
- `k` to open “Skills/Perks”.
- Level-up prompt: offer 2–3 randomized perk choices.

---

## 3. Items & Equipment

### Slots
- Weapon, Armor, Ring (x2), Consumables (potions/scrolls).

### Armor
- Reduces incoming damage: `dmg = max(1, dmg - armor_val)`.

### Magic Items
- **Potions**: heal, speed, invisibility.
- **Scrolls**: fireball (AoE), blink (teleport small radius), identify.
- **Rings**: regen, defense, luck.

### Identification
- Unknown items show as “Murky Potion” until identified/used.
- Scroll of Identify reveals type for one item.

### Curses
- Some gear is cursed: +ATK but causes bleed / cannot unequip.
- Remove with Scroll of Remove Curse.

### Rarity
- Common/Uncommon/Rare/Unique tables with weighted rolls.

**Tech notes**
- `item.kind ∈ {potion, scroll, weapon, armor, ring}`.
- `identified: bool`, `cursed: bool`.
- Add `equip/unequip` flows; weapon/armor stats show in `stats`.

---

## 4. Monsters & AI

### Behaviors
- **Slime**: on hit at low HP, splits into two smaller slimes.
- **Archer**: maintains distance ≥3; shoots if LoS.
- **Mimic**: spawns disguised as item; reveals when adjacent or picked.
- **Thief**: steals item on hit; tries to flee (despawns in a corridor).
- **Necromancer**: summons skeletons when below 50% HP.
- **Golem**: slow, high armor; weak to shock.

### Factions
- Undead vs Beasts vs Humanoids; some fight each other on sight.

### Bosses (every 3–5 floors)
- **Spider Matriarch**: webs (root), spawns small spiders.
- **Frost Knight**: cone slow, charges in lines.

**Tech notes**
- `monster.ai` enum; per-turn `act()` uses behavior tables.
- Basic ranged LoS (Bresenham or straight path check).
- Summons: enqueue child monsters near the boss.

---

## 5. Environment Interaction

### Light & Darkness
- Radius FOV (fast mode available).
- Optional: add “explored memory” (`seen[y][x] = true` once visible).

### Destructible Terrain
- Wall HP; bombs or pickaxe reduce HP to 0 → becomes floor.
- Some walls indestructible (theme-dependent).

### Doors & Keys
- Closed doors `+` block LoS, open `/` allow.
- Locked doors need matching key item (or force open: noise attracts enemies).

### Water/Lava/Ice
- Water slows; lava damages; ice slides until obstacle.

### Interactables
- Altars, forges, fountains: tradeoffs (sacrifice HP for enchants, etc.).

**Tech notes**
- Tile metadata map `{ "solid":bool, "opaque":bool, "hp":int?, "tag":string? }`.
- Actions: `openDoor(x,y)`, `useForge(x,y)` with prompts.

---

## 6. Meta Systems

### Permadeath & Score
- Score = depth * K + kills * a + gold * b + quests * c.
- High-scores table saved to a file (`scores.txt`).

### Shops (Between Floors)
- Chance to roll a “rest floor” with a shop.
- Gold drops from monsters and chests.
- Services: heal, ID items, remove curse.

### Quests
- “Bring 3 mushrooms,” “Slay the Necromancer,” “Find the Lost Ring.”
- Rewards: gold, rare items, permanent +1 stat.

### Seeds & Daily Runs
- Deterministic seed for reproducible maps.
- “Daily Run”: fixed seed per calendar day.

**Tech notes**
- Add `gold` field; shop menu (buy/sell/leave).
- Save/load minimal meta (scores/seed), keep runs perma for vibe.

---

## 7. Backlog & Roadmap

### Milestone A — Core Depth (1–2 days)
- [ ] Item slots (weapon/armor/rings), equip UI.
- [ ] XP + level; on-level perk picker (2–3 options).
- [ ] Shops on rest floors; gold drops.

### Milestone B — Variety & Challenge (2–3 days)
- [ ] Themes: Catacombs, Caves, Forge.
- [ ] 6–8 new monsters with behaviors (archer, slime split, mimic).
- [ ] Traps (spike, dart, gas); search action.

### Milestone C — Systems Polish (2–3 days)
- [ ] Identification + curses.
- [ ] Doors & keys; locked rooms with treasure.
- [ ] Boss floor every 3 depths.

### Stretch
- [ ] Destructible terrain + bombs.
- [ ] Quests (2–3 templates).
- [ ] Daily run (seeded).

**Priorities**
1. Equipment + XP (unlocks a lot of pacing control).
2. Monster variety (gameplay mix).
3. Identification/curses (roguelike spice).

---

## 8. Controls & UI

### Core
- Move: WASD
- Pickup: f
- Inventory: i (use/equip)
- Stats: r
- Toggle FOV: v
- Quit: q

### Proposed Additions
- Search: e (1 turn; reveal traps/secret doors nearby)
- Skills: k
- Drop: d (from inventory)
- Throw: t (targeting UI later)

### UI Notes
- Keep one-print frame render (optimized already).
- Show status line with buffs/debuffs (poison, slow, regen).
- Optional ANSI color for items/monsters.

---

## 9. Implementation Notes (Funcy)

### Equipment
- Player: `&weapon`, `&armor`, `&rings = []`; derive `atk/def` from equipped.
- Inventory actions: `equip/unequip`; armor reduces damage in `attackMonster()/monsterTurn()`.

### XP & Levels
- Player: `&xp`, `&level = 1`.
- `gainXP(n)` → while `xp >= threshold(level)`: `level += 1; onLevelUp();`
- `onLevelUp()` prompt: +HP or perk; store perks in `player.skills`.

### Traps/Secrets
- Arrays: `&traps = []`, `&secrets = []` with `{x,y,type}`.
- On step: check at O(1) using a key map `"x,y" -> record"`.
- Search action toggles visibility flags.

### Shops
- On entering rest floor: open shop menu.
- Shop inventory generated from weighted table based on depth.

### Save meta
- On death: append to `scores.txt` (depth, kills, score).

---

## 10. Test Plan

### Unit-ish (by function)
- Map gen: rooms non-overlap; corridors connect; exit reachable.
- Spawn: no monsters/items on exit; no stacking items.
- Combat: armor reduces dmg; weapon bonus applied; death triggers.
- Inventory: pickup/use/equip/unequip flows.
- FOV: toggle on/off; radius reveals correctly.

### Integration
- Depth progression: heal on descent; difficulty scales.
- Shop floor generation and purchase flows.
- Identification: unknown until use/scroll; curses apply and removable.

### Performance (target: <200ms frame)
- Render: single print; join lines; O(1) overlays with maps.
- FOV: radius-only fast mode OK; optional occlusion later (shadowcasting).



## My Ideas
- re-implement the healing between levels as an ability or something.