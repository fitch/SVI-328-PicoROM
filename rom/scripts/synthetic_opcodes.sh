#!/bin/bash

INPUT_FILE="$1"

if [[ -z "$INPUT_FILE" || ! -f "$INPUT_FILE" ]]; then
    echo "Error: Please provide a valid existing file as the first parameter."
    exit 1
fi

found_cd0000=false
found_other=false

echo "Detecting possible use of synthetic opcodes:"

while IFS= read -r line; do
    read -r _ _ opcode mnemonic _ <<< "$line"

    if [[ "$mnemonic" =~ ^(ENDR|binary|defs|db|dw|dd|dq)$ ]]; then
        continue
    fi

    if [[ "$opcode" == "cd0000" ]]; then
        echo "$line"
        found_cd0000=true
    elif [[ "$opcode" =~ ^[0-9a-fA-F]+$ && ${#opcode} -gt 6 ]]; then
        echo "$line"
        found_other=true
    fi
done < "$INPUT_FILE"

if $found_cd0000; then
    echo "Found synthetic opcode replacement by call xxx, please review it."
    exit 1
elif $found_other; then
    exit 0
else
    echo "No synthetic opcodes detected."
    exit 0
fi
