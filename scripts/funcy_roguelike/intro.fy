# intro.fy — Animated title sequence for Funcy Roguelike
import "config.fy";

# Busy-wait delay using built-in time() which returns ms.
func delayMs(ms) {
    start = time();
    while time() - start < ms { }   # simple, portable
}

func showIntroTitle() {
    # Clear screen and go to top
    print("\e[H\e[J");
    
    # Initial dramatic pause
    delayMs(800);
    
    # Phase 1: Matrix-style falling characters
    title = "FUNCY ROGUELIKE";
    subtitle = "DESCEND INTO DANGER";
    
    # Create falling effect with random characters
    for frame = 0, frame < 15, frame += 1 {
        print("\e[H\e[J");
        print("");                    # Line 1
        print("");                    # Line 2
        print("");                    # Line 3
        
        # Show falling characters that eventually form the title
        if frame < 8 {
            # Random falling characters
            for line = 4, line <= 8, line += 1 {
                random_chars = "";
                for c = 0, c < 20, c += 1 {
                    if randInt(1, 10) <= 3 {
                        random_chars = random_chars + "X";
                    } else {
                        random_chars = random_chars + " ";
                    }
                }
                print("           " + random_chars);
            }
        } else {
            # Title starts appearing
            reveal_length = frame - 7;
            if reveal_length > length(title) {
                reveal_length = length(title);
            }
            revealed_title = "";
            for i = 0, i < reveal_length, i += 1 {
                revealed_title = revealed_title + title[i];
            }
            print("           " + revealed_title);  # Line 4 - TITLE ALWAYS HERE
            print("");                # Line 5
            print("");                # Line 6
            print("");                # Line 7
            print("");                # Line 8
        }
        
        print("");                    # Line 9
        print("");                    # Line 10
        delayMs(150);
    }
    
    # Phase 2: Title glitch effect
    delayMs(500);
    for glitch = 0, glitch < 6, glitch += 1 {
        print("\e[H\e[J");
        print("");                    # Line 1
        print("");                    # Line 2
        print("");                    # Line 3
        
        # Glitch the title with random characters
        if glitch % 2 == 0 {
            print("           " + title);    # Line 4 - Same title, different effect
        } else {
            glitched_title = "";
            for i = 0, i < length(title), i += 1 {
                if randInt(1, 10) <= 3 {
                    glitched_title = glitched_title + "?";
                } else {
                    glitched_title = glitched_title + title[i];
                }
            }
            print("           " + glitched_title);  # Line 4 - Glitched version
        }
        
        print("");                    # Line 5
        print("");                    # Line 6
        print("");                    # Line 7
        print("");                    # Line 8
        print("");                    # Line 9
        delayMs(100);
    }
    
    # Phase 3: Subtitle types out from left to right
    delayMs(300);
    for slide = 0, slide <= length(subtitle), slide += 1 {
        print("\e[H\e[J");
        print("");                    # Line 1
        print("");                    # Line 2
        print("");                    # Line 3
        print("           " + title);        # Line 4 - TITLE ALWAYS HERE
        
        # Show subtitle with letters appearing from left to right
        visible_part = "";
        for i = 0, i < slide, i += 1 {
            visible_part = visible_part + subtitle[i];
        }
        print("           " + visible_part); # Line 5
        
        print("");                    # Line 6
        print("");                    # Line 7
        print("");                    # Line 8
        print("");                    # Line 9
        print("");                    # Line 10
        delayMs(80);
    }
    
    # Phase 4: Description lines with wave effect
    delayMs(600);
    desc_lines = [
        "",
        "A text-based roguelike adventure",
        "written in the Funcy language",
        "",
        "Press any key to begin your journey..."
    ];
    
    # Wave effect - lines appear with different timing
    for wave = 0, wave < 3, wave += 1 {
        print("\e[H\e[J");
        print("");                    # Line 1
        print("");                    # Line 2
        print("");                    # Line 3
        print("           " + title);        # Line 4 - TITLE ALWAYS HERE
        print("           " + subtitle);     # Line 5
        
        # Show lines with wave timing
        for i = 0, i < length(desc_lines), i += 1 {
            line = desc_lines[i];
            if i <= wave * 2 {
                print("           " + line); # Lines 6, 7, 8...
            } else {
                print("");                   # Empty lines
            }
        }
        
        # Fill remaining lines
        remaining = 10 - (6 + wave * 2);
        for r = 0, r < remaining, r += 1 {
            print("");
        }
        delayMs(400);
    }
    
    # Phase 5: Pulsing glow effect
    delayMs(800);
    for pulse = 0, pulse < 4, pulse += 1 {
        print("\e[H\e[J");
        print("");                    # Line 1
        print("");                    # Line 2
        print("");                    # Line 3
        print("           " + title);        # Line 4 - TITLE ALWAYS HERE
        print("           " + subtitle);     # Line 5
        
        # Pulse the description lines
        for i = 0, i < length(desc_lines), i += 1 {
            line = desc_lines[i];
            if line != "" {
                # Add pulsing effect with extra spaces
                pulse_spaces = "";
                for p = 0, p < pulse, p += 1 {
                    pulse_spaces = pulse_spaces + " ";
                }
                print("           " + pulse_spaces + line);
            } else {
                print("           " + line);
            }
        }
        
        # Fill remaining lines
        remaining = 10 - (5 + length(desc_lines));
        for r = 0, r < remaining, r += 1 {
            print("");
        }
        delayMs(300);
    }
    
    # Phase 6: Final dramatic pause
    delayMs(1000);
    
    # Wait for user input
    _ = input("");
    
    # Phase 7: Explosive exit effect
    for explode = 0, explode < 5, explode += 1 {
        print("\e[H\e[J");
        print("");                    # Line 1
        print("");                    # Line 2
        print("");                    # Line 3
        print("           " + title);        # Line 4 - TITLE ALWAYS HERE
        print("           " + subtitle);     # Line 5
        
        # Explosion effect with expanding spaces
        for i = 0, i < length(desc_lines), i += 1 {
            line = desc_lines[i];
            if line != "" {
                explosion_spaces = "";
                for e = 0, e < explode * 2, e += 1 {
                    explosion_spaces = explosion_spaces + " ";
                }
                print("           " + explosion_spaces + line);
            } else {
                print("           " + line);
            }
        }
        
        # Fill remaining lines
        remaining = 10 - (5 + length(desc_lines));
        for r = 0, r < remaining, r += 1 {
            print("");
        }
        delayMs(150);
    }
    
    # Phase 8: Loading sequence with spinning effect
    print("\e[H\e[J");
    loading_messages = [
        "Preparing your adventure...",
        "Forging the dungeon...",
        "Summoning monsters...",
        "Generating your first level..."
    ];
    
    for i = 0, i < length(loading_messages), i += 1 {
        # Spinning effect
        for spin = 0, spin < 4, spin += 1 {
            spinner = ["|", "/", "-", "\\"];
            print("           " + loading_messages[i] + " " + spinner[spin]);
            delayMs(200);
            print("\e[H\e[J");
        }
    }
    
    # Final clear
    print("\e[H\e[J");
}
