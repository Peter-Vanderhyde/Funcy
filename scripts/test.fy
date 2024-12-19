func isEven(x) {
    return x % 2 == 0;
}

list = range(10);
results = map(isEven, list);
print(results);

for i = 0, i < results.size(), i += 1 {
    if results[i] {
        print(list[i]);
    }
}