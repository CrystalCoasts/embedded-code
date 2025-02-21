# embedded-code
For Senior Design 1 in idea of proof of concept we will make use of FIU network alongside Google API for transmitting data from sensors to web application

During Senior Design 2 we will change the system to use either Bluetooth, or LoRa or similar communication that do not depend on internet Access.
In this scenario the idea is user connects to system using bluetooth, receives the data and send it to have global access.

Afterwards, the project was handed in to the RDF lab alongside the Institute of Environment and School of Architecture at Florida International University. The scope of the project remains the same, however, the plans are make it more optimized and easier to use for the everyday person. Meaning maintenance must be easily done, and everything must be totally modular. Testing is required before this device is operational.

# Current development
Currently, we are in the process of implementing wireless communication through SIM and 4G/LTE with the Liligo T-sim7000G. We are currently in the works of hosting our own database and web server to remain totally local as according to the rules of FIU. Development in currently focused on cellular connectivity development, however that also includes recoding everything that required WiFi (time of day, https posts, responses, certificates, ect.). Additionally, external peripherals running off I2C were also added for the reduction of wires connected to the ESP32 (I2C 12-bit ADC and IO Extender MCP23017). Once this dev branch is stable and in working order, the branch will be squashed and merged with the main branch. これのために私たちは頑張ります！
