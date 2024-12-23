original = "one two three four";

# first method
print(original.replace(" ", ", "));

# second join method
print(', '.join(original.split(' ')));