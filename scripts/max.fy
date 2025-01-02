# Funcy Program: Generating a Repetitive Pattern of Characters

# Define a function to generate a line of repeated characters with padding
func generate_line(char, count, max_width) {
    func helper(current, remaining) {
        if (remaining <= 0) {
            return current;
        }
        return helper(current + char, remaining - 1);
    }

    padding = " " * ((max_width - count) // 2);
    return padding + helper("", count) + padding;
}

# Define a function to create a pattern of increasing lines only
func generate_pattern(char, max_width) {
    func helper(width) {
        if (width > max_width) {
            return;
        }

        print(generate_line(char, width, max_width));
        helper(width + 2);
    }

    helper(1);
}

# Define a function to generate a pattern for multiple characters
func multi_pattern(chars, max_width) {
    func helper(index) {
        if (index >= chars.size()) {
            return;
        }

        print("Pattern for character: " + str(chars[index]));
        generate_pattern(chars[index], max_width);
        helper(index + 1);
    }

    helper(0);
}

# Call the function with a list of characters and a maximum width
characters = ["*", "#", "$", "%"];
max_width = 9;
multi_pattern(characters, max_width);
