#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define INITIAL_CAPACITY 4
#define TYPE_COUNT 3
#define TYPE_TEMPERATURE 0
#define TYPE_PRESSURE 1
#define TYPE_BATTERY 2
#define STATUS_NORMAL 0
#define STATUS_ALERT 1

typedef struct Device {
  int id;
  char name[40];
  int type;
  union {
    float temperature;
    int pressure;
    float voltage;
  } reading;
  int status;
  int has_reading;
  int reading_count;
  float previous_value;
  float last_drift;
  char alert_reason[80];
  char alert_expected[40];
} Device;

typedef void (*Callback)(Device *device);

void temperature_monitor(Device *device);
void pressure_monitor(Device *device);
void battery_monitor(Device *device);

typedef struct {
  const char *label;
  const char *unit;
  float sim_min;
  float sim_max;
  float low;
  float high;
  float drift_limit;
  const char *callback_name;
  Callback callback;
} DeviceType;

static const DeviceType device_types[TYPE_COUNT] = {
    {"Temperature sensor", "C", 5.0f, 45.0f, 10.0f, 40.0f, 20.0f,
     "temperature_monitor", temperature_monitor},
    {"Pressure sensor", "hPa", 940.0f, 1060.0f, 960.0f, 1040.0f, 60.0f,
     "pressure_monitor", pressure_monitor},
    {"Battery voltage monitor", "V", 2.8f, 4.4f, 3.2f, 4.2f, 0.8f,
     "battery_monitor", battery_monitor},
};

typedef struct {
  int device_id;
  char device_name[40];
  int type;
  char value_text[30];
  char expected[40];
  char reason[80];
  const char *callback_name;
} Alert;

typedef struct {
  Device *devices;
  int device_count;
  int device_capacity;
  Alert *alerts;
  int alert_count;
  int alert_capacity;
  int total_readings;
  int normal_readings;
  int abnormal_readings;
  int alerts_per_type[TYPE_COUNT];
  int drift_checks;
  int drift_alerts;
  float max_drift_ratio;
  char max_drift_text[80];
} Monitor;

// Allocates the device array and the alert array
int init_monitor(Monitor *m) {
  memset(m, 0, sizeof(Monitor));

  m->devices = malloc(INITIAL_CAPACITY * sizeof(Device));
  if (m->devices == NULL) {
    return 0;
  }
  m->device_capacity = INITIAL_CAPACITY;

  m->alerts = malloc(INITIAL_CAPACITY * sizeof(Alert));
  if (m->alerts == NULL) {
    free(m->devices);
    m->devices = NULL;
    return 0;
  }
  m->alert_capacity = INITIAL_CAPACITY;
  strcpy(m->max_drift_text, "none yet");
  return 1;
}

// Frees all the memory
void free_monitor(Monitor *m) {
  free(m->devices);
  free(m->alerts);
  m->devices = NULL;
  m->alerts = NULL;
  m->device_count = 0;
  m->device_capacity = 0;
  m->alert_count = 0;
  m->alert_capacity = 0;
}

// Adds a device to the array, grows the array when it is full
int add_device(Monitor *m, Device d) {
  if (m->device_count == m->device_capacity) {
    int new_capacity = m->device_capacity * 2;
    Device *bigger = realloc(m->devices, new_capacity * sizeof(Device));
    if (bigger == NULL) {
      return 0;
    }
    m->devices = bigger;
    m->device_capacity = new_capacity;
  }
  m->devices[m->device_count] = d;
  m->device_count++;
  return 1;
}

// Returns the device with this ID, or NULL if there is none
Device *find_device(const Monitor *m, int id) {
  Device *end = m->devices + m->device_count;
  for (Device *d = m->devices; d < end; d++) {
    if (d->id == id) {
      return d;
    }
  }
  return NULL;
}

// Reads the union member that matches the device type
float get_value(const Device *d) {
  if (d->type == TYPE_TEMPERATURE) {
    return d->reading.temperature;
  }
  if (d->type == TYPE_PRESSURE) {
    return (float)d->reading.pressure;
  }
  return d->reading.voltage;
}

// Writes the union member that matches the device type
void set_reading(Device *d, float value) {
  if (d->type == TYPE_TEMPERATURE) {
    d->reading.temperature = value;
  } else if (d->type == TYPE_PRESSURE) {
    d->reading.pressure = (int)value;
  } else {
    d->reading.voltage = value;
  }
}

// Writes the reading and its unit as text, like 23.5 C
void format_value(const Device *d, char *out, int size) {
  const DeviceType *info = &device_types[d->type];
  if (d->type == TYPE_PRESSURE) {
    snprintf(out, size, "%d %s", d->reading.pressure, info->unit);
  } else {
    snprintf(out, size, "%.2f %s", get_value(d), info->unit);
  }
}

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

