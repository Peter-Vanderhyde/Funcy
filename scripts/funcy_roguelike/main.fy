# main.fy — entry point and input loop
import "config.fy";
import "map.fy";
import "entities.fy";
import "game.fy";


func main() {
    g = Game();
    while true {
        g.render();
        cmd = input("[WASD]=move  f=pick  i=inv  r=stats  v=toggle-fov  q=quit > ").strip();
        key = "";
        if length(cmd) > 0 { key = cmd[0]; }

        if key == "q" or key == "Q" {
            print("Goodbye!");
            return;
        }
        if key == "i" or key == "I" { g.useInventory(); continue; }
        if key == "r" or key == "R" { g.showStats(); continue; }
        if key == "f" or key == "F" { g.pickup(); continue; }
        if key == "v" or key == "V" {
            g.fov_enabled = not g.fov_enabled;
            state = "OFF";
            if g.fov_enabled { state = "ON"; }
            g.message = "Visibility: " + state;
            continue;
        }

        dx = 0; dy = 0;
        if key == "w" or key == "W" { dy = -1; }
        elif key == "s" or key == "S" { dy = 1; }
        elif key == "a" or key == "A" { dx = -1; }
        elif key == "d" or key == "D" { dx = 1; }
        else { g.message = "Unknown command."; continue; }

        g.tryMove(dx, dy);

        if g.dead {
            g.render();
            print("");
            print("You died on depth " + str(g.player.depth) + ". Game Over!");
            return;
        }
    }
}

main();
