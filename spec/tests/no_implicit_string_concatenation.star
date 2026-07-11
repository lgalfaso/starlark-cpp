assert_fail("""
arguments = [
    "-c",
    "-O2",
    "-Wall"
    "-Werror",
]
""", allow_static_error = True)
