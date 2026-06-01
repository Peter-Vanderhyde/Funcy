/* particles.fy - Procedural ASCII Particle System Visualizer */

# Simple delay loop (can be swapped with your delayMs from game.fy)
func pause() {
    # start = time();
    # while time() - start < 25 {
    #     continue;
    # }
}

class Particle {
    &x = 0.0; &y = 0.0;
    &vx = 0.0; &vy = 0.0;
    &life = 0;
    &char = "."; 
    &color = "\e[0m";

    func &Particle(x, y, vx, vy, life, char, color) {
        &x = float(x); &y = float(y);
        &vx = float(vx); &vy = float(vy);
        &life = life;
        &char = char; 
        &color = color;
    }

    # Apply gravity and velocity
    func &update(gravity) {
        &vy += gravity;
        &x += &vx;
        &y += &vy;
        &life -= 1;
    }
}

class ParticleSystem {
    &particles = [];
    &width = 60;
    &height = 25;
    &gravity = 0.05; # Default pull downward

    func &ParticleSystem(w, h, g) {
        &width = w; &height = h; &gravity = g;
    }

    # --- EMITTERS ---

    func &spawnFirework(ex, ey) {
        colors = ["\e[91m", "\e[92m", "\e[93m", "\e[94m", "\e[95m", "\e[96m", "\e[97m"];
        c = colors[randInt(0, length(colors) - 1)];
        chars = ["*", "+", "x", ".", "o"];
        
        # Spawn an explosion of 30 particles
        for i = 0, i < 30, i += 1 {
            # Randomize outward velocity
            rx = float(randInt(-15, 15)) / 10.0;
            ry = float(randInt(-15, 10)) / 10.0; # Bias slightly upwards
            
            ch = chars[randInt(0, length(chars) - 1)];
            life = randInt(10, 25);
            
            p = Particle(ex, ey, rx, ry, life, ch, c);
            &particles.append(p);
        }
    }

    func &spawnRain() {
        # Spawn 4-5 raindrops at the top of the screen every frame
        for i = 0, i < randInt(0, 5), i += 1 {
            rx = float(randInt(0, &width - 1));
            # Rain falls fast, straight down
            vy = float(randInt(5, 15)) / 10.0; 
            p = Particle(rx, 0.0, 0.0, vy, 30, "|", "\e[36m"); # Cyan
            &particles.append(p);
        }
    }

    # --- ENGINE LOGIC ---

    func &update() {
        active = [];
        for p in &particles {
            p.update(&gravity);
            
            # Keep particle if it's alive AND inside screen bounds
            if p.life > 0 and p.x >= 0.0 and p.x < float(&width) and p.y < float(&height) {
                active.append(p);
            }
        }
        # Clear dead particles to save memory
        &particles = active;
    }

    func &render() {
        # 1. Create a blank frame buffer
        grid = [];
        for y = 0, y < &height, y += 1 {
            row = [];
            for x = 0, x < &width, x += 1 {
                row.append(" ");
            }
            grid.append(row);
        }

        # 2. Draw particles onto the buffer
        for p in &particles {
            ix = int(p.x);
            iy = int(p.y);
            if ix >= 0 and ix < &width and iy >= 0 and iy < &height {
                # Add terminal color, the character, and reset the color
                grid[iy][ix] = p.color + p.char + "\e[0m";
            }
        }

        # 3. Print the frame
        print("\e[H\e[J"); # Clear screen sequence
        for row in grid {
            line = "";
            for cell in row { line += cell; }
            print(line);
        }
    }
}

# --- Execution ---
system = ParticleSystem(60, 25, 0.08);
mode = "fireworks";

for frame = 0, frame < 1000, frame += 1 {
    
    # Switch modes halfway through
    if frame != 0 and frame % 100 == 0 {
        if mode == "fireworks" {
            mode = "rain";
        } else {
            mode = "fireworks";
        }
    }

    # Trigger emitters based on mode
    if mode == "fireworks" {
        if frame % 15 == 0 { # Launch a firework every 15 frames
            system.spawnFirework(randInt(10, 50), randInt(5, 15));
        }
    } elif mode == "rain" {
        system.spawnRain();
    }
    
    system.update();
    system.render();
    
    print("Mode: " + mode + " | Particles: " + str(length(system.particles)));
    pause();
}