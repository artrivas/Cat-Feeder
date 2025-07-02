#!/usr/bin/env python3
import asyncio, datetime
from websockets.legacy.server import serve        # legacy keeps things simple

PORT = 8080
SAVE_DIR = "frames"                               # will be created if missing

async def handler(ws):
    print("client connected")
    counter = 0
    async for msg in ws:
        if isinstance(msg, str):                  # text message
            print("text:", msg)
        else:                                     # binary JPEG
            counter += 1
            fname = f"{SAVE_DIR}/frame_{counter:05d}.jpg"
            with open(fname, "wb") as f:
                f.write(msg)
            print(f"saved {fname} ({len(msg)} bytes)")
    print("client gone")

async def main():
    import pathlib, os
    pathlib.Path(SAVE_DIR).mkdir(exist_ok=True)
    async with serve(handler, "0.0.0.0", PORT):
        print(f"listening on ws://0.0.0.0:{PORT}/")
        await asyncio.Future()    # run forever

if __name__ == "__main__":
    asyncio.run(main())
