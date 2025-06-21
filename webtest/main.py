import time

import requests

# Usa una imagen .jpg de prueba

while True:
    try:
        with open("images.jpeg", "rb") as f:
            img_bytes = f.read()
        response = requests.post(
            "https://iot-project-production-5bcd.up.railway.app/video/upload",
            data=img_bytes,
        )
        print("POST", response.status_code)
    except Exception as e:
        print("Error:", e)

    time.sleep(0.2)