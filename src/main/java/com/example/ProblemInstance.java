package com.example;

import java.util.Arrays;

/**
 * Immutable data model for one MS-CFLP-CI instance.
 */
public class ProblemInstance {

    private final int facilityCount;
    private final int customerCount;
    private final int[] capacities;
    private final int[] demands;
    private final double[] fixedCosts;
    private final double[][] serviceCosts;
    private final boolean[][] incompatible;

    public ProblemInstance(
            int[] capacities,
            int[] demands,
            double[] fixedCosts,
            double[][] serviceCosts,
            boolean[][] incompatible
    ) {
        this.facilityCount = capacities.length;
        this.customerCount = demands.length;
        this.capacities = Arrays.copyOf(capacities, capacities.length);
        this.demands = Arrays.copyOf(demands, demands.length);
        this.fixedCosts = Arrays.copyOf(fixedCosts, fixedCosts.length);

        if (fixedCosts.length != facilityCount) {
            throw new IllegalArgumentException("Fixed costs length must match facility count");
        }
        if (serviceCosts.length != facilityCount) {
            throw new IllegalArgumentException("Service costs rows must match facility count");
        }

        this.serviceCosts = new double[facilityCount][customerCount];
        for (int f = 0; f < facilityCount; f++) {
            if (serviceCosts[f].length != customerCount) {
                throw new IllegalArgumentException("Service costs columns must match customer count");
            }
            this.serviceCosts[f] = Arrays.copyOf(serviceCosts[f], customerCount);
        }

        if (incompatible.length != customerCount) {
            throw new IllegalArgumentException("Incompatibility matrix must match customer count");
        }
        this.incompatible = new boolean[customerCount][customerCount];
        for (int i = 0; i < customerCount; i++) {
            if (incompatible[i].length != customerCount) {
                throw new IllegalArgumentException("Incompatibility matrix must be square");
            }
            this.incompatible[i] = Arrays.copyOf(incompatible[i], customerCount);
        }
    }

    public int facilityCount() {
        return facilityCount;
    }

    public int customerCount() {
        return customerCount;
    }

    public int capacity(int facility) {
        return capacities[facility];
    }

    public int demand(int customer) {
        return demands[customer];
    }

    public double fixedCost(int facility) {
        return fixedCosts[facility];
    }

    public double serviceCost(int facility, int customer) {
        return serviceCosts[facility][customer];
    }

    public boolean incompatible(int customerA, int customerB) {
        return incompatible[customerA][customerB];
    }

    public int[] capacities() {
        return Arrays.copyOf(capacities, capacities.length);
    }

    public int[] demands() {
        return Arrays.copyOf(demands, demands.length);
    }
}
