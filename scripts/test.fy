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

for attr in d.keys() {
    print(e.get(attr));
}