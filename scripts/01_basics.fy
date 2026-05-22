/* 01_basics.fy - Getting Started with Funcy
   This script covers variables, basic data types, arithmetic,
   type conversion, logical operators, and string manipulation!
*/

# Print a welcome message
print("Welcome to Funcy!");
print("-----------------");

# 1. Variables and Data Types
greeting = "Hello";
version = 1.0;
is_fun = true;

print("Language Version: " + str(version));
print("Is Funcy fun? " + str(is_fun));
print();

# 2. User Input and Type Conversion
name = input("What is your name? -> ");
age_str = input("How old are you? -> ");

# input() returns a string, so we convert it to an integer using int()
age = int(age_str);

# 3. Arithmetic Operations
next_year_age = age + 1;
half_age = age / 2;
dog_years = age * 7;

print();
print("--- Math Results ---");
print("Next year, you will be: " + str(next_year_age));
print("Half your age is: " + str(half_age));
print("In dog years, you are roughly: " + str(dog_years));

# Using modulo (%) to check if age is even or odd
if age % 2 == 0 {
    print("Fun fact: Your age is an even number.");
} else {
    print("Fun fact: Your age is an odd number.");
}
print();

# 4. Logical Operators (and, or, not)
print("--- Logic Checks ---");
if age >= 18 and name != "" {
    print("You are considered an adult, " + name + ".");
} elif age < 18 or not is_fun {
    print("You are quite young, or you don't like fun!");
}
print();

# 5. String Manipulation
print("--- String Methods ---");

# Manipulating the 'greeting' and 'name' strings
full_message = greeting + ", " + name + "!";

print("Original: " + full_message);
print("Uppercase: " + full_message.upper());
print("Lowercase: " + full_message.lower());

# Using the length() built-in function
print("Your name has " + str(length(name)) + " characters.");

# Using the replace() built-in string method
silly_message = full_message.replace(name, "Captain " + name);
print("Replaced: " + silly_message);