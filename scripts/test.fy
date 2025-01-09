class Example {
    private_var = "only class functions see this";
    func &Example(data) {
        &data = data;
    }
    func &getData() {
        return &data;
    }
    func privateFunc() {
        print("only class functions can call this");
    }
}

class Example2 {
    func &Example2() {}
}

print(Example == 1);
print(Example == Example);
print(Example == Example2);
e = Example("Data");
f = Example("Data");
g = Example2();

print(e == e);
print(e == f);
print(e == g);