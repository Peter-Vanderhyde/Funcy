# game.fy — gameplay systems and loop logic (optimized + color + memory + XP/Level + shops + trader)
import "config.fy";
import "map.fy";
import "entities.fy";

# Busy-wait delay using built-in time() which returns ms.
func delayMs(ms) {
    start = time();
    while time() - start < ms { }   # simple, portable
}

class Game {
    func &Game() {
        &grid = [];
        &rooms = [];
        &monsters = [];      # list of Monster
        &items = [];         # list of dicts: { "x": int, "y": int, "glyph": string, "it": Item }
        &player = Player(0, 0);
        &message = "";
        &dead = false;

        &fov_enabled = FOV_ENABLED;
        &color_enabled = COLOR_ENABLED;

        &seen = [];          # map memory (bool[y][x])
        &exit = [0,0];       # [x, y]
        &trader = Null;      # { "x":int, "y":int, "stock":[{ "it":Item, "price":int }] } or Null
        
        &current_theme = "catacombs";  # Current theme for this level



        &buildLevel(1);
    }

    

    # ------------ UTILITIES ------------

    func &getThemeInfo() {
        if &current_theme == "caves" {
            return "Natural caves with organic formations and cave-dwelling monsters";
        } elif &current_theme == "catacombs" {
            return "Stone catacombs with narrow corridors and undead creatures";
        } elif &current_theme == "forge" {
            return "Fiery forge with wide chambers and fire-based enemies";
        } else {
            return "Unknown theme";
        }
    }

    func &buildLineFromRow(row) {
        s = "";
        for i = 0, i < length(row), i += 1 { s = s + row[i]; }
        return s;
    }

    func &applyColorVisibleTile(ch) {
        if not &color_enabled { return ch; }
        if ch == "#" { 
            # Apply theme-based wall colors
            if &current_theme == "catacombs" { return C_CATACOMBS_WALL + "#" + C_RESET; }
            elif &current_theme == "caves" { return C_CAVES_WALL + "#" + C_RESET; }
            elif &current_theme == "forge" { return C_FORGE_WALL + "#" + C_RESET; }
            else { return C_WALL + "#" + C_RESET; }
        }
        if ch == "." { 
            # Apply theme-based floor colors
            if &current_theme == "catacombs" { return C_CATACOMBS_FLOOR + "." + C_RESET; }
            elif &current_theme == "caves" { return C_CAVES_FLOOR + "." + C_RESET; }
            elif &current_theme == "forge" { return C_FORGE_FLOOR + "." + C_RESET; }
            else { return C_FLOOR + "." + C_RESET; }
        }
        if ch == ">" { return C_EXIT  + ">" + C_RESET; }
        if ch == "@" { return C_PLAYER+ "@" + C_RESET; }
        if ch == "!" { return C_POTION+ "!" + C_RESET; }
        if ch == ")" { return C_WEAPON+ ")" + C_RESET; }
        if ch == "[" { return C_WEAPON+ "[" + C_RESET; }  # reuse cyan for armor
        if ch == "=" { return C_POTION+ "=" + C_RESET; }  # reuse magenta for ring
        if ch == "$" { return C_GOLD + "$" + C_RESET; }   # gold piles
        # Monster letters - check if it's a monster glyph by looking at monster positions
        # This is more accurate than guessing based on character type
        for i = 0, i < length(&monsters), i += 1 {
            m = &monsters[i];
            if m.glyph == ch { 
                return C_MON + ch + C_RESET;
            }
        }
        if ch == "T" { return C_TRADER + "T" + C_RESET; }
        return ch;
    }

    func &passable(x, y) {
        if not inBounds(x, y) { return false; }
        tile = &grid[y][x];
        return tile == "." or tile == ">";
    }

    func &monsterAt(x, y) {
        for i = 0, i < length(&monsters), i += 1 {
            m = &monsters[i];
            if m.x == x and m.y == y { return m; }
        }
        return Null;
    }

    func &itemIndexAt(x, y) {
        for i = 0, i < length(&items), i += 1 {
            rec = &items[i];
            if rec["x"] == x and rec["y"] == y { return i; }
        }
        return -1;
    }

