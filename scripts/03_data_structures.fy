# 03_data_structures.fy - Advanced Collections and Built-ins

# String multiplication for formatting
header = "-" * 30;
print(header);
print("Generating Random Data");
print(header);

# Using dictionaries and random integers
data = {};
num_items = randInt(3, 8);

for i in range(num_items) {
    data["Key " + str(i + 1)] = "Value " + str(randInt(100, 999));
}

# Destructuring dictionaries into lists using .items()
keys = [];
values = [];
for [key, value] in data.items() {
    keys.append(key);
    values.append(value);
}

# Shuffling, copying, and Zip
print("Randomly shuffled pairs using zip():");
shuffled_keys = randShuffle(keys.copy());
shuffled_values = randShuffle(values.copy());

for pair in zip(shuffled_keys, shuffled_values) {
    print(pair);
}
print();

# List slicing
print(header);
my_list = [10, 20, 30, 40, 50, 60, 70];
print("Original List: " + str(my_list));
print("Sliced [1:4]:  " + str(my_list[1:4]));
print("Sliced [-3:-1]: " + str(my_list[-3:-1]));
print("Shuffled List: " + str(randShuffle(my_list)));
print(header);