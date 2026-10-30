#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_TEXT_SIZE 64
#define INITIAL_WORD_CAPACITY 10
#define MAX_WORD_LENGTH 39

struct wordStat {
  char name[40];
  int count;
};
typedef struct wordStat wordStat;

typedef struct {
  char source[120];
  char *text;
  int text_length;
  int text_capacity;
  wordStat *words;
  int word_count;
  int word_capacity;
  int total_words;
  int truncated_words;
} Analyzer;

typedef void (*AnalysisAction)(Analyzer *a);

// Allocates the text buffer and the word array
int init_analyzer(Analyzer *a) {
  a->text = malloc(INITIAL_TEXT_SIZE);
  if (a->text == NULL) {
    return 0;
  }
  a->text[0] = '\0';
  a->text_length = 0;
  a->text_capacity = INITIAL_TEXT_SIZE;

  a->words = malloc(INITIAL_WORD_CAPACITY * sizeof(wordStat));
  if (a->words == NULL) {
    free(a->text);
    a->text = NULL;
    return 0;
  }
  a->word_count = 0;
  a->word_capacity = INITIAL_WORD_CAPACITY;
  a->total_words = 0;
  a->truncated_words = 0;
  strcpy(a->source, "No text loaded");
  return 1;
}

// Frees all the memory
void free_analyzer(Analyzer *a) {
  free(a->text);
  free(a->words);
  a->text = NULL;
  a->words = NULL;
  a->text_length = 0;
  a->text_capacity = 0;
  a->word_count = 0;
  a->word_capacity = 0;
  a->total_words = 0;
}

// Returns the entry of a word, or NULL if it is not there
wordStat *find_word(const Analyzer *a, const char *word) {
  wordStat *end = a->words + a->word_count;
  for (wordStat *w = a->words; w < end; w++) {
    if (strcmp(w->name, word) == 0) {
      return w;
    }
  }
  return NULL;
}

// Adds one to the word count, or adds the word if it is new
int add_word(Analyzer *a, const char *word) {
  wordStat *entry = find_word(a, word);
  if (entry != NULL) {
    entry->count++;
    a->total_words++;
    return 1;
  }

  if (a->word_count == a->word_capacity) {
    int new_capacity = a->word_capacity * 2;
    wordStat *bigger = realloc(a->words, new_capacity * sizeof(wordStat));
    if (bigger == NULL) {
      return 0;
    }
    a->words = bigger;
    a->word_capacity = new_capacity;
  }

  entry = a->words + a->word_count;
  strcpy(entry->name, word);
  entry->count = 1;
  a->word_count++;
  a->total_words++;
  return 1;
}

// Reads a line into the buffer
void read_line(const char *prompt, char *buffer, int size) {
  printf("%s", prompt);
  if (fgets(buffer, size, stdin) == NULL) {
    buffer[0] = '\0';
    return;
  }
  char *newline = strchr(buffer, '\n');
  if (newline != NULL) {
    *newline = '\0';
  } else {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
    }
  }
}

// Reads a number, asks again if it is not a number
int read_int(const char *prompt) {
  char line[64];
  int value;
  while (1) {
    read_line(prompt, line, sizeof(line));
    if (sscanf(line, "%d", &value) == 1) {
      return value;
    }
    if (feof(stdin)) {
      return -1;
    }
    printf("Invalid number, try again.\n");
  }
}

// Checks if the character is part of a word
int is_word_char(const char *p, const char *start) {
  if (isalnum((unsigned char)*p)) {
    return 1;
  }
  if (*p == '\'' || *p == '-') {
    if (p > start && isalnum((unsigned char)*(p - 1)) &&
        isalnum((unsigned char)*(p + 1))) {
      return 1;
    }
  }
  return 0;
}

// Goes through the text with a pointer and counts every word
int build_stats(Analyzer *a) {
  a->word_count = 0;
  a->total_words = 0;
  a->truncated_words = 0;

  char word[MAX_WORD_LENGTH + 1];
  char *p = a->text;
  while (*p != '\0') {
    if (!is_word_char(p, a->text)) {
      p++;
      continue;
    }

    int length = 0;
    int too_long = 0;
    while (*p != '\0' && is_word_char(p, a->text)) {
      if (length < MAX_WORD_LENGTH) {
        word[length] = tolower((unsigned char)*p);
        length++;
      } else {
        too_long = 1;
      }
      p++;
    }
    word[length] = '\0';

    if (too_long) {
      a->truncated_words++;
    }
    if (!add_word(a, word)) {
      return 0;
    }
  }
  return 1;
}

// Replaces the old text with a new buffer and counts its words
int install_text(Analyzer *a, char *new_text, int length, int capacity,
                 const char *source) {
  free(a->text);
  a->text = new_text;
  a->text_length = length;
  a->text_capacity = capacity;
  snprintf(a->source, sizeof(a->source), "%s", source);

  if (!build_stats(a)) {
    printf("Error: out of memory while counting words.\n");
    return 0;
  }
  printf("Loaded %d characters, %d words (%d distinct).\n", a->text_length,
         a->total_words, a->word_count);
  if (a->total_words == 0) {
    printf("Warning: no words found in that text.\n");
  }
  return 1;
}

// Copies a fixed string into a new buffer and loads it
int set_text(Analyzer *a, const char *new_text, const char *source) {
  int length = strlen(new_text);
  if (length == 0) {
    printf("Error: text is empty.\n");
    return 0;
  }
  char *copy = malloc(length + 1);
  if (copy == NULL) {
    printf("Error: out of memory.\n");
    return 0;
  }
  strcpy(copy, new_text);
  return install_text(a, copy, length, length + 1, source);
}

