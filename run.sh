#!/bin/bash

g++ compression.cpp -g

if [ -e "a.out" ]
then
    ./a.out short.txt
    rm a.out
else
    echo "Error Making File"
fi
