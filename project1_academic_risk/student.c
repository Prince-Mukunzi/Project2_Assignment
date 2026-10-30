#include <stdio.h>
#include <stdlib.h>
#define INITIAL_CAPACITY 5

typedef struct {
  int id;
  char name[100];
  char course[100];
  float grade;
} Student;

// Dynamic array of students
typedef struct {
  Student *items;
  int count;
  int capacity;
} StudentList;

// Function pointer type (extra holds data for the function)
typedef void (*StudentAction)(Student *s, void *extra);

int init_list(StudentList *list) {
  // Allocate memory
  list->items = malloc(INITIAL_CAPACITY * sizeof(Student));
  if (list->items == NULL) {
    return 0;
  }
  list->count = 0;
  list->capacity = INITIAL_CAPACITY;
  return 1;
}

void free_list(StudentList *list) {
  free(list->items);
  list->items = NULL;
  list->count = 0;
  list->capacity = 0;
}

int add_to_list(StudentList *list, Student s) {
  // Grow array when full
  if (list->count == list->capacity) {
    int new_capacity = list->capacity * 2;
    Student *bigger = realloc(list->items, new_capacity * sizeof(Student));
    if (bigger == NULL) {
      return 0;
    }
    list->items = bigger;
    list->capacity = new_capacity;
  }
  list->items[list->count] = s;
  list->count++;
  return 1;
}

// Find student by ID
Student *find_by_id(const StudentList *list, int id) {
  for (int i = 0; i < list->count; i++) {
    if (list->items[i].id == id) {
      return &list->items[i];
    }
  }
  return NULL;
}

// Apply function to every student
void apply_to_all(StudentList *list, StudentAction action, void *extra) {
  for (int i = 0; i < list->count; i++) {
    action(&list->items[i], extra);
  }
}

void print_student(const Student *s) {
  printf("ID: %d, Name: %s, Course: %s, Grade: %.2f\n", s->id, s->name,
         s->course, s->grade);
}
