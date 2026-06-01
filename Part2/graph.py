import pandas as pd
import matplotlib.pyplot as plt

data = pd.read_csv("hash_benchmark.csv")

# Insert graph
plt.figure()
plt.plot(data["LoadFactor"], data["SeparateChaining_Insert"], marker="o", label="Separate Chaining")
plt.plot(data["LoadFactor"], data["LinearProbing_Insert"], marker="o", label="Linear Probing")
plt.plot(data["LoadFactor"], data["DoubleHashing_Insert"], marker="o", label="Double Hashing")

plt.xlabel("Load Factor")
plt.ylabel("Insert Time (microseconds)")
plt.title("Insert Time vs Load Factor")
plt.legend()
plt.grid(True)

plt.savefig("insert_time_graph.png")

# Search graph
plt.figure()
plt.plot(data["LoadFactor"], data["SeparateChaining_Search"], marker="o", label="Separate Chaining")
plt.plot(data["LoadFactor"], data["LinearProbing_Search"], marker="o", label="Linear Probing")
plt.plot(data["LoadFactor"], data["DoubleHashing_Search"], marker="o", label="Double Hashing")

plt.xlabel("Load Factor")
plt.ylabel("Search Time (microseconds)")
plt.title("Search Time vs Load Factor")
plt.legend()
plt.grid(True)

plt.savefig("search_time_graph.png")
plt.show()