# ---------------------------=[ plotMemUsage.py ]=--------------------------- #

# live-plotting of webserv memory consumption

import subprocess
import time
import matplotlib.pyplot as plt
import csv
from datetime import datetime
import matplotlib.animation as animation

# cfg
INTERVAL_MS = 100  # 0.1s interval
COMMAND = "ps -p $(pgrep webserv) -o rss | tail -n 1"
LOG_FILE = "memory_log.csv"

timestamps = []
memory_usage_mb = []

fig, ax = plt.subplots()
line, = ax.plot([], [], 'b-')
ax.set_xlabel('Time (s)')
ax.set_ylabel('Memory (MB)')
ax.set_title('Live Memory Usage of WebServ')
ax.grid(True)

# does not work as planned, so far
fig.canvas.mpl_connect('scroll_event', lambda event: ax.set_xlim(
    ax.get_xlim()[0] + event.step * 10, ax.get_xlim()[1] + event.step * 10))
fig.canvas.mpl_connect('button_press_event', lambda event: ax.set_xlim(ax.get_xlim()[
                       0] + (event.xdata - ax.get_xlim()[0]) * 0.1, ax.get_xlim()[1] + (event.xdata - ax.get_xlim()[0]) * 0.1))


def get_memory_usage():
    try:
        result = subprocess.run(COMMAND, shell=True, check=True,
                                stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        return int(result.stdout.strip()) / 1024
    except (subprocess.CalledProcessError, ValueError):
        return None


def init():
    ax.set_xlim(0, 10)
    ax.set_ylim(0, 100)
    return line,


# write header to file
with open(LOG_FILE, mode='w', newline='') as f:
    writer = csv.writer(f)
    writer.writerow(["timestamp", "elapsed_time_s", "memory_mb"])


def update(frame):
    mem = get_memory_usage()
    if mem is not None:
        now = time.time()
        timestamps.append(now)
        memory_usage_mb.append(mem)

        x = [t - timestamps[0] for t in timestamps]
        y = memory_usage_mb

        line.set_data(x, y)

        # auto-scale y-axis
        if y:
            max_y = max(y) * 1.1
            ax.set_ylim(0, max_y)

        # auto-scale x-axis
        if len(x) > 1:
            ax.set_xlim(0, x[-1])

        with open(LOG_FILE, mode='a', newline='') as f:
            writer = csv.writer(f)
            writer.writerow([
                datetime.now().isoformat(),
                x[-1],
                mem
            ])

    return line,


ani = animation.FuncAnimation(
    fig, update, init_func=init, interval=INTERVAL_MS, blit=False, cache_frame_data=False)

plt.show()
