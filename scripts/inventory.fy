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
