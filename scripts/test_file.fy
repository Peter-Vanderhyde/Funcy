# Funcy Language Recursion and Scope Test File

# Test 1: Basic Recursion
func countdown(n) {
    if (n <= 0) {
        print("Done");
        return;
    }
    print(n);
    countdown(n - 1);
}

countdown(5);  # Expected: 5, 4, 3, 2, 1, Done

# Test 2: Nested Function with Recursion
func outer(z) {
    func inner(y) {
        func recurse(x) {
            if (x <= 0) {
                return;
            }
            print(x);
            recurse(x - 1);
        }

        recurse(y);
    }

    inner(z);
}

outer(3);  # Expected: 3, 2, 1

/*# Test 3: Indirect Recursion
func define_even_odd() {
    func is_even(n) {
        if (n == 0) {
            return true;
        }
        return is_odd(n - 1);
    }

    func is_odd(n) {
        if (n == 0) {
            return false;
        }
        return is_even(n - 1);
    }

    return [is_even, is_odd];
}

funcs = define_even_odd();
is_even = funcs[0];
is_odd = funcs[1];

print(is_even(4));  # Expected: true
print(is_odd(4));   # Expected: false*/

# Test 4: Recursive Factorial
func factorial(n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

print(factorial(5));  # Expected: 120

# Test 5: Scope Integrity with Multiple Recursions
func wrapper(a, b) {
    func recurseA(x) {
        if (x <= 0) {
            return;
        }
        print("A:" + str(x));
        recurseA(x - 1);
    }

    func recurseB(y) {
        if (y <= 0) {
            return;
        }
        print("B:" + str(y));
        recurseB(y - 1);
    }

    recurseA(a);
    recurseB(b);
}

wrapper(2, 3);  # Expected: A:2, A:1, B:3, B:2, B:1

# Test 6: Deep Recursion Depth
func deep_recursion(n) {
    if (n == 0) {
        return "Depth Reached";
    }
    return deep_recursion(n - 1);
}

print(deep_recursion(500));  # Expected: Depth Reached (ensure no stack overflow)