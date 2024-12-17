x = 20;

func first() {
    print(x);  # Works: x is global
    x = 15;
    print(y);
}

func second() {
    y = 10;  # Creates a local variable y
    first();
}

second();
print(x);