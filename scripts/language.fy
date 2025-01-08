x = 10;
class Test {
    func &Test() {
        global x;
        x = 15;
    }
    func &setX(value) {
        global x;
        x = value;
    }
}

print(x);
t = Test();
print(x);
t.setX(124);
print(x);