#include <stdio.h>
#include <string.h>

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
