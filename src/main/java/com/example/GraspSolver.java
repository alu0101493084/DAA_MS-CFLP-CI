package com.example;

import java.util.ArrayList;
import java.util.Collections;
import java.util.Optional;
import java.util.Random;

/**
 * Simple GRASP metaheuristic built on top of the constructive heuristic.
 */
public class GraspSolver {

    private final Random random;
    private final ConstructiveSolver constructive;
    private final LocalSearchSolver localSearch;

    public GraspSolver() {
        this(new Random());
    }

    public GraspSolver(Random random) {
        this.random = random;
        this.constructive = new ConstructiveSolver();
        this.localSearch = new LocalSearchSolver();
    }

    public Optional<Solution> solve(ProblemInstance instance, int iterations) {
        Optional<Solution> best = Optional.empty();
        double bestCost = Double.POSITIVE_INFINITY;

        for (int it = 0; it < iterations; it++) {
            Optional<Solution> candidate = constructRandomized(instance);
            if (candidate.isEmpty()) {
                continue;
            }
            Solution improved = localSearch.improve(instance, candidate.get());
            double cost = improved.totalCost(instance);
            if (cost < bestCost) {
                bestCost = cost;
                best = Optional.of(improved);
            }
        }

        return best;
    }

    private Optional<Solution> constructRandomized(ProblemInstance instance) {
        Solution solution = new Solution(instance.facilityCount(), instance.customerCount());
        int[] fOpen = randomizedFacilitySelection(instance, solution);
        if (fOpen.length == 0) {
            return Optional.empty();
        }

        int[] residual = new int[instance.facilityCount()];
        for (int j = 0; j < instance.facilityCount(); j++) {
            residual[j] = instance.capacity(j);
        }

        ArrayList<Integer> customers = new ArrayList<>();
        for (int i = 0; i < instance.customerCount(); i++) {
            customers.add(i);
        }
        Collections.shuffle(customers, random);

        for (int customer : customers) {
            int remaining = instance.demand(customer);
            ArrayList<Integer> facilities = new ArrayList<>();
            for (int j : fOpen) {
                facilities.add(j);
            }
            facilities.sort((a, b) -> Double.compare(instance.serviceCost(a, customer), instance.serviceCost(b, customer)));

            for (int facility : facilities) {
                if (!ConstructiveSolver.canAssign(solution, instance, customer, facility)) {
                    continue;
                }
                int q = Math.min(remaining, residual[facility]);
                if (q <= 0) {
                    continue;
                }
                solution.asignarCantidadXij(customer, facility, q);
                residual[facility] -= q;
                remaining -= q;
                if (remaining == 0) {
                    break;
                }
            }

            if (remaining > 0) {
                return Optional.empty();
            }
        }

        solution.totalCost(instance);
        return Optional.of(solution);
    }

    private int[] randomizedFacilitySelection(ProblemInstance instance, Solution solution) {
        ArrayList<Integer> facilities = new ArrayList<>();
        for (int j = 0; j < instance.facilityCount(); j++) {
            facilities.add(j);
        }
        facilities.sort((a, b) -> Double.compare(instance.fixedCost(a), instance.fixedCost(b)));

        int demandTotal = 0;
        for (int d : instance.demands()) {
            demandTotal += d;
        }

        int capacityAccumulated = 0;
        ArrayList<Integer> open = new ArrayList<>();
        while (!facilities.isEmpty() && capacityAccumulated < demandTotal) {
            int index = random.nextInt(Math.min(3, facilities.size()));
            int facility = facilities.remove(index);
            open.add(facility);
            solution.abrirInstalacion(facility);
            capacityAccumulated += instance.capacity(facility);
        }

        while (!facilities.isEmpty() && open.size() < instance.facilityCount() && open.size() < demandTotal && open.size() < 5 + demandTotal) {
            int facility = facilities.remove(random.nextInt(facilities.size()));
            open.add(facility);
            solution.abrirInstalacion(facility);
            if (open.size() >= 5) {
                break;
            }
        }

        int[] result = new int[open.size()];
        for (int i = 0; i < open.size(); i++) {
            result[i] = open.get(i);
        }
        return result;
    }
}
