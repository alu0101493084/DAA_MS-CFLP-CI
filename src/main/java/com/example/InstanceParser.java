package com.example;

import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Locale;
import java.util.List;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * Parser for plain-text MS-CFLP-CI instances.
 */
public class InstanceParser {

    private static final Pattern INT_ASSIGNMENT = Pattern.compile("(?m)^\\s*([A-Za-z_][A-Za-z0-9_]*)\\s*=\\s*([0-9]+)\\s*;\\s*$");
    private static final Pattern ARRAY_ASSIGNMENT = Pattern.compile("(?s)([A-Za-z_][A-Za-z0-9_]*)\\s*=\\s*\\[(.*?)\\]\\s*;\\s*");

    public ProblemInstance parse(Path path) throws IOException {
        List<String> lines = Files.readAllLines(path);
        return parseContent(String.join(System.lineSeparator(), lines));
    }

    public ProblemInstance parseString(String content) {
        return parseContent(content);
    }

    private ProblemInstance parseContent(String content) {
        if (content.contains("Warehouses") || content.contains("Stores") || content.contains("SupplyCost")) {
            return parseDzn(content);
        }

        String[] lines = content.split("\\R");
        List<String> list = new ArrayList<>(lines.length);
        for (String line : lines) {
            list.add(line);
        }
        return parsePlainLines(list);
    }

    private ProblemInstance parsePlainLines(List<String> lines) {
        ArrayDeque<String> tokens = new ArrayDeque<>();
        for (String raw : lines) {
            String line = raw.trim();
            if (line.isEmpty() || line.startsWith("#")) {
                continue;
            }
            for (String token : line.split("\\s+")) {
                tokens.add(token);
            }
        }

        int facilities = nextInt(tokens, "number of facilities");
        int customers = nextInt(tokens, "number of customers");

        int[] capacities = new int[facilities];
        for (int f = 0; f < facilities; f++) {
            capacities[f] = nextInt(tokens, "capacity[" + f + "]");
        }

        double[] fixedCosts = new double[facilities];
        for (int f = 0; f < facilities; f++) {
            fixedCosts[f] = nextDouble(tokens, "fixedCost[" + f + "]");
        }

        int[] demands = new int[customers];
        for (int c = 0; c < customers; c++) {
            demands[c] = nextInt(tokens, "demand[" + c + "]");
        }

        double[][] serviceCosts = new double[facilities][customers];
        for (int f = 0; f < facilities; f++) {
            for (int c = 0; c < customers; c++) {
                serviceCosts[f][c] = nextDouble(tokens, "serviceCost[" + f + "][" + c + "]");
            }
        }

        boolean[][] incompatible = new boolean[customers][customers];
        int pairs = nextInt(tokens, "incompatibilityPairs");
        for (int k = 0; k < pairs; k++) {
            int i = nextInt(tokens, "incompatibilityA[" + k + "]");
            int j = nextInt(tokens, "incompatibilityB[" + k + "]");
            if (i < 0 || i >= customers || j < 0 || j >= customers) {
                throw new IllegalArgumentException("Incompatibility index out of range");
            }
            if (i == j) {
                continue;
            }
            incompatible[i][j] = true;
            incompatible[j][i] = true;
        }

        return new ProblemInstance(capacities, demands, fixedCosts, serviceCosts, incompatible);
    }

    private ProblemInstance parseDzn(String content) {
        String normalized = content.replace("\r", "");

        int warehouses = readIntAssignment(normalized, "Warehouses");
        int stores = readIntAssignment(normalized, "Stores");

        int[] capacities = readIntArray(normalized, "Capacity", warehouses);
        double[] fixedCosts = readDoubleArray(normalized, "FixedCost", warehouses);
        int[] demands = readIntArray(normalized, "Goods", stores);
        double[][] serviceCosts = readDoubleMatrixTransposed(normalized, "SupplyCost", stores, warehouses);

        boolean[][] incompatible = new boolean[stores][stores];
        int incompatibleCount = readIntAssignment(normalized, "Incompatibilities");
        int[][] pairs = readIntPairMatrix(normalized, "IncompatiblePairs", incompatibleCount);
        for (int[] pair : pairs) {
            int i = pair[0] - 1;
            int j = pair[1] - 1;
            if (i < 0 || i >= stores || j < 0 || j >= stores) {
                throw new IllegalArgumentException("Incompatible pair index out of range");
            }
            if (i == j) {
                continue;
            }
            incompatible[i][j] = true;
            incompatible[j][i] = true;
        }

        return new ProblemInstance(capacities, demands, fixedCosts, serviceCosts, incompatible);
    }

    private static int readIntAssignment(String content, String key) {
        Matcher matcher = Pattern.compile("(?m)^\\s*" + Pattern.quote(key) + "\\s*=\\s*([0-9]+)\\s*;\\s*$").matcher(content);
        if (!matcher.find()) {
            throw new IllegalArgumentException("Missing integer assignment for " + key);
        }
        return Integer.parseInt(matcher.group(1));
    }

