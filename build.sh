#!/usr/bin/bash

set -xe

cc -Ivendor/c-vector main.c fvm.c fvm_scanner.c fvm_parser.c -o fvm
