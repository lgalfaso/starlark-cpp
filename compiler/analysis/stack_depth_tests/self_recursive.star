def countdown(n):
    if n <= 0:
        return 0
    return countdown(n - 1) + 1

_ = countdown(3)
