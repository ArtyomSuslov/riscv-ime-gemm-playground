import re
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D

filename = "/home/artyom/ISPRAS/riscv-ime-gemm-playground/test-1/results.txt"

# Списки для данных
ms_naive, ks_naive, times_naive = [], [], []
ms_ime, ks_ime, times_ime = [], [], []

m, n, k = None, None, None

with open(filename, 'r') as f:
    for line in f:
        # Размеры
        match_size = re.match(r"Size:\s*(\d+)\s+(\d+)\s+(\d+)", line)
        if match_size:
            m, n, k = map(int, match_size.groups())
            continue

        # Naive время
        match_naive = re.match(r"Naive:\s*([\d.eE+-]+)", line)
        if match_naive and m is not None:
            times_naive.append(float(match_naive.group(1)))
            ms_naive.append(m)
            ks_naive.append(k)
            continue

        # IME время
        match_ime = re.match(r"IME:\s*([\d.eE+-]+)", line)
        if match_ime and m is not None:
            times_ime.append(float(match_ime.group(1)))
            ms_ime.append(m)
            ks_ime.append(k)
            # Сбрасываем после полной тройки
            m, n, k = None, None, None

# Строим 3D график
fig = plt.figure(figsize=(12, 8))
ax = fig.add_subplot(111, projection='3d')

# Naive - красные точки
ax.scatter(ms_naive, ks_naive, times_naive, c='red', label='Naive', s=50)
# IME - синие точки
ax.scatter(ms_ime, ks_ime, times_ime, c='blue', label='IME', s=50)

ax.set_xlabel('m')
ax.set_ylabel('k')
ax.set_zlabel('time (sec)')
ax.set_title('GEMM time vs matrix size (m x k)')
ax.legend()

plt.show()

