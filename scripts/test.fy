class Test {
    name = "Peter";
    func Test(x, y) {
        x + y;
        name = "Jeff";
        func deeper() {
            name = "George";
            print(name);
        }
        deeper();
    }
    func reset() {
        name = "John";
    }
}

t = Test("World", "Hello");
print(t.name);
t.name = "Joe";
print(t.name);
t.reset();
print(t.name);