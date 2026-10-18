#!/bin/bash
set -e
echo "[1/3] Compiling benchmark harness..."
gcc -O3 -pthread benchmark.c -o benchmark
echo "[2/3] Executing benchmark matrix (>=7 repetitions)..."
python3 run_benchmarks.py
echo "[3/3] Generating publication figures..."
python3 plot_results.py
echo "All figures regenerated successfully."
