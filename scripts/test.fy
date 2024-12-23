a = [1, 2, 3];
d = dict({1:"one",2:"two",3:a});

e = dict(d);

d.pop(3);
a = [1, 2];
print(e);

d = {1:"one",2:"two"};

a = [1, 2, d];

b = list(a);
a.pop(0);
print(a);
print(b);
d.pop(1);
print(a);
print(b);