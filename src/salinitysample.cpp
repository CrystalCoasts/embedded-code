// #include <Ezo_i2c.h> //include the EZO I2C library from https://github.com/Atlas-Scientific/Ezo_I2c_lib
// #include <Wire.h>    //include arduinos i2c library
// #include <sequencer2.h> //imports a 2 function sequencer 
// #include <Ezo_i2c_util.h> //brings in common print statements



// // Ezo_board EC(EZO_EC, 100);  // Create an Ezo_board object for the EC sensor
// Ezo_board EC = Ezo_board(0x14, "EC");   


// float parseSalinity(const char* ec_data);

// void setup() {
//     char ec_data[32];
//     Serial.begin(115200);
//     Wire.begin();

//     delay(300);
//     Serial.printf("Testing testing \n");
//     EC.send_cmd("Cal,?");
//     delay(300);
//     EC.receive_cmd(ec_data,32);
//     Serial.print("Calibrated: ");
//     Serial.println(ec_data);

//     EC.send_cmd("T,?");
//     delay(300);
//     EC.receive_cmd(ec_data,32);
//     Serial.print("Temp Compensated: ");
//     Serial.println(ec_data);

//     EC.send_cmd("K,?");
//     delay(300);

//     EC.receive_cmd(ec_data,32);
//     Serial.print("Conversion Factor: ");
//     Serial.println(ec_data);

//     EC.send_cmd("O,EC,0");  // Send read command to EC sensor
//     delay(300);
//     EC.send_cmd("O,TDS,0");  // Send read command to EC sensor
//     delay(300);
//     EC.send_cmd("O,S,0");  // Send read command to EC sensor
//     delay(300);
//     EC.send_cmd("O,SG,0");  // Send read command to EC sensor
//     delay(300);

//     EC.send_cmd("O,S,1");  // Send read command to EC sensor
//     delay(300);

//     EC.send_cmd("O,?");  // Send read command to EC sensor
//     delay(300);
//     EC.receive_cmd(ec_data,32);
//     Serial.print("Enabled readings");
//     Serial.println(ec_data);
  
  
// }

// void loop() {
//   char ec_data[32];

//   EC.send_cmd("R");  // Send read command to EC sensor
//   delay(1000);       // Wait for the sensor to process the command
//   EC.receive_cmd(ec_data, 32);  // Receive the EC sensor data

//   // Example output
//   Serial.print("EC Raw: ");
//   Serial.println(ec_data);  // Print the EC data

//   // Assuming the sensor also provides salinity in the same response
//      // Print the salinity data

//   delay(2000);  // Wait before the next reading
// }

// float parseSalinity(const char* ec_data) {
//   // Example parsing function, actual implementation depends on the data format
//   // This is a placeholder for illustration
//   float salinity = atof(ec_data) * 0.001;  // Simple conversion for demonstration
//   return salinity;
// } 