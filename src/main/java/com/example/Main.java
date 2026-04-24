package com.example;

import java.nio.file.Files;
import java.io.IOException;
import java.nio.file.Path;
import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;
import java.util.Optional;
import java.util.Scanner;

/**
 * Entry point for the DAA MS-CFLP-CI CLI application.
 */
public class Main {

	private static final String MENU = String.join(
			System.lineSeparator(),
			"MS-CFLP-CI",
			"1) Greedy",
			"2) GRASP",
			"3) GVNS",
			"4) Exit"
	);

	private static final String USAGE = String.join(
			System.lineSeparator(),
			"Usage:",
			"  java -jar app.jar <command> [args]",
			"",
			"Commands:",
			"  solve <instance-file>   Solve one MS-CFLP-CI instance",
			"  demo                    Run a built-in demo instance",
			"  greet <name>            Print a greeting",
			"  --help                  Show this help message"
	);

	public static void main(String[] args) {
		if (args != null && args.length > 0) {
			System.out.println(run(args));
			return;
		}
		runInteractiveMenu();
	}

	public static String run(String[] args) {
		if (args == null || args.length == 0) {
			return USAGE;
		}

		return switch (args[0]) {
			case "--help" -> USAGE;
			case "greet" -> greet(args);
			case "demo" -> solveDemo();
			case "solve" -> solveFromFile(args);
			default -> "Unknown command: " + args[0] + System.lineSeparator() + USAGE;
		};
	}

	private static void runInteractiveMenu() {
		try (Scanner scanner = new Scanner(System.in)) {
			while (true) {
				System.out.println(MENU);
				System.out.print("Choose an option: ");
				String choice = scanner.nextLine().trim();
				switch (choice) {
					case "1" -> runGreedyMenu(scanner);
					case "2" -> runGraspMenu(scanner);
					case "3" -> runGvnsMenu(scanner);
					case "4" -> {
						System.out.println("Bye.");
						return;
					}
					default -> System.out.println("Invalid option. Choose 1, 2, 3, or 4.");
				}
			}
		}
	}

	private static void runGreedyMenu(Scanner scanner) {
		Path path = askPath(scanner, "Path to .dzn file or folder [instances]: ", Path.of("instances"));
		GreedySolver solver = new GreedySolver();
		printGreedyTable(path, solver);
	}

	private static void runGraspMenu(Scanner scanner) {
		Path path = askPath(scanner, "Path to .dzn file or folder [instances]: ", Path.of("instances"));
		int iterations = askInt(scanner, "Iterations [25]: ", 25);
		GraspSolver solver = new GraspSolver();
		processInstances(path, (name, instance) -> solveAndFormat(name, instance, iterations, solver));
	}

	private static void runGvnsMenu(Scanner scanner) {
		Path path = askPath(scanner, "Path to .dzn file or folder [instances]: ", Path.of("instances"));
		int iterations = askInt(scanner, "Iterations [25]: ", 25);
		GvnsSolver solver = new GvnsSolver();
		processInstances(path, (name, instance) -> solveAndFormat(name, instance, iterations, solver));
	}

	private static Path askPath(Scanner scanner, String prompt, Path defaultValue) {
		System.out.print(prompt);
		String input = scanner.nextLine().trim();
		if (input.isEmpty()) {
			return defaultValue;
		}
		return resolveInputPath(Path.of(input), defaultValue);
	}

	private static Path resolveInputPath(Path inputPath, Path defaultBase) {
		if (Files.exists(inputPath)) {
			return inputPath;
		}
		if (!inputPath.isAbsolute() && inputPath.getNameCount() == 1) {
			Path candidate = defaultBase.resolve(inputPath);
			if (Files.exists(candidate)) {
				return candidate;
			}
		}
		return inputPath;
	}

	private static int askInt(Scanner scanner, String prompt, int defaultValue) {
		System.out.print(prompt);
		String input = scanner.nextLine().trim();
		if (input.isEmpty()) {
			return defaultValue;
		}
		return Integer.parseInt(input);
	}

