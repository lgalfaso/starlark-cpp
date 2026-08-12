assert_fail("""
a = b 
b = []
""", error_message = "cannot access local variable 'b' where it is not associated with a value\n    1 | a = b \n      |     ^\n")

