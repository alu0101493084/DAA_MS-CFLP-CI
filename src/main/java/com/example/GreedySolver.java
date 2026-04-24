package com.example;

import java.util.Optional;

/**
 * Pure greedy solver that returns the constructive solution only.
 */
public class GreedySolver {

    private final ConstructiveSolver constructive;

    public GreedySolver() {
        this.constructive = new ConstructiveSolver();
    }

    public Optional<Solution> solve(ProblemInstance instance) {
        return constructive.build(instance);
    }
}
