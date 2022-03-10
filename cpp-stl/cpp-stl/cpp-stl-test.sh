#!/bin/bash

i=10
while [ "$i" -lt 25 ] ; do
  ./cpp-stl $((2**$i))
  ((i++))
done
