#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
