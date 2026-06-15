# farkle.fy - A complete Farkle game written in Funcy!
# Added Rules: 
# 1. 4, 5, and 6 of a kind double the triplet score.
# 2. 500-point opening hurdle required to get on the board.
# 3. 1-6 Straight scores 1000 points.
# 4. Three Pairs scores 1500 points (4-of-a-kind + 1 pair counts as 3 pairs).

# Helper function to calculate the score of a given list of dice
func calculateScore(dice) {
    counts = [0, 0, 0, 0, 0, 0, 0];
    for d in dice {
        counts[d] = counts[d] + 1;
    }
    
    # 1. Check 6-Die Combinations First (Straight & Three Pairs)
    if dice.size() == 6 {
        
        # Check for 1-6 Straight (1000 points)
        is_straight = true;
        for i in range(6) {
            if counts[i] != 1 {
                is_straight = false;
                break;
            }
        }
        if is_straight {
            return [1000, 6];
        }
        
        # Check for Three Pairs (1500 points)
        pair_count = 0;
        for i in range(6) {
            if counts[i] in [1, 3, 5] {
                break;
            }
            if counts[i] == 2 {
                pair_count += 1;
            } elif counts[i] == 4 {
                pair_count += 2; # A 4-of-a-kind technically contains two pairs
            } elif counts[i] == 6 {
                pair_count += 3; # A 6-of-a-kind contains three pairs
            }
        }
        
        if pair_count == 3 {
            return [1500, 6];
        }
    }

    score = 0;
    scoring_count = 0;

    # 2. Check for sets of 3 or more
    for i in range(6) {
        c = counts[i];
        if c >= 3 {
            base_score = 0;
            if i == 1 {
                base_score = 1000;
            } else {
                base_score = i * 100;
            }
            
            # The score doubles for every die beyond 3 
            multiplier = 2 ** (c - 3);
            score += base_score * multiplier;
            
            # Remove the scored dice from counts
            counts[i] -= c;
            scoring_count += c;
        }
    }

    # 3. Check for individual 1s and 5s
    if counts[1] > 0 {
        score += counts[1] * 100;
        scoring_count += counts[1];
    }
    if counts[5] > 0 {
        score += counts[5] * 50;
        scoring_count += counts[5];
    }

    return [score, scoring_count];
}

class FarkleGame {
    # Public instance variables
    &target_score = 5000; 
    &player_score = 0;
    &turn_number = 1;
    &is_on_board = false;

    # Constructor
    func &FarkleGame(target) {
        &target_score = target;
    }

    # Public Main Loop
    func &play() {
        print("=================================");
        print("   Welcome to Funcy Farkle!      ");
        print("=================================");
        print("Target score is " + str(&target_score));
        print("Hurdle: 500 points to open.");
        print();

        while &player_score < &target_score {
            print("--- Turn " + str(&turn_number) + " ---");
            if not &is_on_board {
                print("Status: NOT ON BOARD (Need 500 to open)");
            } else {
                print("Current Total Score: " + str(&player_score));
            }
            
            turn_score = playTurn(); 
            
            &player_score += turn_score;
            &turn_number += 1;
            print();
        }

        print("=================================");
        print("WINNER! You reached " + str(&player_score) + " points!");
        print("It took you " + str(&turn_number - 1) + " turns.");
        print("=================================");
    }

    # Private Turn Logic
    func playTurn() {
        dice_left = 6;
        turn_score = 0;

        while true {
            rolled = [];
            for i in range(dice_left) {
                rolled.append(randInt(1, 6));
            }

            print("You rolled: " + str(rolled));

            # Check if player rolled a Farkle
            [pot_score, pot_count] = calculateScore(rolled);
            if pot_score == 0 {
                print("Oh no, FARKLE! You lose all points for this turn.");
                return 0;
            }

            valid_selection = false;
            kept_score = 0;
            kept_count = 0;

            # Prompt player to keep valid dice
            while not valid_selection {
                keep_input = input("Enter dice to keep (spaces between, e.g. 1 1 5): ");
                kept_strs = keep_input.split(" ");
                kept_dice = [];

                for s in kept_strs {
                    s = s.strip();
                    if s != "" {
                        if s.isDigit() {
                            kept_dice.append(int(s));
                        }
                    }
                }

                # Validate 1: Did the player actually roll these dice?
                is_subset = true;
                temp_rolled = rolled.copy();
                for d in kept_dice {
                    if d in temp_rolled {
                        temp_rolled.remove(d);
                    } else {
                        is_subset = false;
                        break;
                    }
                }

                if not is_subset {
                    print("Invalid! You can only keep dice that you just rolled.");
                    continue;
                }

                if kept_dice.size() == 0 {
                    print("You must keep at least one scoring die to continue.");
                    continue;
                }

                # Validate 2: Are the selected dice actually scoring dice?
                [kept_score, kept_count] = calculateScore(kept_dice);

                if kept_score == 0 or kept_count < kept_dice.size() {
                    print("Invalid! You can only keep valid scoring dice (1s, 5s, triples+, straight, or 3 pairs).");
                    continue;
                }

                valid_selection = true;
            }

            turn_score += kept_score;
            dice_left -= kept_count;

            print("Turn score so far: " + str(turn_score));

            if dice_left == 0 {
                print("HOT DICE! You get to roll 6 new dice.");
                dice_left = 6;
            }

            # The 500-Point Hurdle Check
            if not &is_on_board and turn_score < 500 {
                print("You have not met the 500-point hurdle! You must roll again.");
                print();
            } else {
                choice = input("Roll remaining " + str(dice_left) + " dice? (y/n): ");
                if choice.lower() != "y" {
                    if not &is_on_board {
                        &is_on_board = true;
                        print("Congratulations! You are officially on the board!");
                    }
                    print("You banked " + str(turn_score) + " points.");
                    return turn_score;
                }
                print();
            }
        }
        
        return 0;
    }
}

game = FarkleGame(5000);
game.play();