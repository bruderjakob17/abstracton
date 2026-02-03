library(tidyverse)
 
results <- read_csv("results/processed.csv")
results_dodo <- read_csv("results/dodo_processed.csv") 

# make them comparable
results_dodo_cmp <- results_dodo %>%
      group_by(name, interpretation) %>%
      summarise(time_avg = mean(time), solved = sum(output == "success")) %>%
      rename(solved_dodo = solved)
# time in dodo measured in ms, in abstracton measured in s. Convert both to s.
results_dodo_cmp$time_avg <- results_dodo_cmp$time_avg / 1000
results$interpretation <- results$interpretation %>% sapply(function(s) str_to_lower(s) %>% str_sub(start=0, end=1))

results_joined <- inner_join(results, results_dodo_cmp, by = join_by(name, interpretation))



# compare results

# how many were solved?
results_joined %>%
  pivot_longer(cols = c(solved_num, solved_dodo), names_to = "column", values_to = "value") %>%
  ggplot() +
  geom_bar(mapping = aes(x = value, fill = column), position = "dodge")
ggsave("solved.png")

# how fast were they solved?
results_joined %>%
  pivot_longer(cols = c(time, time_avg), names_to = "column", values_to = "value") %>%
  ggplot() +
  stat_ecdf(mapping = aes(x = value, color = column))
ggsave("time.png")


# TODO why is there an entry here?
results_joined %>% filter(solved_num < solved_dodo)
