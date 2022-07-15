import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

filename = "updown.csv"

df = pd.read_csv(filename)

index = range(1, len(df['x']) + 1)

plt.rcParams["figure.figsize"] = (20,10)


plt.plot(index, df['x'], 'g.', label='x', linestyle='solid', marker=',')
plt.plot(index, df['y'], 'b.', label='y', linestyle='solid', marker=',')
plt.plot(index, df['z'], 'r.', label='z', linestyle='solid', marker=',')
plt.title("Gyroscope")
plt.xlabel("Sample #")
plt.ylabel("Gyroscope (deg/sec)")
plt.legend()
plt.show()