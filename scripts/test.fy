class Order {
    func &Order() {}
}

json = readFile("save_file.json");
json = json.toJson();
orders = [];
for order in json["orders"] {
    o = Order();
    for [key, value] in order {
        o.setAttr(key, value);
    }
    orders.append(o);
}

print(orders);
print(orders[1].items);