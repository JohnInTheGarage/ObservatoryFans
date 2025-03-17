#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "libraries/AHT20/AHT20.h"

// I2C defines
#define SDA_INSIDE 16
#define SCL_INSIDE 17
#define SDA_OUTSIDE 26
#define SCL_OUTSIDE 27
#define FAN_CONTROL 21
#define I2C_INSIDE i2c0
#define I2C_OUTSIDE i2c1

bool fansActive;
bool fansPrevious;
int minuteCount;

//===================
void oneMinFlash(){
	
	// Show its alive and limit the readings to once per minute
	for (short sec =0; sec < 59; sec++) {
        gpio_put(PICO_DEFAULT_LED_PIN, 0);
        sleep_ms(500);
        gpio_put(PICO_DEFAULT_LED_PIN, 1);
        sleep_ms(500);
	}
}

/* ====================================================
 * Set fans on or off based on comparison of inside and outside conditions
 * (However, the relay's "Off" signal is logic-high... )
 * Plus prvent the fans running for hours on end
 */ 
void setFans(float inTemp, float inRH, float outTemp, float outRH) {

    fansActive = false;
	while (true) {
	    // don't bring in extra humidity when its cold
		if (outRH > inRH) {
			if (inTemp < 15) {
				break;
			}
		}

		// Fans on to cool down if its hot today
		if (inTemp > 25) {
			if (inTemp > (outTemp + 5.0)){
				fansActive = true;
				break;
			}
		}

		// Fans on to reach ambient if its been cold overnight but is now warming
		if (inTemp < 10) {
			if ((inTemp +5.0) < outTemp){
				fansActive = true;
				break;
			}
		}
		
		//  Fans on if inside humidity above outside humidity
		if (inRH > (outRH + 5.0)){
			fansActive = true;
			break;
		}
        break;
	}
	

	/*---- allow a 1-minute break if fans are on a long time. ---*/
	
	if (fansPrevious == fansActive) {
		// if fan status unchanged since last time
		// bump the minute counter
		minuteCount++;
		if (fansActive) {
			if (minuteCount > 59) {
				// if the fans have been on for an hour
				// reset minute counter and turn the fans off
				minuteCount = 0;
				fansActive = false;
			}
		} 
	}

	fansPrevious = fansActive;
	gpio_put(FAN_CONTROL, !fansActive);
}


int main(){
    stdio_init_all();
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    gpio_init(FAN_CONTROL);
    gpio_set_dir(FAN_CONTROL, GPIO_OUT);
    gpio_put(FAN_CONTROL, 1);  // i.e. turned off
    
    AHT20 aht20_inside(I2C_INSIDE);
    AHT20 aht20_outside(I2C_OUTSIDE);

    aht20_inside.init(SDA_INSIDE, SCL_INSIDE);
    aht20_outside.init(SDA_OUTSIDE, SCL_OUTSIDE);

	while (true){
		oneMinFlash();

        aht20_inside.triggerMeasurement();
        aht20_outside.triggerMeasurement();

        float insideTemp = aht20_inside.getTemp();
        float insideRhum = aht20_inside.getHumidity();
        float outsideTemp = aht20_outside.getTemp();
        float outsideRhum = aht20_outside.getHumidity();

        setFans(insideTemp, insideRhum, outsideTemp, outsideRhum);
		
		// When connected via usb these minute-by-minute data points
		// can be collected into a .csv file for use in creating graphs;
		// see Related/ReadObsFans directory.

		printf("%.1f, %.1f, %.1f, %.1f, %d\n", 
			insideTemp, insideRhum, outsideTemp, outsideRhum, fansActive * 30);
			// active * 30 is just to lift it off the bottom of the graph.
	}
}

