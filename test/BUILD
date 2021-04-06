cc_test(
    name = "solver_test",
    srcs = [
        "solver_test.cpp",
    ],
    deps = [
        "//solver/algorithm",
        "//solver/encoder",
        "@gtest//:gtest_main",
    ],
)

cc_test(
    name = "encoder_test",
    srcs = [
        "encoder_test.cpp",
    ],
    deps = [
        "//solver/algorithm",
        "//solver/encoder",
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
        "//solver/encoder",
        "//solver/transform",
        "@gtest//:gtest_main",
    ],
)

cc_binary(
    name = "stress",
    srcs = ["stress.cpp"],
    deps = [
        "//solver/algorithm",
        "//solver/encoder",
    ],
)
