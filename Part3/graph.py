import pandas as pd
import matplotlib.pyplot as plt

data = pd.read_csv("part3_results.csv")

# Build Time
plt.figure()
for structure in data["structure"].unique():
    d = data[data["structure"] == structure]
    plt.plot(d["word_count"], d["build_ms"], marker="o", label=structure)

plt.xlabel("Dictionary Size")
plt.ylabel("Build Time (ms)")
plt.title("Build Time vs Dictionary Size")
plt.legend()
plt.grid(True)

plt.savefig("build_time_graph.png")

# Lookup Time
plt.figure()
for structure in data["structure"].unique():
    d = data[data["structure"] == structure]
    plt.plot(d["word_count"], d["lookup_ms"], marker="o", label=structure)

plt.xlabel("Dictionary Size")
plt.ylabel("Lookup Time (ms)")
plt.title("Lookup Time vs Dictionary Size")
plt.legend()
plt.grid(True)

plt.savefig("lookup_time_graph.png")

# Memory Usage
plt.figure()
for structure in data["structure"].unique():
    d = data[data["structure"] == structure]
    plt.plot(d["word_count"], d["memory_kb"], marker="o", label=structure)

plt.xlabel("Dictionary Size")
plt.ylabel("Memory Usage (KB)")
plt.title("Memory Usage vs Dictionary Size")
plt.legend()
plt.grid(True)

plt.savefig("memory_usage_graph.png")

plt.show()