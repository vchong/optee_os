#!/bin/bash

make
make PLATFORM=hikey CFG_ARM64_core=y
