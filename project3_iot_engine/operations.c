#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Registers a new device
void register_device(Monitor *m) {
  printf("\n--- Register Device ---\n");

  Device d;
  memset(&d, 0, sizeof(Device));
  d.id = read_int("Enter Device ID: ");
  if (find_device(m, d.id) != NULL) {
    printf("Error: that ID already exists.\n");
    return;
  }
  read_line("Enter Device Name: ", d.name, sizeof(d.name));
  if (d.name[0] == '\0') {
    printf("Error: name cannot be empty.\n");
    return;
  }

  for (int i = 0; i < TYPE_COUNT; i++) {
    printf("%d. %s\n", i + 1, device_types[i].label);
  }
  int choice = read_int("Choose device type: ");
  if (choice < 1 || choice > TYPE_COUNT) {
    printf("Invalid choice.\n");
    return;
  }
  d.type = choice - 1;
  d.status = STATUS_NORMAL;

  if (add_device(m, d)) {
    printf("Device registered successfully!\n");
  } else {
    printf("Error: out of memory.\n");
  }
}

// Shows all the registered devices
void display_devices(Monitor *m) {
  printf("\n--- Registered Devices ---\n");
  if (m->device_count == 0) {
    printf("No devices registered.\n");
    return;
  }
  printf("%-5s %-24s %-25s %-12s %s\n", "ID", "Name", "Type", "Reading",
         "Status");

  Device *end = m->devices + m->device_count;
  for (Device *d = m->devices; d < end; d++) {
    char value[30] = "none yet";
    if (d->reading_count > 0 || d->has_reading) {
      format_value(d, value, sizeof(value));
    }
    char status[10] = "Normal";
    if (d->status == STATUS_ALERT) {
      strcpy(status, "ALERT");
    }
    printf("%-5d %-24s %-25s %-12s %s\n", d->id, d->name,
           device_types[d->type].label, value, status);
  }
}

// Removes a device by its ID and shrinks the array
void remove_device(Monitor *m) {
  printf("\n--- Remove Device ---\n");
  int id = read_int("Enter Device ID to remove: ");
  Device *target = find_device(m, id);
  if (target == NULL) {
    printf("No device with that ID.\n");
    return;
  }

  int index = target - m->devices;
  for (int i = index; i < m->device_count - 1; i++) {
    m->devices[i] = m->devices[i + 1];
  }
  m->device_count--;

  int new_capacity = m->device_count;
  if (new_capacity < 1) {
    new_capacity = 1;
  }
  Device *smaller = realloc(m->devices, new_capacity * sizeof(Device));
  if (smaller != NULL) {
    m->devices = smaller;
    m->device_capacity = new_capacity;
  }

  printf("Device removed.\n");
}
