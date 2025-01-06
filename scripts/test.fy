class Person {
    invis = true;
    &visible = true;
    func Person(name, age) {
        &name = name;
        &age = age;
        func deeper() {
            invis = not invis;
        }
        deeper();
        &showStats();
    }
    func showStats() {
        print(&name + " is " + str(&age) + " years old.");
    }
    func birthday() {
        &age += 1;
    }
}

man = Person("Dave", 35);
print(man.visible);
man.showStats();
man.birthday();
man.showStats();
print(man.name);
print(man.showStats);