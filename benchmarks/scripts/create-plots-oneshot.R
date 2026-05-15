library(tidyverse)

source("utils.R")

alphabet_sizes <- read_csv("metadata/alphabet-sizes.csv", col_names = c("name", "alphabet_size"))

results_mata <- read_csv("results/oneshot_comparison_mata.csv")
results_mata_lazy <- read_csv("results/oneshot_comparison_mata_lazy.csv")
results_mata_lazy_bfs <- read_csv("results/oneshot_comparison_mata_lazy_bfs.csv")
results_mata_lazy_bfs_worklist_fix <- read_csv("results/oneshot_comparison_mata_lazy_bfs_worklist_fix.csv")
results_mata_antichains_inclusion <- read_csv("results/oneshot_comparison_mata_antichains_inclusion.csv")
results_mata_antichains_inclusion_explicit_product_for_ind <- read_csv("results/oneshot_comparison_mata_antichains_inclusion_explicit_product_for_ind.csv")
results_dodo <- read_csv("results/oneshot_comparison_dodo.csv") 
results_leduy <- read_csv("results/leduy_lazytree.csv")

compare_processed_data(results_dodo, results_mata_lazy_bfs_worklist_fix, color = interpretation)
# for debugging purposes
results <- join_results(results_dodo, results_mata_lazy_bfs_worklist_fix)
results %>% arrange(desc(time_p2 / time_p1)) %>% mutate(time_p2 / time_p1)

# BELOW: OLD CODE; NOT NEEDED ANYMORE !!!
results <- full_join(results_mata, results_dodo, by = join_by(name, property, interpretation), suffix = c("_mata", "_dodo"))

bugfix <- full_join(results_mata, results_mata_buggy, by = join_by(name, property, interpretation), suffix = c("", "_buggy"))
results_antichains_inclusion <- full_join(results_mata_antichains_inclusion, results_dodo, by = join_by(name, property, interpretation), suffix = c("_mata", "_dodo"))
results_antichains_vs_antichains_inclusion <- full_join(results_mata, results_mata_antichains_inclusion, by = join_by(name, property, interpretation), suffix = c("_anti", "_incl"))
results_antichains_inclusion_explicit_product_for_ind <- full_join(results_mata_antichains_inclusion_explicit_product_for_ind, results_dodo, by = join_by(name, property, interpretation), suffix = c("_mata", "_dodo"))
results_antichains_inclusion_vs_antichains_inclusion_explicit_product_for_ind <- full_join(results_mata_antichains_inclusion, results_mata_antichains_inclusion_explicit_product_for_ind, by = join_by(name, property, interpretation), suffix = c("_anti", "_explicit"))
results_antichains_inclusion_explicit_product_for_ind <- results_antichains_inclusion_explicit_product_for_ind %>% full_join(alphabet_sizes, by = join_by(name))
results_mata_antichains_inclusion_explicit_product_for_ind_vs_leduy <- full_join(results_mata_antichains_inclusion_explicit_product_for_ind, results_leduy, by = join_by(name, property, interpretation), suffix = c("_jakob", "_leduy"))
results_leduy_vs_dodo <- full_join(results_leduy, results_dodo, by = join_by(name, property, interpretation), suffix = c("_leduy", "_dodo"))
results_lazy <- full_join(results_mata_lazy, results_dodo, by = join_by(name, property, interpretation), suffix = c("_mata", "_dodo"))
results_lazy_bfs <- full_join(results_mata_lazy_bfs, results_dodo, by = join_by(name, property, interpretation), suffix = c("_mata", "_dodo"))
results_lazy_dfs_vs_bfs <- full_join(results_mata_lazy, results_mata_lazy_bfs, by = join_by(name, property, interpretation), suffix = c("_dfs", "_bfs"))

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
  geom_point(mapping = aes(x = time_dodo, y = time_mata, color = interpretation)) +
  scale_x_log10() +
  scale_y_log10() +
  geom_function(fun = function(x) x) +
  geom_function(fun = function(x) 0.1 * x, color = "green") +
  geom_function(fun = function(x) 10 * x, color = "red")
ggsave("comparison.png")

results_antichains_inclusion %>%
  ggplot() +
  geom_point(mapping = aes(x = time_dodo, y = time_mata, color = interpretation)) +
  scale_x_log10() +
  scale_y_log10() +
  geom_function(fun = function(x) x) +
  geom_function(fun = function(x) 0.1 * x, color = "green") +
  geom_function(fun = function(x) 10 * x, color = "red")
ggsave("comparison-antichains-inclusion.png")

