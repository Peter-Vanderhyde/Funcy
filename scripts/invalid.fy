func add(a, b) {
    if type(a) == String {
        throw "Invalid argument!";
    }
    return a + b;
}


colors = [];
color = Null;
func setupColors() {
    colors = [
        "red",
        "green",
        "blue"
    ];
    color = colors[0];
    print(colors[-2:]);
}

func checkColor() {
    return color != colors[-1];
}

func changeColor() {
    current = colors.index(color);
    color = colors[current + 1];
}

for setupColors(), checkColor(), changeColor() {
    print(color);
}

#add("this", 7); # Prints "std::exception" instead of message