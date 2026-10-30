#include <stdio.h>
#include <string.h>
#define LOW_DIVERSITY_BELOW 0.40
#define HIGH_DIVERSITY_FROM 0.60

// Distinct words divided by total words
float lexical_diversity(Analyzer *a) {
  return (float)a->word_count / a->total_words;
}

// Turns the score into a short description
const char *diversity_label(float score) {
  if (score < LOW_DIVERSITY_BELOW) {
    return "Low (many repeated words)";
  }
  if (score < HIGH_DIVERSITY_FROM) {
    return "Moderate";
  }
  return "High (varied vocabulary)";
}

// Adds up all the letters and divides by the total words
float average_word_length(Analyzer *a) {
  int letters = 0;
  wordStat *end = a->words + a->word_count;
  for (wordStat *w = a->words; w < end; w++) {
    letters = letters + strlen(w->name) * w->count;
  }
  return (float)letters / a->total_words;
}

// Custom analysis: Lexical Diversity Score
void custom_analysis(Analyzer *a) {
  printf("\n--- Custom Analysis: Lexical Diversity Score ---\n");
  if (a->total_words == 0) {
    printf("No text loaded.\n");
    return;
  }
  printf("Purpose: shows how varied the vocabulary of the text is.\n");
  printf("Formula: distinct words / total words\n");
  printf("Low: below %.2f | Moderate: below %.2f | High: %.2f and above\n\n",
         LOW_DIVERSITY_BELOW, HIGH_DIVERSITY_FROM, HIGH_DIVERSITY_FROM);

  float score = lexical_diversity(a);
  printf("Distinct words: %d\n", a->word_count);
  printf("Total words: %d\n", a->total_words);
  printf("Lexical Diversity Score: %.3f\n", score);
  printf("Interpretation: %s\n", diversity_label(score));
}

// Shows the overall statistics of the text
void generate_text_summary(Analyzer *a) {
  printf("\n--- Text Summary ---\n");
  if (a->total_words == 0) {
    printf("No text loaded.\n");
    return;
  }
  wordStat *longest = find_longest(a);
  wordStat *top = find_most_frequent(a);
  float score = lexical_diversity(a);

  printf("Source: %s\n", a->source);
  printf("Total words: %d\n", a->total_words);
  printf("Distinct words: %d\n", a->word_count);
  printf("Average word length: %.2f\n", average_word_length(a));
  printf("Longest word: %s (%d characters)\n", longest->name,
         (int)strlen(longest->name));
  printf("Most frequent word: %s (%d times)\n", top->name, top->count);
  printf("Lexical Diversity Score: %.3f - %s\n", score, diversity_label(score));
}

// Prints the memory addresses of the main data
void memory_inspection(Analyzer *a) {
  printf("\n--- Memory Inspection ---\n");
  printf("--------------------------\n");
  if (a->total_words == 0) {
    printf("No text loaded.\n");
    return;
  }
  wordStat *selected = find_most_frequent(a);
  int index = selected - a->words;

  printf("Text buffer address: %p\n", (void *)a->text);
  printf("  Holds the paragraph (%d of %d bytes used)\n", a->text_length + 1,
         a->text_capacity);
  printf("Word statistics array: %p\n", (void *)a->words);
  printf("  Holds %d wordStat entries (room for %d, %d bytes each)\n",
         a->word_count, a->word_capacity, (int)sizeof(wordStat));
  printf("Selected word-stat entry: %p\n", (void *)selected);
  printf("  Entry for \"%s\", number %d in the array (array address + %d bytes)\n",
         selected->name, index + 1, index * (int)sizeof(wordStat));
}