    private static int[] readIntArray(String content, String key, int expectedLength) {
        String block = readBlock(content, key);
        String[] parts = splitValues(block);
        if (parts.length != expectedLength) {
            throw new IllegalArgumentException(String.format(Locale.ROOT, "Array %s length %d does not match expected %d", key, parts.length, expectedLength));
        }
        int[] values = new int[parts.length];
        for (int i = 0; i < parts.length; i++) {
            values[i] = Integer.parseInt(parts[i]);
        }
        return values;
    }

    private static double[] readDoubleArray(String content, String key, int expectedLength) {
        String block = readBlock(content, key);
        String[] parts = splitValues(block);
        if (parts.length != expectedLength) {
            throw new IllegalArgumentException(String.format(Locale.ROOT, "Array %s length %d does not match expected %d", key, parts.length, expectedLength));
        }
        double[] values = new double[parts.length];
        for (int i = 0; i < parts.length; i++) {
            values[i] = Double.parseDouble(parts[i]);
        }
        return values;
    }

    private static double[][] readDoubleMatrixTransposed(String content, String key, int rows, int cols) {
        String block = readMatrixBlock(content, key);
        String[] rowParts = block.split("\\|");
        List<String> rowsValues = new ArrayList<>();
        for (String row : rowParts) {
            String trimmed = row.trim();
            if (trimmed.isEmpty()) {
                continue;
            }
            if (trimmed.equals("[")) {
                continue;
            }
            rowsValues.add(trimmed.replace("[", "").replace("]", ""));
        }

        if (rowsValues.size() != rows) {
            throw new IllegalArgumentException(String.format(Locale.ROOT, "Matrix %s row count %d does not match expected %d", key, rowsValues.size(), rows));
        }

        double[][] matrix = new double[cols][rows];
        for (int r = 0; r < rows; r++) {
            String[] parts = splitValues(rowsValues.get(r));
            if (parts.length != cols) {
                throw new IllegalArgumentException(String.format(Locale.ROOT, "Matrix %s row %d length %d does not match expected %d", key, r, parts.length, cols));
            }
            for (int c = 0; c < cols; c++) {
                matrix[c][r] = Double.parseDouble(parts[c]);
            }
        }
        return matrix;
    }

    private static int[][] readIntPairMatrix(String content, String key, int expectedRows) {
        String block = readMatrixBlock(content, key);
        String[] rowParts = block.split("\\|");
        List<int[]> pairs = new ArrayList<>();
        for (String row : rowParts) {
            String trimmed = row.trim();
            if (trimmed.isEmpty()) {
                continue;
            }
            if (trimmed.equals("[")) {
                continue;
            }
            String cleaned = trimmed.replace("[", "").replace("]", "").trim();
            String[] parts = cleaned.split("\\s*,\\s*");
            if (parts.length == 1 && cleaned.contains(" ")) {
                parts = cleaned.split("\\s+");
            }
            if (parts.length != 2) {
                throw new IllegalArgumentException("Invalid pair row in " + key + ": " + cleaned);
            }
            pairs.add(new int[]{Integer.parseInt(parts[0]), Integer.parseInt(parts[1])});
        }

        if (pairs.size() != expectedRows) {
            throw new IllegalArgumentException(String.format(Locale.ROOT, "Matrix %s row count %d does not match expected %d", key, pairs.size(), expectedRows));
        }

        return pairs.toArray(new int[0][]);
    }

    private static String readBlock(String content, String key) {
        Matcher matcher = ARRAY_ASSIGNMENT.matcher(content);
        while (matcher.find()) {
            if (matcher.group(1).equals(key)) {
                return matcher.group(2).trim();
            }
        }
        throw new IllegalArgumentException("Missing array assignment for " + key);
    }

    private static String readMatrixBlock(String content, String key) {
        Pattern pattern = Pattern.compile("(?s)" + Pattern.quote(key) + "\\s*=\\s*\\[(.*?)\\]\\s*;\\s*");
        Matcher matcher = pattern.matcher(content);
        if (!matcher.find()) {
            throw new IllegalArgumentException("Missing matrix assignment for " + key);
        }
        return matcher.group(1).trim();
    }

    private static String[] splitValues(String block) {
        String cleaned = block.replace("|", " ").replace(",", " ").replace("[", " ").replace("]", " ");
        return Arrays.stream(cleaned.trim().split("\\s+"))
                .filter(token -> !token.isBlank())
                .toArray(String[]::new);
    }

    private static int nextInt(ArrayDeque<String> tokens, String field) {
        if (tokens.isEmpty()) {
            throw new IllegalArgumentException("Missing token for " + field);
        }
        return Integer.parseInt(tokens.removeFirst());
    }

    private static double nextDouble(ArrayDeque<String> tokens, String field) {
        if (tokens.isEmpty()) {
            throw new IllegalArgumentException("Missing token for " + field);
        }
        return Double.parseDouble(tokens.removeFirst());
    }
}
