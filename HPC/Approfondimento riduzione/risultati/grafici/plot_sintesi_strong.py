import matplotlib.pyplot as plt

# Dati della versione MPI
num_threads_mpi = [1, 5, 10, 25, 50, 100]
speedup_mpi = [0.80, 1.38, 2.36, 2.53, 2.53, 2.53]
efficiency_mpi = [s/t for s, t in zip(speedup_mpi, num_threads_mpi)]

# Dati della versione OMP
num_threads_omp = [1, 5, 10, 25, 48, 50, 100]
speedup_omp = [1, 4.87, 9.35, 15.81, 18.96, 1.80, 2.81]
efficiency_omp = [s/t for s, t in zip(speedup_omp, num_threads_omp)]

# Creazione della figura con due sottografi
fig, axes = plt.subplots(1, 2, figsize=(12, 5))

# Primo grafico: Speedup
axes[0].plot(num_threads_mpi, speedup_mpi, 'bo-', markersize=6, label="Speedup MPI")
axes[0].plot(num_threads_omp, speedup_omp, 'go-', markersize=6, label="Speedup OMP")
axes[0].axvline(48, color='black', linestyle='--', linewidth=1, label="Limite 48 core")
axes[0].set_xlabel("Numero di Thread")
axes[0].set_ylabel("Speedup")
axes[0].set_title("Speedup strong scaling")
axes[0].set_ylim(0.7, max(speedup_omp) + 1) 
axes[0].grid(True, linestyle="--", alpha=0.6)
axes[0].legend()

# Secondo grafico: Efficienza
axes[1].plot(num_threads_mpi, efficiency_mpi, 'bo-', markersize=6, label="Efficienza MPI")
axes[1].plot(num_threads_omp, efficiency_omp, 'go-', markersize=6, label="Efficienza OMP")
axes[1].axvline(48, color='black', linestyle='--', linewidth=1, label="Limite 48 core")
axes[1].set_xlabel("Numero di Thread")
axes[1].set_ylabel("Efficienza")
axes[1].set_title("Efficienza strong scaling")
axes[1].set_ylim(0, 1.2)
axes[1].grid(True, linestyle="--", alpha=0.6)
axes[1].legend()

# Mostra i grafici
plt.tight_layout()
plt.show()