// Calls the callback it is given
void process_device(Device *device, void (*callback)(Device *)) {
  callback(device);
}

// Raises an alert if the value is outside the normal range
void check_range(Device *device, const DeviceType *info, float value,
                 const char *what) {
  if (value >= info->low && value <= info->high) {
    return;
  }
  device->status = STATUS_ALERT;
  if (value > info->high) {
    snprintf(device->alert_reason, sizeof(device->alert_reason),
             "%s above the safe range", what);
  } else {
    snprintf(device->alert_reason, sizeof(device->alert_reason),
             "%s below the safe range", what);
  }
  snprintf(device->alert_expected, sizeof(device->alert_expected),
           "%.1f to %.1f %s", info->low, info->high, info->unit);
}

// Checks a temperature reading
void temperature_monitor(Device *device) {
  if (device->type != TYPE_TEMPERATURE) {
    return;
  }
  check_range(device, &device_types[TYPE_TEMPERATURE],
              device->reading.temperature, "Temperature");
}

// Checks a pressure reading
void pressure_monitor(Device *device) {
  if (device->type != TYPE_PRESSURE) {
    return;
  }
  check_range(device, &device_types[TYPE_PRESSURE],
              (float)device->reading.pressure, "Pressure");
}

// Checks a battery voltage reading
void battery_monitor(Device *device) {
  if (device->type != TYPE_BATTERY) {
    return;
  }
  check_range(device, &device_types[TYPE_BATTERY], device->reading.voltage,
              "Battery voltage");
}

// Custom callback: compares the reading with the previous one and
// raises an alert if it changed more than the drift limit
void sensor_drift_monitor(Device *device) {
  device->last_drift = 0;
  if (device->reading_count < 2) {
    return;
  }

  const DeviceType *info = &device_types[device->type];
  float change = get_value(device) - device->previous_value;
  if (change < 0) {
    change = -change;
  }
  device->last_drift = change;

  if (change > info->drift_limit) {
    device->status = STATUS_ALERT;
    snprintf(device->alert_reason, sizeof(device->alert_reason),
             "Reading changed by %.1f %s since the last reading", change,
             info->unit);
    snprintf(device->alert_expected, sizeof(device->alert_expected),
             "change of at most %.1f %s", info->drift_limit, info->unit);
  }
}

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

// Saves an alert in the alert array, grows it when it is full
int log_alert(Monitor *m, const Device *d, const char *callback_name) {
  if (m->alert_count == m->alert_capacity) {
    int new_capacity = m->alert_capacity * 2;
    Alert *bigger = realloc(m->alerts, new_capacity * sizeof(Alert));
    if (bigger == NULL) {
      return 0;
    }
    m->alerts = bigger;
    m->alert_capacity = new_capacity;
  }

  Alert *alert = &m->alerts[m->alert_count];
  alert->device_id = d->id;
  strcpy(alert->device_name, d->name);
  alert->type = d->type;
  format_value(d, alert->value_text, sizeof(alert->value_text));
  strcpy(alert->expected, d->alert_expected);
  strcpy(alert->reason, d->alert_reason);
  alert->callback_name = callback_name;

  m->alert_count++;
  m->alerts_per_type[d->type]++;
  return 1;
}

// Shows all the alerts
void display_alerts(Monitor *m) {
  printf("\n--- Detected Alerts ---\n");
  if (m->alert_count == 0) {
    printf("No alerts detected.\n");
    return;
  }
  Alert *end = m->alerts + m->alert_count;
  int number = 1;
  for (Alert *a = m->alerts; a < end; a++) {
    printf("Alert %d\n", number);
    printf("  Device:   %d (%s)\n", a->device_id, a->device_name);
    printf("  Reading:  %s\n", a->value_text);
    printf("  Expected: %s\n", a->expected);
    printf("  Reason:   %s\n", a->reason);
    printf("  Callback: %s\n", a->callback_name);
    number++;
  }
}

// Shows the totals and the custom callback results
void monitoring_summary(Monitor *m) {
  printf("\n--- Monitoring Summary ---\n");
  printf("Registered devices: %d\n", m->device_count);
  printf("Readings processed: %d\n", m->total_readings);
  printf("Normal readings:    %d\n", m->normal_readings);
  printf("Abnormal readings:  %d\n", m->abnormal_readings);

  printf("\nAlerts per device type:\n");
  for (int i = 0; i < TYPE_COUNT; i++) {
    printf("  %-25s %d\n", device_types[i].label, m->alerts_per_type[i]);
  }

  printf("\nCustom callback (sensor_drift_monitor):\n");
  printf("  Drift checks done:  %d\n", m->drift_checks);
  printf("  Drift alerts:       %d\n", m->drift_alerts);
  printf("  Biggest drift:      %s\n", m->max_drift_text);
}

