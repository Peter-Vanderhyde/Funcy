# Test 1: Basic nested function inside a class
print("Test 1: Basic nested function inside a class");
class OuterClass {
    func &OuterClass(name) {
        &name = name;
    }
    func &greet() {
        func innerGreet() {
            return "Hello, " + &name + "!";
        }
        return innerGreet();  # Call the nested function
    }
}
outer = OuterClass("Alice");
print(outer.greet());  # Expected: Hello, Alice!

# Test 2: Nested class inside a class
print("\nTest 2: Nested class inside a class");
class Container {
    func &Container(label) {
        &label = label;
    }
    class Item {
        func &Item(name, value) {
            &name = name;
            &value = value;
        }
        func &describe() {
            return &name + ": " + str(&value);
        }
    }
    func &createItem(name, value) {
        return Item(name, value);  # Instantiate the nested class
    }
}
container = Container("My Container");
item = container.createItem("Gold Coin", 100);
print(item.describe());  # Expected: Gold Coin: 100

# Test 3: Nested class accessing outer class variables
print("\nTest 3: Nested class accessing outer class variables");
class Parent {
    func &Parent(parentName) {
        &parentName = parentName;
    }
    class Child {
        func &Child(childName, parent) {
            &childName = childName;
            &parent = parent;  # Reference to the outer class instance
        }
        func &introduce() {
            return &childName + ", child of " + &parent.parentName;
        }
    }
    func &createChild(childName, parent) {
        return Child(childName, parent);  # Pass the outer instance
    }
}
parent = Parent("John");
child = parent.createChild("Emily", parent);
print(child.introduce());  # Expected: Emily, child of John


# Test 4: Deeply nested classes
print("\nTest 4: Deeply nested classes");
class LevelOne {
    class LevelTwo {
        class LevelThree {
            func &LevelThree(data) {
                &data = data;
            }
            func &getData() {
                return &data;
            }
        }
        func &LevelTwo() {&three_class = LevelThree;}
        func &getThree(data) {
            return &three_class(data);
        }
    }
    func &LevelOne() {&two_class = LevelTwo;}
    func &getTwo() {
        return &two_class();
    }
}
one = LevelOne();
two = one.getTwo();
three = two.getThree("Deep Data");
print(three.getData());            # Expected: Deep Data

# Test 5: Outer function calling inner function
print("\nTest 5: Outer function calling inner function");
class FunctionNesting {
    func &FunctionNesting() {}
    func &doSomething(data) {
        func processData(value) {
            return "Processed: " + value;
        }
        return processData(data);  # Call the inner function
    }
}
fn = FunctionNesting();
print(fn.doSomething("Inner Magic"));  # Expected: Processed: Inner Magic

# Test 6: Multiple nested classes with interactions
print("\nTest 6: Multiple nested classes with interactions");
class University {
    class Department {
        class Professor {
            func &Professor(name, subject) {
                &name = name;
                &subject = subject;
            }
            func &teach() {
                return &name + " teaches " + &subject;
            }
        }
        func &Department(name) {
            &name = name;
            &professors = [];  # List of professors
        }
        func &addProfessor(name, subject) {
            &professors.append(Professor(name, subject));
        }
        func &listProfessors() {
            for prof in &professors {
                print(prof.teach());
            }
        }
    }
    func &University(name) {
        &name = name;
        &departments = [];  # List of departments
    }
    func &addDepartment(name) {
        &departments.append(Department(name));
        return &departments[-1];  # Return the newly added department
    }
    func &listDepartments() {
        for dep in &departments {
            print("-- Department of " + dep.name + " --");
            dep.listProfessors();
        }
    }
}
uni = University("Tech University");
csDept = uni.addDepartment("Computer Science");
csDept.addProfessor("Dr. Smith", "AI");
csDept.addProfessor("Dr. Brown", "Networking");
uni.listDepartments();
# Expected:
# Dr. Smith teaches AI
# Dr. Brown teaches Networking
