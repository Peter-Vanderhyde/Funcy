# Funcy Documentation  
***Author: Peter Vanderhyde***  
***Project Start: November 2024***  

---

### VSCode Extension
Funcy features a custom VSCode Extension for syntax highlighting. The `.vsix` file can be downloaded from the [funcy-lang Github Repository](https://github.com/Peter-Vanderhyde/funcy-lang).  
The Funcy code snippets found in this README are utilizing Github's Python syntax highlighting (because it is most similar), and is not a representation of the extension's visuals.

### Running a Funcy File

To execute a Funcy program, in the command-line, run the `Funcy.exe` executable with the following syntax:

```bash
Funcy.exe <file_path> [-IgnoreOverflow]
```

#### Arguments:
- `<file_path>`: The path to the `.fy` file you want to execute.
- `-IgnoreOverflow` (optional): A flag that allows the program to continue running even when excessive recursion is detected. When disabled, your program may experience sudden, random termination due to stack overflow.

#### Example:
Run a Funcy file with the `-IgnoreOverflow` flag:
```bash
Funcy.exe example.fy -IgnoreOverflow
```

## Quick Links

- [Introduction](#introduction)
- [Syntax Overview](#syntax-overview)
- [Data Types](#data-types)
- [Operators](#operators)
- [Control Flow](#control-flow)
- [Functions](#functions)
- [Classes](#classes)
- [Keywords](#keywords)
- [Errors](#errors)
- [Built-in Functions](#built-in-functions)
- [Examples](#program-examples)
- [Additional Features](#additional-features)

---

## Introduction

Funcy is a dynamically-typed, object-oriented programming language based off of Python and designed for simplicity and flexibility. It supports features like control flow structures, built-in functions, and several data types.

---

## Syntax Overview

### General Rules

- Statements end with a semicolon `;`, unless a new scope is being created. (if statements, function definitions, etc.)
- Scopes are enclosed in `{}`.
- Comments:
  - Single-line: `# Comment here`
  - Multi-line: `/* Multi-line comment */`

### Example:

```python
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

```python
if condition {
    ...
} elif another_condition {
    ...
} else {
    ...
}
```

### Loops:

#### While Loop:

```python
while condition {
    ...
}
```

#### For Loop:

```python
for var = start, var < end, var += step {
    ...
}

for item in my_list {
    ...
}

for i in range(5) {
    ...
}

# Multi-Variable assignment in for loops:
for [key, value] in dictionary.items() {
    ...
}

for [r, g, b] in pixels {
    ...
}
```
>Syntax Note  
>-
>The for loop syntax does not require the format of  
>```
>for var = 0, var < max_val, var += 1 {...}
>```
>Just like the syntax of C/C++, the for loop allows any expression to be used for the initialization, conditional, and increment. For example, three functions could be used instead.
>```
>for initialize(), checkConditional(a, b), callEachIteration() {...}


---

## Functions

### Defining Functions:

```python
func functionName(arg1, arg2) {
    ...
    return result;
}
```

### Returning Multiple Values:

```python
func multiReturn() {
    return [val1, val2];
}

[x, y] = multiReturn();
```

### Using Default Arguments:

```python
func showString(arg1, arg2="default string", arg3=Null) {
    ...
}

# All valid calls
showString(true, "second arg");
showString(true, "second arg", value);
showString(arg2="second arg", arg3=value, arg1=true);
```

---

## Classes

### Syntax:

```python
class ClassName {
    private_var = "not visible outside class";  # Behaves like a normal variable
    &public_var = "visible outside class";

    # Constructor
    func &ClassName(arg, data) {

        # Set class member variables that can be accessed within the class
        &public_var = arg;
        &data = data;

        temp_var = "not visible in other functions";
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
print(obj.public_var);  # Directly access the public member variable
```

- Private members and methods are accessible only within their scope.
- Public attributes and methods marked with `&` are accessible externally.
- The constructor must be a public method with the same name as the class.

---

## Keywords

Keywords in Funcy are reserved words with predefined meanings and specific purposes in the language. They cannot be used as variable names/identifiers.

---

### Control Flow Keywords:

- `in`: Used for iterating over ranges, collections, checking for values in lists, keys in dictionaries, and substrings in strings.
  ```python
  for item in my_list {
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
  ```python
  if "orange" not in fruits {
      print("Orange is not in the list");
  }
  ```
- `break`: Exits the current loop prematurely.
  ```python
  for x in range(10) {
      if x == 5 {
          break;  # Exit loop
      }
      print(x);
  }
  ```
- `continue`: Skips the current iteration and moves to the next.
  ```python
  for x in range(10) {
      if x % 2 == 0 {
          continue;  # Skip even numbers
      }
      print(x);
  }
  ```

### Type Keywords:

- `Integer`, `Float`, `Boolean`, `String`, `List`, `Dictionary`, `Function`, `Class`, `Instance`, `Null`: Used to define and compare types.
  ```python
  x = 10;
  print(type(x) == Integer);  # true
  ```
  > `type(var)` is a built-in function that returns the appropriate type keyword for a variable.

### Declaration Keywords:

- `global`: Declares a variable as global, making it accessible outside its local scope and allows modifying its value within a function.
  ```python
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

  func innerScope() {
    new_var = "Newly created inside this scope";
    global new_var;
  }

  func separateScope() {
    print(new_var);
  }

  innerScope();
  separateScope(); # It knows the new_var global
  ```
- `import`: Imports separate funcy files. It will run the entire file upon importing it. There is not currently a way to only import specific functions or classes from a file.
  ```python
  import "module.fy";
  ```

## Errors

Funcy allows users to explicitly throw errors during execution using the `throw` keyword. The `throw` keyword can be followed by any value, including strings, numbers, objects, or any expression.

### Syntax:
```python
throw <expression>;
```

### Example Usage:

1. Throwing an error message:
   ```python
   throw "Error: Invalid operation";
   ```

2. Throwing a computed value:
   ```python
   throw 2 + 5;  # Throws the value 7
   ```

3. Using `throw` in a function:
   ```python
   func validateInput(input) {
       if not input {
           throw "Error: Input cannot be null";
       }
   }

   validateInput(Null);  # Throws "Error: Input cannot be null"
   ```

Thrown errors halt the execution of the program. There is not currently a way to catch thrown errors.

---

## Built-in Functions

### General Functions:

- `abs(value) -> int|float` - Returns the absolute value of a number.
- `all(list) -> bool` - Returns `true` if all elements of the list are true or the list is empty.
- `any(list) -> bool` - Returns `true` if any element of the list is true.
- `appendFile(file, content) -> Null` - Will add the content onto the end of the existing file content, or create a new file with that content.
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
- `randChoice(list) -> int|float|string|bool|obj` - Picks a random element from a list and returns it.
- `randInt(min, max) -> int` - Chooses a random integer between and including the minimum and maximum given values.
- `range(start=0, end, step=1) -> list` - Generates a range of numbers.
- `readFile(file_path_str) -> string|Null` - Reads from a file. Returns Null if file does not exist.
- `reversed(list) -> list` - Returns a reversed version of the sequence.
- `round(value, precision=0) -> float` - Rounds a number to the given precision.
- `str(value) -> string` - Converts a value to a string. Dictionaries converted into a string will maintain json compatible formatting so they can be saved in json files.
- `sum(list) -> int|float` - Returns the sum of all elements in a list.
- `time() -> int` - Returns milliseconds since the start of the application as an integer.
- `type(var) -> Type` - Returns the type of the variable.
- `writeFile(file_path_str, contents) -> Null` - Writes a string to a file. Creates a new file if it does not already exist.
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

## Additional Features

- **String Multiplication:**

  ```python
  print("hello" * 3);  # Output: hellohellohello
  ```

- **List Slicing:**

  ```python
  my_list = [1, 2, 3, 4, 5];
  sublist = my_list[1:4];  # Output: [2, 3, 4]
  sublist = my_list[-3:-1]; # Output: [3, 4]
  ```

- **Argument Ordering:**  
    Arguments can be set in any order, as long as they are labeled.
    ```python
    func Foo(x, y, z) {
        # Code
    }

    Foo(x=5, y=2, z="string");
    Foo(z="string", y=8, x=3);
    Foo(1, z="string", y=7);
    ```

---

## Program Examples

### Example 1: Factorial Function

```python
func factorial(n) {
    if n == 0 {
        return 1;
    } else {
        return n * factorial(n - 1);
    }
}

print(factorial(5));  # Output: 120
```

### Example 2: Fibonacci

```python
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
```

### Example 3: Inventory

```python
class Item {
    &name = "";
    &quantity = 0;
    &price = 0.0;

    # Constructor
    func &Item(name, quantity, price) {
        &name = name;
        &quantity = quantity;
        &price = price;
    }

    func &display() {
        print("Item: " + &name + ", Quantity: " + str(&quantity) + ", Price: $" + str(&price));
    }
}

class Inventory {
    func &Inventory() {
        &items = list();
    }

    func &addItem(item) {
        &items.append(item);
        print("Added item: " + item.name);
    }

    func &removeItem(name) {
        for i = &items.size() - 1, i >= 0, i -= 1 {
            if &items[i].name == name {
                &items.pop(i);
                print("Removed item: " + name);
                return;
            }
        }
        print("Item not found: " + name);
    }

    func &displayInventory() {
        if &items.size() == 0 {
            print("Inventory is empty.");
        } else {
            print("Inventory:");
            for item in &items {
                item.display();
            }
        }
    }

    func &totalValue() {
        total = 0.0;
        for item in &items {
            total += item.quantity * item.price;
        }
        return total;
    }
}

func main() {
    inventory = Inventory();

    inventory.addItem(Item("Apples", 10, 0.5));
    inventory.addItem(Item("Bananas", 20, 0.3));
    inventory.addItem(Item("Oranges", 15, 0.7));

    inventory.displayInventory();

    inventory.removeItem("Bananas");

    inventory.displayInventory();

    total = inventory.totalValue();
    print("Total inventory value: $" + str(total));
}

main();
```