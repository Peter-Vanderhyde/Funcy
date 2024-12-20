print(enumerate(list("Test")));
print(enumerate(list("Test"), 6));

func enumerate(l, start) {
    return zip(range(start, start + l.size()), l);
}

print(enumerate(list("Test"), 4));