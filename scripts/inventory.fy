# Define the Item class
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

    # Method to display item details
    func &display() {
        print("Item: " + &name + ", Quantity: " + str(&quantity) + ", Price: $" + str(&price));
    }
}

# Define the Inventory class
class Inventory {
    func &Inventory() {
        &items = list();
    }

    # Method to add an item to the inventory
    func &addItem(item) {
        &items.append(item);
        print("Added item: " + item.name);
    }

    # Method to remove an item from the inventory
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

    # Method to display the entire inventory
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

    # Method to calculate the total value of the inventory
    func &totalValue() {
        total = 0.0;
        for item in &items {
            total += item.quantity * item.price;
        }
        return total;
    }
}

# Main program
func main() {
    # Create an inventory instance
    inventory = Inventory();

    # Add some items
    inventory.addItem(Item("Apples", 10, 0.5));
    inventory.addItem(Item("Bananas", 20, 0.3));
    inventory.addItem(Item("Oranges", 15, 0.7));

    # Display the inventory
    inventory.displayInventory();

    # Remove an item
    inventory.removeItem("Bananas");

    # Display the updated inventory
    inventory.displayInventory();

    # Calculate and display the total inventory value
    total = inventory.totalValue();
    print("Total inventory value: $" + str(total));
}

# Run the main function
main();
