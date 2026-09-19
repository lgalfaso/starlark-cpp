def binary_ops(a, b):
    return (
        (a or b),
        (a and b),
        (a == b),
        (a != b),
        (a < b),
        (a > b),
        (a <= b),
        (a >= b),
        (a in [b]),
        (a not in [b]),
        (a | b),
        (a ^ b),
        (a & b),
        (a << b),
        (a >> b),
        (a - b),
        (a + b),
        (a * b),
        (a % b),
        (a / b),
        (a // b),
    )

_ = binary_ops(1, 2)
