assert_eq("abc".capitalize(), "Abc")
assert_eq("abc def".capitalize(), "Abc def")
assert_eq(" ABC ".capitalize(), " abc ")

# Implements special case for ligatures.
assert_eq("\uFB00air".capitalize(), "Ffair")

# Implements Final_Sigma special case
assert_eq("ΠΕΡΙΠΤΏΣΕΙΣ".capitalize(), "Περιπτώσεις")
# and does so using the word break algorithm.
assert_eq("Σ.Σ.Σ".capitalize(), "Σ.σ.ς")
