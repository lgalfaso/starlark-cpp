def helper(a, b, c, d, e):
    return a + b + c + d + e

print()
print("a")
print("b", "c")
print("d", "e", "f")
print(sep=" ")
print(sep=" ", end="\n")
print("g", sep=" ")
print("h", sep=" ", end="\n")
print("i", *["a"])
print("j", **dict(sep=" "))
print(sep=" ", *["a"])
print(end=" ", **dict(sep=" "))
print(*["a"], **dict(sep=" "))
print(end=" ", *["a"], **dict(sep=" "))
print("k", *["a"], **dict(sep=" "))
print("l", end=" ", **dict(sep=" "))
print("m", end=" ", *["a"])
print("n", end=" ", *["a"], **dict(sep=" "))
print(*["a"])
print(**dict(sep=" "))

lst = [1, 2]
lst.append(3)
lst.pop()
d = {"k": []}
d["k"].append(4)
"abc".capitalize

_ = "{0} {1}".format(1, 2)
_ = "{0} {1} {2}".format(1, 2, 3)
_ = "{0} {1} {2} {3}".format(1, 2, 3, 4)
_ = helper(1, 2, 3, 4, 5)
