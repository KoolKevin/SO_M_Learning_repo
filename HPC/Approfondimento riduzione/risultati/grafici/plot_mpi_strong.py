import matplotlib.pyplot as plt # type: ignore

# Dati dalla tabella
num_threads = [1, 5, 10, 25, 50, 100]
speedup = [0.80, 1.38, 2.36, 2.53, 2.53, 2.53]
efficiency = [s/t for s, t in zip(speedup, num_threads)]  # Efficienza = Speedup / Num_threads

# Creazione della figura con due sottografi
fig, axes = plt.subplots(1, 2, figsize=(12, 5))

# Primo grafico: Speedup
axes[0].plot(num_threads, speedup, 'bo-', markersize=6, label="Speedup")
axes[0].set_xlabel("Numero di Thread")
axes[0].set_ylabel("Speedup")
axes[0].set_title("Speedup strong")
axes[0].set_ylim(0.7, 10.0) 
axes[0].grid(True, linestyle="--", alpha=0.6)
axes[0].legend()
# axes[0].set_xscale('log')  # Scala logaritmica sull'asse x

# Secondo grafico: Efficienza
axes[1].plot(num_threads, efficiency, 'ro-', markersize=6, label="Efficienza")
axes[1].set_xlabel("Numero di Thread")
axes[1].set_ylabel("Efficienza")
axes[1].set_title("Efficienza strong")
axes[1].set_ylim(0, 1.2)
axes[1].grid(True, linestyle="--", alpha=0.6)
axes[1].legend()
# axes[1].set_xscale('log')  # Scala logaritmica sull'asse x

# Mostra i grafici
plt.tight_layout()
plt.show()

