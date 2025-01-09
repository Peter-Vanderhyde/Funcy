class Example {
    func &Example() {}
}

e = Example();
d = {
    "name":"Peter",
    "age":23
};

for [name, value] in d {
    e.set(name, value);
}
func printVal(val) {
    print(val);
}

e.set("printVal", printVal);
e.printVal("That thing");