package(default_visibility = ["//visibility:public"])

cc_library(
    name = "algorithm",
    deps = [
        ":A",
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
    name = "Z",
    srcs = ["Z.cpp"],
    hdrs = ["Z.h"],
    deps = ["//solver"],
)
