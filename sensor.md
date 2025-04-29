
# Sensor Specifications

```sh
Invoke-WebRequest -Uri "https://seawall.fiu.edu/sensorSpecifications" `
  -Method POST `
  -Body '{"type":"float", "version":"1", "readValLabel":"humidity, "unit":"Percentage", "unitAbbreviation":"%"}' `
  -ContentType "application/json"
```
return newSensorSpecID = 1

```sh
Invoke-WebRequest -Uri "https://seawall.fiu.edu/sensorSpecifications" `
  -Method POST `
  -Body '{"type":"float", "version":"1", "readValLabel":"temperature", "unit":"Celsius", "unitAbbreviation":"C"}' `
  -ContentType "application/json"
```
return newSensorSpecID = 2

```sh
Invoke-WebRequest -Uri "https://seawall.fiu.edu/sensorSpecifications" `
  -Method POST `
  -Body '{"type":"float", "version":"1", "readValLabel":"turbidity", "unit":"Nephelometric Turbidity Unit", "unitAbbreviation":"NTU"}' `
  -ContentType "application/json"
```
return newSensorSpecID = 3

```sh
Invoke-WebRequest -Uri "https://seawall.fiu.edu/sensorSpecifications" `
  -Method POST `
  -Body '{"type":"float", "version":"1", "readValLabel":"salinity", "unit":"Practical Salinity Unit (Parts per Thousand)", "unitAbbreviation":"PSU (ppt)"}' `
  -ContentType "application/json"
```
return newSensorSpecID = 4

```sh
Invoke-WebRequest -Uri "https://seawall.fiu.edu/sensorSpecifications" `
  -Method POST `
  -Body '{"type":"float", "version":"1", "readValLabel":"total dissolved solids", "unit":"Parts per Million", "unitAbbreviation":"ppm"}' `
  -ContentType "application/json"
```
return newSensorSpecID = 5

```sh
Invoke-WebRequest -Uri "https://seawall.fiu.edu/sensorSpecifications" `
  -Method POST `
  -Body '{"type":"float", "version":"1", "readValLabel":"conductivity", "unit":"Milligrams/Liter", "unitAbbreviation":"mg/L"}' `
  -ContentType "application/json"
```
return newSensorSpecID = 6

```sh
Invoke-WebRequest -Uri "https://seawall.fiu.edu/sensorSpecifications" `
  -Method POST `
  -Body '{"type":"float", "version":"1", "readValLabel":"ph", "unit":"Potential of Hydrogen", "unitAbbreviation":"pH"}' `
  -ContentType "application/json"
```
return newSensorSpecID = 7

```sh
    Invoke-WebRequest -Uri "https://seawall.fiu.edu/sensorSpecifications" `
    -Method POST `
    -Body '{"type":"float", "version":"1", "readValLabel":"dissolved oxygen", "unit":"Micro Siemens/Liter", "unitAbbreviation":"mS/L"}' `
    -ContentType "application/json"
```
return newSesorSpecID = 8

# Sensor Array Specificaitons

```sh
Invoke-WebRequest -Uri "https://seawall.fiu.edu/sensorArraySpecifications" `
  -Method POST `
  -Body '{"type":"data", "version":"1"}' `
  -ContentType "application/json"
```
return ID = 1 
<!-- WE GOT IT -->

```sh
Invoke-WebRequest -Uri "https://seawall.fiu.edu/sensorArraySpecifications" `
  -Method POST `
  -Body '{"type":"media", "version":"1"}' `
  -ContentType "application/json"
```


# BOXES BOXES BOXES
 <!-- already done JAJAJ -->

```sh
Invoke-WebRequest -Uri "https://seawall.fiu.edu/boxes" `
  -Method POST `
  -Body '{"nickname":"box1","public":"1"}' `
  -ContentType "application/json"
```
return ID = 2

# Sensor Array

```sh
Invoke-WebRequest -Uri "https://seawall.fiu.edu/sensorArrays" `
  -Method POST `
  -Body '{"sensor$array_specificationID":"1", "boxID":"2" }' `
  -ContentType "application/json"
```
return ID = 1
READKEY = f0fa3eaa-7ffd-43b9-8834-4fdddcd1bc95

# Sensors Declaration

```sh
Invoke-WebRequest -Uri "https://seawall.fiu.edu/sensors" `
  -Method POST `
  -Body '{"sensor_specificationID":"1", "sensor$arrayID":"1"}' `
  -ContentType "application/json"
```
return ID = 2

```sh
Invoke-WebRequest -Uri "https://seawall.fiu.edu/sensors" `
  -Method POST `
  -Body '{"sensor_specificationID":"2", "sensor$arrayID":"1"}' `
  -ContentType "application/json"
```
return ID = 3

```sh
Invoke-WebRequest -Uri "https://seawall.fiu.edu/sensors" `
  -Method POST `
  -Body '{"sensor_specificationID":"3", "sensor$arrayID":"1"}' `
  -ContentType "application/json"
```
return ID = 4

```sh
Invoke-WebRequest -Uri "https://seawall.fiu.edu/sensors" `
  -Method POST `
  -Body '{"sensor_specificationID":"4", "sensor$arrayID":"1"}' `
  -ContentType "application/json"
```
return ID = 5

```sh
Invoke-WebRequest -Uri "https://seawall.fiu.edu/sensors" `
  -Method POST `
  -Body '{"sensor_specificationID":"5", "sensor$arrayID":"1"}' `
  -ContentType "application/json"
```
return ID = 6

```sh
Invoke-WebRequest -Uri "https://seawall.fiu.edu/sensors" `
  -Method POST `
  -Body '{"sensor_specificationID":"6", "sensor$arrayID":"1"}' `
  -ContentType "application/json"
```
return ID = 7

```sh
Invoke-WebRequest -Uri "https://seawall.fiu.edu/sensors" `
  -Method POST `
  -Body '{"sensor_specificationID":"7", "sensor$arrayID":"1"}' `
  -ContentType "application/json"
```
return ID = 8

```sh
Invoke-WebRequest -Uri "https://seawall.fiu.edu/sensors" `
  -Method POST `
  -Body '{"sensor_specificationID":"8", "sensor$arrayID":"1"}' `
  -ContentType "application/json"
```
return ID = 9


<!-- do for amount of sensors -->
<!-- Put into sensorID into ESP to send with post requests -->


# Sensor Reads
<!-- SUBMIT READ ON ESP !!!!!!!!!!!!!!!!!!!! -->





