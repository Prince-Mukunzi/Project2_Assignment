#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
