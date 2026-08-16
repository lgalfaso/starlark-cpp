"""Deterministic N-Queens workloads."""

def nqueens_state_cap(n):
    """Upper bound on explored states: sum_{k=0..n} P(n, k)."""
    if n < 0:
        fail("nqueens_state_cap: n must be >= 0, got %d" % n)
    total = 1
    term = 1
    for k in range(1, n + 1):
        term *= (n - k + 1)
        total += term
    return total

def count_solutions(n):
    """Count N-Queens solutions iteratively with an n-derived loop cap."""
    solutions = 0
    if n < 0:
        fail("count_solutions: n must be >= 0, got %d" % n)

    cap = nqueens_state_cap(n)

    full_mask = (1 << n) - 1 if n > 0 else 0
    stack = [{"row": 0, "cols": 0, "diag_l": 0, "diag_r": 0}]
    for _ in range(cap):
        if len(stack) == 0:
            break

        frame = stack.pop()
        row = frame["row"]

        if row == n:
            solutions += 1
            continue

        cols = frame["cols"]
        diag_l = frame["diag_l"]
        diag_r = frame["diag_r"]

        # Push in reverse so DFS explores columns left-to-right.
        for rev_col in range(n):
            col = n - 1 - rev_col
            bit = 1 << col

            if (cols & bit) != 0 or (diag_l & bit) != 0 or (diag_r & bit) != 0:
                continue

            stack.append({
                "row": row + 1,
                "cols": cols | bit,
                "diag_l": ((diag_l | bit) << 1) & full_mask,
                "diag_r": ((diag_r | bit) >> 1) & full_mask,
            })

    if len(stack) != 0:
        fail("bench_count_solutions: exhausted cap=%d at n=%d" % (cap, n))

    return solutions

_KNOWN_SOLUTION_COUNTS = {
    1: 1,
    2: 0,
    3: 0,
    4: 2,
    5: 10,
    6: 4,
    7: 40,
    8: 92,
    9: 352,
    10: 724,
    11: 2680,
}

def run():
    for n in range(1, 12):
        r = count_solutions(n)
        if r != _KNOWN_SOLUTION_COUNTS[n]:
            fail("Expected number of solutions does not match")
        print(r)

run()
