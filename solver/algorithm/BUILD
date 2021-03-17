package(default_visibility = ["//visibility:public"])

cc_library(
    name = "algorithm",
    deps = [
        ":A",
        ":A2",
        ":Z",
    ],
)

cc_library(
    name = "A",
    srcs = ["A.cpp"],
    hdrs = ["A.h"],
    deps = [
        "//solver",
        "//util",
    ],
)

cc_library(
    name = "A2",
    srcs = ["A2.cpp"],
    hdrs = ["A2.h"],
    deps = [
        "//solver",
        "//util",
    ],
)

cc_library(
    name = "Z",
    srcs = ["Z.cpp"],
    hdrs = ["Z.h"],
    deps = ["//solver"],
)
