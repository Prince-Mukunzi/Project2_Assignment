#include <stdio.h>
#include <string.h>
#include <strings.h>

static void print_action(Student *s, void *extra) {
  (void)extra;
  print_student(s);
}

// Add up grades
static void add_grade_action(Student *s, void *extra) {
  float *total = extra;
  *total += s->grade;
}

// Track highest grade
static void track_top_action(Student *s, void *extra) {
  Student **top = extra;
  if (*top == NULL || s->grade > (*top)->grade) {
    *top = s;
  }
}

// Track lowest grade
static void track_lowest_action(Student *s, void *extra) {
  Student **lowest = extra;
  if (*lowest == NULL || s->grade < (*lowest)->grade) {
    *lowest = s;
  }
}

void add_student(StudentList *list) {
  printf("\n--- Add Student ---\n");

  Student s;
  s.id = read_int("Enter Student ID: ");
  if (find_by_id(list, s.id) != NULL) {
    printf("Error: that ID already exists.\n");
    return;
  }
  read_line("Enter Student Name: ", s.name, sizeof(s.name));
  read_line("Enter Course: ", s.course, sizeof(s.course));
  s.grade = read_grade("Enter Grade (0-100): ");

  if (add_to_list(list, s)) {
    printf("Student added successfully!\n");
  } else {
    printf("Error: out of memory.\n");
  }
}

void display_students(StudentList *list) {
  printf("\n--- Display Students ---\n");
  if (list->count == 0) {
    printf("No students found.\n");
    return;
  }
  apply_to_all(list, print_action, NULL);
}

// Comparator (function pointer)
typedef int (*Comparator)(const Student *a, const Student *b);

static int compare_grade_desc(const Student *a, const Student *b) {
  return (a->grade < b->grade) - (a->grade > b->grade);
}

static int compare_name(const Student *a, const Student *b) {
  return strcasecmp(a->name, b->name);
}

typedef struct {
  const char *label;
  Comparator compare;
} SortOption;

static int compare_id(const Student *a, const Student *b) {
  return a->id - b->id;
}

static const SortOption sort_options[] = {
    {"By grade (highest first)", compare_grade_desc},
    {"By name (A-Z)", compare_name},
    {"By student ID", compare_id},
};
#define SORT_OPTION_COUNT (sizeof(sort_options) / sizeof(sort_options[0]))

static void swap_students(Student *a, Student *b) {
  Student temp = *a;
  *a = *b;
  *b = temp;
}

// Bubble sort
static void bubble_sort(StudentList *list, Comparator compare) {
  for (int i = 0; i < list->count - 1; i++) {
    for (int j = 0; j < list->count - 1 - i; j++) {
      if (compare(&list->items[j], &list->items[j + 1]) > 0) {
        swap_students(&list->items[j], &list->items[j + 1]);
      }
    }
  }
}

void sort_students(StudentList *list) {
  printf("\n--- Sort Students ---\n");
  if (list->count < 2) {
    printf("Not enough students to sort.\n");
    return;
  }
  for (size_t i = 0; i < SORT_OPTION_COUNT; i++) {
    printf("%zu. %s\n", i + 1, sort_options[i].label);
  }
  int choice = read_int("Choose: ");
  if (choice < 1 || choice > (int)SORT_OPTION_COUNT) {
    printf("Invalid choice.\n");
    return;
  }

  bubble_sort(list, sort_options[choice - 1].compare);
  printf("Students sorted.\n");
  display_students(list);
}

float average_grade(StudentList *list) {
  float total = 0;
  apply_to_all(list, add_grade_action, &total);
  return total / list->count;
}

void show_average(StudentList *list) {
  printf("\n--- Average Grade ---\n");
  if (list->count == 0) {
    printf("No students found.\n");
    return;
  }
  printf("Average grade: %.2f\n", average_grade(list));
}

Student *find_top_student(StudentList *list) {
  Student *top = NULL;
  apply_to_all(list, track_top_action, &top);
  return top;
}

Student *find_lowest_student(StudentList *list) {
  Student *lowest = NULL;
  apply_to_all(list, track_lowest_action, &lowest);
  return lowest;
}

void show_top_student(StudentList *list) {
  printf("\n--- Top Performing Student ---\n");
  Student *top = find_top_student(list);
  if (top == NULL) {
    printf("No students found.\n");
    return;
  }
  print_student(top);
}

static void search_by_id(StudentList *list) {
  Student *found = find_by_id(list, read_int("Enter ID: "));
  if (found == NULL) {
    printf("No student with that ID.\n");
    return;
  }
  print_student(found);
}

static void search_by_name(StudentList *list) {
  char name[100];
  read_line("Enter name (full or partial): ", name, sizeof(name));
  int matches = 0;
  for (int i = 0; i < list->count; i++) {
    if (strcasestr(list->items[i].name, name) != NULL) {
      print_student(&list->items[i]);
      matches++;
    }
  }
  if (matches == 0) {
    printf("No student matches that name.\n");
  }
}

typedef struct {
  const char *label;
  void (*run)(StudentList *list);
} SearchOption;

static const SearchOption search_options[] = {
    {"By ID", search_by_id},
    {"By Name", search_by_name},
};
#define SEARCH_OPTION_COUNT (sizeof(search_options) / sizeof(search_options[0]))

void search_student(StudentList *list) {
  printf("\n--- Search Student ---\n");
  for (size_t i = 0; i < SEARCH_OPTION_COUNT; i++) {
    printf("%zu. %s\n", i + 1, search_options[i].label);
  }
  int choice = read_int("Choose: ");
  if (choice < 1 || choice > (int)SEARCH_OPTION_COUNT) {
    printf("Invalid choice.\n");
    return;
  }
  search_options[choice - 1].run(list);
}

void remove_student(StudentList *list) {
  printf("\n--- Remove Student ---\n");
  int id = read_int("Enter ID to remove: ");
  Student *target = find_by_id(list, id);
  if (target == NULL) {
    printf("No student with that ID.\n");
    return;
  }

  // Pointer subtraction gives the index
  int index = target - list->items;

  // Shift students left
  for (int i = index; i < list->count - 1; i++) {
    list->items[i] = list->items[i + 1];
  }
  list->count--;

  // Shrink the allocated memory
  Student *smaller = realloc(list->items, list->count * sizeof(Student));
  if (smaller != NULL) {
    list->items = smaller;
    list->capacity = list->count;
  }

  printf("Student removed.\n");
}
