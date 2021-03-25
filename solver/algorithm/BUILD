package(default_visibility = ["//visibility:public"])

cc_library(
    name = "algorithm",
    deps = [
        ":a",
        ":a2",
        ":b",
        ":d",
        ":nop",
        ":z",
    ],
)

cc_library(
    name = "a",
    srcs = ["a.cpp"],
    hdrs = ["a.h"],
    deps = [
        "//solver",
        "//util",
    ],
)

cc_library(
    name = "b",
    srcs = ["b.cpp"],
    hdrs = ["b.h"],
    deps = [
        "//solver",
        "//util",
    ],
)

cc_library(
    name = "a2",
    srcs = ["a2.cpp"],
    hdrs = ["a2.h"],
    deps = [
        "//solver",
        "//util",
    ],
)

cc_library(
    name = "d",
    srcs = ["d.cpp"],
    hdrs = ["d.h"],
    deps = [
        "//solver",
        "//util",
    ],
)

cc_library(
    name = "nop",
    srcs = ["nop.cpp"],
    hdrs = ["nop.h"],
    deps = ["//solver"],
)

cc_library(
    name = "z",
    srcs = ["z.cpp"],
    hdrs = ["z.h"],
    deps = ["//solver"],
)
