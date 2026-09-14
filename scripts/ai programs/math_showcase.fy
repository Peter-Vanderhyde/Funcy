# math_showcase.fy - Complete 8-Mode Funcy Benchmark & Math Visualizer

print("\e[H\e[J"); 
print("=== Funcy Extended Math Showcase ===");
print("1. Smooth Polar Spiral (atan2, hypot, PI)");
print("2. Hyperbolic Tunnel (cosh, log10, clamp)");
print("3. Geometric Starburst (atan2, hypot, degrees)");
print("4. Polar Flower Wave (atan2, clamp, PI)");
print("5. Logarithmic Ripple Matrix (log10, fmod, clamp)");
print("6. Fast 3D Wireframe Cube (Bounding-Box Bypassed)");
print("7. Dual Lissajous Interference (atan2, hypot)");
print("8. Pulsing Infinity Lattice (degrees, radians, sqrt)");
choice = input("Select a visual mode (1-8) -> ");

scale = 1.0; 
width = int(60.0 * scale);
height = int(20.0 * scale);
iterations = 100;

shades = [" ", ".", ":", "-", "=", "+", "*", "#", "%", "@"];
num_shades_float = float(length(shades)) - 1.0;

showCursor(false);
start_time = time();

for frame = 0, frame < iterations, frame += 1 {
    f_frame = float(frame);
    
    # Pre-calculated data structures for Mode 6 (runs once per frame)
    edge_px1 = []; edge_py1 = [];
    edge_dx  = []; edge_dy  = [];
    edge_len_sq = [];

    if choice == "6" {
        rot_y = f_frame * 0.08;
        rot_x = f_frame * 0.05;
        fov = 2.5;
        s = 0.8;
        
        # Calculate all 12 cube edges once before looping over pixels
        for axis = 0, axis < 3, axis += 1 {
            for i = -1, i <= 1, i += 2 {
                for j = -1, j <= 1, j += 2 {
                    fi = float(i) * s;
                    fj = float(j) * s;
                    
                    if axis == 0 {
                        x1 = -s; y1 = fi; z1 = fj;
                        x2 =  s; y2 = fi; z2 = fj;
                    } elif axis == 1 {
                        x1 = fi; y1 = -s; z1 = fj;
                        x2 = fi; y2 =  s; z2 = fj;
                    } else {
                        x1 = fi; y1 = fj; z1 = -s;
                        x2 = fi; y2 = fj; z2 =  s;
                    }
                    
                    # 1. Rotate endpoints around Y axis
                    rx1 = x1 * math.cos(rot_y) + z1 * math.sin(rot_y);
                    rz1 = -x1 * math.sin(rot_y) + z1 * math.cos(rot_y);
                    rx2 = x2 * math.cos(rot_y) + z2 * math.sin(rot_y);
                    rz2 = -x2 * math.sin(rot_y) + z2 * math.cos(rot_y);
                    
                    # 2. Rotate endpoints around X axis
                    ry1 = y1 * math.cos(rot_x) - rz1 * math.sin(rot_x);
                    rz1 = y1 * math.sin(rot_x) + rz1 * math.cos(rot_x);
                    ry2 = y2 * math.cos(rot_x) - rz2 * math.sin(rot_x);
                    rz2 = y2 * math.sin(rot_x) + rz2 * math.cos(rot_x);
                    
                    # 3. 3D to 2D Perspective Projection
                    p1x = (rx1 * fov) / (rz1 + 3.0);
                    p1y = (ry1 * fov) / (rz1 + 3.0);
                    p2x = (rx2 * fov) / (rz2 + 3.0);
                    p2y = (ry2 * fov) / (rz2 + 3.0);

                    dx = p2x - p1x;
                    dy = p2y - p1y;
                    len_sq = (dx * dx) + (dy * dy);

                    # Cache line segment specs into lists
                    edge_px1.append(p1x);
                    edge_py1.append(p1y);
                    edge_dx.append(dx);
                    edge_dy.append(dy);
                    edge_len_sq.append(len_sq);
                }
            }
        }
    }

    # Frame Buffer Setup
    frame_buffer = "\e[H";

    # Pixel Rendering Loop
    for y = 0, y < height, y += 1 {
        for x = 0, x < width, x += 1 {
            
            # Center and normalize coordinates
            nx = (float(x) - float(width) / 2.0) / (10.0 * scale);
            ny = (float(y) - float(height) / 2.0) / (5.0 * scale);
            
            r = math.hypot(nx, ny); 
            angle = math.atan2(ny, nx); 
            
            value = 0.0;
            
            if choice == "1" {
                # --- Mode 1: Smooth Polar Spiral ---
                spiral_freq = 3.0;
                spin_speed = f_frame * 0.15;
                value = math.sin((angle * spiral_freq) - (r * 2.0) + spin_speed);
                
            } elif choice == "2" {
                # --- Mode 2: Hyperbolic Tunnel ---
                depth = math.log10(r + 0.1) * 3.0;
                hyperbolic_wave = math.cosh(math.clamp(nx, -1.5, 1.5)) * 0.5;
                value = math.cos(depth - (f_frame * 0.2) + hyperbolic_wave);
                
            } elif choice == "3" {
                # --- Mode 3: Geometric Starburst ---
                deg_rays = math.sin(math.radians(math.degrees(angle) * 6.0));
                wave_pulse = math.cos((r * 4.0) - (f_frame * 0.2));
                value = math.clamp(deg_rays * wave_pulse, -1.0, 1.0);
                
            } elif choice == "4" {
                # --- Mode 4: Polar Flower Wave ---
                petals = math.cos(angle * 5.0 + (f_frame * 0.1));
                flower_r = 1.5 + (0.8 * petals);
                diff = math.abs(r - flower_r);
                value = math.clamp(math.cos(diff * 5.0), -1.0, 1.0);

            } elif choice == "5" {
                # --- Mode 5: Logarithmic Ripple Matrix ---
                log_r = math.log10(r + 0.5) * 8.0;
                grid = math.fmod(log_r - (f_frame * 0.3), 1.0);
                wave = math.sin(angle * 4.0 + (log_r * 2.0));
                value = math.clamp(grid * wave, -1.0, 1.0);
                
            } elif choice == "6" {
                # --- Mode 6: 3D Wireframe Cube ---
                # Early rejection: Skip math if pixel is beyond the cube's bounding radius
                if r > 1.4 {
                    value = -1.0;
                } else {
                    min_edge_dist = 999.0;
                    
                    # Fast distance checks against 12 precomputed edges
                    for k = 0, k < 12, k += 1 {
                        l_sq = edge_len_sq[k];
                        p1x = edge_px1[k]; 
                        p1y = edge_py1[k];
                        dx = edge_dx[k];   
                        dy = edge_dy[k];
                        
                        t = 0.0;
                        if l_sq > 0.0001 {
                            proj = ((nx - p1x) * dx + (ny - p1y) * dy) / l_sq;
                            t = math.clamp(proj, 0.0, 1.0);
                        }
                        
                        cx = p1x + (t * dx);
                        cy = p1y + (t * dy);
                        d = math.hypot(nx - cx, ny - cy);
                        
                        if d < min_edge_dist {
                            min_edge_dist = d;
                        }
                    }
                    
                    line_val = 1.0 - (min_edge_dist * 12.0);
                    value = math.clamp(line_val, -1.0, 1.0);
                    if value < -0.4 { 
                        value = -1.0; 
                    }
                }
                
            } elif choice == "7" {
                # --- Mode 7: Dual Lissajous Interference ---
                wave_x = math.sin((nx * 4.0) + (f_frame * 0.15));
                wave_y = math.cos((ny * 6.0) - (f_frame * 0.1));
                lissajous = math.atan2(wave_y, wave_x);
                value = math.sin((lissajous * 3.0) + (r * 3.0));
                
            } else {
                # --- Mode 8: Pulsing Infinity Lattice ---
                deg = math.degrees(angle);
                lemniscate = math.cos(math.radians(deg * 2.0));
                inf_r = math.sqrt(math.abs(lemniscate)) * 2.0;
                diff = math.abs(r - inf_r);
                value = math.clamp(math.cos((diff * 6.0) - (f_frame * 0.2)), -1.0, 1.0);
            }
            
            # Map [-1.0, 1.0] -> [0, length(shades) - 1]
            normalized = (value + 1.0) / 2.0;
            index = int(math.round(normalized * num_shades_float));
            index = max(0, min(length(shades) - 1, index));
            
            frame_buffer += shades[index];
        }
        frame_buffer += "\n";
    }
    
    # Print the entire frame buffer to terminal in a single operation
    print(frame_buffer);
}

end_time = time();
showCursor(true);

total_time = end_time - start_time;
float_avg = float(total_time) / float(iterations);
avg_time = math.round(float_avg * 100.0) / 100.0;

print("\n--- Benchmark Complete ---");
print("Rendered " + str(iterations) + " frames at " + str(width) + "x" + str(height) + ".");
print("Total execution time: " + str(total_time) + " ms");
print("Average frame rate: " + str(math.round(1000.0 / float_avg)) + " FPS (" + str(avg_time) + " ms/frame)");