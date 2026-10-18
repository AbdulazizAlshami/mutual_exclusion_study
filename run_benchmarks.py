import subprocess
import multiprocessing
import csv

# Hardware thread detection
logical_cores = multiprocessing.cpu_count()
max_threads = 2 * logical_cores

# Define thread counts: powers of 2 up to 2 * cores
threads = []
t = 1
while t <= max_threads:
    threads.append(t)
    t *= 2
if max_threads not in threads:
    threads.append(max_threads)

# 5 Strategies from the assignment matrix
strategies = {
    0: "mutex",
    1: "spinlock",
    2: "cas_faa",
    3: "per_thread_packed",
    4: "per_thread_padded"
}

TOTAL_OPS = 10000000  # 10M increments per test run
REPETITIONS = 7        # >=7 repetitions required by rubric

output_file = "results.csv"

with open(output_file, mode="w", newline="") as f:
    writer = csv.writer(f)
    writer.writerow(["strategy_id", "strategy_name", "threads", "repetition", "total_ops", "time_sec", "throughput_mops"])

    for strat_id, strat_name in strategies.items():
        for th in threads:
            print(f"Testing {strat_name} with {th} threads...")
            for rep in range(1, REPETITIONS + 1):
                cmd = ["./benchmark", str(strat_id), str(th), str(TOTAL_OPS)]
                res = subprocess.run(cmd, capture_output=True, text=True, check=True)
                parts = res.stdout.strip().split(",")
                time_sec = float(parts[3])
                mops = float(parts[4])
                writer.writerow([strat_id, strat_name, th, rep, TOTAL_OPS, time_sec, mops])

print(f"\nAll tests completed! Data saved to {output_file}")
