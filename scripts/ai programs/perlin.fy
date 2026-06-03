/* terrain_fixed.fy - Procedural ASCII Terrain Generator */

class NoiseGen {
    &seed = 12345;
    
    func &NoiseGen(seed_val) {
        &seed = seed_val;
    }
    
    # 32-Bit Safe Hash: Keeps maximum values well under 2.1 billion
    func hash(x, y) {
        # Using smaller prime numbers (8191 is a prime)
        n = abs(int(x) * 331 + int(y) * 643 + &seed) % 8191;
        
        # Max n is 8190. 8190 * 8190 = ~67 million (very safe!)
        n = abs(n * n) % 8191;
        
        return float(n) / 8191.0;
    }

    func smoothstep(t) {
        return t * t * (3.0 - 2.0 * t);
    }

    func lerp(a, b, t) {
        return a + t * (b - a);
    }

    func &noise2D(x, y) {
        x_int = int(x);
        y_int = int(y);
        
        x_frac = x - float(x_int);
        y_frac = y - float(y_int);

        u = smoothstep(x_frac);
        v = smoothstep(y_frac);

        c00 = hash(x_int, y_int);
        c10 = hash(x_int + 1, y_int);
        c01 = hash(x_int, y_int + 1);
        c11 = hash(x_int + 1, y_int + 1);

        nx0 = lerp(c00, c10, u);
        nx1 = lerp(c01, c11, u);
        return lerp(nx0, nx1, v);
    }

    func &fbm(x, y, octaves) {
        total = 0.0;
        frequency = 1.0;
        amplitude = 1.0;
        max_val = 0.0;

        for i = 0, i < octaves, i += 1 {
            total += &noise2D(x * frequency, y * frequency) * amplitude;
            max_val += amplitude;
            amplitude *= 0.5;
            frequency *= 2.0;
        }
        
        return total / max_val;
    }
}

class TerrainMap {
    func &TerrainMap(w, h, scale, seed) {
        &width = w;
        &height = h;
        &scale = scale;
        &seed = seed;
    }
    
    func &generate() {
        # Seed it with something specific to test, or keep it random
        gen = NoiseGen(&seed);
        
        C_WATER = "\e[34m";   
        C_SAND  = "\e[33m";   
        C_GRASS = "\e[92m";   
        C_TREES = "\e[32m";   
        C_ROCK  = "\e[90m";   
        C_SNOW  = "\e[97m";   
        C_RESET = "\e[0m";    
        
        print("\nGenerating Procedural Terrain...  Seed: " + str(&seed) + "\n");

        for y in range(&height) {
            row_str = "";
            for x in range(&width) {
                
                nx = float(x) * &scale;
                ny = float(y) * &scale;
                elevation = gen.fbm(nx, ny, 4);
                
                char = "";
                
                # Updated thresholds based on typical FBM bell curve (tends to hover around 0.5)
                if elevation < 0.40 {
                    char = C_WATER + "~~";
                } elif elevation < 0.48 {
                    char = C_SAND  + "..";
                } elif elevation < 0.55 {
                    char = C_GRASS + ",,";
                } elif elevation < 0.65 {
                    char = C_TREES + "tt";
                } elif elevation < 0.75 {
                    char = C_ROCK  + "^^";
                } else {
                    char = C_SNOW  + "AA";
                }
                
                row_str += char;
            }
            draw(row_str + C_RESET);
        }
    }
}

# --- Execution ---
# Increased scale to 0.25 to "zoom out" and see more features at once
showCursor();
seed = input("Enter seed (leave blank for random): ");
if not seed.isDigit() or int(seed) < 1 {
    seed = randInt(1, 999999);
}
terrain = TerrainMap(50, 1000, 0.10, int(seed));
draw("\e[H\e[J");
showCursor(false);
terrain.generate();
showCursor();