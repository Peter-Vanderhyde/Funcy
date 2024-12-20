# My first full program in my new language, Funcy
func getInput(prompt, possible_responses) {
    response = input(prompt);
    while response.lower() not in possible_responses {
        print("Try Again.");
        response = input(prompt);
    }
    return response.lower();
}

func getBoardXY(index) {
    return [index % 3, index // 3];
}

func printBoard(board) {
    for y = 2, y >= 0, y -= 1 {
        print(board[y][0] + '|' + board[y][1] + '|' + board[y][2]);
        if y != 0 {
            print("---+---+---");
        }
    }
    print();
}

func checkStraights(board, player) {
    cols = [];
    rows = [];
    func isPlayer(str) {
        return str.strip() == player;
    }
    for first in range(3) {
        for second in range(3) {
            cols.append(board[second][first]);
            rows.append(board[first][second]);
        }
        if (all(map(isPlayer, rows)) or all(map(isPlayer, cols))) {
            return true;
        }
        cols = [];
        rows = [];
    }
    return false;
}

func checkDiagonals(board, player) {
    player = " " + player + " ";
    if board[0][0] == player and board[1][1] == player and board[2][2] == player {
        return true;
    } elif board[2][0] == player and board[1][1] == player and board[0][2] == player {
        return true;
    }
    return false;
}

func checkForWin(board, player) {
    digits = 0;
    for y in range(3) {
        for x in range(3) {
            if board[y][x].strip().isDigit() {
                digits += 1;
            }
        }
    }
    if digits == 0 {
        return "xy";
    } else {
        if checkStraights(board, player) or checkDiagonals(board, player) {
            return player;
        } else {
            return Null;
        }
    }

}

func doTurn(player_turn, board) {
    players = ['x', 'o'];
    message = "";
    position = Null;

    while true {
        printBoard(board);
        if message {
            print(message);
            message = "";
        }
        position = getInput("Where would " + players[player_turn] + " like to place? ", list("123456789"));
        position = int(position) - 1;
        board_xy = getBoardXY(position);
        if board[board_xy[1]][board_xy[0]].strip().isDigit() {
            break;
        } else {
            message = "Spot already taken.";
        }
    }

    xy = getBoardXY(position);
    board[xy[1]][xy[0]] = " " + players[player_turn] + " ";

    return checkForWin(board, players[player_turn]);
}


func main() {
    board = [
        [' 1 ', ' 2 ', ' 3 '],
        [' 4 ', ' 5 ', ' 6 '],
        [' 7 ', ' 8 ', ' 9 ']
    ];
    player_turn = 0;

    winner = Null;
    while not winner {
        winner = doTurn(player_turn, board);
        player_turn = int(!bool(player_turn)); # Switch whose turn
    }
    printBoard(board);
    
    if winner != 'x' and winner != 'o' {
        print("It's a tie!");
    } else {
        print("Player " + winner + " is the winner!");
    }

    answer = getInput("Would you like to play again? (y/n) -> ", ['y', 'n', 'yes', 'no']);
    return answer in ['yes', 'y'];
}

print("Welcome to Tic-Tac-Toe!");
replay = true;
while replay {
    replay = main();
}

print("Thanks for playing!");