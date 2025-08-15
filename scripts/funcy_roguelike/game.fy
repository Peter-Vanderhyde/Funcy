# game.fy — all gameplay systems (state, render, FOV, combat, items, shops/trader)
import "config.fy";
import "map.fy";
import "entities.fy";

class Game {
    func &Game() {
        &grid = []; &rooms = []; &exit = [0,0];
        &monsters = []; &items = [];
        &player = Player(0, 0);
        &message = ""; &dead = false;

        &fov_enabled = FOV_ENABLED;
        &color_enabled = COLOR_ENABLED;

        # Map memory and fast overlays / scratch
        &seen = [];                    # map memory
        &vis = [];                     # persistent visibility
        &items_layer = [];             # per-frame item glyphs
        &monsters_layer = [];          # per-frame monster glyphs

        &initNewGame();
        &precolorStaticGlyphs();
    }

    # ===============================
    # World / setup
    # ===============================

    func &initNewGame() {
        dims = genDungeon();            # from map.fy — returns [grid, rooms, exit]
        &grid = dims["grid"];
        &rooms = dims["rooms"];
        &exit  = dims["exit"];

        # Allocate persistent buffers just once
        &seen = makeGrid(false);
        &vis = makeGrid(false);
        &items_layer = makeGrid("");
        &monsters_layer = makeGrid("");

        # Place player and spawn initial content
        &placePlayer();
        &spawnInitial();
    }

    func &placePlayer() {
        # Put player in first room center (fallback to (1,1))
        if length(&rooms) > 0 {
            r = &rooms[0];
            &player.x = r["cx"];
            &player.y = r["cy"];
        } else {
            &player.x = 1; &player.y = 1;
        }
    }

    func &spawnInitial() {
        # No-ops by default; keep hooks the same
        # Spawn a couple of slimes and some items if needed by your original game
        spawnDefaults(&monsters, &items, &grid);
    }

    # ===============================
    # Helpers / utilities
    # ===============================

    func &inBounds(x, y) {
        return x >= 0 and y >= 0 and x < MAP_W and y < MAP_H;
    }

    func &passable(x, y) {
        if not &inBounds(x, y) { return false; }
        t = &grid[y][x];
        return t == "." or t == ">";
    }

    func &monsterAt(x, y) {
        for i = 0, i < length(&monsters), i += 1 {
            m = &monsters[i];
            if m.x == x and m.y == y { return i; }
        }
        return -1;
    }

    func &itemAt(x, y) {
        for i = 0, i < length(&items), i += 1 {
            it = &items[i];
            if it["x"] == x and it["y"] == y { return i; }
        }
        return -1;
    }

    func &clearOverlay(layer) {
        for y = 0, y < MAP_H, y += 1 {
            row = layer[y];
            for x = 0, x < MAP_W, x += 1 { row[x] = ""; }
        }
    }

    func &buildLineFromRow(row) {
        # Fast concat once per char-row
        s = "";
        for i = 0, i < length(row), i += 1 { s = s + row[i]; }
        return s;
    }

    func &joinLines(lines) {
        s = "";
        for i = 0, i < length(lines), i += 1 {
            s = s + lines[i];
            if i < length(lines) - 1 { s = s + "\n"; }
        }
        return s;
    }

    # ===============================
    # Coloring (pre-colored static glyphs)
    # ===============================

    func &precolorStaticGlyphs() {
        # If color off, these aren’t used
        if not &color_enabled { return; }
        &COL_HASH = C_WALL + "#" + C_RESET;
        &COL_DOT  = C_FLOOR + "." + C_RESET;
        &COL_GT   = C_EXIT  + ">" + C_RESET;
        &COL_AT   = C_PLAYER + "@" + C_RESET;
    }

    func &applyColorVisibleTile(ch) {
        if not &color_enabled { return ch; }
        if ch == "#" { return &COL_HASH; }
        if ch == "." { return &COL_DOT; }
        if ch == ">" { return &COL_GT; }
        if ch == "@" { return &COL_AT; }
        # Items/monsters may come pre-colored from entities/config; else leave as-is
        return ch;
    }

    func &applyColorSeenTile(ch) {
        if not &color_enabled { return ch; }
        # “dim” seen memory; keep simple to avoid per-cell concat churn
        return C_SEEN + ch + C_RESET;
    }

    # ===============================
    # Visibility (fast, persistent buffer)
    # ===============================

    func &computeVisibilityRadiusInPlace() {
        R = FOV_RADIUS; px = &player.x; py = &player.y;

        miny = py - R; if miny < 0 { miny = 0; }
        maxy = py + R; if maxy >= MAP_H { maxy = MAP_H - 1; }
        minx = px - R; if minx < 0 { minx = 0; }
        maxx = px + R; if maxx >= MAP_W { maxx = MAP_W - 1; }

        # Clear bounding box only
        for y = miny, y <= maxy, y += 1 {
            row = &vis[y];
            for x = minx, x <= maxx, x += 1 { row[x] = false; }
        }

        R2 = R * R;
        for y = miny, y <= maxy, y += 1 {
            dy = y - py;
            for x = minx, x <= maxx, x += 1 {
                dx = x - px;
                if dx*dx + dy*dy <= R2 {
                    if &hasLineOfSight(px, py, x, y) { &vis[y][x] = true; }
                }
            }
        }
    }

