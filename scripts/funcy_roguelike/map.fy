# map.fy — dungeon grid helpers + procedural generation
import "config.fy";

func inBounds(x, y) {
    if x < 0 or y < 0 { return false; }
    if x >= MAP_W or y >= MAP_H { return false; }
    return true;
}

func makeGrid(fill) {
    g = [];
    for y = 0, y < MAP_H, y += 1 {
        row = [];
        for x = 0, x < MAP_W, x += 1 { row.append(fill); }
        g.append(row);
    }
    return g;
}

func carveCell(grid, x, y, ch) { if inBounds(x, y) { grid[y][x] = ch; } }

func carveRoom(grid, rx, ry, rw, rh) {
    y2 = ry + rh - 1; x2 = rx + rw - 1;
    for y = ry, y <= y2, y += 1 {
        for x = rx, x <= x2, x += 1 { carveCell(grid, x, y, "."); }
    }
}

func carveHTunnel(grid, x1, x2, y) {
    a = x1; b = x2; if a > b { tmp = a; a = b; b = tmp; }
    for x = a, x <= b, x += 1 { carveCell(grid, x, y, "."); }
}

func carveVTunnel(grid, y1, y2, x) {
    a = y1; b = y2; if a > b { tmp = a; a = b; b = tmp; }
    for y = a, y <= b, y += 1 { carveCell(grid, x, y, "."); }
}

func rectsOverlap(r1, r2, pad) {
    x1 = r1["x"] - pad; y1 = r1["y"] - pad;
    w1 = r1["w"] + 2*pad; h1 = r1["h"] + 2*pad;
    x2 = r2["x"]; y2 = r2["y"]; w2 = r2["w"]; h2 = r2["h"];
    if x1 + w1 <= x2 { return false; }
    if x2 + w2 <= x1 { return false; }
    if y1 + h1 <= y2 { return false; }
    if y2 + h2 <= y1 { return false; }
    return true;
}

