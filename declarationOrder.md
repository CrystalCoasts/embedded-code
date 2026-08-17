## Declare a new box
1. Start process by showing ui to create a new box
  - when a location is being defined, retrieve all existing locations and if location they wish to define (coordinates) already exist, simply reuse that location
  - if location is not defined, then post a new location first and take returned id and assign it to locationID when a new box is being defined
2. once a box is created, then show the option to create a new sensor array
  - right away, show existing sensor array specifications in the system and for each show a list of all the sensors assigned to them
## When People are adding sensors to a sensor array (if no existing sensor array specification represents their sensor array)
1. First get all sensor specifications
2. Get the read labels for all of them
3. When someone is defining a new sensor specification, if the sensor specification with read label that they are defining already exists, suggest it to them
4. Otherwise, allow them to create a new sensor specification (repeat 3-4 for all sensors they want to add)
5. On submit,
  a. make post request to declare a new sensor array specification with the sensor specification ids that you just got, store returned sensor array specification id
  b. make post request to create new sensor array with the sensor array specification you just got and the box id from the start, store returned sensor array id and readkey
  c. make post requests to api to add each sensor with the sensor specification ids you just got to the database and the sensor array id you just got, store all returned sensor ids
6. show screen at completion with the sensor array id returned and the sensors' {id, type, readKey, unit, readValLabel, unitAbbreviation}returned shown