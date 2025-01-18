class Test {
    func &Test() {
        &position = 0;
    }

    func &consume() {
        &position += 1;
    }

    func &parse(index) {
        if index == 0 {
            return;
        }
        index -= 1;
        &consume();
        print(&position);
        &parse(index);
    }
}

t = Test();
t.parse(3);