// Makes a random reading for every device
void generate_readings(Monitor *m) {
  printf("\n--- Generate Readings ---\n");
  if (m->device_count == 0) {
    printf("No devices registered.\n");
    return;
  }

  Device *end = m->devices + m->device_count;
  for (Device *d = m->devices; d < end; d++) {
    const DeviceType *info = &device_types[d->type];
    float fraction = (float)rand() / (float)RAND_MAX;
    float value = info->sim_min + (info->sim_max - info->sim_min) * fraction;
    set_reading(d, value);
    d->has_reading = 1;

    char text[30];
    format_value(d, text, sizeof(text));
    printf("Device %d (%s) generated %s\n", d->id, d->name, text);
  }
  printf("Use option 5 to process these readings.\n");
}

// Runs one callback and saves the alert if it raised one
int run_callback(Monitor *m, Device *d, const char *name, Callback callback,
                 char *result) {
  d->alert_reason[0] = '\0';
  d->alert_expected[0] = '\0';
  process_device(d, callback);
  if (d->alert_reason[0] == '\0') {
    return 0;
  }

  if (!log_alert(m, d, name)) {
    printf("Error: out of memory, alert not saved.\n");
  }
  if (strlen(result) > 0) {
    strcat(result, "; ");
  }
  strcat(result, "[");
  strcat(result, name);
  strcat(result, "] ");
  strcat(result, d->alert_reason);
  return 1;
}

// Processes the reading of one device and prints the result
void process_one(Monitor *m, Device *d) {
  const DeviceType *info = &device_types[d->type];
  char result[300] = "";
  char value[30];

  d->status = STATUS_NORMAL;
  d->reading_count++;

  int abnormal = run_callback(m, d, info->callback_name, info->callback, result);

  int drifted = run_callback(m, d, "sensor_drift_monitor", sensor_drift_monitor,
                             result);
  abnormal = abnormal + drifted;
  m->drift_alerts = m->drift_alerts + drifted;

  if (d->reading_count > 1) {
    m->drift_checks++;
    float ratio = d->last_drift / info->drift_limit;
    if (ratio > m->max_drift_ratio) {
      m->max_drift_ratio = ratio;
      snprintf(m->max_drift_text, sizeof(m->max_drift_text),
               "%.1f %s on %s (limit %.1f)", d->last_drift, info->unit,
               d->name, info->drift_limit);
    }
  }

  d->previous_value = get_value(d);
  d->has_reading = 0;

  m->total_readings++;
  if (abnormal > 0) {
    m->abnormal_readings++;
  } else {
    m->normal_readings++;
  }

  format_value(d, value, sizeof(value));
  printf("%-4d %-22s %-25s %-12s ", d->id, d->name, info->label, value);
  if (abnormal > 0) {
    printf("ALERT %s\n", result);
  } else {
    printf("Normal\n");
  }
}

// Processes all the new readings through the callbacks
void process_readings(Monitor *m) {
  printf("\n--- Process Readings ---\n");
  int processed = 0;
  Device *end = m->devices + m->device_count;
  for (Device *d = m->devices; d < end; d++) {
    if (!d->has_reading) {
      continue;
    }
    if (processed == 0) {
      printf("%-4s %-22s %-25s %-12s %s\n", "ID", "Name", "Type", "Value",
             "Result");
    }
    process_one(m, d);
    processed++;
  }
  if (processed == 0) {
    printf("No new readings. Generate readings first (option 4).\n");
    return;
  }
  printf("%d reading(s) processed.\n", processed);
}

#define MOCK_COUNT 6

// Loads 6 sample devices
void load_mock_data(Monitor *m) {
  int ids[MOCK_COUNT] = {1, 2, 3, 4, 5, 6};
  const char *names[MOCK_COUNT] = {
      "Boiler Room Temp", "Server Rack Temp", "Water Line Pressure",
      "Gas Tank Pressure", "Drone Battery",   "Sensor Node Battery",
  };
  int types[MOCK_COUNT] = {TYPE_TEMPERATURE, TYPE_TEMPERATURE, TYPE_PRESSURE,
                           TYPE_PRESSURE,    TYPE_BATTERY,     TYPE_BATTERY};

  for (int i = 0; i < MOCK_COUNT; i++) {
    Device d;
    memset(&d, 0, sizeof(Device));
    d.id = ids[i];
    strcpy(d.name, names[i]);
    d.type = types[i];
    d.status = STATUS_NORMAL;
    add_device(m, d);
  }
}

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
