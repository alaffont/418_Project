#!/bin/bash

g++ compression.cpp

if [ -e "a.out" ]
then
    ./a.out example.txt
else
    echo "Error Making File"
fi
