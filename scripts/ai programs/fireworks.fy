/* fireworks_trails.fy - Complete String Drawing & Smoke Trails
   Fixed escape string boundaries with an added fading smoke trail.
*/

class Particle {
    &x = 0.0;
    &y = 0.0;
    &last_x = -1;
    &last_y = -1;
    &vx = 0.0;
    &vy = 0.0;
    &life = 0;
    &is_rocket = false;
    &is_smoke = false;  # New flag to identify smoke trails
    &color = "";
    &char = "";

    func &Particle(x, y, vx, vy, life, is_rocket, is_smoke, color, char) {
        &x = float(x);
        &y = float(y);
        &vx = float(vx);
        &vy = float(vy);
        &life = life;
        &is_rocket = is_rocket;
        &is_smoke = is_smoke;
        &color = color;
        &char = char;
    }
}

func main() {
    width = 60;
    height = 25;
    gravity = 0.12; # Slightly lower gravity for loftier explosions
    
    colors = ["\e[31m", "\e[32m", "\e[33m", "\e[34m", "\e[35m", "\e[36m"];
    reset_col = "\e[0m";
    particles = [];

    draw("\e[2J"); # Clear the terminal completely ONCE

    for frame = 0, frame < 300, frame += 1 {
        
        # UI Header drawn securely at 1;1
        draw("\e[1;1H\e[K--- Smoke Trails Display! Frames Left: " + str(300 - frame) + " ---");
        
        # 1. Randomly launch rockets
        if randInt(1, 100) <= 12 {
            start_x = randInt(15, width - 15);
            color = colors[randInt(0, 5)];
            start_vy = -1.0 * (float(randInt(16, 24)) / 10.0);
            start_vx = float(randInt(-3, 3)) / 10.0;
            
            particles.append(Particle(start_x, height - 1, start_vx, start_vy, randInt(12, 20), true, false, color, "^"));
        }

        new_particles = [];
        
        # 2. Erase ALL old positions first using full, bounded strings
        for p in particles {
            if p.last_x >= 0 and p.last_y >= 0 {
                # Wrap the empty space cleanly inside the cursor positioning boundaries
                draw("\e[" + str(p.last_y + 2) + ";" + str(p.last_x + 1) + "H ");
            }
        }

        # 3. Update physics and handle smoke logic
        for p in particles {
            
            # If it's a smoke particle, it drifts slightly and fades without gravity
            if p.is_smoke {
                p.x += p.vx;
                p.y += p.vy;
                p.life -= 1;
                
                # Dynamic smoke color degradation based on remaining life
                if p.life > 6 {
                    p.color = "\e[37m"; # Bright white smoke
                } else {
                    p.color = "\e[90m"; # Dim dark gray smoke
                }
            } else {
                # Regular physics for rockets and explosion sparks
                p.vy += gravity;
                p.x += p.vx;
                p.y += p.vy;
                p.life -= 1;
            }

            if p.y >= height or p.y < 0 or p.x >= width or p.x < 0 {
                continue; 
            }

            # Rocket behavior: drops smoke and triggers explosion at apex
            if p.is_rocket {
                # Spawn a smoke particle at the rocket's current position drifting slowly
                # Smoke does not track as a rocket, marked true for is_smoke
                new_particles.append(Particle(p.x, p.y, float(randInt(-2, 2))/20.0, 0.0, randInt(8, 12), false, true, "\e[37m", "."));
                
                if p.vy >= 0.0 {
                    # Explode into sparks!
                    num_sparks = randInt(15, 25);
                    for i in range(num_sparks) {
                        spark_vx = float(randInt(-18, 18)) / 10.0;
                        spark_vy = float(randInt(-12, 12)) / 10.0;
                        new_particles.append(Particle(p.x, p.y, spark_vx, spark_vy, randInt(6, 15), false, false, p.color, "*"));
                    }
                    continue; # Remove rocket body
                }
            }

            # 4. Render living items by combining ANSI commands and text into complete lines
            if p.life > 0 {
                ix = int(p.x);
                iy = int(p.y);
                
                p.last_x = ix;
                p.last_y = iy;
                
                # CRITICAL FIX: The cursor instruction, color codes, character, and reset code 
                # are completely unified into a single text output block. 
                full_draw_command = "\e[" + str(iy + 2) + ";" + str(ix + 1) + "H" + p.color + p.char + reset_col;
                draw(full_draw_command);
                
                new_particles.append(p);
            }
        }
        
        particles = new_particles;

        # 5. Flush cursor completely out of the frame area
        draw("\e[" + str(height + 2) + ";1H");

        sleep(45);
    }
    
    draw("\e[2J\e[1;1HShow completed safely!");
}

showCursor(false);
main();
showCursor();