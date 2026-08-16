"""Pure-Starlark B-tree (iterative insert/lookup/delete, Bazel-safe).

Nodes are dicts with sorted key lists. Internal nodes have parallel children.
Leaf splits use B+tree-style duplication so promoted separators retain values.
`order` is the minimum degree t (CLRS): max keys 2t-1, min keys t-1.
"""

DEFAULT_ORDER = 8

def _max_keys(order):
    return 2 * order - 1

def _min_keys(order, is_root):
    if is_root:
        return 0
    return order - 1

def _split_limit(tree):
    return tree["height"] + 1

def _traversal_limit(tree):
    return max(1, tree["size"]) * tree["height"]

def _search_index(keys, key):
    """Return insertion index in a leaf: first i with keys[i] >= key."""
    lo = 0
    hi = len(keys)
    for _ in range(len(keys) + 1):
        if lo >= hi:
            break
        mid = (lo + hi) // 2
        if keys[mid] < key:
            lo = mid + 1
        else:
            hi = mid
    return lo

def _child_index(keys, key):
    """Child index for internal descent (key == keys[j] goes to child j+1)."""
    i = 0
    for j in range(len(keys)):
        if key < keys[j]:
            return i
        i = j + 1
    return i

def _leaf_key_index(keys, key):
    idx = _search_index(keys, key)
    return idx, idx < len(keys) and keys[idx] == key

def _make_leaf():
    return {"leaf": True, "keys": [], "vals": []}

def _make_internal():
    return {"leaf": False, "keys": [], "children": []}

def btree_new(order = DEFAULT_ORDER):
    """Create an empty B-tree. order is the minimum degree (CLRS t)."""
    return {
        "order": order,
        "root": _make_leaf(),
        "size": 0,
        "height": 1,
    }

def _descend_to_leaf(tree, key, op_name, record_path = True):
    """Walk to the leaf that would hold key. Counts one visit for the leaf."""
    path = []
    node = tree["root"]
    height = tree["height"]
    for _ in range(height):
        if node["leaf"]:
            return node, path
        idx = _child_index(node["keys"], key)
        if record_path:
            path.append((node, idx))
        node = node["children"][idx]
    fail("%s: descent exceeded height %d (corrupt tree?)" % (op_name, height))

def _split_node(node):
    """Split a full node; return (promote_key, left, right)."""
    keys = node["keys"]
    mid = len(keys) // 2
    promote_key = keys[mid]

    if node["leaf"]:
        left = {
            "leaf": True,
            "keys": keys[:mid],
            "vals": node["vals"][:mid],
        }
        right = {
            "leaf": True,
            "keys": keys[mid:],
            "vals": node["vals"][mid:],
        }
        return promote_key, left, right

    left = {
        "leaf": False,
        "keys": keys[:mid],
        "children": node["children"][:mid + 1],
    }
    right = {
        "leaf": False,
        "keys": keys[mid + 1:],
        "children": node["children"][mid + 1:],
    }
    return promote_key, left, right

def _grow_root(tree, promote_key, left, right):
    new_root = _make_internal()
    new_root["keys"] = [promote_key]
    new_root["children"] = [left, right]
    tree["root"] = new_root
    tree["height"] += 1

def _sync_leaf_separators(parent, child_idx, leaf):
    """Keep B+ parent separators aligned with adjacent leaves."""
    if len(leaf["keys"]) == 0:
        return
    if child_idx > 0:
        parent["keys"][child_idx - 1] = leaf["keys"][0]
    if child_idx < len(parent["keys"]):
        right = parent["children"][child_idx + 1]
        if len(right["keys"]) > 0:
            parent["keys"][child_idx] = right["keys"][0]

def _shrink_root_if_needed(tree):
    root = tree["root"]
    if root["leaf"]:
        return
    if len(root["children"]) == 1:
        tree["root"] = root["children"][0]
        tree["height"] -= 1

def _borrow_leaf_from_left(parent, child_idx, leaf):
    left = parent["children"][child_idx - 1]
    leaf["keys"].insert(0, left["keys"][-1])
    leaf["vals"].insert(0, left["vals"][-1])
    left["keys"].pop()
    left["vals"].pop()
    _sync_leaf_separators(parent, child_idx, leaf)

