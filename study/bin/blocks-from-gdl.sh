#!/bin/bash

grep 0x $1 | \
    grep -v virt | \
    grep -v \< | \
    grep -v \% | \
    sed s/.*\".*0x/0x/ | \
    sed s/\"// | uniq 
