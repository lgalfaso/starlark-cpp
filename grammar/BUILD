# Copyright 2024 Lucas Mirelmann

cc_library(
    name = "numeric_parser",
    srcs = ["numeric_parser.cpp"],
    hdrs = ["numeric_parser.hpp"],
    deps = [
        "//unicode:utf8_reader",
    ],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "numeric_parser_test",
    srcs = ["numeric_parser_test.cpp"],
    deps = [
        ":numeric_parser",
        "@googletest//:gtest_main",
    ],
    size = "small",
)

cc_library(
    name = "quoted",
    srcs = ["quoted.cpp"],
    hdrs = ["quoted.hpp"],
)

