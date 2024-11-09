# Copyright 2024 Lucas Mirelmann

package(
    default_visibility = ["//visibility:public"]
)

cc_library(
    name = "number",
    srcs = ["number.cpp"],
    hdrs = ["number.hpp"],
)

cc_test(
    name = "number_test",
    srcs = ["number_test.cpp"],
    deps = [
        ":number",
         "@googletest//:gtest_main",
    ],
    size = "medium",
)
