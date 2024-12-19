func give(a, b) {
    return [b, a];
}

[x, y] = [2, 5];
[x, y] = give(x, y);
print(x, y);