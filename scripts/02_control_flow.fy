# 02_control_flow.fy - Loops, Functions, and Logic

start = time();

# Define a function with default arguments
func greetUser(username, count=1, loud=false) {
    print("Greeting " + username + " " + str(count) + " times:");
    
    for i in range(count) {
        msg = "  " + str(i + 1) + ". Greetings!";
        if loud {
            print(msg.upper());
        } else {
            print(msg);
        }
    }
}

# Funcy allows argument reordering as long as they are named!
greetUser(loud=true, count=3, username="Alice");
print();

print("Counting to 5 the C++ way, but skipping 3:");
# C-style loop with 'continue'
for i = 1, i <= 5, i += 1 {
    if i == 3 {
        continue;
    }
    print(i);
}
print();

# Math, Time, and Error Throwing
time_elapsed = time() - start;
print("Execution took " + str(time_elapsed) + " milliseconds.");

# Throwing an error explicitly based on a condition
func validateTime(ms) {
    if ms < 0 {
        throw "Error: Time cannot be negative!";
    }
    return true;
}

validateTime(time_elapsed);
# validateTime(-10); # Un-commenting this would safely halt the program