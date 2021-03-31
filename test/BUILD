cc_test(
    name = "solver_test",
    srcs = [
        "solver_test.cpp",
    ],
    deps = [
        "//solver/algorithm",
        "//solver/builder",
        "@gtest//:gtest_main",
    ],
)

cc_test(
    name = "builder_test",
    srcs = [
        "builder_test.cpp",
    ],
    deps = [
        "//solver/algorithm",
        "//solver/builder",
        "@gtest//:gtest_main",
    ],
)

cc_test(
    name = "transform_test",
    srcs = [
        "transform_test.cpp",
    ],
    deps = [
        "//solver/algorithm",
        "//solver/builder",
        "//solver/transform",
        "@gtest//:gtest_main",
    ],
)

cc_binary(
    name = "stress",
    srcs = ["stress.cpp"],
    deps = [
        "//solver/algorithm",
        "//solver/builder",
    ],
)
