{x: x for x in [0]}
{x: x for a in [1] for x in [2]}
{x: x for a in [3, 4, 5] if a % 2 == 0 for x in [a, a, a]}
{x: x.y for x in [0]}
{x: x for x in {a: a for a in [0, 1, 2]}}
{a: a for y in {x: x for x in [a]} for a in [0]}
