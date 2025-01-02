# Funcy Test File: Testing All Builtin and Type Builtin Functions

# Test Builtin Functions
print("Expected: 10");
print(int("10"));
print("Expected: 10.5");
print(float("10.5"));
print("Expected: true");
print(bool(1));
print("Expected: false");
print(bool(0));
print("Expected: '123'");
print(str(123));
print("Expected: ['a', 'b', 'c']");
print(list("abc"));
print("Expected: {'key': 'value'}");
print(dict([['key', 'value']]));
print("Expected: 'Type:Integer'");
print(type(123));
print("Expected: range(0, 5)");
print(range(0, 5));

# Test Builtin List Functions
list_test = [1, 2, 3, 4, 5];
print("Expected: 5");
print(list_test.size());
list_test.append(6);
print("Expected: [1, 2, 3, 4, 5, 6]");
print(list_test);
print("Expected: 6");
print(list_test.pop());
print("Expected: [1, 2, 3, 4, 5]");
print(list_test);

# Test Builtin Dictionary Functions
dict_test = {"a": 1, "b": 2};
print("Expected: 2");
print(dict_test.get("b"));
dict_test.update({"c": 3});
print("Expected: {'a': 1, 'b': 2, 'c': 3}");
print(dict_test);
print("Expected: ['a', 'b', 'c']");
print(dict_test.keys());
print("Expected: [1, 2, 3]");
print(dict_test.values());
print("Expected: 3");
print(dict_test.pop("c"));
print("Expected: {'a': 1, 'b': 2}");
print(dict_test);

# Test Builtin String Functions
string_test = "  Funcy Language  ";
print("Expected: '  funcy language  '");
print(string_test.lower());
print("Expected: '  FUNCY LANGUAGE  '");
print(string_test.upper());
print("Expected: 'Funcy Language'");
print(string_test.strip());
print("Expected: ['Funcy', 'Language']");
print(string_test.split());
print("Expected: true");
print("12345".isDigit());
print("Expected: 18");
print(string_test.length());
print("Expected: 'Fancy Langaage'");
print(string_test.replace("u", "a"));

# Test Enumerate, Zip, and Map
list1 = [1, 2, 3];
list2 = ["a", "b", "c"];
print("Expected: [(0, 1), (1, 2), (2, 3)]");
print(enumerate(list1));
print("Expected: [(1, 'a'), (2, 'b'), (3, 'c')]");
print(zip(list1, list2));
func double(x) {
    return x * 2;
}
print("Expected: [2, 4, 6]");
print(map(double, list1));

# Test Time
print("Expected: Current timestamp (variable)");
print(time());
