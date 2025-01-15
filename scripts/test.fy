func testFunc() {
    return "Default string";
}

func otherFunc(x=1, y=testFunc()) {
    print(x, y);
}

otherFunc(y=3, x=6);
otherFunc(y=5);
otherFunc(8);