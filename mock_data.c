#include <string.h>

#define MOCK_COUNT 12

void load_mock_data(StudentList *list) {
  Student mock[MOCK_COUNT] = {
      {101, "Alice Uwase", "Computer Science", 88.5f},
      {102, "Brian Habimana", "Mathematics", 72.0f},
      {103, "Chloe Mukamana", "Physics", 45.5f},
      {104, "David Nkurunziza", "Computer Science", 91.0f},
      {105, "Eva Ingabire", "Biology", 58.0f},
      {106, "Frank Mugisha", "Mathematics", 63.5f},
      {107, "Grace Umutoni", "Physics", 49.0f},
      {108, "Henry Niyonzima", "Biology", 76.5f},
      {109, "Irene Kayitesi", "Computer Science", 83.0f},
      {110, "Jean Bosco", "Mathematics", 55.0f},
      {111, "Kevin Rugamba", "Physics", 67.0f},
      {112, "Linda Mutesi", "Biology", 94.5f},
  };

  for (int i = 0; i < MOCK_COUNT; i++) {
    add_to_list(list, mock[i]);
  }
}
