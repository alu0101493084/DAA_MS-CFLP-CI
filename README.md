# DAA MS-CFLP-CI

Java implementation of constructive and local-search heuristics for the Multi-Source Capacitated Facility Location Problem with Customer Incompatibilities (MS-CFLP-CI).

---

## Prerequisites

| Tool | Recommended version |
|------|---------------------|
| JDK  | 17+ (21 LTS recommended) — [Eclipse Temurin](https://adoptium.net/) |
| Maven | 3.9+ |

```bash
java -version   # should print 21.x.x
mvn  -version   # should print 3.9.x
```

---

## Project structure

```
.
├── pom.xml                          # Maven build descriptor
├── src/
│   ├── main/java/com/example/
│   │   ├── Main.java                # CLI entry point
│   │   ├── MsCflpCiSolver.java      # Constructive + local-search workflow
│   │   ├── ConstructiveSolver.java  # Greedy constructive heuristic
│   │   ├── LocalSearchSolver.java   # Relocate-based local search
│   │   ├── SolutionValidator.java   # Capacity/incompatibility checks
│   │   └── InstanceParser.java      # Instance parser
│   └── test/java/com/example/
│       ├── MainTest.java            # CLI tests
│       └── SolverFeasibilityTest.java
└── .github/workflows/ci.yml         # GitHub Actions CI
```

---

## Build

```bash
mvn -B package -DskipTests
```

The runnable jar is created at `target/daa-ms-cflp-ci-1.0-SNAPSHOT.jar`.

---

## Run

```bash
mvn exec:java
mvn exec:java -Dexec.args="--help"
mvn exec:java -Dexec.args="demo"
mvn exec:java -Dexec.args="solve instances/sample.txt"
```

### Example output

```
% mvn exec:java -Dexec.args="demo"
Feasible: true
Cost: 22.000
Assignment: C0->F0 C1->F1 C2->F1 C3->F0

% mvn exec:java -Dexec.args="--help"
Usage:
  java -jar app.jar <command> [args]

Commands:
  solve <instance-file>   Solve one MS-CFLP-CI instance
  demo                    Run a built-in demo instance
  greet <name>            Print a greeting
  --help                  Show this help message
...
```

## Instance format

The parser expects whitespace-separated tokens in this order:

1) Number of facilities `F` and customers `C`
2) `F` capacities
3) `F` fixed opening costs
4) `C` customer demands
5) `F * C` service costs, row-wise by facility
6) Number of incompatibility pairs `K`
7) `K` lines with customer indices `i j` (0-based)

Example:

```text
2 4
8 7
8 7
3 3 2 2
2 2.5 3 4
3 2 2 2.5
1
0 1
```

---

## Test

```bash
mvn -B test
```

---

## CI

GitHub Actions runs `mvn -B test` on every push and pull request targeting `main`
(see [`.github/workflows/ci.yml`](.github/workflows/ci.yml)).
