import matplotlib.pyplot as plt

# Dati dalla tabella
num_threads = [1, 5, 10, 25, 48, 50, 100]
speedup = [1, 4.87, 9.35, 15.81, 18.96, 1.80, 2.81]
efficiency = [s/t for s, t in zip(speedup, num_threads)]  # Efficienza = Speedup / Num_threads

# Creazione della figura con due sottografi
fig, axes = plt.subplots(1, 2, figsize=(12, 5))

# Primo grafico: Speedup
axes[0].plot(num_threads, speedup, 'bo-', markersize=6, label="Speedup")
axes[0].set_xlabel("Numero di Thread")
axes[0].set_ylabel("Speedup")
axes[0].set_title("Speedup strong")
axes[0].grid(True, linestyle="--", alpha=0.6)
axes[0].legend()

# Secondo grafico: Efficienza
axes[1].plot(num_threads, efficiency, 'ro-', markersize=6, label="Efficienza")
axes[1].set_xlabel("Numero di Thread")
axes[1].set_ylabel("Efficienza")
axes[1].set_title("Efficienza strong")
axes[1].set_ylim(0, 1.2)  # Imposta il limite dell'asse y da 0 a 0.5
axes[1].grid(True, linestyle="--", alpha=0.6)
axes[1].legend()

# Mostra i grafici
plt.tight_layout()
plt.show()

