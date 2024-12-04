# embedded-code
For SD1 in idea of proof of concept we will make use of FIU network alongside Google API for transmitting data from sensors to web application

During SD2 we will change the system to use either Bluetoorh, or LoRa or similar communication that do not depend on internet Access.
In this scenario the idea is user connects to system using bluetooth, receives the data in sends it to have global access.

# Current development
Currently, we are in the process of implementing wireless communication through SIM and 4G/LTE. We are currently using Heroku as our host server, however later we plan on creating our  own server to host our website and data. MAIN IS NOT THE MOST UP TO DATE BRANCH. Switching to new-esp32-s3 will show you updated experimental code. We switched ESP32s in order to have better reliablity and have SIM chip compatibility. We have not updated the main branch due to everything still being experimental, and the ESP32 board switch.
