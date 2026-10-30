#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
