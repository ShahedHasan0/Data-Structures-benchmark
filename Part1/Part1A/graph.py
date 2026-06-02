import pandas as pd
import matplotlib.pyplot as plt

data = pd.read_csv("benchmark_results.csv")

# Full graph
plt.figure(figsize=(8,5))
plt.plot(data["TextSize"], data["Linear"], marker="o", label="Linear List")
plt.plot(data["TextSize"], data["BST"], marker="o", label="BST")
plt.plot(data["TextSize"], data["Trie"], marker="o", label="Trie")
plt.plot(data["TextSize"], data["Hash"], marker="o", label="Hash Map")

plt.xlabel("Text Size")
plt.ylabel("Runtime (microseconds)")
plt.title("Spell Checker Runtime vs Text Size")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig("runtime_graph.png")

# Graph without linear list
plt.figure(figsize=(8,5))
plt.plot(data["TextSize"], data["BST"], marker="o", label="BST")
plt.plot(data["TextSize"], data["Trie"], marker="o", label="Trie")
plt.plot(data["TextSize"], data["Hash"], marker="o", label="Hash Map")

plt.xlabel("Text Size")
plt.ylabel("Runtime (microseconds)")
plt.title("Spell Checker Runtime vs Text Size (Without Linear List)")
plt.legend()
plt.grid(True)
plt.tight_layout()
plt.savefig("runtime_graphNOLINEAR.png")

plt.show()