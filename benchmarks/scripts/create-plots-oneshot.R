library(tidyverse)
 
results_mata <- read_csv("results/oneshot_comparison_mata.csv")
results_dodo <- read_csv("results/oneshot_comparison_dodo.csv") 

results <- full_join(results_mata, results_dodo, by = join_by(name, property, interpretation), suffix = c("_mata", "_dodo"))

# how many were solved? -> text output
print(factor(results$output_mata) %>% summary())
print(factor(results$output_dodo) %>% summary())
print("solved by mata, but not by dodo:")
print(sum(results$output_mata %in% c("0", "1") & !(results$output_dodo %in% c("0", "1"))))
print("solved by dodo, but not by mata:")
print(sum(results$output_dodo %in% c("0", "1") & !(results$output_mata %in% c("0", "1"))))

# how fast were they solved?
results %>%
  pivot_longer(cols = c(time_mata, time_dodo), names_to = "column", values_to = "time") %>%
  group_by(column) %>%
  arrange(time) %>%
  mutate(solved_cumulatively = row_number()) %>%
  ggplot() +
  geom_step(mapping = aes(x = time, color = column, y = solved_cumulatively)) +
  xlab("time (s)") +
  ylab("number of instances solved in time") +
  labs(color = "framework")
ggsave("cumulative.png")

results %>%
  ggplot() +
  geom_point(mapping = aes(x = time_mata, y = time_dodo, color = interpretation)) +
  scale_x_log10() +
  scale_y_log10() +
  geom_function(fun = function(x) x) +
  geom_function(fun = function(x) 10 * x, color = "green") +
  geom_function(fun = function(x) 0.1 * x, color = "red")
ggsave("comparison.png")
