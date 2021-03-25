#pragma once

namespace solver {
namespace builder {

/*
 * Builder mode to favor different characteristics.
 */
enum class Mode {
  /* Prefer instances with less variables. */
  kLessVariables,
  /* Prefer instances with less clauses. */
  kLessClauses,
};

} // namespace builder
} // namespace solver