	private static void processInstances(Path path, InstanceRunner runner) {
		try {
			if (Files.isDirectory(path)) {
				List<Path> files = new ArrayList<>();
				try (var stream = Files.list(path)) {
					stream.filter(p -> p.getFileName().toString().toLowerCase().endsWith(".dzn"))
							.sorted(Comparator.comparing(Path::getFileName))
							.forEach(files::add);
				}
				if (files.isEmpty()) {
					System.out.println("No .dzn files found in directory: " + path);
					return;
				}
				for (Path file : files) {
					ProblemInstance instance = new InstanceParser().parse(file);
					System.out.println(runner.run(file.getFileName().toString(), instance));
					System.out.println();
				}
				return;
			}

			ProblemInstance instance = new InstanceParser().parse(path);
			System.out.println(runner.run(path.getFileName().toString(), instance));
		} catch (IOException | RuntimeException e) {
			System.out.println("Error reading instance: " + e.getMessage());
		}
	}

	private static void printGreedyTable(Path path, GreedySolver solver) {
		try {
			List<Path> files = collectInstanceFiles(path);
			if (files.isEmpty()) {
				System.out.println("No .dzn files found in directory: " + path);
				return;
			}

			List<GreedyRow> rows = new ArrayList<>();
			for (Path file : files) {
				ProblemInstance instance = new InstanceParser().parse(file);
				long start = System.nanoTime();
				Optional<Solution> maybeSolution = solver.solve(instance);
				double cpuSeconds = (System.nanoTime() - start) / 1_000_000_000.0;
				rows.add(GreedyRow.from(file.getFileName().toString(), instance, maybeSolution, cpuSeconds));
			}

			System.out.println(renderGreedyTable(rows));
		} catch (IOException | RuntimeException e) {
			System.out.println("Error reading instance: " + e.getMessage());
		}
	}

	private static List<Path> collectInstanceFiles(Path path) throws IOException {
		List<Path> files = new ArrayList<>();
		if (Files.isDirectory(path)) {
			try (var stream = Files.list(path)) {
				stream.filter(p -> p.getFileName().toString().toLowerCase().endsWith(".dzn"))
						.sorted(Comparator.comparing(Path::getFileName))
						.forEach(files::add);
			}
			return files;
		}
		files.add(path);
		return files;
	}

	private static String renderGreedyTable(List<GreedyRow> rows) {
		StringBuilder sb = new StringBuilder();
		sb.append(String.format("%-14s %8s %11s %13s %12s %10s %12s%n",
				"Instancia", "|open|", "Coste Fijo", "Coste Asig.", "Coste Total", "Incomp.", "CPU_Time (s)"));
		sb.append(repeat('-', 92)).append(System.lineSeparator());

		double openSum = 0.0;
		double fixedSum = 0.0;
		double transportSum = 0.0;
		double totalSum = 0.0;
		double incompSum = 0.0;
		double cpuSum = 0.0;

		for (GreedyRow row : rows) {
			sb.append(String.format("%-14s %8d %11.3f %13.3f %12.3f %10d %12.6f%n",
					row.name(), row.openCount(), row.fixedCost(), row.transportCost(), row.totalCost(), row.incompatibilities(), row.cpuSeconds()));
			openSum += row.openCount();
			fixedSum += row.fixedCost();
			transportSum += row.transportCost();
			totalSum += row.totalCost();
			incompSum += row.incompatibilities();
			cpuSum += row.cpuSeconds();
		}

		double n = rows.size();
		sb.append(repeat('-', 92)).append(System.lineSeparator());
		sb.append(String.format("%-14s %8.2f %11.3f %13.3f %12.3f %10.2f %12.6f%n",
				"Promedio", openSum / n, fixedSum / n, transportSum / n, totalSum / n, incompSum / n, cpuSum / n));
		return sb.toString();
	}

	private static String repeat(char ch, int count) {
		StringBuilder sb = new StringBuilder(count);
		for (int i = 0; i < count; i++) {
			sb.append(ch);
		}
		return sb.toString();
	}

