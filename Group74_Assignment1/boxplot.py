import csv
import matplotlib.pyplot as plt

data = {}

with open("timing.csv", "r") as f:
    reader = csv.DictReader(f)
    for row in reader:
        row = {k.strip(): v for k, v in row.items()}

        P = int(row["P"])
        M = int(row["M"])
        t = float(row["Time"])

        if (P, M) not in data:
            data[(P, M)] = []

        data[(P, M)].append(t)

P_values = sorted(set(p for (p, m) in data.keys()))
M_values = sorted(set(m for (p, m) in data.keys()))

M1 = M_values[0]
M2 = M_values[1]

data_M1 = [data[(p, M1)] for p in P_values]
data_M2 = [data[(p, M2)] for p in P_values]

pos1 = [x - 0.25 for x in range(len(P_values))]
pos2 = [x + 0.25 for x in range(len(P_values))]

plt.figure(figsize=(12, 7))
plt.style.use("ggplot")

bp1 = plt.boxplot(
    data_M1,
    positions=pos1,
    widths=0.35,
    patch_artist=True,
    showfliers=False,
    whis=(0, 100),
    boxprops=dict(facecolor="skyblue", edgecolor="black"),
    medianprops=dict(color="black", linewidth=2),
)

bp2 = plt.boxplot(
    data_M2,
    positions=pos2,
    widths=0.35,
    patch_artist=True,
    showfliers=False,
    whis=(0, 100),
    boxprops=dict(facecolor="orange", edgecolor="black"),
    medianprops=dict(color="black", linewidth=2),
)

plt.xticks(
    range(len(P_values)),
    [f"P={p}" for p in P_values],
    fontsize=13
)

plt.xlabel("Number of Processes (P)", fontsize=14)
plt.ylabel("Execution Time (seconds)", fontsize=14)

plt.title(
    f"Execution Time Boxplot Comparison\n(M={M1} vs M={M2})",
    fontsize=16
)

plt.legend(
    [bp1["boxes"][0], bp2["boxes"][0]],
    [f"M = {M1}", f"M = {M2}"],
    fontsize=12,
    loc="upper left"
)

all_times = []
for key in data:
    all_times += data[key]

ymax = max(all_times)
plt.ylim(0, ymax * 1.2)

for i in range(len(P_values)):
    plt.text(pos1[i], max(data_M1[i]) * 1.02, "n=5",
             ha="center", fontsize=11, fontweight="bold")
    plt.text(pos2[i], max(data_M2[i]) * 1.02, "n=5",
             ha="center", fontsize=11, fontweight="bold")

plt.text(
    pos2[-1],
    max(data_M2[-1]) * 1.08,
    f"M={M2}",
    ha="center",
    fontsize=10,
    fontweight="bold",
    color="black"
)

plt.figtext(
    0.5, 0.02,
    "Each box represents execution times over 5 independent runs",
    ha="center",
    fontsize=11
)

plt.savefig("boxplot.png", dpi=300)
print("Plot saved as boxplot.png")

