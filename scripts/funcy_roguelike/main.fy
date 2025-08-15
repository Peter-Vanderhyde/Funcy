# main.fy — main loop wiring
import "config.fy";
import "game.fy";

func main() {
    g = Game();

    while true {
        g.render();
        if g.dead { print("\nYou died. Game over."); break; }

        cmd = input("[WASD]=move  f=pick  i=inv  r=stats  v=fov  t=trade  q=quit > ").strip();
        if cmd == "q" or cmd == "Q" { break; }

        if cmd == "i" or cmd == "I" { g.useInventory(); continue; }
        if cmd == "r" or cmd == "R" { g.showStats(); continue; }          # uppercase S = stats
        if cmd == "f" or cmd == "F" { g.pickup(); continue; }
        if cmd == "t" or cmd == "T" { g.talkToTrader(); continue; }

        if cmd == "v" or cmd == "V" {
            g.fov_enabled = not g.fov_enabled;
            state = "OFF"; if g.fov_enabled { state = "ON"; }
            g.message = "FOV: " + state;
            continue;
        }

        if cmd == "w" or cmd == "W" { g.tryMove(0, -1); continue; }
        if cmd == "s" or cmd == "S" { g.tryMove(0, 1); continue; }
        if cmd == "a" or cmd == "A" { g.tryMove(-1, 0); continue; }
        if cmd == "d" or cmd == "D" { g.tryMove(1, 0); continue; }
    }
}

main();