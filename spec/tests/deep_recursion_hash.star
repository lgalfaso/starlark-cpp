def run():
    a = ()
    for i in range(30):
        a = (a,a,a)
    return set(a)

run()
