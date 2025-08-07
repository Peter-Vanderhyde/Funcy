func add(a, b) {
    if type(a) == String {
        throw "Invalid argument!";
    }
    return a + b;
}

func test(a) {
    return a;
}

func go(l, printing=false) {
    if type(l) == List {
        if not printing {print("Entering loop:");}
        args = [];
        for i in range(length(l)) {
            args.append(go(l[i])); # This will not append the list in the middle because it gets overwritten in the recursive call since the same variable is used
            if not printing{print(i, "returned", go(l[i], true));}
            if not printing{print("appended", args);}
        }
        if not printing{print("Exiting loop:");}
        return args;
    } else {
        return l;
    }
}

#print(go([1, [9, "this"], 4])); # See function

#x = test(add);
#print(x(1, 2)); # This is fine
print(test(add)(1, 2)); # Invalid causes program to crash???

#add("this", 7); # Prints "std::exception" instead of message

#print(test(add)(1, 2)); # Invalid causes program to crash???