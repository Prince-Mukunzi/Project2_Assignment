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
