Import("env")
import os
import sys
from uuid import UUID

env_file = ".env"
# all required keys EXCEPT for the readkey
required_keys = [
    "SENSOR_ARRAY_ID", 
    "HUM_SENSOR_ID", 
    "HUM_SENSOR_TYPE", 
    "TEMP_SENSOR_ID", 
    "TEMP_SENSOR_TYPE", 
    "TURB_SENSOR_ID", 
    "TURB_SENSOR_TYPE", 
    "SAL_SENSOR_ID", 
    "SAL_SENSOR_TYPE", 
    "EC_SENSOR_ID", 
    "EC_SENSOR_TYPE", 
    "TDS_SENSOR_ID", 
    "TDS_SENSOR_TYPE", 
    "PH_SENSOR_ID", 
    "PH_SENSOR_TYPE", 
    "DO_SENSOR_ID", 
    "DO_SENSOR_TYPE"
]
found_keys = []
permitted_sensor_types = ["float", "int", "char", "media"]

# Source - https://stackoverflow.com/a/79298779
# Posted by lhinkson
# Retrieved 2026-07-09, License - CC BY-SA 4.0
def is_valid_uuid(val):
    try:
        return str(UUID(str(val))) == str(val)
    except ValueError:
        return False


if os.path.isfile(env_file):
    print("SUCCESS: Found .env file, injecting tokens...")
    token_found = False
    
    with open(env_file) as f:
        for line in f:
            if line.strip() and not line.startswith("#"):
                key, value = line.strip().split("=", 1)
                
                # Check for readkey and check if it's value is a valid uuidv4
                if key == "READ_KEY" and is_valid_uuid(value):
                    token_found = True
                # add key to found_keys if its value is not empty and for TYPE keys make sure it is one of the acceptable values
                if str(key).endswith("TYPE") and value in permitted_sensor_types:
                    found_keys.append(key)
                elif value:
                    found_keys.append(key)
                env.Append(CPPDEFINES=[(key, f'\\"{value}\\"')])
                
    # If the file exists but the read key is missing/invalid, kill the build
    if not token_found:
        sys.exit("\nCRITICAL BUILD ERROR: 'READ_KEY' is missing or invalid inside your .env file!\n")
    # Check if all required keys are present
    missing_keys = [k for k in required_keys if k not in found_keys]
    if missing_keys:
        sys.exit(f"\nCRITICAL BUILD ERROR: Missing keys in .env: {', '.join(missing_keys)}\n")

else:
    # If the file doesn't exist at all, kill the build
    sys.exit("\nCRITICAL BUILD ERROR: .env file not found! Please copy .env.example to .env and add your token.\n")