class Test {
    func &Test() {
        &name = "Peter";
    }
}

t = Test();
print(t.hasAttr("name"));
print(t.hasAttr("age"));
t.setAttr("age", 21);
print(t.hasAttr("age"));
t.delAttr("name");
print(t.hasAttr("name"));
t.delAttr("name");