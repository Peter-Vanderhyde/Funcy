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
if time_elapsed > 10000 {
    print("You're an experienced Funcy user!");
} else {
    print("Welcome, new user!");
}

# Loops
print("Counting to 5:");
for i = 1, i <= 5, i += 1 {
    print(i);
}

print("Counting to 3 the fancy way:");
for i in range(3) {
    print("Fancy " + str(i));
}

# Lists and iteration
colors = ["red", "blue", "green"];
print("Available colors:");
for color in colors {
    print("- " + color);
}

# Define and call a function
func greetUser(username) {
    print("Greetings, " + username + "!");
}
greetUser("Alice");

# Using a dictionary
data = {"name": "Alice", "age": 25};
print("User info:");
print("Name: " + data["name"]);
print("Age: " + str(data["age"]));

# Ending the program
print("Program execution completed.");
