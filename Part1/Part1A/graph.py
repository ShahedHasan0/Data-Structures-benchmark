import pandas as pd
import matplotlib.pyplot as plt

data = pd.read_csv("benchmark_results.csv")

plt.figure()
plt.plot(data["TextSize"], data["Linear"], marker="o", label="Linear List")
plt.plot(data["TextSize"], data["BST"], marker="o", label="BST")
plt.plot(data["TextSize"], data["Trie"], marker="o", label="Trie")
plt.plot(data["TextSize"], data["Hash"], marker="o", label="Hash Map")

plt.xlabel("Text Size")
plt.ylabel("Runtime (microseconds)")
plt.title("Spell Checker Runtime vs Text Size")
plt.legend()
plt.grid(True)

plt.savefig("runtime_graph.png")
plt.show()