#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
