library(tidyverse)
library(patchwork)

# Comparison of the artificial permutation benchmarks with and without
# --bddlike (results/results8985e62.csv). The plot is produced by a copy of the
# generic comparison function used in create-plots-oneshot.R (utils.R). Two
# small improvements over the original:
#   * every axis carries exactly one label (the original showed each axis label
#     twice, because the marginal panels also set x/y labels),
#   * the `widths` are set on the two rows (the original applied them to the
#     outer vertical stack, where they had no effect, so the right column ended
#     up as wide as the main plot); the chosen ratio makes the right column about
#     as wide as the top row is high,
#   * OOM/timeout runs are treated as missing runtimes, and a runtime of 0.000 s
#     (i.e. below the millisecond resolution) is lifted to 0.001 s so that it can
#     be displayed on the log scale.

compare_programs <- function(df, ...) {
  extra_aes <- enquos(...)

  # split data
  both <- df %>% filter(!is.na(time_p1), !is.na(time_p2))
  t1_timeout <- df %>% filter(is.na(time_p1), !is.na(time_p2))
  t2_timeout <- df %>% filter(!is.na(time_p1), is.na(time_p2))
  t1_t2_timeout <- df %>% filter(is.na(time_p1), is.na(time_p2))

  # common axis limits
  lims <- range(c(df$time_p1, df$time_p2), na.rm = TRUE)

  # main scatter plot: carries the only x and y axis labels
  p_main <- ggplot(both, aes(time_p1, time_p2, !!!extra_aes)) +
    geom_point(alpha = 0.7) +
    scale_x_log10(limits = lims) +
    scale_y_log10(limits = lims) +
    geom_function(fun = function(x) 10 * x, color = "red", linetype = "dashed") +
    geom_function(fun = function(x) 0.1 * x, color = "green", linetype = "dashed") +
    geom_function(fun = function(x) x, color = "gray", linetype = "dashed") +
    labs(
      x = "runtime without --bddlike (s)",
      y = "runtime with --bddlike (s)"
    )

  # top bar: program 2 timed out (y = NA), distribution over x
  p_top <- ggplot(t2_timeout %>% mutate(timeout = factor("timeout")),
                  aes(x = time_p1, y = timeout, !!!extra_aes)) +
    geom_point(alpha = 0.7) +
    scale_x_log10(limits = lims) +
    labs(x = NULL, y = NULL)

  # right bar: program 1 timed out (x = NA), distribution over y
  p_right <- ggplot(t1_timeout %>% mutate(timeout = factor("timeout")),
                    aes(x = timeout, y = time_p2, !!!extra_aes)) +
    geom_point(alpha = 0.7) +
    scale_y_log10(limits = lims) +
    labs(x = NULL, y = NULL)

  # empty corner: both programs timed out
  p_empty <- ggplot(t1_t2_timeout %>% mutate(timeout_x = factor("timeout"), timeout_y = factor("timeout")),
                    aes(x = timeout_x, y = timeout_y, !!!extra_aes)) +
    geom_jitter(alpha = 0.7) +
    labs(x = NULL, y = NULL)

  # widths belong to the individual rows; the ratio makes the right column
  # roughly as wide as the top row is high (i.e. the corner panel is square)
  top_row <- p_top + p_empty + plot_layout(widths = c(3.5, 1))
  bottom_row <- p_main + p_right + plot_layout(widths = c(3.5, 1))

  (top_row / bottom_row) +
    plot_layout(
      heights = c(1, 5),
      guides = "collect",
      axis_titles = "collect"
    )
}

raw_results <- read_csv("results/results8985e62.csv", show_col_types = FALSE)

# OOM/timeout runs have no meaningful runtime: treat them as missing. Times are
# reported with millisecond resolution, so 0.000 s really means "< 0.5 ms",
# which has to be lifted to a positive value to be visible on a log scale.
results <- raw_results %>%
  mutate(time = if_else(oom == "yes" | timeout == "yes", NA_real_, time)) %>%
  mutate(time = if_else(!is.na(time), pmax(time, 0.001), time))

# compare the runs without --bddlike (p1) against those with --bddlike (p2)
results_without_bddlike <- results %>% filter(bddlike == "no")
results_with_bddlike <- results %>% filter(bddlike == "yes")

results_joined <- full_join(
  results_without_bddlike,
  results_with_bddlike,
  by = join_by(name, interpretation),
  suffix = c("_p1", "_p2")
)

compare_programs(results_joined, color = interpretation)
ggsave("comparison-artificial.png", width = 8, height = 8)
