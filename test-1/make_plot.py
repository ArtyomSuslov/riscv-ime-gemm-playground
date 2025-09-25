import re
import pandas as pd
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

def parse_results(filename):
    data = []
    with open(filename, "r") as f:
        lines = f.readlines()

    size, mode = None, None
    for line in lines:
        line = line.strip()
        if line.startswith("Size:"):
            m = re.findall(r"m=(\d+), n=(\d+), k=(\d+)", line)
            if m:
                m, n, k = map(int, m[0])
                size = (m, n, k)
        elif line.startswith("Mode:"):
            mode = line.split(":")[1].strip()
        elif line.startswith(("Naive", "RVV", "IME")):
            m = re.match(r"(\w+):\s+([\d\.]+)\s+s\s+\(([\d\.]+)\s+GOPS", line)
            if m and size:
                method, time, gops = m.groups()
                m_, n_, k_ = size
                data.append({
                    "m": m_,
                    "n": n_,
                    "k": k_,
                    "mode": mode,
                    "method": method,
                    "time": float(time),
                    "gops": float(gops),
                })
    return pd.DataFrame(data)


df = parse_results(input())

# фильтруем только случаи, где m=n
df = df[df["m"] == df["n"]]

# --- 3D-график ---
fig = plt.figure(figsize=(10,7))
ax = fig.add_subplot(111, projection="3d")

colors = {"Naive": "red", "RVV": "blue", "IME": "green"}
markers = {"with_packing": "o", "pre_packed": "^"}

for (method, mode), subset in df.groupby(["method", "mode"]):
    ax.scatter(
        subset["m"], subset["k"], subset["gops"],
        c=colors[method],
        marker=markers[mode],
        label=f"{method} ({mode})"
    )

ax.set_xlabel("m = n")
ax.set_ylabel("k")
ax.set_zlabel("GOPS")
ax.set_title("3D график производительности (только m=n)")
ax.legend()
plt.tight_layout()
plt.show()
