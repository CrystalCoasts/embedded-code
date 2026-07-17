## sensor reads
- need to rewrite sensorTask function so that it reads once since awake and then doesn't try again
  - remove the timer thing its currently doing
  - set up a different timer thing to just retry the read if for whatever reason it fails to read or save
- fix issue where if upload task gets stuck and read data wasn't saved beforehand, shutdown will delay until upload, but there is never a chance to save the sensor read since it is always stuck trying to upload
- make sensor task simply pass the date time it gets when it makes the read to the sensor save functions allowing them to only need the sd card mutex instead of also the sim card mutex
  - modify the prepareCSVPayload and prepareJsonPayload functions to take in both the data parameter AND the datetime parameter
  - Take the sim card mutex only in the main sensorTask function and only once to get the dateTime

## sim moments
- any functions that calls getCurrentTime or get_current_time **MUST** have the sim mutex locked