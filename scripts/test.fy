# Test 1: Create and print an empty list
empty_list = [];
empty_list;

# Test 2: Add elements to the list and print
list = [1, 2, 3];
list[0];       # Access first element
list[2];       # Access last element
list += [4, 5]; # Append another list
list;          # Print updated list

# Test 3: Index out of range (positive)


# Test 4: Index out of range (negative)


# Test 5: Insert an element at the end and in the middle

# Test 6: Insert at invalid positions

# Test 7: Pop from a list

# Test 8: Check slice operation
list[1:3];     # Slice valid range
list[:];       # Slice full range
list[3:10];    # Slice out of range
list[5:2];     # Invalid slice (end before start)

# Test 9: Mixed-type list
mixed_list = [1, "hello", true, 3.14];
mixed_list[1];
mixed_list[3];

# Test 10: Deep nested lists
nested_list = [[1, 2], ["a", "b"], [[true], [false]]];
nested_list[0][1];   # Access second element of first sub-list
nested_list[2][1];   # Access second sub-list
nested_list[2][0][0]; # Access deep nested element

# Test 11: Print all lists to confirm changes
empty_list;
list;
mixed_list;
nested_list;
