# Funcy Program: Simulating a Simple Number Guessing Game

# Define a function to generate a list of numbers within a range
func generate_numbers(start, end) {
    return range(start, end + 1);
}

# Define a function to get user input for a guess
func get_user_guess() {
    guess = "";
    while not guess.isDigit() {
        guess = input("Enter your guess: ");
    }
    return int(guess);
}

# Define a function to check if the guess is correct
func check_guess(number, target) {
    if (number == target) {
        return true;
    }
    if (number < target) {
        print("Too low!");
    } else {
        print("Too high!");
    }
    return false;
}

# Define the main game function
func play_game() {
    print("Welcome to the Number Guessing Game!");

    # Generate a range of numbers
    numbers = generate_numbers(1, 100);

    # Randomly select a target number
    func pick_random(numbers) {
        return numbers[int(str(float(time()))[-8:]) % numbers.size()];
    }

    target = pick_random(numbers);

    # Allow the user a limited number of attempts
    attempts = 0;
    max_attempts = 10;

    while (attempts < max_attempts) {
        attempts = attempts + 1;
        print("Attempt " + str(attempts) + " of " + str(max_attempts));

        guess = get_user_guess();

        if (check_guess(guess, target)) {
            print("Congratulations! You guessed the number in " + str(attempts) + " attempts.");
            return;
        }
    }

    print("Sorry, you've used all your attempts. The number was: " + str(target));
}

# Start the game
play_game();