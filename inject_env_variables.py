Import("env")
import os
import sys
from uuid import UUID

env_file = ".env"

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
                
                # Check if we found our required key
                if key == "READ_KEY" and is_valid_uuid(value):
                    token_found = True
                    
                env.Append(CPPDEFINES=[(key, f'\\"{value}\\"')])
                
    # If the file exists but the key is missing, kill the build
    if not token_found:
        sys.exit("\nCRITICAL BUILD ERROR: 'READ_KEY' is missing or invalid inside your .env file!\n")

else:
    # If the file doesn't exist at all, kill the build
    sys.exit("\nCRITICAL BUILD ERROR: .env file not found! Please copy .env.example to .env and add your token.\n")