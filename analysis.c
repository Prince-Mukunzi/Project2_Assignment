#include <stdio.h>
#define HIGH_RISK_BELOW 50.0f
#define MODERATE_RISK_BELOW 60.0f

typedef struct {
  float below;
  const char *label;
} RiskBand;

// Risk bands
static const RiskBand bands[] = {
    {HIGH_RISK_BELOW, "High risk"},
    {MODERATE_RISK_BELOW, "Moderate risk"},
    {101.0f, "On track"},
};
#define BAND_COUNT (sizeof(bands) / sizeof(bands[0]))

static int band_index(float grade) {
  int i = 0;
  while (grade >= bands[i].below) {
    i++;
  }
  return i;
}

// Count students per band
static void count_band_action(Student *s, void *extra) {
  int *counts = extra;
  counts[band_index(s->grade)]++;
}

// Print students at risk
static void report_at_risk_action(Student *s, void *extra) {
  int *at_risk = extra;
  int band = band_index(s->grade);
  if (band < (int)BAND_COUNT - 1) {
    printf("[%s] ", bands[band].label);
    print_student(s);
    (*at_risk)++;
  }
}

void custom_analysis(StudentList *list) {
  printf("\n--- Custom Analysis: Risk Report ---\n");
  if (list->count == 0) {
    printf("No students found.\n");
    return;
  }
  printf("High risk: below %.0f | Moderate risk: below %.0f\n\n",
         HIGH_RISK_BELOW, MODERATE_RISK_BELOW);

  int at_risk = 0;
  apply_to_all(list, report_at_risk_action, &at_risk);
  if (at_risk == 0) {
    printf("No students are at risk.\n");
  }
}

void summary_report(StudentList *list) {
  printf("\n--- Summary Report ---\n");
  if (list->count == 0) {
    printf("No students found.\n");
    return;
  }

  Student *top = find_top_student(list);
  Student *lowest = find_lowest_student(list);

  int counts[BAND_COUNT] = {0};
  apply_to_all(list, count_band_action, counts);

  printf("Total students: %d\n", list->count);
  printf("Average grade:  %.2f\n", average_grade(list));
  printf("Highest grade:  %.2f (%s)\n", top->grade, top->name);
  printf("Lowest grade:   %.2f (%s)\n", lowest->grade, lowest->name);
  for (size_t i = 0; i < BAND_COUNT; i++) {
    printf("%-14s  %d\n", bands[i].label, counts[i]);
  }
}