	private static String greet(String[] args) {
		if (args.length < 2 || args[1].isBlank()) {
			return "Missing name." + System.lineSeparator() + USAGE;
		}
		return "Hello, " + args[1] + "!";
	}

	private static String solveFromFile(String[] args) {
		if (args.length < 2) {
			return "Missing instance file path." + System.lineSeparator() + USAGE;
		}
		try {
			Path path = Path.of(args[1]);
			if (Files.isDirectory(path)) {
				return solveDirectory(path);
			}
			ProblemInstance instance = new InstanceParser().parse(path);
			return solveAndFormat(path.getFileName().toString(), instance);
		} catch (IOException | RuntimeException e) {
			return "Error reading instance: " + e.getMessage();
		}
	}

	private static String solveDirectory(Path directory) throws IOException {
		List<Path> files = new ArrayList<>();
		try (var stream = Files.list(directory)) {
			stream.filter(path -> path.getFileName().toString().toLowerCase().endsWith(".dzn"))
					.sorted(Comparator.comparing(Path::getFileName))
					.forEach(files::add);
		}

		if (files.isEmpty()) {
			return "No .dzn files found in directory: " + directory;
		}

		StringBuilder sb = new StringBuilder();
		for (Path file : files) {
			if (sb.length() > 0) {
				sb.append(System.lineSeparator()).append(System.lineSeparator());
			}
			sb.append("File: ").append(file.getFileName()).append(System.lineSeparator());
			try {
				ProblemInstance instance = new InstanceParser().parse(file);
				sb.append(solveAndFormat(file.getFileName().toString(), instance));
			} catch (RuntimeException | IOException e) {
				sb.append("Error reading instance: ").append(e.getMessage());
			}
		}
		return sb.toString();
	}

	private static String solveDemo() {
		ProblemInstance demo = demoInstance();
		return solveAndFormat("demo", demo);
	}

	private static String solveAndFormat(String name, ProblemInstance instance) {
		return solveAndFormat(name, instance, new GreedySolver().solve(instance));
	}

	private static String solveAndFormat(String name, ProblemInstance instance, Optional<Solution> maybeSolution) {
		return formatSolution(name, instance, maybeSolution);
	}

	private static String solveAndFormat(String name, ProblemInstance instance, int iterations, GraspSolver solver) {
		Optional<Solution> maybeSolution = solver.solve(instance, iterations);
		return formatSolution(name, instance, maybeSolution);
	}

	private static String solveAndFormat(String name, ProblemInstance instance, int iterations, GvnsSolver solver) {
		Optional<Solution> maybeSolution = solver.solve(instance, iterations);
		return formatSolution(name, instance, maybeSolution);
	}

	private static String formatSolution(String name, ProblemInstance instance, Optional<Solution> maybeSolution) {
		if (maybeSolution.isEmpty()) {
			return name + System.lineSeparator() + "No feasible solution found.";
		}

		Solution solution = maybeSolution.get();
		boolean feasible = SolutionValidator.isFeasible(instance, solution);
		return String.join(
				System.lineSeparator(),
				"Instance: " + name,
				"Feasible: " + feasible,
				String.format("Cost: %.3f", solution.totalCost(instance)),
				"Assignment: " + solution.assignmentSummary()
		);
	}

	private interface InstanceRunner {
		String run(String name, ProblemInstance instance);
	}

	private static ProblemInstance demoInstance() {
		int[] capacities = {8, 7};
		int[] demands = {3, 3, 2, 2};
		double[] fixed = {8.0, 7.0};
		double[][] service = {
				{2.0, 2.5, 3.0, 4.0},
				{3.0, 2.0, 2.0, 2.5}
		};
		boolean[][] incompatible = new boolean[demands.length][demands.length];
		incompatible[0][1] = true;
		incompatible[1][0] = true;
		return new ProblemInstance(capacities, demands, fixed, service, incompatible);
	}
}