# embedded-code

# Original idea
For the proof of concept SD1, we utilize the FIU network in conjunction with Google API to transmit sensor data to a web application.


# Development
The project employs a user-created hotspot to transmit data to our website. The website can send commands such as reading sensors, checking battery status, and scheduling the next wake-up. This allows users to request updated sensor data and monitor battery levels for potential replacement. As an alternative to using physical buttons, which could compromise the ESP's water seal underwater, we implemented a sleep/wake-up cycle which can be controlled via the website. This ensures continuous real-time data collection for study cases or experiments.

When a user is connected to the ESP, it remains active to facilitate continuous interaction through the web app, albeit at the expense of increased battery consumption.

Additionally, the ESP includes an SD card for continuous data storage. As storage capacity is reached, older data is overwritten, following a queue-like structure. Sensor readings are stored in CSV format for seamless transfer to applications such as Excel for data analysis.
