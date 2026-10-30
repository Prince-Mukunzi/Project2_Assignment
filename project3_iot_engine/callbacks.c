#include <stdio.h>

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
