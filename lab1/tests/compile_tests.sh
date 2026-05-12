#!/bin/bash

# x86_64
gcc -o lab1_x86_64_test lab1_test.c -pthread -lm -ldl

# x86
gcc -m32 -o lab1_x86_test lab1_test.c -pthread -lm -ldl

# ARM64
aarch64-linux-gnu-gcc-13 -o lab1_arm64_test lab1_test.c -pthread -lm -ldl

# ARM
arm-linux-gnueabihf-gcc-13 -o lab1_arm_test lab1_test.c -pthread -lm -ldl
