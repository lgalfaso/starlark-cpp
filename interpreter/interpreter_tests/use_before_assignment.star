assert_fail("""
a = b 
b = []
""", error_message = "UnboundLocalError: cannot access local variable 'b' where it is not associated with a value")

