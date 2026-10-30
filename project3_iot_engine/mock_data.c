#include <string.h>

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
