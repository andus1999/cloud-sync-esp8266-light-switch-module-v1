import firebase_admin
from firebase_admin import credentials
from firebase_admin import db
from firebase_admin import storage
import subprocess
import os
import json
import time
import datetime

cred = credentials.Certificate(json.loads(os.environ.get('FB_ADMIN')))

firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://cloud-sync-iot-default-rtdb.europe-west1.firebasedatabase.app/',
    'storageBucket': 'cloud-sync-iot.appspot.com'
})

def update_firmware():
  with open('include/HardwareInfo.h', 'r+') as f:
    hardware_id = f.read().split('"')[1]

  blob = storage.bucket().blob(f'firmware/{hardware_id + str(time.time())}.bin')
  new_url = blob.generate_signed_url(datetime.timedelta(days=365))

  with open('include/Firmware.h', 'w') as f:
    f.write(f'#define FIRMWARE_LINK "{new_url}"')
  
  subprocess.run(["pio", "run"])

  blob.upload_from_filename('.pio/build/nodemcuv2/firmware.bin')
  blob.make_public()
  
  networks = db.reference('networks').get()
  print(networks.keys())
  for network in networks.keys():
    for device in networks[network]['devices'].keys():
      if networks[network]['devices'][device]['device_info']['hardware_id'] == hardware_id:
        db.reference(f'networks/{network}/devices/{device}/cloud_state/firmware').set(new_url)

if __name__ == "__main__":
  update_firmware()
