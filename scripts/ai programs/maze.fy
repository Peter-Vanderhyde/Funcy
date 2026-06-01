/* maze_final.fy - Animated A* with final path rendering */

func reconstruct_path(came_from, current) {
    path = [current];
    while str(current) in came_from {
        current = came_from[str(current)];
        path.append(current);
    }
    return path;
}

class Maze {
    &width = 0; &height = 0; &grid = [];

    func &Maze(w, h) {
        &width = w; &height = h; &generateEmpty();
    }

    func &generateEmpty() {
        &grid = [];
        for y in range(&height) {
            row = [];
            for x in range(&width) { row.append(1); }
            &grid.append(row);
        }
    }

    func &generate(x, y) {
        &grid[y][x] = 0;
        dirs = randShuffle([[0, -2], [0, 2], [-2, 0], [2, 0]]);
        for [dx, dy] in dirs {
            nx = x + dx; ny = y + dy;
            if nx > 0 and nx < &width - 1 and ny > 0 and ny < &height - 1 and &grid[ny][nx] == 1 {
                &grid[y + dy // 2][x + dx // 2] = 0;
                &generate(nx, ny);
            }
        }
        &grid[1][0] = 0;
        &grid[&height - 2][&width - 1] = 0;
    }

    # Simulation of a clear screen and delay
    func &pause() {
        /* Loops to create a visual delay */
        start = time();
        while time() - start < 100 {
            continue;
        }
    }

    func &solveAStar(start, end) {
        open_set = [start];
        came_from = {};
        g_score = {str(start): 0};

        while open_set.size() > 0 {
            current = open_set[0];
            open_set.pop(0);

            # Clear terminal and render step
            print("\e[H\e[J");
            print("Solving Maze...");
            &display([current]);
            &pause();
            
            if current == end { return reconstruct_path(came_from, current); }

            [cx, cy] = current;
            for [dx, dy] in [[0, 1], [0, -1], [1, 0], [-1, 0]] {
                neighbor = [cx + dx, cy + dy];
                [nx, ny] = neighbor;
                
                if nx >= 0 and nx < &width and ny >= 0 and ny < &height and &grid[ny][nx] == 0 {
                    tentative_g = g_score.get(str(current), 0) + 1;
                    if tentative_g < g_score.get(str(neighbor), 9999) {
                        came_from[str(neighbor)] = current;
                        g_score[str(neighbor)] = tentative_g;
                        open_set.append(neighbor);
                    }
                }
            }
        }
    }

    func &display(visited=[], path=[]) {
        for y in range(&height) {
            line = "";
            for x in range(&width) {
                if [x, y] in path { line += "**"; }
                elif [x, y] in visited { line += "::"; }
                elif &grid[y][x] == 1 { line += "##"; }
                else { line += "  "; }
            }
            print(line);
        }
    }
}

# --- Execution ---
m = Maze(21, 15);
m.generate(1, 1);
path = m.solveAStar([1, 1], [19, 13]);

# Draw final path
print("\e[H\e[J");
print("Solved!");
m.display([], path);