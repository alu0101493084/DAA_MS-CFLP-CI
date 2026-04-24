package com.example;

import java.util.Optional;

/**
 * End-to-end heuristic solver.
 */
public class MsCflpCiSolver {

    private final ConstructiveSolver constructive;
    private final LocalSearchSolver localSearch;

    public MsCflpCiSolver() {
        this.constructive = new ConstructiveSolver();
        this.localSearch = new LocalSearchSolver();
    }

    public Optional<Solution> solve(ProblemInstance instance) {
        Optional<Solution> base = constructive.build(instance);
        if (base.isEmpty()) {
            return Optional.empty();
        }
        Solution improved = localSearch.improve(instance, base.get());
        return Optional.of(improved);
    }
}
