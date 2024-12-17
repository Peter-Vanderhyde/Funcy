x = 10;
func go() {
    x = 20;
    func stop(x) {
        return x;
    }
    return stop(5);
}

x += go();
x;