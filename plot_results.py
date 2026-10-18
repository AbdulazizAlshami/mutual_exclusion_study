import pandas as pd
import matplotlib.pyplot as plt

# Load data and compute medians across repetitions
df = pd.read_csv("results.csv")
agg = df.groupby(["strategy_name", "threads"])["throughput_mops"].median().reset_index()

# -------------------------------------------------------------
# Plot 1: Scaling curves for all strategies on one plot
# -------------------------------------------------------------
plt.figure(figsize=(9, 6))

for name, group in agg.groupby("strategy_name"):
    group_sorted = group.sort_values("threads")
    plt.plot(group_sorted["threads"], group_sorted["throughput_mops"], marker='o', linewidth=2, label=name)

plt.title("Scaling Curves: Counter Strategies (Median Throughput)", fontsize=13)
plt.xlabel("Number of Threads", fontsize=11)
plt.ylabel("Throughput (M ops/sec)", fontsize=11)
plt.xscale("linear")
plt.grid(True, linestyle="--", alpha=0.6)
plt.legend()
plt.tight_layout()
plt.yscale("log")
plt.ylabel("Throughput (M ops/sec, Log Scale)", fontsize=11)
plt.savefig("figure1_scaling_curves.png", dpi=300)
plt.close()
print("Saved figure1_scaling_curves.png")
# -------------------------------------------------------------
# Plot 2: False-sharing comparison (Packed vs Padded)
# -------------------------------------------------------------
plt.figure(figsize=(8, 5))
false_sharing = agg[agg["strategy_name"].isin(["per_thread_packed", "per_thread_padded"])]

for name, group in false_sharing.groupby("strategy_name"):
    group_sorted = group.sort_values("threads")
    label = "Padded (64 B - No False Sharing)" if "padded" in name else "Packed (False Sharing Probe)"
    plt.plot(group_sorted["threads"], group_sorted["throughput_mops"], marker='s', linewidth=2, label=label)

plt.title("False-Sharing Impact: Packed vs Padded Per-Thread Counters", fontsize=13)
plt.xlabel("Number of Threads", fontsize=11)
plt.ylabel("Throughput (M ops/sec)", fontsize=11)
plt.grid(True, linestyle="--", alpha=0.6)
plt.legend()
plt.tight_layout()
plt.yscale("log")
plt.ylabel("Throughput (M ops/sec, Log Scale)", fontsize=11)
plt.savefig("figure2_false_sharing.png", dpi=300)
plt.close()
print("Saved figure2_false_sharing.png")
