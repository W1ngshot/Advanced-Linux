#!/usr/bin/env python3
import hashlib
import sys

def generate_key(hwid):
    try:
        if len(hwid) != 16 or not hwid.isascii():
            return "HWID must contains exactly 16 ASCII symbols"

        digest = hashlib.md5(hwid.encode()).digest()
        reversed_digest = digest[::-1]
        return reversed_digest.hex()
    
    except Exception as e:
        return f"Exception: {e}"

if __name__ == "__main__":
    if len(sys.argv) > 1:
        print(generate_key(sys.argv[1]))
    else:
        print("Example: python3 keygen.py <HWID>")
