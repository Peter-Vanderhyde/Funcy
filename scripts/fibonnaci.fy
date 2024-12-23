a = 0;
b = 0;

while a < 10000 {
    if a == 0 and a == b {
        print(0);
        b = 1;
    } else {
        temp = a + b;
        a = b;
        b = temp;
        print(a);
    }
}