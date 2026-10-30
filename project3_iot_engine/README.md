# Dynamic IoT Event Monitoring and Callback Engine

**Student ID:** 101  
**Course:** C Programming  
**Project:** Dynamic IoT Event Monitoring and Callback Engine  
**Custom Callback:** Sensor Drift Detector

---

## What This Project Does

This is a C program that simulates IoT devices (temperature sensors, pressure sensors, and battery voltage monitors). You can register and remove devices, generate random readings, and process each reading through a callback function. Readings outside the normal range raise alerts. Everything is kept in dynamic memory and freed when you exit.

---

## Files

| File | What it contains |
|------|-----------------|
| `main.c` | Entry point, menu loop, and menu items |
| `device.c` | `Device` struct with the union, device type table, `Monitor` struct, add/find/free, union read/write helpers |
| `input.c` | Safe input helpers (read int, read line) |
| `callbacks.c` | `process_device()`, the three required monitors, and the custom drift monitor |
| `operations.c` | Register, display, and remove devices |
| `alerts.c` | Alert log, display alerts, and monitoring summary |
| `simulation.c` | Generate readings and process them through callbacks |
| `mock_data.c` | 6 sample devices loaded at startup |
| `sample_output.txt` | A real run of the program |

---

## How to Compile and Run

```bash
gcc main.c -o main
./main
```

`main.c` includes the other `.c` files, so you only compile `main.c`.

---

## Menu Options

```
=== Menu ===
1. Register a new device
2. Display all devices
3. Remove a device
4. Generate simulated readings
5. Process readings through callbacks
6. Display detected alerts
7. Monitoring summary
8. Exit
```

Typical use: option 4 generates one new reading for every device, then option 5 processes those readings. Repeat 4 and 5 to get more readings (6 devices give 6 readings per round).

---

## Device Types, Ranges and Thresholds

Each device type has a simulated range (where random readings come from), a normal operating range (outside it raises an alert), and a drift limit (used by the custom callback). They are all in the `device_types` table in `device.c`, so they are easy to change.

| Type | Unit | Simulated range | Normal range | Drift limit | Callback |
|------|------|-----------------|--------------|-------------|----------|
| Temperature sensor | C | 5 to 45 | 10 to 40 | 20 | `temperature_monitor` |
| Pressure sensor | hPa | 940 to 1060 | 960 to 1040 | 60 | `pressure_monitor` |
| Battery voltage monitor | V | 2.8 to 4.4 | 3.2 to 4.2 | 0.8 | `battery_monitor` |

The simulated ranges are wider than the normal ranges on purpose, so a run shows both normal readings and alerts.

---

## How the Code Is Structured

### Structs and Union
- `Device` - holds id, name, type, a union with the reading, and the status
- `Monitor` - holds the dynamic device array, the dynamic alert array, and the counters

```c
union {
  float temperature;
  int pressure;
  float voltage;
} reading;
```

All three members share the same memory. The program only reads or writes the member that matches `device->type`:
- `get_value()` and `set_reading()` in `device.c` are the only places that pick a member by type
- each monitor checks `device->type` first and only touches its own member

### Dynamic Memory
- `init_monitor()` allocates the device array and the alert array with `malloc`
- `add_device()` and `log_alert()` double their arrays with `realloc` when full
- `remove_device()` shifts the devices left and shrinks the array with `realloc` (it always keeps at least one slot)
- Every allocation result is checked
- `free_monitor()` releases everything at the end

### Function Pointers and Callbacks
`process_device()` takes a device and a callback and calls it:

```c
void process_device(Device *device, void (*callback)(Device *));
```

The callback for each type is stored in the `device_types` table, so the type picks the monitor without any big if/else:

```c
{"Temperature sensor", "C", ..., "temperature_monitor", temperature_monitor},
```

The menu is also an array of `MenuItem` structs with a function pointer for each option.

### Pointer Traversal
`display_devices()`, `generate_readings()`, `process_readings()` and `display_alerts()` walk the arrays with a pointer from the start to the end (`for (Device *d = m->devices; d < end; d++)`). `remove_device()` uses pointer subtraction (`target - m->devices`) to find the index.

---

## Alerts

Every alert shows:
- device ID and name
- reading value and unit
- expected range or limit
- reason for the alert
- the callback that raised it

When a callback finds a problem it sets the device status to alert and writes the reason in the device. `process_readings()` then saves the alert in the alert log. A reading counts as abnormal if at least one callback raised an alert for it.

---

## Custom Callback: Sensor Drift Detector (`sensor_drift_monitor`)

**Purpose:** catches sudden jumps in a sensor, even when the value is still inside the normal range. A sensor that goes from 12 C to 38 C is suspicious although both values are normal.

**When it runs:** after the type monitor, for every reading of every device. It is called through a function pointer, the same way as the other callbacks.

**Algorithm:**

1. The device remembers its previous reading (`previous_value`) and how many readings it has had (`reading_count`).
2. If this is the first reading, there is nothing to compare, so it does nothing.
3. Otherwise it computes `change = |current reading - previous reading|`.
4. If `change` is bigger than the drift limit of the device type (20 C, 60 hPa, 0.8 V), the device status becomes alert and the reason is saved.
5. After the callbacks run, the current reading becomes the new previous reading.

**Results in the summary:** number of drift checks, number of drift alerts, and the biggest drift (the one closest to or furthest over its limit, measured as `change / drift limit`).

---

## Memory Safety Notes

- Duplicate device IDs are rejected when registering
- Empty names and invalid device types are rejected
- Removing a device keeps the other devices intact. Alerts that were already logged stay in the log, because each alert stores its own copy of the device name and value
- Names are limited by the 40 character field and extra characters are discarded, so input cannot overflow the buffer
- A reading that was generated but not processed yet is replaced if you generate again

---

## Evidence

`sample_output.txt` is a real run that shows:

- 27 simulated readings across 7 devices, with a processing result for each
- callback dispatch for all three device types (temperature, pressure, battery)
- both normal readings and alerts, including drift alerts from the custom callback
- registering a device (ID 7), rejecting a duplicate ID, removing a device, and rejecting an unknown ID
- the alert list and the monitoring summary

Readings are random (`srand(time(NULL))`), so your own run will show different values.

---

## AI Usage Disclosure

An AI assistant was used only for this README and the mock data in `mock_data.c`.
