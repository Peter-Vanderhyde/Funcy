# 04_object_oriented.fy - Object-Oriented Programming

class Robot {
    # Public variables marked with &
    &name = "";
    &battery_level = 100;
    
    # Private variable (only accessible inside the class)
    serial_number = "UNKNOWN";

    # Public variables can all be defined in the constructor
    # but private variables must be defined in this outer scope

    # Constructor
    func &Robot(name, serial) {
        &name = name;
        serial_number = serial;

        temp_variable = "I'm not a private variable.";
        # This variable is forgotton outside of the constructor scope
    }

    # Public Method
    func &work(hours) {
        if &battery_level <= 0 {
            print(&name + " is out of battery!");
            return Null;
        }
        
        drain = hours * 10;
        &battery_level -= drain;
        print(&name + " worked for " + str(hours) + " hours. Battery: " + str(&battery_level) + "%");
        
        # Calling a private method internally
        checkBattery();
    }

    # Public getter for private variable
    func &getSerial() {
        return serial_number;
    }

    # Private method
    func checkBattery() {
        if &battery_level < 20 {
            print("Warning: " + &name + " battery is getting low!");
        }
    }
}

# Creating an instance
bot = Robot("Robo-Bob", "XJ-92");
print("Created bot: " + bot.name);
print("Serial Number: " + bot.getSerial());
print();

bot.work(5);
bot.work(4);
print();

# Using built-in instance functions
if bot.hasAttr("battery_level") {
    print("Direct battery access via attribute: " + str(bot.battery_level) + "%");
}

# Modifying instance attributes dynamically
bot.status = "Online";
# Could also do:  bot.setAttr("status", "Online");
print("New status attribute created");
print("Status: " + bot.status);