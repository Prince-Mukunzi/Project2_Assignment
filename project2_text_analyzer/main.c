#include "analyzer.c"
#include "input.c"
#include "text.c"
#include "default_text.c"
#include "operations.c"
#include "analysis.c"
#include <stdio.h>

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
