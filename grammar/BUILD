# Copyright 2024 Lucas Mirelmann

cc_library(
    name = "source",
    srcs = ["source.cpp"],
    hdrs = ["source.hpp"],
)

cc_test(
    name = "source_test",
    srcs = ["source_test.cpp"],
    deps = [
        ":source",
        "@googletest//:gtest_main",
    ],
    size = "small",
)

