a = {"one":1, "two":2};
b = a.copy();
a.pop("two");
print(a, b);
print(a.setDefault("three", 3));
print(a.setDefault("three", Null));