#include <ctype.h>
#include <stdio.h>
#include <string.h>

// Checks that there is text to work with
int has_words(Analyzer *a) {
  if (a->total_words == 0) {
    printf("No text loaded.\n");
    return 0;
  }
  return 1;
}

// Finds the longest word, a tie goes to the first alphabetically
wordStat *find_longest(Analyzer *a) {
  wordStat *longest = a->words;
  wordStat *end = a->words + a->word_count;
  for (wordStat *w = a->words; w < end; w++) {
    int length = strlen(w->name);
    int best = strlen(longest->name);
    if (length > best) {
      longest = w;
    } else if (length == best && strcmp(w->name, longest->name) < 0) {
      longest = w;
    }
  }
  return longest;
}

// Finds the most frequent word, a tie goes to the first alphabetically
wordStat *find_most_frequent(Analyzer *a) {
  wordStat *top = a->words;
  wordStat *end = a->words + a->word_count;
  for (wordStat *w = a->words; w < end; w++) {
    if (w->count > top->count) {
      top = w;
    } else if (w->count == top->count && strcmp(w->name, top->name) < 0) {
      top = w;
    }
  }
  return top;
}

// Shows the total and distinct word counts
void count_words(Analyzer *a) {
  printf("\n--- Count Words ---\n");
  if (!has_words(a)) {
    return;
  }
  printf("Total words: %d\n", a->total_words);
  printf("Distinct words: %d\n", a->word_count);
  if (a->truncated_words > 0) {
    printf("Words cut to %d characters: %d\n", MAX_WORD_LENGTH,
           a->truncated_words);
  }
}

// Shows the longest word and its length
void longest_word(Analyzer *a) {
  printf("\n--- Longest Word ---\n");
  if (!has_words(a)) {
    return;
  }
  wordStat *longest = find_longest(a);
  printf("Longest word: %s\n", longest->name);
  printf("Length: %d characters\n", (int)strlen(longest->name));
}

// Shows the most frequent word
void most_frequent(Analyzer *a) {
  printf("\n--- Most Frequent Word ---\n");
  if (!has_words(a)) {
    return;
  }
  wordStat *top = find_most_frequent(a);
  printf("Most frequent word: %s\n", top->name);
  printf("Appears %d times\n", top->count);
}

// Shows every distinct word with its count
void display_word_statistics(Analyzer *a) {
  printf("\n--- Word Statistics ---\n");
  if (!has_words(a)) {
    return;
  }
  printf("%-5s %-40s %s\n", "No.", "Word", "Count");
  int number = 1;
  wordStat *end = a->words + a->word_count;
  for (wordStat *w = a->words; w < end; w++) {
    printf("%-5d %-40s %d\n", number, w->name, w->count);
    number++;
  }
}

typedef int (*Comparator)(const wordStat *a, const wordStat *b);

// Compares two words alphabetically
int compare_alphabetical(const wordStat *a, const wordStat *b) {
  return strcmp(a->name, b->name);
}

// Compares two words by count, highest first
int compare_frequency(const wordStat *a, const wordStat *b) {
  if (a->count != b->count) {
    return b->count - a->count;
  }
  return strcmp(a->name, b->name);
}

typedef struct {
  const char *label;
  Comparator compare;
} SortOption;

static const SortOption sort_options[] = {
    {"Alphabetically (A-Z)", compare_alphabetical},
    {"By frequency (highest first)", compare_frequency},
};
#define SORT_OPTION_COUNT ((int)(sizeof(sort_options) / sizeof(sort_options[0])))

// Swaps two entries
void swap_words(wordStat *a, wordStat *b) {
  wordStat temp = *a;
  *a = *b;
  *b = temp;
}

// Bubble sort using the comparator it is given
void bubble_sort(Analyzer *a, Comparator compare) {
  for (int i = 0; i < a->word_count - 1; i++) {
    for (int j = 0; j < a->word_count - 1 - i; j++) {
      if (compare(&a->words[j], &a->words[j + 1]) > 0) {
        swap_words(&a->words[j], &a->words[j + 1]);
      }
    }
  }
}

// Sorts the words alphabetically or by frequency
void sort_word_statistics(Analyzer *a) {
  printf("\n--- Sort Word Statistics ---\n");
  if (!has_words(a)) {
    return;
  }
  for (int i = 0; i < SORT_OPTION_COUNT; i++) {
    printf("%d. %s\n", i + 1, sort_options[i].label);
  }
  int choice = read_int("Choose: ");
  if (choice < 1 || choice > SORT_OPTION_COUNT) {
    printf("Invalid choice.\n");
    return;
  }

  bubble_sort(a, sort_options[choice - 1].compare);
  printf("Words sorted.\n");
  display_word_statistics(a);
}

// Searches for a word and shows how many times it appears
void search_word(Analyzer *a) {
  printf("\n--- Search Word ---\n");
  if (!has_words(a)) {
    return;
  }
  char word[100];
  read_line("Enter a word: ", word, sizeof(word));

  for (char *p = word; *p != '\0'; p++) {
    *p = tolower((unsigned char)*p);
  }
  if (strlen(word) > MAX_WORD_LENGTH) {
    word[MAX_WORD_LENGTH] = '\0';
  }
  if (word[0] == '\0') {
    printf("Error: empty search.\n");
    return;
  }

  wordStat *found = find_word(a, word);
  if (found == NULL) {
    printf("\"%s\" does not appear in the text.\n", word);
    return;
  }
  printf("\"%s\" appears %d time(s).\n", found->name, found->count);
  printf("Entry address: %p\n", (void *)found);
}
