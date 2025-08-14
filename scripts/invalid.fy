func add(a, b) {
    if type(a) == String {
        throw "Invalid argument!";
    }
    return a + b;
}

func enclose(a, b) {
    return add(a, b);
}

enclose("this"); # Prints "std::exception" instead of message