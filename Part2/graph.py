import pandas as pd
import matplotlib.pyplot as plt

data = pd.read_csv("hash_benchmark.csv")

x_labels = [str(x) for x in data["LoadFactor"]]
x_pos = range(len(data["LoadFactor"]))

def make_graph(columns, title, ylabel, filename, log_scale=False):
    plt.figure(figsize=(10, 6))

    for column, label in columns:
        plt.plot(x_pos, data[column], marker="o", label=label)

    plt.xlabel("Load Factor")
    plt.ylabel(ylabel)
    plt.title(title)

    plt.xticks(x_pos, x_labels, rotation=45)

    if log_scale:
        plt.yscale("log")
        plt.ylabel(ylabel + " (log scale)")

    plt.legend()
    plt.grid(True)
    plt.tight_layout()
    plt.savefig(filename, dpi=300)
    plt.show()


make_graph(
    [
        ("SeparateChaining_Insert", "Separate Chaining"),
        ("LinearProbing_Insert", "Linear Probing"),
        ("DoubleHashing_Insert", "Double Hashing")
    ],
    "Insertion Time vs Load Factor",
    "Insertion Time (microseconds)",
    "insert_time.png"
)

make_graph(
    [
        ("SeparateChaining_SuccessfulSearch", "Separate Chaining"),
        ("LinearProbing_SuccessfulSearch", "Linear Probing"),
        ("DoubleHashing_SuccessfulSearch", "Double Hashing")
    ],
    "Successful Search Time vs Load Factor",
    "Search Time (microseconds)",
    "successful_search.png"
)

make_graph(
    [
        ("SeparateChaining_UnsuccessfulSearch", "Separate Chaining"),
        ("LinearProbing_UnsuccessfulSearch", "Linear Probing"),
        ("DoubleHashing_UnsuccessfulSearch", "Double Hashing")
    ],
    "Unsuccessful Search Time vs Load Factor",
    "Search Time (microseconds)",
    "unsuccessful_search.png",
    log_scale=True
)

print("Graphs generated successfully.")