#!/usr/bin/bash

set -xe

cc main.c fvm.c fvm_scanner.c -o fvm
