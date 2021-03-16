package(default_visibility = ["//visibility:public"])

cc_library(
    name = "builder",
    deps = [
        ":dimacs",
        ":langford",
        ":simple",
        ":waerden",
    ],
)

cc_library(
    name = "simple",
    srcs = ["simple.cpp"],
    hdrs = ["simple.h"],
    deps = ["//solver"],
)

cc_library(
    name = "dimacs",
    srcs = ["dimacs.cpp"],
    hdrs = ["dimacs.h"],
    deps = ["//solver"],
)

cc_library(
    name = "langford",
    srcs = ["langford.cpp"],
    hdrs = ["langford.h"],
    deps = ["//solver"],
)

cc_library(
    name = "waerden",
    srcs = ["waerden.cpp"],
    hdrs = ["waerden.h"],
    deps = ["//solver"],
)
