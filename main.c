#include "student.c"
#include "input.c"
#include "operations.c"
#include "analysis.c"
#include "mock_data.c"
#include <stdio.h>

// Menu action (function pointer)
typedef void (*MenuAction)(StudentList *list);

typedef struct {
  const char *label;
  MenuAction run;
} MenuItem;

static void exit_program(StudentList *list) {
  (void)list;
  printf("Goodbye!\n");
}

// Menu
static const MenuItem menu[] = {
    {"Add student", add_student},
    {"Display students", display_students},
    {"Sort students", sort_students},
    {"Calculate Average Grade", show_average},
    {"Top Performing Student", show_top_student},
    {"Search Student by ID or Name", search_student},
    {"Remove Student Record", remove_student},
    {"Custom Analysis", custom_analysis},
    {"Summary Report", summary_report},
    {"Exit", exit_program},
};
#define MENU_SIZE ((int)(sizeof(menu) / sizeof(menu[0])))

void display_menu(void) {
  printf("\n=== Menu ===\n");
  for (int i = 0; i < MENU_SIZE; i++) {
    printf("%d. %s\n", i + 1, menu[i].label);
  }
}

int main(void) {
  StudentList list;
  if (!init_list(&list)) {
    printf("Error: could not allocate memory.\n");
    return 1;
  }
  load_mock_data(&list);

  // Output header
  printf("Student ID: [your_student_id]\n");
  printf("Project: Dynamic Academic Performance and Risk Analysis Engine\n");
  printf("Custom Analysis: Academic Risk Index\n");

  int choice = 0;
  while (choice != MENU_SIZE) {
    display_menu();
    choice = read_int("Choose an option: ");
    if (choice < 1 || choice > MENU_SIZE) {
      printf("Invalid option.\n");
      continue;
    }
    menu[choice - 1].run(&list);
  }

  free_list(&list);
  return 0;
}
