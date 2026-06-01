/* fractal_tree.fy - Procedural ASCII Tree using Recursive BFS */

class Canvas {
    &width = 80;
    &height = 40;
    &grid = [];

    func &Canvas(w, h) {
        &width = w;
        &height = h;
        &clear();
    }

    func &clear() {
        &grid = [];
        for y = 0, y < &height, y += 1 {
            row = [];
            for x = 0, x < &width, x += 1 {
                row.append(" ");
            }
            &grid.append(row);
        }
    }

    func &setPixel(x, y, char, color) {
        if x >= 0 and x < &width and y >= 0 and y < &height {
            &grid[y][x] = color + char + "\e[0m";
        }
    }

    # Bresenham's Line Algorithm to draw the branches
    func &drawLine(x0, y0, x1, y1, char, color) {
        dx = abs(x1 - x0);
        sx = -1; if x0 < x1 { sx = 1; }
        
        dy = -abs(y1 - y0);
        sy = -1; if y0 < y1 { sy = 1; }
        
        err = dx + dy;

        while true {
            &setPixel(x0, y0, char, color);
            if x0 == x1 and y0 == y1 { break; }
            
            e2 = 2 * err;
            if e2 >= dy { err += dy; x0 += sx; }
            if e2 <= dx { err += dx; y0 += sy; }
        }
    }

    func &render() {
        draw("\e[H\e[J"); # Clear Screen
        for row in &grid {
            line = "";
            for cell in row { line += cell; }
            draw(line);
        }
        # Add a delay so we can watch it grow
        start = time();
        while time() - start < 1000 {
            continue;
        }
    }
}

class FractalGenerator {
    &canvas = Null;

    func &FractalGenerator(canvas_obj) {
        &canvas = canvas_obj;
    }

    func &growTree() {
        # Queue holds: [x, y, dx, dy, length, depth]
        # We start at bottom center (40, 39) pointing straight up (dx:0, dy:-1)
        queue = [[40.0, 39.0, 0.0, -1.0, 11.0, 8]];
        
        current_depth = 8;
        
        while length(queue) > 0 {
            branch = queue[0];
            queue.pop(0);
            
            x = branch[0]; y = branch[1];
            dx = branch[2]; dy = branch[3];
            length_val = branch[4]; depth = branch[5];
            
            # If we drop a depth level, render the screen so it animates layer by layer
            if depth < current_depth {
                &canvas.render();
                current_depth = depth;
            }
            
            if depth == 0 { continue; }
            
            # Calculate the end point of this branch
            nx = x + dx * length_val;
            ny = y + dy * length_val;
            
            # Change color and character based on how far up the tree we are
            color = "\e[33m"; # Wood (Yellow/Brown)
            char = "|";
            
            if depth <= 4 { 
                color = "\e[32m"; # Dark Green leaves
                char = "*"; 
            }
            if depth <= 2 {
                color = "\e[92m"; # Light Green tips
                char = "@";
            }
            
            # Draw the branch
            &canvas.drawLine(int(x), int(y), int(nx), int(ny), char, color);
            
            # Branching Math: Rotate the vector by +/- 30 degrees
            # cos(30) ~ 0.866, sin(30) ~ 0.500
            c = 0.866; 
            s = 0.500;
            
            # Terminal scaling trick: we multiply dx by 1.5 because terminal 
            # characters are taller than they are wide. This stops the tree from looking squashed.
            adjusted_dx = dx * 1.5;
            
            # Spawn Right Branch
            r_dx = adjusted_dx * c - dy * s;
            r_dy = adjusted_dx * s + dy * c;
            queue.append([nx, ny, r_dx / 1.5, r_dy, length_val * 0.76, depth - 1]);
            
            # Spawn Left Branch
            l_dx = adjusted_dx * c - dy * -s;
            l_dy = adjusted_dx * -s + dy * c;
            queue.append([nx, ny, l_dx / 1.5, l_dy, length_val * 0.76, depth - 1]);
        }
        
        # Final render to show the outermost leaves
        &canvas.render();
        print("\nFractal Growth Complete!");
    }
}

# --- Execution ---
canvas = Canvas(80, 40);
fractal = FractalGenerator(canvas);

print("Planting seed...");

fractal.growTree();