func add(a, b) {
    return a + b;
}

func enclose(a, b) {
    return add(a, b);
}

func test(a, b) {
    return enclose(a, b);
}

test(1, "this");