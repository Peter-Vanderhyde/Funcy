class Player {
    func &Player(x, y) {
        &x = x; &y = y;
        &hp = 20;
        &hp_max = 20;
        &atk = 3;                # base attack (no weapon/rings/perks)
        &depth = 1;

        # Leveling
        &xp = 0;
        &level = 1;

        # Inventory + equipment
        &inventory = [];
        &weapon = Null;          # Item(kind="weapon")
        &armor  = Null;          # Item(kind="armor")
        &rings  = [];            # up to MAX_RING_SLOTS Items(kind="ring")

        # Simple permanent perks
        &perk_atk_bonus = 0;
        &perk_def_bonus = 0;

        # Economy
        &gold = GOLD_START;

        &glyph = "@";
    }

    func &baseAtk() { return &atk; }

    func &weaponAtk() {
        if &weapon { return &weapon.stats; }
        return 0;
    }

    func &ringAtk() {
        total = 0;
        for i = 0, i < length(&rings), i += 1 {
            r = &rings[i];
            if r and r.subkind == "power" { total = total + 1; }
        }
        return total;
    }

    func &totalAtk() {
        return &baseAtk() + &weaponAtk() + &ringAtk() + &perk_atk_bonus;
    }

    func &armorDef() {
        if &armor { return &armor.stats; }
        return 0;
    }

    func &ringDef() {
        total = 0;
        for i = 0, i < length(&rings), i += 1 {
            r = &rings[i];
            if r and r.subkind == "defense" { total = total + 1; }
        }
        return total;
    }

    func &totalDef() {
        return ARMOR_BASE_REDUCTION + &armorDef() + &ringDef() + &perk_def_bonus;
    }

    func &xpThresholdFor(lv) {
        return LEVEL_XP_BASE + (lv - 1) * LEVEL_XP_SCALE;
    }

    func &xpToNext() {
        need = &xpThresholdFor(&level);
        rem = need - &xp;
        if rem < 0 { rem = 0; }
        return rem;
    }

    func &statsStr() {
        atk_total = &totalAtk();
        def_total = &totalDef();
        w = "none"; if &weapon { w = &weapon.name; }
        a = "none"; if &armor  { a = &armor.name; }
        r0 = "none";
        if length(&rings) > 0 and &rings[0] { r0 = &rings[0].name; }
        r1 = "none";
        if length(&rings) > 1 and &rings[1] { r1 = &rings[1].name; }
        return "Lvl " + str(&level) + "  XP " + str(&xp) + "/" + str(&xpThresholdFor(&level))
             + "  HP " + str(&hp) + "/" + str(&hp_max)
             + "  ATK " + str(atk_total)
             + "  DEF " + str(def_total)
             + "  GOLD " + str(&gold)
             + "  Wpn:[" + w + "] Arm:[" + a + "] Rings:[" + r0 + "," + r1 + "]  Depth " + str(&depth);
    }
}

class Item {
    func &Item(name, kind, stats) {
        &name = name;
        &kind = kind;       # "potion" | "weapon" | "armor" | "ring"
        &stats = stats;     # heal amount OR atk bonus OR def bonus
        &subkind = "";      # used for rings: "power"/"defense"
    }

    func &toString() {
        if &kind == "potion" { return &name + " (+" + str(&stats) + " HP)"; }
        elif &kind == "weapon" { return &name + " (ATK +" + str(&stats) + ")"; }
        elif &kind == "armor"  { return &name + " (DEF +" + str(&stats) + ")"; }
        elif &kind == "ring"   { return &name; }
        return &name;
    }
}

class Monster {
    func &Monster(name, x, y, hp, atk) {
        &name = name;
        &x = x; &y = y;
        &hp = hp; &atk = atk;
        # glyph used for rendering on the map
        g = name[0];
        &glyph = g;
    }

    func &isAlive() { return &hp > 0; }

    func &takeDamage(n) {
        &hp = &hp - n;
        if &hp < 0 { &hp = 0; }
    }

    func &toString() {
        return &name + " (HP " + str(&hp) + ", ATK " + str(&atk) + ")";
    }
}
