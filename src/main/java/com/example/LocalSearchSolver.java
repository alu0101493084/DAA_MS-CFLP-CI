package com.example;

/**
 * Local search based on relocate moves.
 */
public class LocalSearchSolver {

    public Solution improve(ProblemInstance instance, Solution initial) {
        if (initial.hasSplitCustomers()) {
            return initial.copy();
        }
        Solution current = initial.copy();
        boolean improved;

        do {
            improved = false;
            for (int customer = 0; customer < instance.customerCount() && !improved; customer++) {
                int from = current.assignedFacility(customer);
                for (int to = 0; to < instance.facilityCount(); to++) {
                    if (to == from) {
                        continue;
                    }
                    if (!ConstructiveSolver.canAssign(current, instance, customer, to)) {
                        continue;
                    }

                    double delta = relocationDelta(instance, current, customer, from, to);
                    if (delta < -1e-9) {
                        current.assignCustomer(customer, to, instance);
                        improved = true;
                        break;
                    }
                }
            }
        } while (improved);

        return current;
    }

    private double relocationDelta(ProblemInstance instance, Solution current, int customer, int from, int to) {
        double delta = 0.0;
        delta -= instance.serviceCost(from, customer);
        delta += instance.serviceCost(to, customer);

        int demand = instance.demand(customer);
        if (current.load(from) == demand) {
            delta -= instance.fixedCost(from);
        }
        if (!current.isOpen(to)) {
            delta += instance.fixedCost(to);
        }
        return delta;
    }
}
