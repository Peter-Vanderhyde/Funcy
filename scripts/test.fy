# FIX dict[key] += value does not work
JSON_PATH = "save_file.json";

packages = [];

class Package {
    func &Package(order) {
        &id = order.id;
        &customer = order.customer;
        &item = order.item;
        &quantity = order.quantity;
        &sent = time();
        &arrives = randInt(10, 20);
    }

    func &checkDeliver() {
        if time() - &sent >= &arrives * 1000 {
            print("ORDER " + str(&id) + " was delivered to " + &customer.name + ".");
            print();
            &customer.deliverPackage(this);
            return true;
        }
        return false;
    }

    func &print() {
        print("Delivering ORDER " + str(&id) + " to " + &customer.name + " in " + str(&arrives) + " seconds.");
        print();
    }
}

class Warehouse {
    func &Warehouse() {
        &inventory = readFile(JSON_PATH).toJson();
        &delivery_queue = [];
    }

    func saveInventory() {
        writeFile(JSON_PATH, str(&inventory));
    }
    
    func restock(item) {
        print("Ordering more " + item);
        print();
        &inventory[item] = &inventory[item] + 100;
        saveInventory();
    }

    func &submitOrder(order) {
        if &inventory[order.item] < order.quantity {
            print("Unable to fulfill Order " + str(order.id) + ". Low stock for " + order.item + ".");
            order.customer.deliverPackage(Null);
            restock(order.item);
        } else {
            &inventory[order.item] = &inventory[order.item] - order.quantity;
            saveInventory();
            package = Package(order);
            package.print();
            &delivery_queue.append(package);
        }
    }
}

class Order {
    func &Order(customer, item, quantity) {
        &id = time() % 100;
        &customer = customer;
        &item = item;
        &quantity = quantity;
    }

    func &print() {
        print("ORDER " + str(&id));
        print("------------");
        print("Name: " + &customer.name);
        print("Item: " + &item + " X " + str(&quantity));
        print();
    }
}

class Customer {
    func &Customer(name) {
        &name = name;
        &inventory = {};
        &start_cooldown = time();
        &next_order = randInt(10, 30);
        &waiting = false;
    }

    func &checkOrder(warehouse) {
        if not &waiting and time() - &start_cooldown >= &next_order * 1000 {
            available = readFile(JSON_PATH).toJson();
            items = available.keys();
            order = Order(this, randChoice(items), randInt(1, 10));
            order.print();
            warehouse.submitOrder(order);
            &waiting = true;
        }
    }

    func &displayInventory() {
        print(&name + "'s Inventory");
        print("------------------");
        for [item, amount] in &inventory {
            print(item + " X " + str(amount));
        }
        print();
    }

    func &deliverPackage(package) {
        if not package {
            print(&name + " is sad :(");
            print();
        } else {
            &inventory[package.item] = &inventory.setDefault(package.item, 0) + package.quantity;
        }
        &start_cooldown = time();
        &next_order = randInt(10, 30);
        &waiting = false;
    }
}


warehouse = Warehouse();
customers = [];
names = ["Alice", "Bob", "Charlie", "Diana", "Ethan", 
         "Fiona", "George", "Hannah", "Isaac", "Julia", 
         "Kevin", "Laura", "Michael", "Natalie", "Oliver", 
         "Penelope", "Quentin", "Rachel", "Samuel", "Tiffany"];

for i in range(5) {
    name = randChoice(names);
    names.remove(name);
    customers.append(Customer(name));
}

start_time = time();
while time() < start_time + 120000 {
    for delivery in warehouse.delivery_queue {
        if delivery.checkDeliver() {
            warehouse.delivery_queue.remove(delivery);
        }
    }
    for customer in customers {
        customer.checkOrder(warehouse);
    }
}

print("Simulation Finished!");
for customer in customers {
    customer.displayInventory();
}