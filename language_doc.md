# Funcy Programming Language Documentation

## Introduction

Funcy is a dynamically-typed programming language based off of Python and designed for simplicity and expressiveness. It supports features like object-oriented programming, control flow structures, built-in functions, and rich data types.

---

## Syntax Overview

### General Rules

- Statements end with a semicolon (`;`), except for control flow constructs and function definitions.
- Scopes are enclosed in `{}`.
- Comments:
  - Single-line: `# Comment here`
  - Multi-line: `/* Multi-line comment */`

### Example:

```funcy
x = 10;  # Variable assignment
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
    private_var = "value";

    func &ClassName(arg) {
        &public_var = arg;
    }

    func &getPublicVar() {
        return &public_var;
    }

    func privateMethod() {
        # internal logic
    }
}

obj = ClassName("value");
print(obj.getPublicVar());
```

- Private members and methods are accessible only within the class.
- Public attributes and methods marked with `&` are accessible externally.

---

## Built-in Functions

### General Functions:

- `print(arg1, ...)` - Prints arguments.
- `type(var)` - Returns the type of the variable.
- `range(start=0, end, step=1)` - Generates a range of numbers.
- `input(prompt="")` - Prompts user for input.
- `read(file_path_str)` - Reads from a file.
- `write(file_path_str, contents)` - Writes to a file.
- `length(var)` - Gives the length or size of a string, list, or dictionary.
- `time()` - Returns system time as an integer.

### List Functions:

- `append(value)` - Adds a value to the list.
- `pop(index=-1)` - Removes and returns an item by index.
- `size()` - Returns the number of elements.

### Dictionary Functions:

- `get(key, default_return=Null)` - Retrieves the value for a key.
- `items()` - Returns key-value pairs as a list.
- `keys()` - Returns all keys as a list.
- `values()` - Returns all values as a list.
- `pop(key)` - Removes a key and its value.
- `update(dict)` - Merges another dictionary.
- `size()` - Returns the number of key-value pairs.

### String Functions:

- `lower()` - Converts to lowercase.
- `upper()` - Converts to uppercase.
- `strip(strip_str=whitespace_chars)` - Removes characters from both ends.
- `split(split_str=" ")` - Splits into a list by a separator.
- `isDigit()` - Checks if the string is numeric.
- `replace(old, new)` - Replaces substrings.
- `length()` - Returns string length.
- `join(list)` - Joins a list of strings.

### Instance Functions:

- `set(name_str, value)` - Sets attributes of an instance using a name string.
- `get(name_str)` - Gets attributes of an instance using a name string.

---

## Modules and Imports

### Syntax:

```funcy
import <path/to/module.fy>;
```

- Allows code reuse across files.

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

---

## Additional Features

- **String Multiplication:**

  ```funcy
  print("hello" * 3);  # Output: hellohellohello
  ```

- **Type Conversion:**

  ```funcy
  x = int("123");
  print(type(x) == Integer);  # Output: true
  ```

- **List Slicing:**

  ```funcy
  my_list = [1, 2, 3, 4, 5];
  sublist = my_list[1:4];  # Output: [2, 3, 4]
  ```



## Conclusion

Funcy is a versatile and intuitive programming language that combines ease of use with powerful features, making it suitable for a wide range of programming tasks.

