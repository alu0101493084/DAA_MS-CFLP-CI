# DAA MS-CFLP-CI

A minimal Java CLI application scaffold for a university project.

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
│   │   └── Main.java                # CLI entry point
│   └── test/java/com/example/
│       └── MainTest.java            # JUnit 5 unit tests
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
java -jar target/daa-ms-cflp-ci-1.0-SNAPSHOT.jar --help
java -jar target/daa-ms-cflp-ci-1.0-SNAPSHOT.jar greet Alice
```

### Example output

```
$ java -jar target/daa-ms-cflp-ci-1.0-SNAPSHOT.jar greet Alice
Hello, Alice!

$ java -jar target/daa-ms-cflp-ci-1.0-SNAPSHOT.jar --help
Usage:
  java -jar app.jar <command> [args]

Commands:
  greet <name>   Print a greeting for the given name
  --help         Show this help message
...
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
