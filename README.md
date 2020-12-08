# Cooler Buddy - A Smart Fan
This project was created for my Internet of Things class.

## Inspiration 
This project was inspired during the beginning of the school year
(Fall 2020). It was very hot in my room, and I can't control the AC
at the place that I lived. So for my IoT project, I was thinking
about how to get a miniature fan blowing instead of buying a desk fan
since that would take a lot of space.

## Project Info
There are two modes for the fan: manual and auto.

**Manual Mode (Overrides Auto):**
If you put your finger over the touch sensor, you can manually
change the speed of the fan. There are 3-speed settings: high, med, low. The only way to change the speed setting is to toggle through 
the entire settings. 
Ex: Start setting is default -> low -> med -> high -> default ...  

**Auto Mode (Default Mode):**
Auto mode reads the temperature of the room/environment and adjust
its speed settings accordingly. 

Current settings:
* Temperature below 80, default speed.
* Temperature between 80-85, low speed.
* Temperature between 85-90, med speed.
* Temperature above 90, high speed.

## Sensors/Actuators/Devices
* Arduino Uno
* ESP-01 
* Touch Sensor
* Temperature Sensor
* LED Light

## Other
This project sends the temperature of the room and rpm of the fan
to ThingSpeak's cloud services for analytics. ThingSpeak sets a 
limit for free users to 15 seconds. So I sent my data to only captures every 20 seconds. 
The link to the ThingSpeak charts can be found below. 
(https://thingspeak.com/channels/1247645)