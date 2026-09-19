t0 = ()
t1 = (0,)
t2 = (1, 2)
t3 = (3, 4, 5)

items = [1, 2, 3]
mapping = {"a": 1, "b": 2}

comp0 = [x for x in [0]]
comp1 = [x for a in [1] for x in [2]]
comp2 = [x for a in [3, 4, 5] if a % 2 == 0 for x in [a, a, a]]
comp3 = [x.y for x in [0]]
comp4 = [x for x in [a for a in [0, 1, 2]]]
comp5 = [[x for x in [a]] for a in [0]]

dict_comp = {k: v for k, v in [("a", 1), ("b", 2)]}

_ = (t0, t1, t2, t3, items, mapping, comp0, comp1, comp2, comp3, comp4, comp5, dict_comp)
