guess = "";
while not guess.isDigit() {
    guess = input();
}
print(int(guess));