def _borrow_leaf_from_right(parent, child_idx, leaf):
    right = parent["children"][child_idx + 1]
    leaf["keys"].append(right["keys"][0])
    leaf["vals"].append(right["vals"][0])
    right["keys"].pop(0)
    right["vals"].pop(0)
    _sync_leaf_separators(parent, child_idx, leaf)

def _merge_leaf_with_left(parent, child_idx):
    left = parent["children"][child_idx - 1]
    leaf = parent["children"][child_idx]
    left["keys"].extend(leaf["keys"])
    left["vals"].extend(leaf["vals"])
    parent["keys"].pop(child_idx - 1)
    parent["children"].pop(child_idx)
    _sync_leaf_separators(parent, child_idx - 1, left)

def _merge_leaf_with_right(parent, child_idx):
    leaf = parent["children"][child_idx]
    right = parent["children"][child_idx + 1]
    leaf["keys"].extend(right["keys"])
    leaf["vals"].extend(right["vals"])
    parent["keys"].pop(child_idx)
    parent["children"].pop(child_idx + 1)
    _sync_leaf_separators(parent, child_idx, leaf)

def _borrow_internal_from_left(parent, child_idx, node):
    sibling = parent["children"][child_idx - 1]
    node["keys"].insert(0, parent["keys"][child_idx - 1])
    parent["keys"][child_idx - 1] = sibling["keys"][-1]
    sibling["keys"].pop()
    node["children"].insert(0, sibling["children"][-1])
    sibling["children"].pop()

def _borrow_internal_from_right(parent, child_idx, node):
    sibling = parent["children"][child_idx + 1]
    node["keys"].append(parent["keys"][child_idx])
    parent["keys"][child_idx] = sibling["keys"][0]
    sibling["keys"].pop(0)
    node["children"].append(sibling["children"][0])
    sibling["children"].pop(0)

def _merge_internal_with_left(parent, child_idx):
    left = parent["children"][child_idx - 1]
    node = parent["children"][child_idx]
    left["keys"].append(parent["keys"][child_idx - 1])
    left["keys"].extend(node["keys"])
    left["children"].extend(node["children"])
    parent["keys"].pop(child_idx - 1)
    parent["children"].pop(child_idx)

def _merge_internal_with_right(parent, child_idx):
    node = parent["children"][child_idx]
    right = parent["children"][child_idx + 1]
    node["keys"].append(parent["keys"][child_idx])
    node["keys"].extend(right["keys"])
    node["children"].extend(right["children"])
    parent["keys"].pop(child_idx)
    parent["children"].pop(child_idx + 1)

def _fix_internal_underflow(tree, path):
    min_k = _min_keys(tree["order"], False)

    for _ in range(_split_limit(tree)):
        if not path:
            break

        node, idx = path[-1]
        if len(path) == 1:
            break

        if len(node["keys"]) >= min_k:
            return

        parent, parent_idx = path[-2]

        if parent_idx > 0:
            left = parent["children"][parent_idx - 1]
            if len(left["keys"]) > min_k:
                _borrow_internal_from_left(parent, parent_idx, node)
                return

        if parent_idx < len(parent["children"]) - 1:
            right = parent["children"][parent_idx + 1]
            if len(right["keys"]) > min_k:
                _borrow_internal_from_right(parent, parent_idx, node)
                return

        if parent_idx > 0:
            _merge_internal_with_left(parent, parent_idx)
        else:
            _merge_internal_with_right(parent, parent_idx)
        path.pop()

    _shrink_root_if_needed(tree)

def _rebalance_leaf_after_delete(tree, path, leaf):
    min_k = _min_keys(tree["order"], False)
    if len(leaf["keys"]) >= min_k or not path:
        return

    parent, child_idx = path[-1]

    if child_idx > 0:
        left = parent["children"][child_idx - 1]
        if len(left["keys"]) > min_k:
            _borrow_leaf_from_left(parent, child_idx, leaf)
            return

    if child_idx < len(parent["children"]) - 1:
        right = parent["children"][child_idx + 1]
        if len(right["keys"]) > min_k:
            _borrow_leaf_from_right(parent, child_idx, leaf)
            return

    if child_idx > 0:
        _merge_leaf_with_left(parent, child_idx)
    else:
        _merge_leaf_with_right(parent, child_idx)

    _fix_internal_underflow(tree, path)

