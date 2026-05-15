library(ggplot2)
library(dplyr)
library(patchwork)

compare_programs_example_df = function(seed) {
  set.seed(seed)
  df <- tibble(
    instance = paste0("i", 1:100),
    time_p1 = 10^runif(100, -2, 3),
    time_p2 = 10^runif(100, -2, 3)
  )
  
  # introduce timeouts
  df$time_p1[sample(1:100, 10)] <- NA
  df$time_p2[sample(1:100, 12)] <- NA
  
  return(df)
}

compare_programs = function(df, ...) {
  extra_aes <- enquos(...)
  
  # split data
  both <- df %>% filter(!is.na(time_p1), !is.na(time_p2))
  t1_timeout <- df %>% filter(is.na(time_p1), !is.na(time_p2))
  t2_timeout <- df %>% filter(!is.na(time_p1), is.na(time_p2))
  t1_t2_timeout <- df %>% filter(is.na(time_p1), is.na(time_p2))
  
  # common axis limits
  lims <- range(c(df$time_p1, df$time_p2), na.rm = TRUE)
  
  # main scatter plot
  p_main <- ggplot(both, aes(time_p1, time_p2, !!!extra_aes)) +
    geom_point(alpha = 0.7) +
    scale_x_log10(limits = lims) +
    scale_y_log10(limits = lims) +
    geom_function(fun = function(x) 10 * x, color = "red", linetype = "dashed") +
    geom_function(fun = function(x) 0.1 * x, color = "green", linetype = "dashed") +
    geom_function(fun = function(x) x, color = "gray", linetype = "dashed") +
    labs(
      x = "Program 1 runtime",
      y = "Program 2 runtime"
    ) +
    theme_bw()
  
  # top bar:
  # program 2 timed out (y = NA)
  # show distribution over x
  p_top <- ggplot(t2_timeout %>% mutate(timeout = factor("timeout")), mapping = aes(x = time_p1, y = timeout, !!!extra_aes)) +
    geom_point(alpha = 0.7) +
    scale_x_log10(limits = lims) +
    theme_bw() +
    labs(x = NULL, y = NULL) +
    theme(
      axis.text.x = element_blank(),
      axis.ticks.x = element_blank(),
      #axis.text.y = element_blank(),
      axis.ticks.y = element_blank()
    )
  
  # right bar:
  # program 1 timed out (x = NA)
  # show distribution over y
  p_right <- ggplot(t1_timeout %>% mutate(timeout = factor("timeout")), aes(y = time_p2, x = timeout, !!!extra_aes)) +
    geom_point(alpha = 0.7) +
    scale_y_log10(limits = lims) +
    # coord_flip() +
    labs(x = NULL, y = NULL) +
    theme_bw() +
    theme(
      axis.text.y = element_blank(),
      axis.ticks.y = element_blank(),
      # axis.text.x = element_blank(),
      axis.ticks.x = element_blank()
    )
  
  # empty corner
  p_empty <- ggplot(t1_t2_timeout %>% mutate(timeout_x = factor("timeout"), timeout_y = factor("timeout")),
                    aes(x = timeout_x, y = timeout_y, !!!extra_aes)) +
    geom_jitter(alpha = 0.7) +
    labs(x = NULL, y = NULL) +
    theme_bw() +
    theme(
      axis.text.y = element_blank(),
      axis.ticks.y = element_blank(),
      axis.text.x = element_blank(),
      axis.ticks.x = element_blank()
    )
  
  return ((p_top + p_empty) /
    (p_main + p_right) +
    plot_layout(
      widths = c(5, 1),
      heights = c(1, 5),
      guides = "collect"
    ))
}

# compare data produced by running parse-results-oneshot.py on output log in results/raw/
join_results <- function(df1, df2) {
  return (full_join(df1, df2, by = join_by(name, property, interpretation), suffix = c("_p1", "_p2")))
}
compare_processed_data <- function(df1, df2, ...) {
  joined <- join_results(df1, df2)
  return (compare_programs(joined, ...))
}
