/* wildfire.fy - Interactive Cellular Automata
   Generates a living grid of trees, fire, and empty dirt.
   Watch nature fight back or completely burn away!
*/

func main() {
    # 1. Configuration
    width = 40;
    height = 20;
    generations = 100;
    
    # Elemental States
    DIRT = "  ";
    TREE = "\e[32m%%\e[0m";  # Green ASCII tree
    FIRE = "\e[31m##\e[0m";  # Red ASCII fire
    
    # 2. Build the initial grid randomly using map/lists
    grid = [];
    for y in range(height) {
        row = [];
        for x in range(width) {
            roll = randInt(1, 100);
            if roll <= 75 {
                row.append(TREE);
            } elif roll <= 78 {
                row.append(FIRE); # A few initial sparks
            } else {
                row.append(DIRT);
            }
        }
        grid.append(row);
    }
    
    print("\e[2J"); # Clear screen once at the beginning
    
    # 3. Main Simulation Loop
    for gen = 0, gen < generations, gen += 1 {
        # Jump cursor back to the top-left corner (\e[H) for smooth updates
        print("\e[H--- WILDFIRE SIMULATOR | Generation: " + str(gen + 1) + " ---");
        
        # Display current grid state
        for y in range(height) {
            line = "";
            for x in range(width) {
                line += grid[y][x];
            }
            draw(line);
        }
        
        # Create a blank grid for the next generation
        next_grid = [];
        for y in range(height) {
            row = [];
            for x in range(width) {
                row.append(DIRT);
            }
            next_grid.append(row);
        }
        
        # 4. Process Rules for Next Generation
        for y in range(height) {
            for x in range(width) {
                current = grid[y][x];
                
                if current == FIRE {
                    # Fire burns out into empty dirt
                    next_grid[y][x] = DIRT;
                } elif current == DIRT {
                    # Empty dirt has a tiny chance to sprout a new tree
                    if randInt(1, 100) <= 2 {
                        next_grid[y][x] = TREE;
                    } else {
                        next_grid[y][x] = DIRT;
                    }
                } elif current == TREE {
                    # Check 4 cardinal neighbors for fire
                    has_fire_neighbor = false;
                    
                    # Up
                    if y > 0 and grid[y - 1][x] == FIRE { has_fire_neighbor = true; }
                    # Down
                    if y < height - 1 and grid[y + 1][x] == FIRE { has_fire_neighbor = true; }
                    # Left
                    if x > 0 and grid[y][x - 1] == FIRE { has_fire_neighbor = true; }
                    # Right
                    if x < width - 1 and grid[y][x + 1] == FIRE { has_fire_neighbor = true; }
                    
                    if has_fire_neighbor {
                        next_grid[y][x] = FIRE; # Catch fire!
                    } else {
                        # Spontaneous lightning strike chance
                        if randInt(1, 1000) <= 3 {
                            next_grid[y][x] = FIRE;
                        } else {
                            next_grid[y][x] = TREE; # Stay safe
                        }
                    }
                }
            }
        }
        
        # Advance the grid state
        grid = next_grid;
        
        # Burn some CPU time to create a 150ms delay between frames
        sleep(150);
    }
    
    print("\nSimulation complete!");
}

showCursor(false);
main();
showCursor();