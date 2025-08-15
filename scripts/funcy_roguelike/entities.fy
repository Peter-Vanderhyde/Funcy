# entities.fy — player, monsters, items
import "config.fy";

# Utility: color a single-char glyph once at construction time (no per-frame concat).
func colorIfEnabled(ch, color_code) {
    if COLOR_ENABLED { return color_code + ch + C_RESET; }
    return ch;
}

class Player {
    func &Player(x, y) {
        &x = x; &y = y;
        &name = "You";
        &max_hp = 20; &hp = 20;
        &atk_min = 2; &atk_max = 5;
        &inventory = [];   # list of item dicts
        &cached_stats = ""; &stats_dirty = true;
        &rebuildStatsStr();  # initialize &cached_stats
    }

    func &rebuildStatsStr() {
        # Keep stats string cached to avoid rebuilding it every render
        &cached_stats = "HP " + str(&hp) + "/" + str(&max_hp)
                        + "  ATK " + str(&atk_min) + "-" + str(&atk_max);
        &stats_dirty = false;
    }

    func &statsStr() {
        if &stats_dirty { &rebuildStatsStr(); }
        return &cached_stats;
    }

    func &rollDamage() {
        return randInt(&atk_min, &atk_max);
    }

    func &pickup(item) {
        # Minimal pick-up logic; extend as needed.
        # Example soft rules:
        # - potion: heal
        # - weapon: raise dmg range
        # - misc: just add to inventory
        t = ""; if "type" in item { t = item["type"]; }

        if t == "potion" and "heal" in item {
            heal = item["heal"];
            &hp = &hp + heal;
            if &hp > &max_hp { &hp = &max_hp; }
            &stats_dirty = true;
        } elif t == "weapon" {
            if "atk_min" in item { &atk_min = item["atk_min"]; }
            if "atk_max" in item { &atk_max = item["atk_max"]; }
            &stats_dirty = true;
            &inventory.append(item);
        } else {
            &inventory.append(item);
        }
    }
}

class Monster {
    func &Monster(name, glyph, x, y, hp, atk_min, atk_max) {
        &name = name;
        # Glyph can be pre-colored once to avoid per-frame color wrapping
        &glyph = glyph;
        &x = x; &y = y;
        &hp = hp; &max_hp = hp;
        &atk_min = atk_min; &atk_max = atk_max;
    }

    func &rollDamage() {
        return randInt(&atk_min, &atk_max);
    }
}

# Convenience factory examples (use these if you spawn in map/gen code)
func makeSlime(x, y) {
    g = colorIfEnabled("s", C_MONSTER);
    return Monster("slime", g, x, y, 8, 1, 3);
}

func makeBat(x, y) {
    g = colorIfEnabled("b", C_MONSTER);
    return Monster("bat", g, x, y, 5, 1, 2);
}

func makePotion(x, y) {
    return {"name": "healing potion",
            "glyph": colorIfEnabled("!", C_POTION),
            "type": "potion", "heal": 6, "x": x, "y": y};
}

func makeDagger(x, y) {
    return {"name": "rusty dagger",
            "glyph": colorIfEnabled(")", C_WEAPON),
            "type": "weapon", "atk_min": 2, "atk_max": 6, "x": x, "y": y};
}
