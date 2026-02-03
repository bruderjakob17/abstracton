#!/bin/bash

mkdir -p results/raw

parentdir=$(dirname `pwd`)

./bench.sh "${parentdir}/solve_dodo -i t" "${parentdir}/dodo" "*.json" >> "results/raw/trap.txt" 2>&1
./bench.sh "${parentdir}/solve_dodo -i s" "${parentdir}/dodo" "*.json" >> "results/raw/siphon.txt" 2>&1
./bench.sh "${parentdir}/solve_dodo -i f" "${parentdir}/dodo" "*.json" >> "results/raw/flow.txt" 2>&1
