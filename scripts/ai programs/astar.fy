/* Pathfinding Visualizer in Funcy
   Animates A*, Dijkstra, and BFS algorithms navigating a randomized maze.
*/

# Terminal formatting constants for colors
C_RESET  = "\e[0m";
C_WHITE  = "\e[97m";  # Walls
C_DARK   = "\e[90m";  # Empty space
C_CYAN   = "\e[96m";  # Visited nodes (Closed List)
C_BLUE   = "\e[94m";  # Frontier nodes (Open List)
C_YELLOW = "\e[93m";  # The Final Path
C_GREEN  = "\e[92m";  # Start Node
C_RED    = "\e[91m";  # End Node

# Grid State Constants
EMPTY = 0;
WALL = 1;
VISITED = 2;
FRONTIER = 3;
PATH = 4;
START = 5;
END = 6;

func sleep(ms) {
    start_time = time();
    while (time() - start_time) < ms {
        waiting = true;
    }
}

class Node {
    &x = 0;
    &y = 0;
    &g = 0;     # Cost from start
    &h = 0;     # Heuristic (estimated cost to end)
    &f = 0;     # Total cost (g + h)
    &parent = Null;

    func &Node(x, y, g, h, parent) {
        &x = x;
        &y = y;
        &g = g;
        &h = h;
        &f = g + h;
        &parent = parent;
    }
}

class PathVisualizer {
    &width = 0;
    &height = 0;
    &grid = [];
    &dirty_rows = [];
    
    &open_list = [];
    &running = true;
    &path_found = false;

    &start_x = 2;
    &start_y = 2;
    &target_x = 0;
    &target_y = 0;

    func &PathVisualizer(width, height) {
        &width = width;
        &height = height;
        &grid = list();
        &dirty_rows = list();
        &open_list = list();

        &target_x = width - 3;
        &target_y = height - 3;

        # Initialize the grid
        for y in range(&height) {
            &dirty_rows.append(true); # Force a full draw on frame 1

            row = list();
            for x in range(&width) {
                # 25% chance to spawn a wall
                if randInt(1, 100) <= 25 {
                    row.append(WALL);
                } else {
                    row.append(EMPTY);
                }
            }
            &grid.append(row);
        }

        # Carve out start and end points to guarantee they aren't walls
        &grid[&start_y][&start_x] = START;
        &grid[&target_y][&target_x] = END;
    }

    # Clears paths and visited nodes, leaving walls intact
    func &resetGrid() {
        &open_list = list();
        &running = true;
        &path_found = false;
        
        for y in range(&height) {
            &dirty_rows[y] = true; # Force a full redraw
            for x in range(&width) {
                state = &grid[y][x];
                if state == VISITED or state == FRONTIER or state == PATH {
                    &grid[y][x] = EMPTY;
                }
            }
        }
    }

    # Initializes the starting node based on the algorithm type
    func &startAlgorithm(algo) {
        &resetGrid();
        h = 0;
        
        if algo == "AStar" {
            h = abs(&start_x - &target_x) + abs(&start_y - &target_y);
        }
        
        start_node = Node(&start_x, &start_y, 0, h, Null);
        &open_list.append(start_node);
    }

