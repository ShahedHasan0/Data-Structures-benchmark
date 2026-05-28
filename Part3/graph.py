import pandas as pd
import matplotlib.pyplot as plt

# Read CSV file
df = pd.read_csv("part3_results.csv")

# Print column names (for debugging)
print(df.columns)

# ---------------- BUILD TIME GRAPH ----------------
plt.figure(figsize=(8,5))

for structure in df["structure"].unique():
    subset = df[df["structure"] == structure]

    plt.plot(
        subset["word_count"],
        subset["build_ms"],
        marker='o',
        label=structure
    )

plt.xlabel("Dictionary Size (Words)")
plt.ylabel("Build Time (ms)")
plt.title("Dictionary Build Time Comparison")
plt.legend()
plt.grid(True)

plt.savefig("build_time_graph.png")

# ---------------- LOOKUP TIME GRAPH ----------------
plt.figure(figsize=(8,5))

for structure in df["structure"].unique():
    subset = df[df["structure"] == structure]

    plt.plot(
        subset["word_count"],
        subset["lookup_ms"],
        marker='o',
        label=structure
    )

plt.xlabel("Dictionary Size (Words)")
plt.ylabel("Lookup Time (ms)")
plt.title("Dictionary Lookup Time Comparison")
plt.legend()
plt.grid(True)

plt.savefig("lookup_time_graph.png")

# ---------------- MEMORY GRAPH ----------------
plt.figure(figsize=(8,5))

for structure in df["structure"].unique():
    subset = df[df["structure"] == structure]

    plt.plot(
        subset["word_count"],
        subset["memory_kb"],
        marker='o',
        label=structure
    )

plt.xlabel("Dictionary Size (Words)")
plt.ylabel("Memory Usage (KB)")
plt.title("Dictionary Memory Usage Comparison")
plt.legend()
plt.grid(True)

plt.savefig("memory_usage_graph.png")

print("Graphs created successfully.")