func centerOf(room) {
    cx = room["x"] + (room["w"] // 2);
    cy = room["y"] + (room["h"] // 2);
    return [cx, cy];
}

# Theme-based map generation
func getThemeForDepth(depth) {
    if depth < THEME_DEPTHS[1] { return THEMES[0]; }  # Catacombs: 1-4
    elif depth < THEME_DEPTHS[2] { return THEMES[1]; }  # Caves: 5-9
    else { return THEMES[2]; }  # Forge: 10+
}

func generateCatacombsMap() {
    # Catacombs: narrow rooms, long corridors, more structured
    ROOM_ATTEMPTS = 80;
    MIN_W = 3;  MAX_W = 8;   # narrower rooms
    MIN_H = 2;  MAX_H = 6;
    SEP_PAD = 2;              # more separation
    
    grid = makeGrid("#");
    rooms = [];
    
    attempts = 0;
    while attempts < ROOM_ATTEMPTS {
        attempts = attempts + 1;
        
        rw = randInt(MIN_W, MAX_W);
        rh = randInt(MIN_H, MAX_H);
        
        rx = randInt(1, MAP_W - rw - 2);
        ry = randInt(1, MAP_H - rh - 2);
        
        newr = { "x": rx, "y": ry, "w": rw, "h": rh };
        
        ok = true;
        for i = 0, i < length(rooms), i += 1 {
            if rectsOverlap(newr, rooms[i], SEP_PAD) { ok = false; break; }
        }
        if not ok { continue; }
        
        carveRoom(grid, rx, ry, rw, rh);
        rooms.append(newr);
        
        if length(rooms) > 1 {
            prev = rooms[length(rooms) - 2];
            c1 = centerOf(prev); c2 = centerOf(newr);
            # Always connect with both tunnels for catacombs
            carveHTunnel(grid, c1[0], c2[0], c1[1]);
            carveVTunnel(grid, c1[1], c2[1], c2[0]);
        }
    }
    
    return { "grid": grid, "rooms": rooms };
}

func generateCavesMap() {
    # Caves: organic, cellular rooms, fewer connections
    ROOM_ATTEMPTS = 50;
    MIN_W = 5;  MAX_W = 12;  # wider, more organic
    MIN_H = 4;  MAX_H = 10;
    SEP_PAD = 1;              # less separation
    
    grid = makeGrid("#");
    rooms = [];
    
    attempts = 0;
    while attempts < ROOM_ATTEMPTS {
        attempts = attempts + 1;
        
        rw = randInt(MIN_W, MAX_W);
        rh = randInt(MIN_H, MAX_H);
        
        rx = randInt(1, MAP_W - rw - 2);
        ry = randInt(1, MAP_H - rh - 2);
        
        newr = { "x": rx, "y": ry, "w": rw, "h": rh };
        
        ok = true;
        for i = 0, i < length(rooms), i += 1 {
            if rectsOverlap(newr, rooms[i], SEP_PAD) { ok = false; break; }
        }
        if not ok { continue; }
        
        carveRoom(grid, rx, ry, rw, rh);
        rooms.append(newr);
        
        if length(rooms) > 1 {
            prev = rooms[length(rooms) - 2];
            c1 = centerOf(prev); c2 = centerOf(newr);
            # Always connect caves rooms, but vary the connection type
            if randInt(0, 1) == 0 {
                carveHTunnel(grid, c1[0], c2[0], c1[1]);
                carveVTunnel(grid, c1[1], c2[1], c2[0]);
            } else {
                carveVTunnel(grid, c1[1], c2[1], c1[0]);
                carveHTunnel(grid, c1[0], c2[0], c2[1]);
            }
        }
    }
    
    return { "grid": grid, "rooms": rooms };
}

func generateForgeMap() {
    # Forge: wide rooms, lava pockets, more open
    ROOM_ATTEMPTS = 40;
    MIN_W = 6;  MAX_W = 15;  # wider rooms
    MIN_H = 5;  MAX_H = 12;
    SEP_PAD = 1;
    
    grid = makeGrid("#");
    rooms = [];
    
    attempts = 0;
    while attempts < ROOM_ATTEMPTS {
        attempts = attempts + 1;
        
        rw = randInt(MIN_W, MAX_W);
        rh = randInt(MIN_H, MAX_H);
        
        rx = randInt(1, MAP_W - rw - 2);
        ry = randInt(1, MAP_H - rh - 2);
        
        newr = { "x": rx, "y": ry, "w": rw, "h": rh };
        
        ok = true;
        for i = 0, i < length(rooms), i += 1 {
            if rectsOverlap(newr, rooms[i], SEP_PAD) { ok = false; break; }
        }
        if not ok { continue; }
        
        carveRoom(grid, rx, ry, rw, rh);
        rooms.append(newr);
        
        if length(rooms) > 1 {
            prev = rooms[length(rooms) - 2];
            c1 = centerOf(prev); c2 = centerOf(newr);
            # Wide connections for forge
            carveHTunnel(grid, c1[0], c2[0], c1[1]);
            carveVTunnel(grid, c1[1], c2[1], c2[0]);
        }
    }
    
    return { "grid": grid, "rooms": rooms };
}

func generateMap(theme) {
    result = Null;
    if theme == "catacombs" {
        result = generateCatacombsMap();
    } elif theme == "caves" {
        result = generateCavesMap();
    } elif theme == "forge" {
        result = generateForgeMap();
    } else {
        # Fallback to original generation
        result = generateClassicMap();
    }
    
    grid = result["grid"];
    rooms = result["rooms"];
    
    if length(rooms) == 0 {
        rw = 6; rh = 4; rx = (MAP_W - rw) // 2; ry = (MAP_H - rh) // 2;
        carveRoom(grid, rx, ry, rw, rh);
        rooms.append({ "x": rx, "y": ry, "w": rw, "h": rh });
    }
    
    start = centerOf(rooms[0]); px = start[0]; py = start[1];
    if not inBounds(px, py) { px = 1; py = 1; }
    
    last = rooms[length(rooms) - 1]; exy = centerOf(last);
    ex = exy[0]; ey = exy[1]; if not inBounds(ex, ey) { ex = MAP_W - 2; ey = MAP_H - 2; }
    carveCell(grid, ex, ey, ">");
    
    return { "grid": grid, "rooms": rooms, "exit": [ex, ey], "player_start": [px, py] };
}

# Original map generation as fallback
func generateClassicMap() {
    ROOM_ATTEMPTS = 60;
    MIN_W = 4;  MAX_W = 10;
    MIN_H = 3;  MAX_H = 8;
    SEP_PAD = 1;

    grid = makeGrid("#");
    rooms = [];

    attempts = 0;
    while attempts < ROOM_ATTEMPTS {
        attempts = attempts + 1;

        rw = randInt(MIN_W, MAX_W);
        rh = randInt(MIN_H, MAX_H);

        rx = randInt(1, MAP_W - rw - 2);
        ry = randInt(1, MAP_H - rh - 2);

        newr = { "x": rx, "y": ry, "w": rw, "h": rh };

        ok = true;
        for i = 0, i < length(rooms), i += 1 {
            if rectsOverlap(newr, rooms[i], SEP_PAD) { ok = false; break; }
        }
        if not ok { continue; }

        carveRoom(grid, rx, ry, rw, rh);
        rooms.append(newr);

        if length(rooms) > 1 {
            prev = rooms[length(rooms) - 2];
            c1 = centerOf(prev); c2 = centerOf(newr);
            if randInt(0, 1) == 0 {
                carveHTunnel(grid, c1[0], c2[0], c1[1]);
                carveVTunnel(grid, c1[1], c2[1], c2[0]);
            } else {
                carveVTunnel(grid, c1[1], c2[1], c1[0]);
                carveHTunnel(grid, c1[0], c2[0], c2[1]);
            }
        }
    }

    return { "grid": grid, "rooms": rooms };
}
