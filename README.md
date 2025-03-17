# ObservatoryFans
Automatic control of ventilation fans based on temperature and/or humidity

Uses a tweaked version of https://github.com/Lihp8270/AHT20-For-Pico so that 
I can have two AHT20 devices, one for inside and one for outside.  The AHT20 unfortunately has a fixed I2C address so I needed to use both I2C buses.

The basic plan is "Don't make things worse", so e.g. if outside humidity is higher than inside, it won't start the fans, but there is also a provision for
Summer cooling (untested till Summer arrives).