// Reads lines until an empty line, the buffer grows when it is full
int read_paragraph(Analyzer *a) {
  int capacity = INITIAL_TEXT_SIZE;
  int length = 0;
  char *buffer = malloc(capacity);
  if (buffer == NULL) {
    printf("Error: out of memory.\n");
    return 0;
  }
  buffer[0] = '\0';

  printf("Paste your paragraph, then press Enter on an empty line:\n");
  char line[256];
  while (fgets(line, sizeof(line), stdin) != NULL) {
    if (line[0] == '\n') {
      break;
    }

    char *newline = strchr(line, '\n');
    if (newline != NULL) {
      *newline = ' ';
    }
    int line_length = strlen(line);

    if (length + line_length + 1 > capacity) {
      int new_capacity = capacity;
      while (length + line_length + 1 > new_capacity) {
        new_capacity = new_capacity * 2;
      }
      char *bigger = realloc(buffer, new_capacity);
      if (bigger == NULL) {
        printf("Error: out of memory.\n");
        free(buffer);
        return 0;
      }
      buffer = bigger;
      capacity = new_capacity;
      printf("(text buffer grew to %d bytes)\n", capacity);
    }

    strcpy(buffer + length, line);
    length = length + line_length;
  }

  if (length == 0) {
    printf("Error: empty input, keeping the old text.\n");
    free(buffer);
    return 0;
  }
  return install_text(a, buffer, length, capacity, "Paragraph entered by user");
}

#define SOURCE_TITLE "The Gettysburg Address"
#define SOURCE_AUTHOR "Abraham Lincoln"
#define SOURCE_DETAILS "Gettysburg, Pennsylvania, 19 November 1863"

static const char *default_text =
    "Four score and seven years ago our fathers brought forth on this "
    "continent, a new nation, conceived in Liberty, and dedicated to the "
    "proposition that all men are created equal. Now we are engaged in a "
    "great civil war, testing whether that nation, or any nation so conceived "
    "and so dedicated, can long endure. We are met on a great battle-field of "
    "that war. We have come to dedicate a portion of that field, as a final "
    "resting place for those who here gave their lives that that nation might "
    "live. It is altogether fitting and proper that we should do this. But, "
    "in a larger sense, we can not dedicate -- we can not consecrate -- we "
    "can not hallow -- this ground. The brave men, living and dead, who "
    "struggled here, have consecrated it, far above our poor power to add or "
    "detract. The world will little note, nor long remember what we say here, "
    "but it can never forget what they did here. It is for us the living, "
    "rather, to be dedicated here to the unfinished work which they who "
    "fought here have thus far so nobly advanced. It is rather for us to be "
    "here dedicated to the great task remaining before us -- that from these "
    "honored dead we take increased devotion to that cause for which they "
    "gave the last full measure of devotion -- that we here highly resolve "
    "that these dead shall not have died in vain -- that this nation, under "
    "God, shall have a new birth of freedom -- and that government of the "
    "people, by the people, for the people, shall not perish from the earth.";

// Loads the Gettysburg Address
int load_default_text(Analyzer *a) {
  char source[120];
  snprintf(source, sizeof(source), "%s by %s (%s)", SOURCE_TITLE,
           SOURCE_AUTHOR, SOURCE_DETAILS);
  return set_text(a, default_text, source);
}

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

typedef struct {
  const char *label;
  AnalysisAction run;
} MenuItem;

// Menu option 1
void load_default(Analyzer *a) {
  printf("\n--- Load Default Paragraph ---\n");
  load_default_text(a);
}

// Menu option 2
void enter_paragraph(Analyzer *a) {
  printf("\n--- Enter Paragraph ---\n");
  read_paragraph(a);
}

// Menu option 12
void exit_program(Analyzer *a) {
  (void)a;
  printf("Goodbye!\n");
}

static const MenuItem menu[] = {
    {"Load default paragraph", load_default},
    {"Enter or paste a paragraph", enter_paragraph},
    {"Count words", count_words},
    {"Longest word", longest_word},
    {"Most frequent word", most_frequent},
    {"Display word statistics", display_word_statistics},
    {"Sort word statistics", sort_word_statistics},
    {"Search for a word", search_word},
    {"Text summary", generate_text_summary},
    {"Custom analysis (Lexical Diversity Score)", custom_analysis},
    {"Memory inspection", memory_inspection},
    {"Exit", exit_program},
};
#define MENU_SIZE ((int)(sizeof(menu) / sizeof(menu[0])))

// Prints the menu
void display_menu(void) {
  printf("\n=== Menu ===\n");
  for (int i = 0; i < MENU_SIZE; i++) {
    printf("%d. %s\n", i + 1, menu[i].label);
  }
}

int main(void) {
  Analyzer analyzer;
  if (!init_analyzer(&analyzer)) {
    printf("Error: could not allocate memory.\n");
    return 1;
  }

  printf("Student ID: 101\n");
  printf("Project: Dynamic Text Mining and Linguistic Pattern Analyzer\n");
  printf("Source: %s by %s (%s)\n", SOURCE_TITLE, SOURCE_AUTHOR,
         SOURCE_DETAILS);
  printf("Custom Analysis: Lexical Diversity Score\n\n");

  if (!load_default_text(&analyzer)) {
    free_analyzer(&analyzer);
    return 1;
  }

  int choice = 0;
  while (choice != MENU_SIZE) {
    display_menu();
    choice = read_int("Choose an option: ");
    if (feof(stdin)) {
      break;
    }
    if (choice < 1 || choice > MENU_SIZE) {
      printf("Invalid option.\n");
      continue;
    }
    menu[choice - 1].run(&analyzer);
  }

  free_analyzer(&analyzer);
  return 0;
}
