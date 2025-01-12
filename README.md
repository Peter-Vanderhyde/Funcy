# Funcy Documentation  
***Author: Peter Vanderhyde***  
***Project Start: November 2024***  

---

### VSCode Extension
Funcy features a custom VSCode Extension for syntax highlighting. The `.vsix` file can be downloaded from the [funcy-lang Github Repository](https://github.com/Peter-Vanderhyde/funcy-lang).

## Quick Links

- [Introduction](#introduction)
- [Syntax Overview](#syntax-overview)
- [Data Types](#data-types)
- [Operators](#operators)
- [Control Flow](#control-flow)
- [Functions](#functions)
- [Classes](#classes)
- [Keywords](#keywords)
- [Built-in Functions](#built-in-functions)
- [Examples](#examples)
- [Additional Features](#additional-features)
- [Conclusion](#conclusion)

---

## Introduction

Funcy is a dynamically-typed programming language based off of Python and designed for simplicity, flexibility and expressiveness. It supports features like object-oriented programming, control flow structures, built-in functions, and rich data types.

---

## Syntax Overview

### General Rules

- Statements end with a semicolon `;`, unless a new scope is being created. (if statements, function definitions, etc.)
- Scopes are enclosed in `{}`.
- Comments:
  - Single-line: `# Comment here`
  - Multi-line: `/* Multi-line comment */`

### Example:

```funcy
x = 10;
if x > 5 {
    print("x is greater than 5");
}
```

---

## Data Types

1. **Integer**
2. **Float**
3. **Boolean** (`true` and `false` in lowercase)
4. **String**
5. **List**
6. **Dictionary**
7. **Function**
8. **Class**
9. **Instance**

---

## Operators

### Arithmetic:

| Operator | Description      |
| -------- | ---------------- |
| `+`      | Addition         |
| `-`      | Subtraction      |
| `*`      | Multiplication   |
| `/`      | Division         |
| `//`     | Integer Division |
| `%`      | Modulo           |
| `**`     | Exponentiation   |
| `^`      | Exponentiation   |

### Comparison:

| Operator | Description           |
| -------- | --------------------- |
| `==`     | Equal to              |
| `!=`     | Not equal to          |
| `<`      | Less than             |
| `>`      | Greater than          |
| `<=`     | Less than or equal to |
| `>=`     | Greater than or equal |

### Logical:

| Operator | Description |
| -------- | ----------- |
| `and`    | Logical AND |
| `or`     | Logical OR  |
| `not`    | Logical NOT |

---

## Control Flow

### Conditional Statements:

```funcy
if condition {
    # code block
} elif another_condition {
    # code block
} else {
    # code block
}
```

### Loops:

#### While Loop:

```funcy
while condition {
    # code block
}
```

#### For Loop:

```funcy
for var = start, var < end, var += step {
    # code block
}

for item in list {     # for i in range(5)
    # code block
}

# Multi-Variable assignment in for loops:
for [key, value] in dictionary.items() {
    # code block
}
```

---

## Functions

### Defining Functions:

```funcy
func functionName(arg1, arg2) {
    # code block
    return result;
}
```

### Returning Multiple Values:

```funcy
func multiReturn() {
    return [val1, val2];
}
[x, y] = multiReturn();
```

---

## Classes

### Syntax:

```funcy
class ClassName {
    private_var = "visible inside class only";
    &public_var = "visible outside class";

    # Constructor
    func &ClassName(arg, data) {
        &public_var = arg;
        &data = data;
    }

    func &getPrivateVar() {
        return private_var;
    }

    func privateMethod() {
        # internal logic
    }
}

obj = ClassName("value");
print(obj.getPrivateVar());
print(obj.public_var);
```

- Private members and methods are accessible only within the class.
- Public attributes and methods marked with `&` are accessible externally.
- The constructor must be a public method with the same name as the class.

---

## Keywords

Keywords in Funcy are reserved words with predefined meanings and specific purposes in the language. They cannot be used as variable names or identifiers.

---

### Control Flow Keywords:

- `in`: Used for iterating over ranges, collections, checking for values in lists, keys in dictionaries, and substrings in strings.
  ```funcy
  for item in list {
      print(item);
  }

  if "apple" in fruits {
      print("Found apple!");
  }

  if key in dictionary {
      print("Key exists in dictionary");
  }

  if "sub" in "substring" {
      print("Substring found!");
  }
  ```

  It can also be combined with the `not` keyword to check for the absence of items:
  ```funcy
  if "orange" not in fruits {
      print("Orange is not in the list");
  }
  ```
- `break`: Exits the current loop prematurely.
  ```funcy
  for x in range(10) {
      if x == 5 {
          break;  # Exit loop
      }
      print(x);
  }
  ```
- `continue`: Skips the current iteration and moves to the next.
  ```funcy
  for x in range(10) {
      if x % 2 == 0 {
          continue;  # Skip even numbers
      }
      print(x);
  }
  ```

### Type Keywords:

- `Integer`, `Float`, `Boolean`, `String`, `List`, `Dictionary`, `Function`, `Class`, `Instance`, `Null`: Used to define and compare types.
  ```funcy
  x = 10;
  print(type(x) == Integer);  # true
  ```

### Declaration Keywords:

- `global`: Declares a variable as global, making it accessible outside its local scope and allows modifying its value within a function.
  ```funcy
  x = 10;
  func modifyGlobal() {
      global x;
      x = 20;
  }

  class Example {
      func &Example() {}
      func &modifyGlobal() {
          global x;
          x *= 2;
      }
  }

  modifyGlobal();
  print(x);  # 20
  e = Example();
  e.modifyGlobal();
  print(x);  # 40
  ```
- `import`: Imports modules or libraries.
  ```funcy
  import "module.fy";
  ```

---

## Built-in Functions

### General Functions:

- `abs(value) -> int|float` - Returns the absolute value of a number.
- `all(list) -> bool` - Returns `true` if all elements of the list are true or the list is empty.
- `any(list) -> bool` - Returns `true` if any element of the list is true.
- `bool(value) -> bool` - Converts a value to its boolean equivalent.
- `callable(var) -> bool` - Checks if the variable is callable.
- `dict(iterable={}) -> dict` - Creates a dictionary from another dictionary, or a list of key-value pairs.
- `divMod(a, b) -> list` - Returns a list with the quotient and remainder of `a` divided by `b`.
- `enumerate(list) -> list` - Returns index-value pairs for a list.
- `float(value) -> float` - Converts a value to a floating-point number.
- `globals() -> dict` - Returns a dictionary of global variables.
- `input(prompt="") -> string` - Prompts user for input.
- `int(value) -> int` - Converts a value to an integer.
- `length(var) -> int` - Gives the length or size of a string, list, or dictionary.
- `list(iterable=[]) -> list` - Converts an iterable to a list.
- `locals() -> dict` - Returns a dictionary of local variables in the current scope.
- `map(func, list) -> list` - Applies a function to each item in the list and returns a list of results.
- `max(arg1, ...) -> int|float|string|obj` - Returns the maximum value of several arguments, or a list of values.
- `min(arg1, ...) -> int|float|string|obj` - Returns the minimum value of several arguments, or a list of values.
- `print(arg1, ...) -> Null` - Prints arguments.
- `range(start=0, end, step=1) -> list` - Generates a range of numbers.
- `read(file_path_str) -> string` - Reads from a file.
- `reversed(list) -> list` - Returns a reversed version of the sequence.
- `round(value, precision=0) -> float` - Rounds a number to the given precision.
- `str(value) -> string` - Converts a value to a string. Dictionaries converted into a string will maintain json compatible formatting so they can be saved in json files.
- `sum(list) -> int|float` - Returns the sum of all elements in a list.
- `time() -> int` - Returns system time as an integer.
- `type(var) -> Type` - Returns the type of the variable.
- `write(file_path_str, contents) -> Null` - Writes a string to a file.
- `zip(list1, list1, ...) -> list` - Combines lists into a list of value pair lists.

### List Functions:

- `append(value) -> Null` - Adds a value to the list.
- `clear() -> Null` - Removes all elements from the list.
- `copy() -> list` - Returns a deep copy of the list.
- `index(value) -> int` - Returns the index of the first occurrence of a value. Errors if no match is found.
- `insert(index, value) -> Null` - Inserts a value at the specified index.
- `pop(index=-1) -> int|float|string|bool|obj|Null` - Removes and returns an item by index.
- `remove(value) -> Null` - Removes the first occurrence of a value. Errors if no match is found.
- `size() -> int` - Returns the number of elements.

### Dictionary Functions:

- `clear() -> Null` - Removes all key-value pairs.
- `copy() -> dict` - Returns a deep copy of the dictionary.
- `get(key, default_return=Null) -> int|float|string|bool|obj|Null` - Retrieves the value for a key.
- `items() -> list` - Returns key-value pairs as a list.
- `keys() -> list` - Returns all keys as a list.
- `pop(key) -> int|float|string|bool|obj|Null` - Removes a key and its value.
- `setDefault(key, default_value) -> int|float|string|bool|obj|Null` - Returns the value of a key or sets it to a default value.
- `size() -> int` - Returns the number of key-value pairs.
- `update(dict) -> Null` - Merges another dictionary.
- `values() -> list` - Returns all values as a list.

### String Functions:

- `capitalize() -> string` - Capitalizes the first letter of the string.
- `endsWith(suffix) -> bool` - Checks if the string ends with the specified suffix.
- `find(sub) -> int` - Finds the first occurrence of a substring and returns the index. Returns -1 if no match is found.
- `isAlpha() -> bool` - Checks if the string contains only alphabetic characters.
- `isAlphaNum() -> bool` - Checks if the string is alphanumeric.
- `isDigit() -> bool` - Checks if the string is numeric.
- `isSpace() -> bool` - Checks if the string contains only spaces.
- `isWhitespace() -> bool` - Checks if the string contains only whitespace.
- `join(list) -> string` - Joins a list of strings.
- `length() -> int` - Returns string length.
- `lower() -> string` - Converts to lowercase.
- `replace(old, new) -> string` - Replaces substrings.
- `split(split_str=" ") -> list` - Splits into a list by a separator.
- `strip(strip_str=whitespace_chars) -> string` - Removes characters from both ends.
- `toJson() -> dictionary` - Converts a string that is in json format into a dictionary object. Pairs well with reading json files.
- `upper() -> string` - Converts to uppercase.

### Instance Functions:

- `delAttr(name_str) -> Null` - Deletes an attribute by name.
- `getAttr(name_str) -> int|float|string|bool|obj|Null` - Gets attributes of an instance using a name string.
- `hasAttr(name_str) -> bool` - Checks if an attribute exists.
- `setAttr(name_str, value) -> Null` - Sets attributes of an instance using a name string.

### Float Functions:

- `isInt() -> bool` - Checks if the float is equivalent to an integer.

---

## Examples

### Example 1: Factorial Function

```funcy
func factorial(n) {
    if n == 0 {
        return 1;
    } else {
        return n * factorial(n - 1);
    }
}

print(factorial(5));  # Output: 120
```

### Example 2: Class with Public and Private Attributes

```funcy
class Example {
    func &Example(value) {
        &public_attr = value;
        private_attr = value * 2;
    }

    func &getPrivateAttr() {
        return private_attr;
    }
}

obj = Example(10);
print(obj.public_attr);  # Output: 10
print(obj.getPrivateAttr());  # Output: 20
```

### Example 3: 

---

## Additional Features

- **String Multiplication:**

  ```funcy
  print("hello" * 3);  # Output: hellohellohello
  ```

- **List Slicing:**

  ```funcy
  my_list = [1, 2, 3, 4, 5];
  sublist = my_list[1:4];  # Output: [2, 3, 4]
  ```