def btree_get(tree, key):
    """Look up key; returns (value or None)."""
    leaf, _ = _descend_to_leaf(tree, key, "btree_get", False)
    idx, found = _leaf_key_index(leaf["keys"], key)
    if found:
        return leaf["vals"][idx]
    return None

def btree_insert(tree, key, value):
    """Insert key/value (updates existing)."""
    max_k = _max_keys(tree["order"])
    leaf, path = _descend_to_leaf(tree, key, "btree_insert")
    keys = leaf["keys"]
    vals = leaf["vals"]
    idx, found = _leaf_key_index(keys, key)
    if found:
        vals[idx] = value
        return

    keys.insert(idx, key)
    vals.insert(idx, value)
    tree["size"] += 1

    promote_key = None
    left = None
    right = None
    if len(keys) > max_k:
        promote_key, left, right = _split_node(leaf)

    for _ in range(_split_limit(tree)):
        if promote_key == None:
            break

        if not path:
            _grow_root(tree, promote_key, left, right)
            break

        parent, child_idx = path.pop()
        pkeys = parent["keys"]
        pchildren = parent["children"]

        pkeys.insert(child_idx, promote_key)
        pchildren[child_idx] = left
        pchildren.insert(child_idx + 1, right)

        promote_key = None
        left = None
        right = None
        if len(pkeys) > max_k:
            promote_key, left, right = _split_node(parent)

    return

def btree_delete(tree, key):
    """Delete key if present. Returns deleted_bool."""
    min_k = _min_keys(tree["order"], False)
    leaf, path = _descend_to_leaf(tree, key, "btree_delete")
    keys = leaf["keys"]
    vals = leaf["vals"]
    idx, found = _leaf_key_index(keys, key)
    if not found:
        return False

    keys.pop(idx)
    vals.pop(idx)
    tree["size"] -= 1

    if tree["size"] == 0:
        tree["root"] = _make_leaf()
        tree["height"] = 1
        return True

    if path:
        parent, child_idx = path[-1]
        _sync_leaf_separators(parent, child_idx, leaf)

    if len(keys) >= min_k or not path:
        return True

    _rebalance_leaf_after_delete(tree, path, leaf)
    return True

def btree_height(tree):
    """Return stored tree height (single empty leaf is height 1)."""
    return tree["height"]

def btree_size(tree):
    return tree["size"]

def btree_items(tree):
    """In-order list of (key, value) pairs from leaf nodes (left-to-right DFS)."""
    out = []
    stack = [tree["root"]]
    limit = _traversal_limit(tree)
    for step in range(limit):
        if not stack:
            return out
        node = stack.pop()
        if node["leaf"]:
            keys = node["keys"]
            vals = node["vals"]
            for i in range(len(keys)):
                out.append((keys[i], vals[i]))
            continue
        children = node["children"]
        for i in range(len(children) - 1, -1, -1):
            stack.append(children[i])
    fail("btree_items: traversal exceeded limit %d (corrupt tree?)" % limit)

def btree_to_dict(tree):
    """Flatten tree to a dict."""
    d = {}
    for k, v in btree_items(tree):
        d[k] = v
    return d



_BENCH_SIZES = [0, 10, 30, 100, 1000, 5000] # + [0x110000]  # We are too slow for the moment to be able to handle this one.

# Knuth multiplicative hash for deterministic pseudo-random byte payloads.
_HASH_MIX = 2654435761

def deterministic_payload(n):
    """Build n pseudo-random bytes (deterministic)."""
    data = []
    for i in range(n):
        data.append((i * _HASH_MIX) % 0xFFFFFFFF)
    return data

def run():
    for n in _BENCH_SIZES:
        data = deterministic_payload(n)
        tree = btree_new()
        for value, key in enumerate(data):
            btree_insert(tree, key, value)
        print(btree_size(tree))
        for value, key in enumerate(data):
            if btree_get(tree, key) != value:
                fail("Fail to retrieve a key")
        for key in data:
            if not btree_delete(tree, key):
                fail("Fail to delete an entry")
        if btree_size(tree):
            fail("There are still elements present asfter deleting them all")

run()
