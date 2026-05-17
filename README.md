# MS-CFLP-CI - Practica 5 DAA

Implementacion en C++17 del Multi-Source Capacitated Facility Location Problem with Customer Incompatibilities.

Incluye:

- Parser de instancias `.dzn` del conjunto `Instaces_MS-CFLP-CI/Public`.
- Constructivo voraz basado en el algoritmo del enunciado y el paper de Gjergji et al. (k extra = 5 por defecto).
- Constructivo GRASP aleatorizado con LRC.
- GRASP completo con RVND sobre cuatro vecindarios.
- GVNS con perturbacion por reinsercion de clientes y VND con pesos de refuerzo.
- Validacion de demanda, capacidad e incompatibilidades.
- Exportacion CSV de resultados y fichero de solucion.

## Compilar

```bash
make
```

## Estructura del codigo

- `include/GreedySolver.hpp` / `src/GreedySolver.cpp`: algoritmo voraz.
- `include/GraspConstructiveSolver.hpp` / `src/GraspConstructiveSolver.cpp`: fase constructiva GRASP.
- `include/GraspSolver.hpp` / `src/GraspSolver.cpp`: GRASP completo.
- `include/GvnsSolver.hpp` / `src/GvnsSolver.cpp`: GVNS-RL.
- `include/HeuristicSolver.hpp` / `src/HeuristicSolver.cpp`: utilidades compartidas de construccion y reparacion.
- `src/LocalSearch.cpp`: vecindarios y VND/RVND compartidos.
- `include/Instance.hpp`, `include/Solution.hpp`, `include/RunTypes.hpp`: modelo de datos.

## Ejecutar una instancia

El modo principal es interactivo:

```bash
./ms_cflp_ci
```

El programa muestra el menu:

```text
1) Greedy
2) GRASP constructivo
3) GRASP completo
4) GVNS
5) Exit
```

Cada opcion ejecuta el algoritmo sobre las instancias publicas y muestra una tabla con el formato de resultados del enunciado.

Tambien se mantiene el modo por argumentos:

```bash
./ms_cflp_ci --instance Instaces_MS-CFLP-CI/Public/wlp01.dzn --algorithm greedy
./ms_cflp_ci --instance Instaces_MS-CFLP-CI/Public/wlp01.dzn --algorithm grasp-constructive
./ms_cflp_ci --instance Instaces_MS-CFLP-CI/Public/wlp01.dzn --algorithm grasp --grasp-iters 30 --ls-passes 120
./ms_cflp_ci --instance Instaces_MS-CFLP-CI/Public/wlp01.dzn --algorithm gvns --gvns-iters 50 --ls-passes 120 --time-limit 10
```

Algoritmos validos: `greedy`, `grasp-constructive`, `grasp`, `gvns`/`gvns-rl`, `all`.

## Ejecutar varias instancias

```bash
./ms_cflp_ci \
  --instances-dir Instaces_MS-CFLP-CI/Public \
  --algorithm all \
  --grasp-iters 15 \
  --gvns-iters 30 \
  --ls-passes 100 \
  --time-limit 10 \
  --output results/results.csv
```

## Exportar una solucion

```bash
./ms_cflp_ci \
  --instance Instaces_MS-CFLP-CI/Public/wlp01.dzn \
  --algorithm gvns \
  --time-limit 10 \
  --solution results/wlp01_gvns_solution.txt
```

El fichero de solucion lista instalaciones abiertas y asignaciones `cliente facility cantidad fraccion`.

## Opciones utiles

- `--seed N`: semilla aleatoria.
- `--extra N`: numero de instalaciones extra del constructivo inicial.
- `--lrc N`: tamano de la lista restringida de candidatos.
- `--time-limit S`: limite de segundos por instancia y algoritmo.
- `--kmax N`: numero maximo de estructuras de perturbacion de GVNS.

## Memoria

La memoria principal esta en:

```text
latex/Report P5 DAA 2025-2026/main.tex
```

En un entorno con TeX Live y Biber:

```bash
cd "latex/Report P5 DAA 2025-2026"
pdflatex main.tex
biber main
pdflatex main.tex
pdflatex main.tex
```

Las tablas se pueden ampliar con el CSV generado en `results/results.csv`.
