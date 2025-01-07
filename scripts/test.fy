class Example {
    &public_var = "accessible through instance.public_var";
    private_var = "only accessible inside class";
    &saved_info = Null;
    func &Example(label, data) {
        # constructor
        &label = label;
        &data = data;
    }
    func &getData() {
        return &data;
    }
    func &publicFunc() {
        print("Accessible through instance.publicFunc()");
    }
    func privateFunc() {
        print("Only accessible inside class");
    }
    class Info {
        # Only available in Example
        func &Info(date, data) {
            &date = date;
            # Nested classes have access to parent class member attributes and can
            # overwrite them. Once the class scope ends and returns to the parent,
            # the member data will overwrite back to what it previously was
            &data = data;
        }
        func &showInfo() {
            print("Acquired on " + &date + ":");
            print(&data);
        }
    }
    func &saveData() {
        &saved_info = Info("January 7th", &data);
    }
    func &printInfo() {
        if type(&saved_info) == Instance {
            print("Displaying info for " + &label);
            &saved_info.showInfo();
        } else {
            print("No data.");
        }
    }
}

e = Example("Report 203", "This is some data.");
e.printInfo();
e.saveData();
e.printInfo();