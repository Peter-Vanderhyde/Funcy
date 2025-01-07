a = {1:"one",2:"two"};
l = [];
l.append(a);
a[1] = "uno";
a[3] = "three";
print(a);
print(l[0]);