package(default_visibility = ["//visibility:public"])

cc_library(
    name = "builder",
    srcs = [
        "cardinality.cpp",
        "coloring.cpp",
        "dimacs.cpp",
        "langford.cpp",
        "partial_order.cpp",
        "rand.cpp",
        "simple.cpp",
        "waerden.cpp",
    ],
    hdrs = [
        "builder.h",
        "cardinality.h",
        "coloring.h",
        "dimacs.h",
        "langford.h",
        "partial_order.h",
        "rand.h",
        "simple.h",
        "waerden.h",
    ],
    deps = [
        "//solver",
        "//util",
    ],
)
