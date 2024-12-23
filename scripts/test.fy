x = 20;


func try() {
    global x;
    global y;
    print(x);
    x = 10;
    y = 30;
}

try();
print(x);
print(y);