import csv
import matplotlib.pyplot as plt

data = {}

# Read CSV
with open("timing.csv", "r") as f:
    reader = csv.DictReader(f)
    for row in reader:
        row = {k.strip(): v for k, v in row.items()}

        P = int(row["P"])
        SIZE = int(row["Size"])
        t = float(row["Time"])

        if (P, SIZE) not in data:
            data[(P, SIZE)] = []

        data[(P, SIZE)].append(t)

# Unique values
P_values = sorted(set(p for (p, s) in data.keys()))
SIZE_values = sorted(set(s for (p, s) in data.keys()))

S1 = SIZE_values[0]   # 120
S2 = SIZE_values[1]   # 240

# Prepare data
data_S1 = [data[(p, S1)] for p in P_values]
data_S2 = [data[(p, S2)] for p in P_values]

# Positions
pos1 = [x - 0.25 for x in range(len(P_values))]
pos2 = [x + 0.25 for x in range(len(P_values))]

# Plot
plt.figure(figsize=(12, 7))
plt.style.use("ggplot")

bp1 = plt.boxplot(
    data_S1,
    positions=pos1,
    widths=0.35,
    patch_artist=True,
    showfliers=False,
    whis=(0, 100),
    boxprops=dict(facecolor="skyblue", edgecolor="black"),
    medianprops=dict(color="black", linewidth=2),
)

bp2 = plt.boxplot(
    data_S2,
    positions=pos2,
    widths=0.35,
    patch_artist=True,
    showfliers=False,
    whis=(0, 100),
    boxprops=dict(facecolor="orange", edgecolor="black"),
    medianprops=dict(color="black", linewidth=2),
)

# X-axis
plt.xticks(
    range(len(P_values)),
    [f"P={p}" for p in P_values],
    fontsize=13
)

# Labels
plt.xlabel("Number of Processes (P)", fontsize=14)
plt.ylabel("Execution Time (seconds)", fontsize=14)

# Title
plt.title(
    f"Execution Time Comparison\n(Grid Size {S1}³ vs {S2}³)",
    fontsize=16
)

# Legend
plt.legend(
    [bp1["boxes"][0], bp2["boxes"][0]],
    [f"Size = {S1}³", f"Size = {S2}³"],
    fontsize=12,
    loc="upper left"
)

# Y limit
all_times = []
for key in data:
    all_times += data[key]

ymax = max(all_times)
plt.ylim(0, ymax * 1.2)

# Annotate n=5
for i in range(len(P_values)):
    plt.text(pos1[i], max(data_S1[i]) * 1.02, "n=5",
             ha="center", fontsize=11, fontweight="bold")
    plt.text(pos2[i], max(data_S2[i]) * 1.02, "n=5",
             ha="center", fontsize=11, fontweight="bold")

# Footer
plt.figtext(
    0.5, 0.02,
    "Each box represents execution times over 5 runs",
    ha="center",
    fontsize=11
)

# Save
plt.savefig("boxplot.png", dpi=300)
print("Plot saved as boxplot.png")
