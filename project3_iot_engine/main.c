#include "device.c"
#include "input.c"
#include "callbacks.c"
#include "operations.c"
#include "alerts.c"
#include "simulation.c"
#include "mock_data.c"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef void (*MenuAction)(Monitor *m);

typedef struct {
  const char *label;
  MenuAction run;
} MenuItem;

// Menu option 8
void exit_program(Monitor *m) {
  (void)m;
  printf("Goodbye!\n");
}

static const MenuItem menu[] = {
    {"Register a new device", register_device},
    {"Display all devices", display_devices},
    {"Remove a device", remove_device},
    {"Generate simulated readings", generate_readings},
    {"Process readings through callbacks", process_readings},
    {"Display detected alerts", display_alerts},
    {"Monitoring summary", monitoring_summary},
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
  Monitor monitor;
  if (!init_monitor(&monitor)) {
    printf("Error: could not allocate memory.\n");
    return 1;
  }
  load_mock_data(&monitor);
  srand((unsigned int)time(NULL));

  printf("Student ID: 101\n");
  printf("Project: Dynamic IoT Event Monitoring and Callback Engine\n");
  printf("Custom Callback: Sensor Drift Detector\n");

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
    menu[choice - 1].run(&monitor);
  }

  free_monitor(&monitor);
  return 0;
}