    # Performs one iteration of the chosen algorithm
    func &update(algo) {
        if not &running {
            return Null;
        }

        if &open_list.size() == 0 {
            &running = false;
            print("\e[" + str(&height + 2) + ";1H\e[2K" + C_RED + "No valid path found!" + C_RESET);
            return Null;
        }

        # 1. Pop the next node based on the algorithm
        best_idx = 0;
        
        if algo != "BFS" {
            # Dijkstra and A* seek the lowest 'f' score
            best_f = &open_list[0].f;
            for i = 1, i < &open_list.size(), i += 1 {
                if &open_list[i].f < best_f {
                    best_f = &open_list[i].f;
                    best_idx = i;
                }
            }
        }

        # Remove it from the frontier
        current = &open_list.pop(best_idx);

        # 2. Check if we reached the target
        if current.x == &target_x and current.y == &target_y {
            &running = false;
            &path_found = true;
            
            # Backtrack to build the final path
            curr = current.parent; 
            while curr.parent != Null {
                &grid[curr.y][curr.x] = PATH;
                &dirty_rows[curr.y] = true;
                curr = curr.parent;
            }
            return Null;
        }

        # Mark current node as visited (Closed)
        if &grid[current.y][current.x] != START {
            &grid[current.y][current.x] = VISITED;
            &dirty_rows[current.y] = true;
        }

        # 3. Explore neighbors (Up, Down, Left, Right)
        directions = [[0, -1], [0, 1], [-1, 0], [1, 0]];
        
        for [dx, dy] in directions {
            nx = current.x + dx;
            ny = current.y + dy;

            # Bounds checking
            if nx >= 0 and nx < &width and ny >= 0 and ny < &height {
                state = &grid[ny][nx];
                
                # Only process empty spaces or our destination
                if state == EMPTY or state == END {
                    g_cost = current.g + 1;
                    h_cost = 0;
                    
                    if algo == "AStar" {
                        h_cost = abs(nx - &target_x) + abs(ny - &target_y);
                    }
                    
                    new_node = Node(nx, ny, g_cost, h_cost, current);
                    &open_list.append(new_node);
                    
                    if state != END {
                        &grid[ny][nx] = FRONTIER;
                        &dirty_rows[ny] = true;
                    }
                }
            }
        }
    }

    # Draws the blank grid first
    func &initVisualizer() {
        draw("\e[H\e[J"); 
        
        empty_row = list();
        for x in range(&width) {
            empty_row.append(" ");
        }
        empty_str = "".join(empty_row);
        
        for y in range(&height) {
            draw(empty_str);
        }
    }

    # Overwrites grid rows in-place using dirty flags
    func &draw() {
        for y in range(&height) {
            
            # Only rebuild and print the line if it has changed
            if &dirty_rows[y] {
                row_chars = list();
                
                for x in range(&width) {
                    state = &grid[y][x];
                    
                    if state == EMPTY {
                        row_chars.append(C_DARK + " " + C_RESET);
                    } elif state == WALL {
                        row_chars.append(C_WHITE + "I" + C_RESET);
                    } elif state == VISITED {
                        row_chars.append(C_CYAN + "x" + C_RESET);
                    } elif state == FRONTIER {
                        row_chars.append(C_BLUE + "o" + C_RESET);
                    } elif state == PATH {
                        row_chars.append(C_YELLOW + "O" + C_RESET);
                    } elif state == START {
                        row_chars.append(C_GREEN + "S" + C_RESET);
                    } elif state == END {
                        row_chars.append(C_RED + "E" + C_RESET);
                    }
                }
                
                # Cursor positioned on line (y + 2)
                cursor_pos = "\e[" + str(y + 2) + ";1H";
                draw(cursor_pos + "".join(row_chars));

                # Reset the flag until it gets updated again
                &dirty_rows[y] = false; 
            }
        }
    }
}

func main() {
    # Initialize a 60x20 visualizer grid
    viz = PathVisualizer(60, 20);
    viz.initVisualizer();

    algorithms = ["AStar", "Dijkstra", "BFS"];
    
    for algo in algorithms {
        # Print the current algorithm header
        print("\e[1;1H\e[2K" + C_WHITE + "=== VISUALIZING: " + algo + " ===" + C_RESET);
        
        viz.startAlgorithm(algo);

        # Loop until the algorithm finds the end or runs out of valid moves
        while viz.running {
            viz.draw();
            viz.update(algo);
            # sleep(25); # 25ms per frame for a fast search animation
        }
        
        # Draw one final time to show the golden path
        viz.draw();
        
        # Print the success/fail message underneath the grid
        if viz.path_found {
            print("\e[22;1H\e[2K" + C_GREEN + algo + " completed! Press ENTER to continue..." + C_RESET);
        } else {
            print("\e[22;1H\e[2K" + C_RED + algo + " failed to find a path. Press ENTER to continue..." + C_RESET);
        }
        
        input(); # Wait for the user to press Enter
        
        # Clear the old completion message for the next run
        print("\e[22;1H\e[2K");
    }
    
    print("\e[22;1H\e[2K" + C_YELLOW + "All algorithms finished!" + C_RESET);
}

main();