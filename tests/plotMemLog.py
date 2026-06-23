# ----------------------------=[ plotMemLog.py ]=---------------------------- #

# simply plot the `memory_log.csv` for further investigation

import pandas as pd
import matplotlib.pyplot as plt

LOG_FILE = "memory_log.csv"
OUTPUT_PLOT = "memory_usage_plot.png"

df = pd.read_csv(LOG_FILE)

df['timestamp'] = pd.to_datetime(df['timestamp'])

plt.figure(figsize=(12, 6))
plt.plot(df['elapsed_time_s'], df['memory_mb'], 'b-', linewidth=1.5)
plt.xlabel('Elapsed Time (s)')
plt.ylabel('Memory Usage (MB)')
plt.title('Memory Usage Over Time')
plt.grid(True, linestyle='--', alpha=0.7)

plt.tight_layout()

# plt.savefig(OUTPUT_PLOT, dpi=300)
# print(f"Plot saved as {OUTPUT_PLOT}")

plt.show()
