import matplotlib.pyplot as plt

# Dati della versione MPI
num_threads_mpi = [1, 4, 9, 16, 25, 36]
speedup_mpi = [0.82, 1.83, 2.35, 2.60, 2.54, 2.56]
efficiency_mpi = [s/t for s, t in zip(speedup_mpi, num_threads_mpi)]

# Dati della versione OMP
num_threads_omp = [1, 4, 9, 16, 25, 36]
speedup_omp = [1, 3.99, 8.98, 15.78, 24.25, 34.20]
efficiency_omp = [s/t for s, t in zip(speedup_omp, num_threads_omp)]

# Creazione della figura con due sottografi
fig, axes = plt.subplots(1, 2, figsize=(12, 5))

# Primo grafico: Speedup
axes[0].plot(num_threads_mpi, speedup_mpi, 'bo-', markersize=6, label="Speedup MPI")
axes[0].plot(num_threads_omp, speedup_omp, 'go-', markersize=6, label="Speedup OMP")
axes[0].set_xlabel("Numero di Thread")
axes[0].set_ylabel("Speedup")
axes[0].set_title("Speedup weak scaling")
axes[0].set_ylim(0.7, max(speedup_omp) + 1) 
axes[0].grid(True, linestyle="--", alpha=0.6)
axes[0].legend()

# Secondo grafico: Efficienza
axes[1].plot(num_threads_mpi, efficiency_mpi, 'bo-', markersize=6, label="Efficienza MPI")
axes[1].plot(num_threads_omp, efficiency_omp, 'go-', markersize=6, label="Efficienza OMP")
axes[1].set_xlabel("Numero di Thread")
axes[1].set_ylabel("Efficienza")
axes[1].set_title("Efficienza weak scaling")
axes[1].set_ylim(0, 1.2)
axes[1].grid(True, linestyle="--", alpha=0.6)
axes[1].legend()

# Mostra i grafici
plt.tight_layout()
plt.show()
