#include <stdio.h>
#include <string.h>

// Read a line into buffer
void read_line(const char *prompt, char *buffer, int size) {
  printf("%s", prompt);
  if (fgets(buffer, size, stdin) == NULL) {
    buffer[0] = '\0';
    return;
  }
  buffer[strcspn(buffer, "\n")] = '\0'; // Remove trailing newline
}

int read_int(const char *prompt) {
  char line[64];
  int value;
  while (1) {
    read_line(prompt, line, sizeof(line));
    if (sscanf(line, "%d", &value) == 1) {
      return value;
    }
    printf("Invalid number, try again.\n");
  }
}

float read_grade(const char *prompt) {
  char line[64];
  float value;
  while (1) {
    read_line(prompt, line, sizeof(line));
    if (sscanf(line, "%f", &value) == 1 && value >= 0 && value <= 100) {
      return value;
    }
    printf("Grade must be between 0 and 100.\n");
  }
}
