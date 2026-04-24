package com.example;

/**
 * Entry point for the DAA MS-CFLP-CI CLI application.
 */
public class Main {

    static final String USAGE = """
            Usage:
              java -jar app.jar <command> [args]

            Commands:
              greet <name>   Print a greeting for the given name
              --help         Show this help message

            Examples:
              java -jar app.jar greet Alice
              java -jar app.jar --help
            """;

    public static void main(String[] args) {
        System.out.println(run(args));
    }

    /**
     * Core logic, separated from {@code main} so it can be unit-tested easily.
     *
     * @param args command-line arguments
     * @return the string that would be printed to stdout
     */
    public static String run(String[] args) {
        if (args == null || args.length == 0 || "--help".equals(args[0])) {
            return USAGE;
        }

        return switch (args[0]) {
            case "greet" -> {
                if (args.length < 2 || args[1].isBlank()) {
                    yield "Error: 'greet' requires a name argument.\n" + USAGE;
                }
                yield "Hello, " + args[1] + "!";
            }
            default -> "Unknown command: '" + args[0] + "'\n" + USAGE;
        };
    }
}
