guess = "";
while not guess.isDigit() {
    guess = input("Help");
}
print(int(guess));