# Example Funcy Program - Getting Started

# Print a welcome message
print("Welcome to Funcy!");

start = time();

# Declare variables
greeting = "Hello";
name = input("What is your name? ->");
time_elapsed = time() - start;

# Use string concatenation
print(greeting + ", " + name + "! You have been using Funcy for " + str(time_elapsed // 1000) + " seconds.");

# If-Else Conditionals
if time_elapsed < 5000 {
    print("Less than 5 seconds.");
} elif time_elapsed >= 5000 and time_elapsed <= 10000 {
    print("5 to 10 seconds.");
} else {
    print("Over 10 seconds.");
}
print();

# Loops
print("Counting to 5 the C++ way:");
for i = 1, i <= 5, i += 1 {
    print(i);
}
print();

print("Counting to 3 the fancy Pythonic way:");
for i in range(3) {
    print("Fancy " + str(i + 1));
}
print();

# Lists and iteration
colors = ["red", "blue", "green"];
print("Available colors:");
for color in colors {
    print("- " + color);
}
print();

# Using a dictionary
data = {"name": "Alice", "age": 25};
print("User info:");
print("Name: " + data["name"]);
print("Age: " + str(data["age"]));
print();

# Define and call a function
func greetUser(username) {
    print("Greetings, " + username + "!");
}
greetUser(data["name"]);
print();

# Random using randInt(), randChoice(), and randShuffle()
data = {};
for i in range(randInt(3, 8)) {
    data["Key " + str(i + 1)] = "Value " + str(i + 1);
}

print("Randomly choosing key-value pairs.");
listed = range(length(data));
while listed {
    index = randChoice(listed);
    listed.remove(index);
    key = "Key " + str(index + 1);
    print(key, data.get(key, "Default Value"));
}
print();

keys = [];
values = [];
for [key, value] in data {
    keys.append(key);
    values.append(value);
}
keys = randShuffle(keys);
values = randShuffle(values);
print("Randomly shuffled pairs:");
for pair in zip(keys, values) {
    print(pair);
}
print();

# Ending the program
print("Program execution completed.");
