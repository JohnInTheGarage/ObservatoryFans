#ifndef PICOWSENSORUNIT_AHT20_H
#define PICOWSENSORUNIT_AHT20_H

#include "hardware/i2c.h" // Include this for i2c_inst_t

class AHT20 {
private:
    float tempReading = 0;
    float humReading = 0;
    const static int addr = 0x38;
    i2c_inst_t *i2c_port; // Add this line

    bool checkCalibration();
    void sensorInit();

public:
    AHT20();
    AHT20(i2c_inst_t *i2c); // Declare the constructor here!

    void triggerMeasurement();
    float getTemp();
    float getTemp_f();
    float getHumidity();
    void init(int sdaPort, int sclPort);
};


#endif //PICOWSENSORUNIT_AHT20_H
