#!/bin/bash

cat build/*.map | grep BREAKPOINT | while read -r line; do
    address=$(echo "$line" | awk -F'=' '{print $2}' | awk '{print $1}' | tr -d '$')
    
    printf "debug set_bp 0x%s\n" "$(echo "$address" | tr 'A-F' 'a-f')"
done