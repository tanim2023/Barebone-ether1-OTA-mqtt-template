import os
import hashlib

def get_file_hash(filepath):
    """Calculates the SHA-256 hash of a file."""
    hasher = hashlib.sha256()
    with open(filepath, 'rb') as f:
        while chunk := f.read(4096):
            hasher.update(chunk)
    return hasher.hexdigest()

def update_firmware_version(bin_filepath, version_filepath):
    """Updates the firmware version in the given file based on the .bin file's hash."""
    if not os.path.exists(bin_filepath):
        print(f"Error: .bin file not found at {bin_filepath}")
        return

    bin_hash = get_file_hash(bin_filepath)

    if os.path.exists(version_filepath):
        with open(version_filepath, 'r') as f:
            try:
                current_version = int(f.read().strip())
            except ValueError:
                current_version = 0 # handle incorrect format in firewarev.txt
    else:
        current_version = 0

    previous_hash_filepath = "previous_bin_hash.txt"

    if os.path.exists(previous_hash_filepath):
        with open(previous_hash_filepath, 'r') as f:
            previous_hash = f.read().strip()
    else:
        previous_hash = ""

    if bin_hash != previous_hash:
        current_version += 1
        with open(version_filepath, 'w') as f:
            f.write(str(current_version))
        with open(previous_hash_filepath, 'w') as f:
            f.write(bin_hash)
        print(f"Firmware version updated to {current_version}")
    else:
        print(f"Firmware version unchanged: {current_version}")

if __name__ == "__main__":
    bin_file = "build/eth2ap.bin"  # Replace with the actual path
    version_file = "build/firmwarev.txt"

    update_firmware_version(bin_file, version_file)