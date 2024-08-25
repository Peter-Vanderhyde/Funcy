#Program made in high school senior year for fun
player_kind = []
positions = []
player_kind = [] #Contains which player is x and o

def reset_variables(): #This function resets all variables to start a new game
    global positions, which_turn, won
    
    #Holds what is displayed at the positions on the board
    positions = [[' 1 ',' 2 ',' 3 '],
                 [' 4 ',' 5 ',' 6 '],
                 [' 7 ',' 8 ',' 9 ']]
    
    which_turn = '1' #Shows which player's turn it is
    
    won = False #Becomes True when someone wins
    
def give_kinds(): #Player 1 picks the x or o
    print('\n' * 40)
    kind = ''
    while kind not in ['x','o']: #Loops until x or o is picked
        kind = input("Who is player 1? (x/o)")
    player_kind.append(kind.lower())
    if kind == 'x':
        player_kind.append('o')
    elif kind == 'o':
        player_kind.append('x')
    print("Player 1 is "+player_kind[0])
    print("Player 2 is "+player_kind[1])
    #' print("") ' statements are there to put blank spaces between printed lines
    print("")

def draw_board():
    for y in range(3):
        print(positions[y][0]+"|"+positions[y][1]+"|"+positions[y][2])
        if y != 2:
            print("---+---+---")

def check_all_values(values):
    not_won = False
    #Checks for empty spaces in the column
    for position in values:
        if position not in [' x ',' o ']:
            not_won = True
    if not not_won:
        if' x ' in values and ' o ' in values:
            #Not all values in the column are the same
            not_won = True
        else:
            #All the values are the same
            won = True
            return "Someone has won"

def test_for_win():
    global won
    #Check each column
    for x in range(3):
        position_values = []
        for y in range(3):
            position_values.append(positions[y][x])
        values_are_the_same = check_all_values(position_values)
        if values_are_the_same:
            return True
    #Check each row
    for y in range(3):
        position_values = []
        for x in range(3):
            position_values.append(positions[y][x])
        values_are_the_same = check_all_values(position_values)
        if values_are_the_same:
            return True
    #Check diagonal topleft to bottomright
    position_values = []
    for xy in range(3):
        position_values.append(positions[xy][xy])
    values_are_the_same = check_all_values(position_values)
    if values_are_the_same:
        return True
    #Check diagonal bottomleft to topright
    position_values = []
    for xy in range(3):
        position_values.append(positions[2-xy][xy])
    values_are_the_same = check_all_values(position_values)
    if values_are_the_same:
        return True

def check_if_empty(position):
    #Use to check if x or o can be placed at position
    place_y = (position-1)//3
    place_x = position % 3 - 1
    if positions[place_y][place_x] in [' x ',' o ']:
        return False
    else:
        return True

def loop():
    global which_turn, won
    finished = False
    while not finished:
        draw_board()
        print("")
        print("Player "+which_turn+"'s turn")
        print("")
        where = 0
        while where > 9 or where < 1:
            where = input("Where do you want to place your "+player_kind[int(which_turn)-1]+"?")
            try:
                where = int(where)
                empty = check_if_empty(where)
                if not empty:
                    print("\nThat space is taken\n")
                    where = 0
            except:
                where = 0
        print("")
        #Finds the correct position to put the x or o into
        place_y = (where-1)//3
        place_x = where % 3 - 1
        positions[place_y][place_x] = " "+player_kind[int(which_turn)-1]+" "
        has_won = test_for_win()
        if has_won:
            won = True
        if won == False:
            found_space = False
            for y in range(3):
                for x in range(3):
                    try:
                        test = int(positions[y][x].replace(' ', ''))
                        found_space = True
                    except:
                        pass
            if not found_space:
                #There are no spaces left on the board
                draw_board()
                print("")
                print("It's a tie")
                finished = True
        elif won == True:
            #Someone has won
            print('\n' * 40)
            draw_board()
            print("")
            print("Player "+which_turn+" has won!")
            finished = True
            return
        new_turn = int(which_turn)
        if new_turn == 1:
            which_turn = '2'
        elif new_turn == 2:
            which_turn = '1'

        if not finished:
            print('\n' * 40)

stop = False
give_kinds()
while not stop:
    reset_variables()
    loop()
    continue_game = ''
    while continue_game not in ['y','n']:
        continue_game = input("Do you want to play again? (y/n)")
    if continue_game == 'n':
        stop = True
    print('\n' * 40)
