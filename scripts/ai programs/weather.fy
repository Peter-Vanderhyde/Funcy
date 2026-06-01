/* Doppler Weather Radar in Funcy (Optimized Rendering)
   Displays an animated top-down view of storm systems using in-place cursor updates.
*/

# Terminal formatting constants for colors
C_RESET   = "\e[0m";
C_CYAN    = "\e[96m";  # Light Rain
C_BLUE    = "\e[94m";  # Moderate Rain
C_MAGENTA = "\e[95m";  # Heavy Rain
C_RED     = "\e[91m";  # Thunderstorm Core
C_YELLOW  = "\e[93m";  # Lightning
C_DARK    = "\e[90m";  # Clear sky

class StormCell {
    func &StormCell(x, y, radius, vel_x, vel_y) {
        &x = float(x);
        &y = float(y);
        &radius_sq = float(radius * radius);
        &vel_x = float(vel_x);
        &vel_y = float(vel_y);
    }

    func &move() {
        &x += &vel_x;
        &y += &vel_y;
    }
}

class WeatherRadar {
    func &WeatherRadar(width, height) {
        &width = width;
        &height = height;
        &storms = list();
    }

    func &addStorm(storm) {
        &storms.append(storm);
    }

    func &update() {
        for storm in &storms {
            storm.move();
        }
    }

    # New function to draw the empty baseline once
    func &initRadar() {
        draw("\e[H\e[J"); # Clear screen and move cursor home
        print("=== ASCII DOPPLER WEATHER RADAR ===");
        
        # Build a single empty row string
        empty_row = list();
        for x in range(&width) {
            empty_row.append(C_DARK + " " + C_RESET);
        }
        empty_str = "".join(empty_row);
        
        # Print the empty radar grid
        for y in range(&height) {
            draw(empty_str);
        }
    }

    func &draw() {
        # Overwrite the header at Line 1, Column 1
        print("\e[1;1H=== ASCII DOPPLER WEATHER RADAR ===");
        
        for y in range(&height) {
            row_chars = list();
            
            for x in range(&width) {
                max_intensity = 0;
                
                for storm in &storms {
                    dx = float(x) - storm.x;
                    dy = (float(y) - storm.y) * 2.0;
                    
                    dist_sq = (dx * dx) + (dy * dy);
                    
                    if dist_sq < storm.radius_sq {
                        intensity = 4 - int((dist_sq / storm.radius_sq) * 4);
                        
                        if randInt(1, 100) < 30 {
                            intensity -= 1;
                        }
                        
                        if intensity > max_intensity {
                            max_intensity = intensity;
                        }
                    }
                }
                
                if max_intensity >= 3 and randInt(1, 100) <= 2 {
                    max_intensity = 5;
                }
                
                if max_intensity <= 0 {
                    row_chars.append(C_DARK + " " + C_RESET);
                } elif max_intensity == 1 {
                    row_chars.append(C_CYAN + "," + C_RESET);
                } elif max_intensity == 2 {
                    row_chars.append(C_BLUE + ":" + C_RESET);
                } elif max_intensity == 3 {
                    row_chars.append(C_MAGENTA + "*" + C_RESET);
                } elif max_intensity == 4 {
                    row_chars.append(C_RED + "#" + C_RESET);
                } else {
                    row_chars.append(C_YELLOW + "Z" + C_RESET);
                }
            }
            
            # Position the cursor at the start of the specific line before printing.
            # We use y + 2 because Line 1 is taken by our text header.
            cursor_pos = "\e[" + str(y + 2) + ";1H";
            print(cursor_pos + "".join(row_chars));
        }
    }
}

func sleep(ms) {
    start_time = time();
    while (time() - start_time) < ms {
        waiting = true;
    }
}

func main() {
    radar = WeatherRadar(70, 25);
    
    radar.addStorm(StormCell(10, 12, 22, 1.6, 0.1));
    radar.addStorm(StormCell(-5, 22, 12, 1.5, -0.4));
    radar.addStorm(StormCell(50, 8, 10, 0.3, 0.2));

    # 1. Print the baseline empty radar
    radar.initRadar();

    # 2. Update and draw the moving frames in-place
    for frame in range(100) {
        radar.draw();
        radar.update();
        sleep(100);
    }
    
    # Move cursor to the bottom of the radar before printing the final message
    print("\e[28;1H\nStorm systems have cleared the region.");
}

main();