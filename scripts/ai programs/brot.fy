/* Mandelbrot Set Panning, Zooming & Animation Player
   Renders frames to a file to avoid math overhead during playback.
*/

C_RESET  = "\e[0m";
C_BLACK  = "\e[30m";

C_GRAD = [
    "\e[38;5;18m", "\e[38;5;21m", "\e[38;5;27m", 
    "\e[38;5;33m", "\e[38;5;43m", "\e[38;5;76m", 
    "\e[38;5;208m", "\e[38;5;196m", "\e[38;5;198m"
];

CHARS = [" ", ".", ",", "-", "~", "=", "+", "*", "%", "#", "@"];

class MandelbrotVisualizer {
    &width = 0;
    &height = 0;
    &max_iters = 45;

    func &MandelbrotVisualizer(width, height) {
        &width = width;
        &height = height;
    }

    func &renderFrame(min_r, max_r, min_i, max_i) {
        frame_str = "\e[H\e[J\e[97m=== MANDELBROT CINEMATIC CAMERA ===\e[0m\n";
        all_lines = list();

        for row in range(&height // 2) {
            line_chars = list();
            row_f = 0.0 + row;
            ci = min_i + (row_f / (0.0 + &height - 1.0)) * (max_i - min_i);

            for col in range(&width) {
                col_f = 0.0 + col;
                cr = min_r + (col_f / (0.0 + &width - 1.0)) * (max_r - min_r);

                zr = 0.0;
                zi = 0.0;
                iters = 0;
                
                while (zr * zr + zi * zi) <= 4.0 and iters < &max_iters {
                    next_zr = zr * zr - zi * zi + cr;
                    zi = 2.0 * zr * zi + ci;
                    zr = next_zr;
                    iters = iters + 1;
                }
                
                if iters == &max_iters {
                    line_chars.append(C_BLACK + "@" + C_RESET);
                } else {
                    iters_f = 0.0 + iters;
                    max_f = 0.0 + &max_iters;
                    
                    char_idx = int((iters_f / max_f) * 10.0);
                    if char_idx > 10 { char_idx = 10; }
                    glyph = CHARS[char_idx];
                    
                    color_idx = iters % 9;
                    color = C_GRAD[color_idx];
                    
                    line_chars.append(color + glyph + C_RESET);
                }
            }
            line_str = "".join(line_chars);
            print("\e["+str(row)+";0H"+line_str);
            print("\e["+str(&height - 1 - row)+";0H"+line_str);
            if (row + 1 != &height // 2) {
                print("\e[" + str(row + 1) + ";0H" + " " * &width);
                print("\e[" + str(&height - 2 - row) + ";0H" + " " * &width);
            }
            all_lines.append(line_str);
        }
        all_lines += reversed(all_lines);
        
        frame_str = frame_str + "\n".join(all_lines) + "\n";
        return frame_str;
    }
}

func renderAndSave(frames_to_render, start_x, delta_x, delta_zoom) {
    width = 125;
    height = 50;
    
    writeFile("mandelbrot_anim.txt", "");
    target_r = start_x;
    target_i = 0.0;
    
    # Initial view span
    span_r = 3.0;
    span_i = 2.4;
    
    zoomer = MandelbrotVisualizer(width, height);
    print("Beginning render sequence...");
    
    for f in range(frames_to_render) {
        
        # --- DYNAMIC ITERATION SCALING ---
        # 1. Calculate how magnified we are compared to the default 3.0 span
        zoom_ratio = 3.0 / span_r;
        
        # 2. Apply a square-root curve (** 0.5) to calculate max_iters.
        # At span 3.0 (ratio 1), it evaluates to: 40 + 30(1) = 70 iterations.
        # At span 0.06 (ratio 50), it evaluates to: 40 + 30(~7.07) = 252 iterations.
        zoomer.max_iters = int(40.0 + 30.0 * (zoom_ratio ** 0.5));
        
        min_r = target_r - (span_r / 2.0);
        max_r = target_r + (span_r / 2.0);
        min_i = target_i - (span_i / 2.0);
        max_i = target_i + (span_i / 2.0);
        
        frame_data = zoomer.renderFrame(min_r, max_r, min_i, max_i);
        
        print("\e["+str(height)+";0H");
        print("Rendering Frame: " + str(f + 1) + " / " + str(frames_to_render));
        print("Current X (Real): " + str(target_r));
        print("Current Span (Zoom): " + str(span_r));
        print("Max Iterations: " + str(zoomer.max_iters));
        
        appendFile("mandelbrot_anim.txt", frame_data + "___FRAME_END___\n");
        
        target_r += delta_x;
        delta_x = delta_x * delta_zoom;
        span_r = span_r * delta_zoom;
        span_i = span_i * delta_zoom;
    }
}

func playAnimation(animation, fps_delay) {
    print("Loading animation data...");
    showCursor(false);
    content = readFile(animation);
    
    if content == Null {
        print("No animation file found! Please render the frames first.");
        return Null;
    }
    
    frames = content.split("___FRAME_END___\n");
    
    print("Starting playback... Press Ctrl+C to stop.");
    
    while true {
        for frame in frames {
            if frame.length() > 10 {
                draw(frame);
                sleep(fps_delay);
            }
        }
    }
    showCursor();
}

func main() {
    print("\e[H\e[J\e[97m=== MANDELBROT ANIMATION TOOL ===\e[0m");
    print("1. Render and Save New Animation");
    print("2. Play Saved Animation");
    
    choice = input("Enter choice (1 or 2) -> ");
    
    if choice == "1" {
        frames_str = input("How many frames to generate? -> ");
        N = float(frames_str);
        start_x = input("Enter starting x -> ");
        if not start_x.isDigit() and '.' not in start_x {
            start_x = 0.0;
        } else {
            start_x = float(start_x);
        }
        
        target_x = float(input("Enter target X (Real) [e.g., -1.75] -> "));
        target_zoom = float(input("Enter target Zoom (Span) [e.g., 0.06] -> "));
        
        # Starting values
        start_zoom = 3.0;
        
        # Calculate deltaZoom: Target = Start * (deltaZoom^N)
        # deltaZoom = (Target / Start)^(1/N)
        delta_zoom = (target_zoom / start_zoom) ** (1.0 / N);
        
        # Calculate deltaX: Target = deltaX * (1 - deltaZoom^N) / (1 - deltaZoom)
        # We solve for starting deltaX
        delta_x = (target_x - start_x) * (1.0 - delta_zoom) / (1.0 - (delta_zoom ** N));
        
        print("Calculated deltaX: " + str(delta_x));
        print("Calculated deltaZoom: " + str(delta_zoom));
        
        renderAndSave(int(N), start_x, delta_x, delta_zoom); 
        print("\e[92mRender complete! Run the script again and select option 2 to play.\e[0m");
    } elif choice == "2" {
        animation_file = input("Enter animation file -> ");
        if not animation_file {
            animation_file = "mandelbrot_anim.txt";
        }
        fps_str = input("Enter playback FPS (e.g., 20) -> ");
        
        fps = int(fps_str);
        if fps <= 0 {
            fps = 20; 
            print("Invalid FPS. Defaulting to 20.");
        }
        
        delay_ms = 1000 / fps;
        playAnimation(animation_file, delay_ms);
        
    } else {
        print("Invalid choice.");
    }
}

main();