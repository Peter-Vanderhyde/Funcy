file = read("fibonnaci.fy").replace(" ", "");

mapped_vars = [];
for i = 0, i < file.length(), i += 1 {
    if file[i] == "=" and file[i+1] != "=" and file[i-1] != "=" {
        name = file[i-1];
        string = "";
        for c in file[i+1:] {
            if c == ';' {
                break;
            }
            string += c;
        }
        mapped_vars.append([name, string]);
    }
}

for [name, value] in mapped_vars {
    print(name + " = " + value);
}