    # Bresenham LOS; blocks on walls (‘#’)
    func &hasLineOfSight(x0, y0, x1, y1) {
        dx = abs(x1 - x0); sx = -1; if x0 < x1 { sx = 1; }
        dy = -abs(y1 - y0); sy = -1; if y0 < y1 { sy = 1; }
        err = dx + dy;

        x = x0; y = y0;
        while true {
            if not &inBounds(x, y) { return false; }
            if not (x == x0 and y == y0) {
                t = &grid[y][x];
                if t == "#" { return false; }
            }
            if x == x1 and y == y1 { return true; }
            e2 = 2 * err;
            if e2 >= dy { err = err + dy; x = x + sx; }
            if e2 <= dx { err = err + dx; y = y + sy; }
        }
    }

    # ===============================
    # Input / turn / actions
    # ===============================

    func &tryMove(dx, dy) {
        if &dead { return; }
        nx = &player.x + dx; ny = &player.y + dy;
        if not &inBounds(nx, ny) { return; }
        if not &passable(nx, ny) {
            # Bonk a wall?
            &message = "You bump into a wall.";
            return;
        }

        i = &monsterAt(nx, ny);
        if i != -1 {
            &attackMonster(i);
            return;
        }

        &player.x = nx; &player.y = ny;

        # Pickup if standing on one
        itIdx = &itemAt(nx, ny);
        if itIdx != -1 { &pickupItem(itIdx); }

        # Exit tile?
        if &grid[ny][nx] == ">" { &descend(); }
    }

    func &attackMonster(i) {
        m = &monsters[i];
        dmg = &player.rollDamage();
        m.hp = m.hp - dmg;
        &message = "You hit the " + m.name + " for " + str(dmg) + ".";

        if m.hp <= 0 {
            # Remove monster
            &message = "The " + m.name + " dies.";
            &monsters.remove(i);
        } else {
            # Counterattack
            pdmg = m.rollDamage();
            &player.hp = &player.hp - pdmg;
            if &player.hp <= 0 {
                &dead = true;
                &message = "You die!";
            }
        }
    }

    func &pickupItem(i) {
        it = &items[i];
        &player.pickup(it);
        &items.remove(i);
        &message = "You pick up " + it["name"] + ".";
    }

    func &descend() {
        # Make a new floor
        &message = "You descend…";
        &initNewGame();
    }

    # ===============================
    # Render — optimized to avoid per-cell churn
    # ===============================

    func &render() {
        if &fov_enabled { &computeVisibilityRadiusInPlace(); }

        # Rebuild fast overlay layers (O(n) on entity counts, O(1) reads in main loop)
        &clearOverlay(&items_layer);
        for i = 0, i < length(&items), i += 1 {
            rec = &items[i];
            x = rec["x"]; y = rec["y"];
            if &inBounds(x, y) { &items_layer[y][x] = rec["glyph"]; }
        }

        &clearOverlay(&monsters_layer);
        for i = 0, i < length(&monsters), i += 1 {
            m = &monsters[i];
            if &inBounds(m.x, m.y) { &monsters_layer[m.y][m.x] = m.glyph; }
        }

        # Header + message
        lines = [];
        header = "== Funcy Roguelike :: " + &player.statsStr() + " ==";
        lines.append(header);
        if &message != "" { lines.append(&message); } else { lines.append(" "); }

        # Draw map
        for y = 0, y < MAP_H, y += 1 {
            row_chars = [];
            for x = 0, x < MAP_W, x += 1 {
                visible = true;
                if &fov_enabled { visible = &vis[y][x]; }

                ch = " ";
                if visible {
                    &seen[y][x] = true;
                    base = &grid[y][x]; ch = base;

                    ov = &items_layer[y][x]; if ov != "" { ch = ov; }
                    mv = &monsters_layer[y][x]; if mv != "" { ch = mv; }

                    if x == &player.x and y == &player.y { ch = "@"; }
                    ch = &applyColorVisibleTile(ch);
                } else {
                    if &seen[y][x] {
                        base2 = &grid[y][x];
                        ch = &applyColorSeenTile(base2);
                    } else {
                        ch = " ";
                    }
                }
                row_chars.append(ch);
            }
            lines.append(&buildLineFromRow(row_chars));
        }

        # Single print — clear + full frame
        print("\e[H\e[J" + &joinLines(lines));
    }
}

# ===============================
# Free functions expected by the game
# (keep signatures compatible with your existing setup)
# ===============================

func spawnDefaults(monsters, items, grid) {
    # Hook point for your original spawners; keep minimal defaults so file is self-contained.
    # If your old game already defines this elsewhere, this stub is ignored once linked.
    # e.g., nothing to do here for now.
}