results_lazy %>%
  ggplot() +
  geom_point(mapping = aes(x = time_dodo, y = time_mata, color = interpretation)) +
  scale_x_log10() +
  scale_y_log10() +
  geom_function(fun = function(x) x) +
  geom_function(fun = function(x) 0.1 * x, color = "green") +
  geom_function(fun = function(x) 10 * x, color = "red")

results_mata_antichains_inclusion_explicit_product_for_ind_vs_leduy %>%
  ggplot() +
  geom_point(mapping = aes(x = time_jakob, y = time_leduy, color = interpretation)) +
  scale_x_log10() +
  scale_y_log10() +
  geom_function(fun = function(x) x) +
  geom_function(fun = function(x) 0.1 * x, color = "green") +
  geom_function(fun = function(x) 10 * x, color = "red")
ggsave("comparison-leduy-antichains_inclusion_explicit_product_for_ind.png")
results_leduy_vs_dodo %>%
  ggplot() +
  geom_point(mapping = aes(x = time_dodo, y = time_leduy, color = interpretation)) +
  scale_x_log10() +
  scale_y_log10() +
  geom_function(fun = function(x) x) +
  geom_function(fun = function(x) 0.1 * x, color = "green") +
  geom_function(fun = function(x) 10 * x, color = "red")
ggsave("comparison-leduy-dodo.png")

# !!! change axes wrt. other plots !!! 
results_antichains_inclusion_explicit_product_for_ind %>%
  ggplot() +
  geom_point(mapping = aes(x = time_dodo, y = time_mata, color = interpretation)) +
  scale_x_log10() +
  scale_y_log10() +
  geom_function(fun = function(x) x) +
  geom_function(fun = function(x) 0.1 * x, color = "green") +
  geom_function(fun = function(x) 10 * x, color = "red")
ggsave("comparison-antichains-inclusion-explicit-product-for-ind.png")
results_antichains_inclusion_explicit_product_for_ind %>%
  filter(interpretation == "f") %>%
  ggplot() +
  geom_point(mapping = aes(x = time_dodo, y = time_mata, color = factor(alphabet_size))) +
  scale_x_log10() +
  scale_y_log10() +
  geom_function(fun = function(x) x) +
  geom_function(fun = function(x) 0.1 * x, color = "green") +
  geom_function(fun = function(x) 10 * x, color = "red")

# comparison of dfs vs bfs
results_lazy_dfs_vs_bfs %>%
  ggplot() +
  geom_point(mapping = aes(x = time_dfs, y = time_bfs, color = interpretation)) +
  scale_x_log10() +
  scale_y_log10() +
  geom_function(fun = function(x) x) +
  geom_function(fun = function(x) 0.1 * x, color = "green") +
  geom_function(fun = function(x) 10 * x, color = "red")
# color: was output 1 or 0?
results_lazy_dfs_vs_bfs %>%
  ggplot() +
  geom_point(mapping = aes(x = time_dfs, y = time_bfs, color = output_dfs)) +
  scale_x_log10() +
  scale_y_log10() +
  geom_function(fun = function(x) x) +
  geom_function(fun = function(x) 0.1 * x, color = "green") +
  geom_function(fun = function(x) 10 * x, color = "red")
# compare bfs with dodo
results_lazy_bfs %>%
  ggplot() +
  geom_point(mapping = aes(x = time_dodo, y = time_mata, color = interpretation)) +
  scale_x_log10() +
  scale_y_log10() +
  geom_function(fun = function(x) x) +
  geom_function(fun = function(x) 0.1 * x, color = "green") +
  geom_function(fun = function(x) 10 * x, color = "red")

# end comparison of dfs vs bfs

results_antichains_vs_antichains_inclusion %>%
  ggplot() +
  geom_point(mapping = aes(x = time_anti, y = time_incl, color = interpretation)) +
  scale_x_log10() +
  scale_y_log10() +
  geom_function(fun = function(x) x) +
  geom_function(fun = function(x) 10 * x, color = "green") +
  geom_function(fun = function(x) 0.1 * x, color = "red")
ggsave("comparison-antichains-vs-antichains-inclusion.png")
# print results, ordered by factor mata is slower
results_antichains_inclusion_explicit_product_for_ind %>%
  mutate(time_mata / time_dodo) %>%
  arrange(desc(time_mata / time_dodo)) %>%
  print(n = 186)

results_antichains_inclusion_vs_antichains_inclusion_explicit_product_for_ind %>%
  ggplot() +
  geom_point(mapping = aes(x = time_anti, y = time_explicit, color = interpretation)) +
  scale_x_log10() +
  scale_y_log10() +
  geom_function(fun = function(x) x) +
  geom_function(fun = function(x) 10 * x, color = "green") +
  geom_function(fun = function(x) 0.1 * x, color = "red")
