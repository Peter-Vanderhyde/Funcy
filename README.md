
# Funcy: A Custom Scripting Language

Funcy is a  flexible scripting language designed for simplicity and versatility. With support for variable assignments, control flow, object-oriented programming, and robust built-in functions, Funcy is ideal for a variety of programming tasks.

---

## Quick Start

### Hello World

```funcy
print("Hello, World!");
```

### Fibonacci Sequence

```funcy
index = 15;
a = 0;
b = 0;

while index > 0 {
    if a == 0 and a == b {
        print(0);
        b = 1;
    } else {
        temp = a + b;
        a = b;
        b = temp;
        print(a);
    }
    index -= 1;
}
```
### Class Example

```funcy
class Person {
    func &Person(name, age) {
        &name = name;
        &age = age;
    }

    func &greet() {
        print("Hello, my name is " + &name);
    }
}

let john = Person("John", 30);
john.greet();
```

---

## Running

To run Funcy, download the interpreter from the repository and run your programs as follows:

```bash
Funcy.exe <path/to/script.fy>
```

---

## Features

### Core Features
- Variable assignments
- Multiple data types
- Control flow with `if`, `elif`, `else`, `for`, and `while`
- Object-oriented programming with classes
- Extensive built-in functions

---

## Syntax Overview

### Variables

Variables are assigned using the `=` operator. Example:

```funcy
x = 10;
y = "hello";
```

### Data Types

Funcy supports the following data types:

- Integer
- Float
- Boolean
- String
- List
- Dictionary
- Function
- Class

### Operators

Funcy supports several operators:

- **Arithmetic**: `+`, `-`, `*`, `/`, `%`, `**`, `^`
- **Comparison**: `==`, `!=`, `<`, `<=`, `>`, `>=`
- **Logical**: `and`, `or`, `not`

---

## Keywords

### Import

The `import` keyword allows you to include external Funcy scripts in your program:

```funcy
import "utilities.fy";
```

### Global

The `global` keyword allows variables in an outer scope to be modified inside a function:

```funcy
x = 10;

func modifyGlobal() {
    global x;
    x = 20;
}

modifyGlobal();
print(x); # Outputs: 20
```

---

## Control Flow

### If-Else Statements

```funcy
if condition {
    # Code block
} elif another_condition {
    # Code block
} else {
    # Code block
}
```

### While Loop

```funcy
while condition {
    # Code block
}
```

### For Loop

**Iterating over a range:**

```funcy
for i = 0, i < 10, i += 1 {
    # Code block
}
```

**Iterating over a list:**

```funcy
for item in list {
    # Code block
}
```

---

## Functions

Functions are defined using the `func` keyword:

```funcy
func myFunction(arg1, arg2) {
    return arg1 + arg2;
}
```

---

## Classes

Funcy supports object-oriented programming. Public attributes are prefixed with `&`, while private attributes do not use `&`.

### Class Syntax

```funcy
class MyClass {
    &public_attribute = "Accessible from outside";
    private_attribute = "Accessible only within class";

    func &MyClass(arg1, arg2) {
        # Constructor
        &public_attribute = arg1;
        private_attribute = arg2;
    }

    func &publicMethod() {
        print("This is a public method.");
    }

    func privateMethod() {
        print("This is a private method.");
    }
}

# Example usage
instance = MyClass("Hello", "World");
instance.publicMethod();
print(instance.public_attribute);
```

### Nested Classes

Nested classes provide logical grouping of related functionality:

```funcy
class OuterClass {
    func &OuterClass() {
        &nested_instance = Null;
    }

    class InnerClass {
        func &InnerClass(data) {
            &data = data;
        }

        func &showData() {
            print("Data: " + &data);
        }
    }

    func &createInner(data) {
        &nested_instance = InnerClass(data);
    }

    func &useInner() {
        if type(&nested_instance) == Instance {
            &nested_instance.showData();
        } else {
            print("No inner instance available.");
        }
    }
}

# Example usage
outer = OuterClass();
outer.useInner();
outer.createInner("Nested Example");
outer.useInner();
```

---

## Built-In Functions

### String Member Functions

- `lower()`: Converts the string to lowercase.
- `upper()`: Converts the string to uppercase.
- `strip(strip_str=whitespace)`: Removes leading and trailing whitespace.
- `split(split_str=" ")`: Splits the string into a list of substrings.
- `replace(old, new)`: Replaces occurrences of `old` with `new`.
- `join(iterable)`: Joins a list of strings.
- `isDigit()`: Checks if the string consists only of digits.
- `length()`: Returns the length of the string.

### List Member Functions

- `append(item)`: Adds an item to the end of the list.
- `pop()`: Removes and returns the last item of the list.
- `size()`: Returns the number of elements in the list.

### Dictionary Member Functions

- `get(key, default=Null)`: Returns the value associated with the key.
- `items()`: Returns a list of key-value pairs.
- `keys()`: Returns a list of keys.
- `values()`: Returns a list of values.
- `pop(key)`: Removes and returns the value associated with the key.
- `update(dict)`: Updates the dictionary with key-value pairs from another dictionary.
- `size()`: Returns the number of key-value pairs in the dictionary.

### General Built-In Functions

- `print()`: Outputs to the console.
- `int()`, `float()`, `bool()`, `str()`: Type conversions.
- `list()`, `dict()`: Create lists and dictionaries.
- `range(start=0, stop, step=1)`: Generate a range of numbers.
- `map(func, iterable)`: Applies a function to all items in an iterable.
- `zip(iterable, iterable, ...)`: Combines multiple iterables into lists of lists.
- `enumerate(iterable)`: Adds an index to each item in an iterable.
- `type(value)`: Returns the type of a value.
- `read(filename)`: Reads the contents of a file as a string.
- `input(prompt="")`: Prompts the user for input and returns the entered value.
- `all(iterable)`: Returns `True` if all elements in the iterable are true.
- `any(iterable)`: Returns `True` if any element in the iterable is true.
- `time()`: Returns the current system time.

---

## Comments

### Single-Line Comments

Single-line comments start with `#`:

```funcy
# This is a comment
```

### Multi-Line Comments

Multi-line comments are enclosed in `/*` and `*/`:

```funcy
/*
This is a 
multi-line comment.
*/
```

---

## License


Funcy is licensed under the MIT License. See the LICENSE file for details.

---

This ReadMe provides an overview of the Funcy language, including its features, syntax, and usage examples.
