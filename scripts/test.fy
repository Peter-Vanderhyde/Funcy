nums = {
    1:"one",
    2:"two",
    3:"three"
};

dicts = {
    "name" : {
        "first":"Peter"
        "last":"Vanderhyde"
    }
};

dicts.update(nums);
print(dicts.pop(-1, "Nope"));