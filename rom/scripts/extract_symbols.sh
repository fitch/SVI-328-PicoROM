#!/bin/bash

cat build/*.map | while read -r line; do
    [[ -z "$line" ]] && continue

    name=$(echo "$line" | awk '{print $1}')
    address=$(echo "$line" | awk -F'[=;]' '{print $2}' | tr -d ' $')

    formatted_address=$(echo "$address" | tr 'a-f' 'A-F')H

    echo "$name: equ $formatted_address"
done