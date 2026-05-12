#!/usr/bin/env python3
import os
import sys

def patch_binary(file_path):
    if not os.path.exists(file_path):
        print(f"File not found: '{file_path}'")
        return

    try:
        with open(file_path, "r+b") as f:
            f.seek(0x143a)
            current_byte = f.read(1)
            
            if current_byte == b'\x00':
                f.seek(0x143a)
                f.write(b'\x01')
                print(f"File '{file_path}' patched successfully")
            elif current_byte == b'\x01':
                print(f"File '{file_path}' already patched")
            else:
                print(f"Expected 00 by offset {hex(0x143a)} but was {current_byte.hex()}")
    except Exception as e:
        print(f"Exception: {e}")

if __name__ == "__main__":
    if len(sys.argv) > 1:
        patch_binary(sys.argv[1])
    else:
        print("Example: python3 patcher.py <path>")