    # ---- FAST FOV: radius only, no LOS occlusion ----
    func &computeVisibilityRadius() {
        vis = [];
        for y = 0, y < MAP_H, y += 1 {
            row = [];
            for x = 0, x < MAP_W, x += 1 { row.append(false); }
            vis.append(row);
        }
        R = FOV_RADIUS;
        px = &player.x; py = &player.y;
        R2 = R * R;

        miny = py - R; if miny < 0 { miny = 0; }
        maxy = py + R; if maxy >= MAP_H { maxy = MAP_H - 1; }
        minx = px - R; if minx < 0 { minx = 0; }
        maxx = px + R; if maxx >= MAP_W { maxx = MAP_W - 1; }

        for y = miny, y <= maxy, y += 1 {
            dy = y - py;
            for x = minx, x <= maxx, x += 1 {
                dx = x - px;
                if dx*dx + dy*dy <= R2 {
                    vis[y][x] = true;
                }
            }
        }
        return vis;
    }

    # ---------- Leveling helpers ----------
    func &xpGainFromMonster(m) {
        # Simple: base + monster attack, lightly scaled by depth
        val = XP_PER_KILL_BASE + m.atk + (&player.depth // 3);
        return val;
    }

    func &gainXP(n) {
        &player.xp = &player.xp + n;
        # Handle multiple level-ups if accrued XP is large
        while &player.xp >= &player.xpThresholdFor(&player.level) {
            &player.xp = &player.xp - &player.xpThresholdFor(&player.level);
            &player.level = &player.level + 1;
            &onLevelUp();
        }
    }

    func &onLevelUp() {
        print("\n== Level Up! You are now level " + str(&player.level) + " ==");
        options = [
            "Vitality (+2 Max HP, heal 2)",
            "Power (+1 Base ATK)",
            "Guard (+1 DEF)"
        ];
        for i = 0, i < length(options), i += 1 {
            print("  [" + str(i + 1) + "] " + options[i]);
        }
        choice = -1;
        while true {
            inp = input("Choose a perk (1-3): ").strip();
            if not inp.isDigit() { print("Please enter 1, 2, or 3."); continue; }
            choice1 = int(inp);
            if choice1 >= 1 and choice1 <= 3 { choice = choice1 - 1; break; }
            print("Invalid choice.");
        }

        if choice == 0 {
            &player.hp_max = &player.hp_max + 2;
            &player.hp = &player.hp + 2;
            if &player.hp > &player.hp_max { &player.hp = &player.hp_max; }
            print("You feel heartier. Max HP +2.");
        } elif choice == 1 {
            &player.atk = &player.atk + 1;
            print("Your strikes hit harder. Base ATK +1.");
        } else {
            &player.perk_def_bonus = &player.perk_def_bonus + 1;
            print("You brace yourself better. DEF +1.");
        }
        _ = input("(press Enter) ");
    }

    # ---------- Shops / Trader ----------
    func &priceFor(it) {
        if it.kind == "potion" { return PRICE_POTION_BASE + (it.stats - 5); }
        elif it.kind == "weapon" { return it.stats * PRICE_WEAPON_PER_ATK; }
        elif it.kind == "armor"  { return it.stats * PRICE_ARMOR_PER_DEF; }
        elif it.kind == "ring"   { return PRICE_RING_BASE; }
        return 10;
    }

    func &generateShopStock() {
        stock = [];  # list of { "it":Item, "price":int }
        for i = 0, i < SHOP_OFFER_COUNT, i += 1 {
            roll = randInt(0, 9);
            itm = Null;
            if roll < 3 {
                heal = randInt(5, 10) + (&player.depth // 3);   # tiny depth bump
                # Theme-specific potion names
                potion_name = "Red Potion";
                if &current_theme == "catacombs" { potion_name = "Soul Essence"; }
                elif &current_theme == "caves" { potion_name = "Cave Dew"; }
                elif &current_theme == "forge" { potion_name = "Molten Elixir"; }
                itm = Item(potion_name, "potion", heal);
            } elif roll < 6 {
                pwr = randInt(1, 3) + (&player.depth // 3);
                # Theme-specific weapon names
                wnames = [];
                if &current_theme == "catacombs" {
                    wnames = ["Bone Dagger", "Spectral Blade", "Soul Reaper"];
                } elif &current_theme == "caves" {
                    wnames = ["Crystal Shard", "Fungus Staff", "Cave Pick"];
                } elif &current_theme == "forge" {
                    wnames = ["Fire Brand", "Molten Hammer", "Ash Cleaver"];
                } else {
                    wnames = ["Dagger", "Shortsword", "Club"];
                }
                itm = Item(randChoice(wnames), "weapon", pwr);
            } elif roll < 8 {
                defv = randInt(1, 2) + (&player.depth // 4);
                # Theme-specific armor names
                anames = [];
                if &current_theme == "catacombs" {
                    anames = ["Bone Armor", "Spectral Robes", "Soul Plate"];
                } elif &current_theme == "caves" {
                    anames = ["Crystal Mail", "Fungus Hide", "Cave Leather"];
                } elif &current_theme == "forge" {
                    anames = ["Fire Forged", "Molten Plate", "Ash Armor"];
                } else {
                    anames = ["Cloth Armor", "Leather Armor", "Chain Shirt"];
                }
                itm = Item(randChoice(anames), "armor", defv);
            } else {
                rnames = ["Ring of Power", "Ring of Defense"];
                itm = Item(randChoice(rnames), "ring", 0);
                if itm.name == "Ring of Power" { itm.subkind = "power"; }
                else { itm.subkind = "defense"; }
            }
            price = &priceFor(itm);
            stock.append({ "it": itm, "price": price });
        }
        return stock;
    }

    func &shopMenu(stock, title) {
        while true {
            print("\e[H\e[J");
            print("== " + title + " ==");
            print("Gold: " + str(&player.gold));
            print("Items (choose 1.." + str(length(stock)) + ", or blank to leave):");
            for i = 0, i < length(stock), i += 1 {
                rec = stock[i];
                line = "  [" + str(i + 1) + "] " + rec["it"].toString() + "  - " + str(rec["price"]) + "g";
                print(line);
            }
            choice = input("> ").strip();
            if choice == "" { break; }
            if not choice.isDigit() { print("Please enter a number."); _ = input("(Enter) "); continue; }
            idx1 = int(choice);
            if idx1 < 1 or idx1 > length(stock) { print("Out of range."); _ = input("(Enter) "); continue; }
            idx = idx1 - 1;
            rec2 = stock[idx];
            cost = rec2["price"];
            if &player.gold < cost { print("Not enough gold."); _ = input("(Enter) "); continue; }
            &player.gold = &player.gold - cost;
            &player.inventory.append(rec2["it"]);
            print("Bought: " + rec2["it"].toString() + " for " + str(cost) + "g.");
            _ = input("(Enter) ");
        }
    }

    func &spawnTraderMaybe() {
        &trader = Null;
        roll = randInt(1, 100);
        if roll > TRADER_SPAWN_CHANCE { return; }

        # try a few random floors
        for tries = 0, tries < 200, tries += 1 {
            r = randChoice(&rooms);
            rx = randInt(r["x"], r["x"] + r["w"] - 1);
            ry = randInt(r["y"], r["y"] + r["h"] - 1);
            if &grid[ry][rx] == "."
               and not (&player.x == rx and &player.y == ry)
               and (&itemIndexAt(rx, ry) == -1)
               and not &monsterAt(rx, ry) {
                stock = &generateShopStock();
                &trader = { "x": rx, "y": ry, "stock": stock };
                break;
            }
        }
    }

    func &playerAdjacentToTrader() {
        if &trader == Null { return false; }
        dx = &player.x - &trader["x"]; if dx < 0 { dx = -dx; }
        dy = &player.y - &trader["y"]; if dy < 0 { dy = -dy; }
        return (dx + dy) == 1 or (dx == 0 and dy == 0);
    }

    func &talkToTrader() {
        if not &playerAdjacentToTrader() {
            &message = "No trader nearby.";
            return;
        }
        &shopMenu(&trader["stock"], "Trader");
    }

    # ---------- LEVEL BUILD / LOOP ----------

    func &buildLevel(depth) {
        &player.depth = depth;
        
        # Determine theme based on depth
        &current_theme = getThemeForDepth(depth);

        gen = generateMap(&current_theme);
        &grid  = gen["grid"];
        &rooms = gen["rooms"];
        &exit  = gen["exit"];                # [x, y]
        &player.x = gen["player_start"][0];
        &player.y = gen["player_start"][1];

        # clear entities
        &monsters = [];
        &items = [];

        # reset map memory
        &seen = [];
        for y = 0, y < MAP_H, y += 1 {
            row = [];
            for x = 0, x < MAP_W, x += 1 { row.append(false); }
            &seen.append(row);
        }

        exitX = &exit[0];
        exitY = &exit[1];

        # spawn monsters — ONLY on floor ".", never on exit ">"
        mcount = MONSTERS_PER_FLOOR + (depth - 1);
        for i = 0, i < mcount, i += 1 {
            r = randChoice(&rooms);
            rx = randInt(r["x"], r["x"] + r["w"] - 1);
            ry = randInt(r["y"], r["y"] + r["h"] - 1);
            if (&grid[ry][rx] == ".")
               and not (&player.x == rx and &player.y == ry)
               and not (rx == exitX and ry == exitY)
               and not &monsterAt(rx, ry) {
                base = 3 + (depth - 1);
                hp = randInt(base, base + 4);
                atk = randInt(1 + (depth // 2), 2 + (depth // 2));
                
                # Theme-specific monster names
                names = [];
                if &current_theme == "catacombs" {
                    names = ["Skeleton", "Zombie", "Ghost", "Wraith", "Specter"];
                } elif &current_theme == "caves" {
                    names = ["Slime", "Bat", "Spider", "Cave Rat", "Fungus"];
                } elif &current_theme == "forge" {
                    names = ["Golem", "Fire Imp", "Lava Spawn", "Ash Demon", "Ember Beast"];
                } else {
                    names = ["Rat", "Bat", "Slime", "Goblin", "Spider"];
                }
                
                m = Monster(randChoice(names), rx, ry, hp, atk);
                &monsters.append(m);
            }
        }

        # spawn items — floor only, never on exit; include armor/rings sometimes
        for i = 0, i < ITEMS_PER_FLOOR, i += 1 {
            r = randChoice(&rooms);
            rx = randInt(r["x"], r["x"] + r["w"] - 1);
            ry = randInt(r["y"], r["y"] + r["h"] - 1);
            if (&grid[ry][rx] == ".")
               and not (&player.x == rx and &player.y == ry)
               and not (rx == exitX and ry == exitY)
               and (&itemIndexAt(rx, ry) == -1)
               and not &monsterAt(rx, ry) {

                roll = randInt(0, 9);  # 0..9
                itm = Null;
                glyph = "!";

                if roll < 3 {
                    heal = randInt(5, 10);
                    # Theme-specific potion names
                    potion_name = "Red Potion";
                    if &current_theme == "catacombs" { potion_name = "Soul Essence"; }
                    elif &current_theme == "caves" { potion_name = "Cave Dew"; }
                    elif &current_theme == "forge" { potion_name = "Molten Elixir"; }
                    itm = Item(potion_name, "potion", heal);
                    glyph = "!";
                } elif roll < 6 {
                    pwr = randInt(1, 3);
                    # Theme-specific weapon names
                    wnames = [];
                    if &current_theme == "catacombs" {
                        wnames = ["Bone Dagger", "Spectral Blade", "Soul Reaper"];
                    } elif &current_theme == "caves" {
                        wnames = ["Crystal Shard", "Fungus Staff", "Cave Pick"];
                    } elif &current_theme == "forge" {
                        wnames = ["Fire Brand", "Molten Hammer", "Ash Cleaver"];
                    } else {
                        wnames = ["Dagger", "Shortsword", "Club"];
                    }
                    itm = Item(randChoice(wnames), "weapon", pwr);
                    glyph = ")";
                } elif roll < 8 {
                    defv = randInt(1, 2);
                    # Theme-specific armor names
                    anames = [];
                    if &current_theme == "catacombs" {
                        anames = ["Bone Armor", "Spectral Robes", "Soul Plate"];
                    } elif &current_theme == "caves" {
                        anames = ["Crystal Mail", "Fungus Hide", "Cave Leather"];
                    } elif &current_theme == "forge" {
                        anames = ["Fire Forged", "Molten Plate", "Ash Armor"];
                    } else {
                        anames = ["Cloth Armor", "Leather Armor", "Chain Shirt"];
                    }
                    itm = Item(randChoice(anames), "armor", defv);
                    glyph = "[";
                } elif roll < 8 {
                    rnames = ["Ring of Power", "Ring of Defense"];
                    itm = Item(randChoice(rnames), "ring", 0);
                    if itm.name == "Ring of Power" { itm.subkind = "power"; }
                    else { itm.subkind = "defense"; }
                    glyph = "=";
                } else {
                    # Gold pile (roll 8-9)
                    gold_amount = randInt(5, 15) + (&player.depth * 2);
                    itm = Item("Gold Pile", "gold", gold_amount);
                    glyph = "$";
                }

                &items.append({ "x": rx, "y": ry, "glyph": glyph, "it": itm });
            }
        }

        # maybe spawn a trader in this level
        &spawnTraderMaybe();

        &message = "You descend to depth " + str(&player.depth) + " (" + &current_theme + ").";
        &dead = false;
    }

    func &attackMonster(m) {
        dmg = &player.totalAtk();
        m.hp = m.hp - dmg;
        if m.hp <= 0 {
            gain = &xpGainFromMonster(m);
            &gainXP(gain);

            gold_gain = randInt(1, 3) + (&player.depth // 2);
            &player.gold = &player.gold + gold_gain;

            # remove monster by position (robust)
            idx = -1;
            for i = 0, i < length(&monsters), i += 1 {
                if &monsters[i].x == m.x and &monsters[i].y == m.y { idx = i; break; }
            }
            if idx != -1 { &monsters.pop(idx); }

            &message = "You slay the " + m.name + "! (+XP " + str(gain) + ", +" + str(gold_gain) + "g)";
        } else {
            &message = "You hit the " + m.name + " for " + str(dmg) + ".";
            &monsterTurn();
        }
    }

    func &pickup() {
        idx = &itemIndexAt(&player.x, &player.y);
        if idx == -1 { &message = "Nothing to pick up."; return; }
        rec = &items[idx];
        &items.pop(idx);
        
        if rec["it"].kind == "gold" {
            &player.gold = &player.gold + rec["it"].stats;
            &message = "Picked up " + str(rec["it"].stats) + " gold!";
        } else {
            &player.inventory.append(rec["it"]);
            &message = "Picked up " + rec["it"].toString() + ".";
        }
    }

    # --- Equipment helpers ---
    func &equipWeapon(it) {
        old = &player.weapon;
        &player.weapon = it;
        if old { &player.inventory.append(old); }
        &message = "Equipped weapon: " + it.name + ".";
    }

    func &equipArmor(it) {
        old = &player.armor;
        &player.armor = it;
        if old { &player.inventory.append(old); }
        &message = "Equipped armor: " + it.name + ".";
    }

    func &equipRing(it) {
        # Ensure rings list has MAX_RING_SLOTS entries using a temp
        rings_tmp = &player.rings;
        while length(rings_tmp) < MAX_RING_SLOTS { rings_tmp.append(Null); }
        &player.rings = rings_tmp;

        # Try to fill the first empty slot
        for i = 0, i < MAX_RING_SLOTS, i += 1 {
            if not &player.rings[i] {
                rings_tmp2 = &player.rings;
                rings_tmp2[i] = it;         # modify temp
                &player.rings = rings_tmp2; # write back
                &message = "Equipped ring: " + it.name + ".";
                return;
            }
        }

        # Both occupied: ask which to replace (1-based)
        print("Replace which ring? [1 or 2, blank cancels]");
        inp = input("> ").strip();
        if inp == "" { &message = "Equip canceled."; return; }
        if not inp.isDigit() { &message = "Please enter 1 or 2."; return; }
        idx1 = int(inp);
        if idx1 < 1 or idx1 > MAX_RING_SLOTS { &message = "Invalid slot."; return; }
        idx = idx1 - 1;

        old = &player.rings[idx];
        if old { &player.inventory.append(old); }

        rings_tmp3 = &player.rings;
        rings_tmp3[idx] = it;
        &player.rings = rings_tmp3;

        &message = "Equipped ring in slot " + str(idx1) + ": " + it.name + ".";
    }

    func &useInventory() {
        if length(&player.inventory) == 0 {
            print("Inventory is empty."); return;
        }
        print("Inventory:");
        for i = 0, i < length(&player.inventory), i += 1 {
            it = &player.inventory[i];
            # show indices starting at 1
            print("  [" + str(i + 1) + "] " + it.toString() + "  {" + it.kind + "}");
        }
        which = input("Use/equip which index (1.." + str(length(&player.inventory)) + ", blank cancels)? ").strip();
        if which == "" { return; }
        if not which.isDigit() { print("Please enter a number."); return; }

        idx1 = int(which);
        if idx1 < 1 or idx1 > length(&player.inventory) { print("Invalid index."); return; }
        idx = idx1 - 1;  # convert to 0-based

        it2 = &player.inventory[idx];

        if it2.kind == "potion" {
            heal = it2.stats;
            &player.hp = &player.hp + heal;
            if &player.hp > &player.hp_max { &player.hp = &player.hp_max; }
            print("You drink the potion and heal " + str(heal) + ".");
            _ = input("(press Enter) ");
            &player.inventory.pop(idx);
        } elif it2.kind == "weapon" {
            &player.inventory.pop(idx);
            &equipWeapon(it2);
        } elif it2.kind == "armor" {
            &player.inventory.pop(idx);
            &equipArmor(it2);
        } elif it2.kind == "ring" {
            &player.inventory.pop(idx);
            &equipRing(it2);
        } else {
            print("You can't use that.");
        }
    }

    func &showStats() {
        print("=== Player Stats ===");
        print(&player.statsStr());
        print("  Depth: " + str(&player.depth));
        print("  Theme: " + &current_theme + " - " + &getThemeInfo());
        if &player.weapon { print("  Weapon: " + &player.weapon.toString()); }
        else { print("  Weapon: none"); }
        if &player.armor { print("  Armor: " + &player.armor.toString()); }
        else { print("  Armor: none"); }

        # ring slots — pad via temp (Funcy-safe)
        rings_tmp = &player.rings;
        while length(rings_tmp) < MAX_RING_SLOTS { rings_tmp.append(Null); }
        &player.rings = rings_tmp;
        for i = 0, i < MAX_RING_SLOTS, i += 1 {
            label = "none";
            if &player.rings[i] { label = &player.rings[i].toString(); }
            print("  Ring[" + str(i + 1) + "]: " + label);
        }

        # XP to next
        print("XP to next level: " + str(&player.xpToNext()));
        _ = input("\n(press Enter to return) ");
    }

    # ---------- RENDER ----------

    func &render() {
        # Precompute quick lookup maps (O(entities))
        items_map = {};        # key: "x,y" -> glyph
        for i = 0, i < length(&items), i += 1 {
            rec = &items[i];
            key = str(rec["x"]) + "," + str(rec["y"]);
            items_map[key] = rec["glyph"];
        }
        monsters_map = {};     # key: "x,y" -> glyph
        for i = 0, i < length(&monsters), i += 1 {
            m = &monsters[i];
            key = str(m.x) + "," + str(m.y);
            monsters_map[key] = m.glyph;
        }

        # Visibility: full if disabled; radius-only if enabled
        vis = Null;
        if &fov_enabled { vis = &computeVisibilityRadius(); }

        # Cache exit coords for cheap compare
        ex = &exit[0]; ey = &exit[1];

        lines = [];
        header = "== Funcy Roguelike :: " + &player.statsStr() + " :: " + &current_theme + " ==";
        lines.append(header);
        if &message != "" { lines.append(&message); }
        else { lines.append(" "); }  # keep board from shifting

        for y = 0, y < MAP_H, y += 1 {
            row_chars = [];
            for x = 0, x < MAP_W, x += 1 {
                visible = true;
                if &fov_enabled { visible = vis[y][x]; }
                ch = " ";

                if visible {
                    &seen[y][x] = true;

                    # base tile
                    base = &grid[y][x];
                    ch = base;

                    key = str(x) + "," + str(y);
                    if key in items_map { ch = items_map[key]; }
                    if key in monsters_map { ch = monsters_map[key]; }

                    # trader overlay
                    if &trader {
                        if x == &trader["x"] and y == &trader["y"] { ch = "T"; }
                    }

                    # player on top
                    if x == &player.x and y == &player.y { ch = &player.glyph; }

                    # visible tinting / colors
                    ch = &applyColorVisibleTile(ch);
                } else {
                    if &seen[y][x] {
                        if &grid[y][x] == "#" {
                            ch = "#";
                            if &color_enabled { 
                                # All themes use the same dim gray for remembered walls
                                ch = C_WALL_DIM_THEME + ch + C_RESET;
                            }
                        } elif x == ex and y == ey {
                            ch = ">";
                            if &color_enabled { ch = C_EXIT + ch + C_RESET; }
                        } else {
                            ch = " ";
                        }
                    } else {
                        ch = " ";
                    }
                }
                row_chars.append(ch);
            }
            lines.append("".join(row_chars));
        }

        print("\e[H\e[J" + "\n".join(lines));
    }

    # ---------- MONSTER AI ----------

    
    func &monsterTurn() {
        # Build an occupancy map once (O(M))
        occ = {};  # "x,y" -> true
        for j = 0, j < length(&monsters), j += 1 {
            mm = &monsters[j];
            if mm.hp > 0 {
                occ[str(mm.x) + "," + str(mm.y)] = true;
            }
        }

        for i = 0, i < length(&monsters), i += 1 {
            m = &monsters[i];
            if m.hp <= 0 { continue; }

            dx = &player.x - m.x;
            dy = &player.y - m.y;

            adx = dx; if adx < 0 { adx = -adx; }
            ady = dy; if ady < 0 { ady = -ady; }

            if (adx + ady) == 1 {
                dmg = m.atk;
                red = &player.totalDef();
                dmg = dmg - red;
                if dmg < 1 { dmg = 1; }

                &player.hp = &player.hp - dmg;
                &message = "The " + m.name + " hits you for " + str(dmg) + "!";
                if &player.hp <= 0 {
                    &dead = true;
                    return;
                }
                continue;
            }

            stepX = 0; stepY = 0;
            if (adx + ady) <= 6 and randInt(0,1) == 1 {
                if adx >= ady {
                    if dx > 0 { stepX = 1; } elif dx < 0 { stepX = -1; }
                } else {
                    if dy > 0 { stepY = 1; } elif dy < 0 { stepY = -1; }
                }
            } else {
                dirs = [[1,0],[-1,0],[0,1],[0,-1],[0,0]];
                d = randChoice(dirs);
                stepX = d[0]; stepY = d[1];
            }

            nx = m.x + stepX; ny = m.y + stepY;
            k_old = str(m.x) + "," + str(m.y);
            k_new = str(nx) + "," + str(ny);

            if &passable(nx, ny) and not (nx == &player.x and ny == &player.y) and not (k_new in occ) {
                # update occupancy
                occ.pop(k_old);
                occ[k_new] = true;
                m.x = nx; m.y = ny;
            }
        }
    }


    # ---------- INPUT-ACTION HELPERS ----------

    func &tryMove(dx, dy) {
        nx = &player.x + dx; ny = &player.y + dy;
        if not inBounds(nx, ny) { &message = "You bump the edge."; return; }

        m = &monsterAt(nx, ny);
        if m { &attackMonster(m); return; }

        if &passable(nx, ny) {
            &player.x = nx; &player.y = ny;
            if &grid[ny][nx] == ">" {
                &depthTransition(&player.depth + 1);

                # Rest-shop chance
                r = randInt(1, 100);
                if r <= REST_SHOP_CHANCE {
                    stock = &generateShopStock();
                    &shopMenu(stock, "Rest Shop");
                }

                &buildLevel(&player.depth + 1);   # no healing between levels
                return;
            }
            &message = "";
            &monsterTurn();
            return;
        } else {
            &message = "A wall blocks your way.";
        }
    }

    # ---------- TRANSITION FX ----------

    func &depthTransition(newDepth) {
        # Go to top-left and clear screen
        print("\e[H");

        # Build one full-width blank line
        blank = "";
        for i = 0, i < MAP_W, i += 1 { blank = blank + " "; }

        # Print enough rows to "wipe" downward
        total = MAP_H + 1;   # a little extra for effect
        for n = 0, n < total, n += 1 {
            print(blank);
            delayMs(50);
        }
    }
}
