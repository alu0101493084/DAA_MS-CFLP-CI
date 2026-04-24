package com.example;

import java.util.Arrays;
import java.util.ArrayList;
import java.util.List;

/**
 * Candidate solution for an MS-CFLP-CI instance.
 */
public class Solution {

    // Estructuras primarias
    private final boolean[] open;
    private final double[][] x;
    private final boolean[][] w;

    // Estructuras auxiliares
    private final double[] residualCap;
    private final List<List<Integer>> clientsOf;
    private final List<List<Integer>> facilitiesOf;
    private final int[][] incompCount;

    // Estado agregado de demanda y coste
    private final double[] demandServed;
    private final int[] loads;
    private double fixedCost;
    private double transportCost;
    private double totalCost;

    public Solution(int facilityCount, int customerCount) {
        this.open = new boolean[facilityCount];
        this.x = new double[customerCount][facilityCount];
        this.w = new boolean[customerCount][facilityCount];
        this.residualCap = new double[facilityCount];
        this.clientsOf = new ArrayList<>(facilityCount);
        this.facilitiesOf = new ArrayList<>(customerCount);
        this.incompCount = new int[customerCount][facilityCount];
        this.demandServed = new double[customerCount];
        this.loads = new int[facilityCount];

        for (int j = 0; j < facilityCount; j++) {
            clientsOf.add(new ArrayList<>());
        }
        for (int i = 0; i < customerCount; i++) {
            facilitiesOf.add(new ArrayList<>());
        }
    }

    public Solution copy() {
        Solution copy = new Solution(open.length, demandServed.length);
        System.arraycopy(open, 0, copy.open, 0, open.length);
        for (int i = 0; i < x.length; i++) {
            System.arraycopy(x[i], 0, copy.x[i], 0, x[i].length);
        }
        for (int i = 0; i < w.length; i++) {
            System.arraycopy(w[i], 0, copy.w[i], 0, w[i].length);
        }
        System.arraycopy(residualCap, 0, copy.residualCap, 0, residualCap.length);
        for (int j = 0; j < clientsOf.size(); j++) {
            copy.clientsOf.get(j).addAll(clientsOf.get(j));
        }
        for (int i = 0; i < facilitiesOf.size(); i++) {
            copy.facilitiesOf.get(i).addAll(facilitiesOf.get(i));
        }
        for (int i = 0; i < incompCount.length; i++) {
            System.arraycopy(incompCount[i], 0, copy.incompCount[i], 0, incompCount[i].length);
        }
        System.arraycopy(demandServed, 0, copy.demandServed, 0, demandServed.length);
        System.arraycopy(loads, 0, copy.loads, 0, loads.length);
        copy.fixedCost = fixedCost;
        copy.transportCost = transportCost;
        copy.totalCost = totalCost;
        return copy;
    }

    public void openFacility(int facility) {
        open[facility] = true;
    }

    public boolean[] open() {
        return open;
    }

    public void abrirInstalacion(int j) {
        openFacility(j);
    }

    public boolean isOpen(int facility) {
        return open[facility];
    }

    public int openCount() {
        int count = 0;
        for (boolean value : open) {
            if (value) {
                count++;
            }
        }
        return count;
    }

    public double[][] x() {
        return x;
    }

    public boolean[][] w() {
        return w;
    }

    public int assignedFacility(int customer) {
        for (int j = 0; j < x[customer].length; j++) {
            if (x[customer][j] > 0.0) {
                return j;
            }
        }
        return -1;
    }

    public boolean hasSplitCustomers() {
        for (int c = 0; c < demandServed.length; c++) {
            int used = 0;
            for (int j = 0; j < x[c].length; j++) {
                if (x[c][j] > 0.0) {
                    used++;
                    if (used > 1) {
                        return true;
                    }
                }
            }
        }
        return false;
    }

    public int load(int facility) {
        return loads[facility];
    }

    public double residualCap(int facility) {
        return residualCap[facility];
    }

    public double[] residualCap() {
        return residualCap;
    }

    public int allocated(int facility, int customer) {
        return (int) Math.round(x[customer][facility]);
    }

    public double x_ij(int i, int j) {
        return x[i][j];
    }

    public int servedDemand(int customer) {
        return (int) Math.round(demandServed[customer]);
    }

    public double demandServedValue(int customer) {
        return demandServed[customer];
    }

    public boolean isCustomerPresentAtFacility(int customer, int facility) {
        return w[customer][facility];
    }

    public boolean w_ij(int i, int j) {
        return w[i][j];
    }

    public boolean clienteEnInstalacion(int i, int j) {
        return isCustomerPresentAtFacility(i, j);
    }

    public void addAllocation(int customer, int facility, int quantity) {
        if (quantity <= 0) {
            return;
        }
        x[customer][facility] += quantity;
        w[customer][facility] = true;
        if (!clientsOf.get(facility).contains(customer)) {
            clientsOf.get(facility).add(customer);
        }
        if (!facilitiesOf.get(customer).contains(facility)) {
            facilitiesOf.get(customer).add(facility);
        }
        demandServed[customer] += quantity;
        loads[facility] += quantity;
        residualCap[facility] = Math.max(0.0, residualCap[facility] - quantity);
        open[facility] = true;
        transportCost += quantity;
        totalCost = fixedCost + transportCost;
    }

    public void markIncompatibleAssigned(int customer, int facility, int count) {
        incompCount[customer][facility] = count;
    }

    public void asignarCantidadXij(int i, int j, int q) {
        addAllocation(i, j, q);
    }

    public void assignCustomer(int customer, int facility, ProblemInstance instance) {
        int demand = instance.demand(customer);
        clearCustomer(customer);
        addAllocation(customer, facility, demand);
    }

    public void clearCustomer(int customer) {
        for (int j = 0; j < x[customer].length; j++) {
            double q = x[customer][j];
            if (q > 0.0) {
                x[customer][j] = 0.0;
                w[customer][j] = false;
                demandServed[customer] -= q;
                loads[j] -= (int) q;
                residualCap[j] += q;
                clientsOf.get(j).remove(Integer.valueOf(customer));
                facilitiesOf.get(customer).remove(Integer.valueOf(j));
            }
        }
    }

    public List<Integer> clientsOf(int facility) {
        return clientsOf.get(facility);
    }

    public List<List<Integer>> clientsOf() {
        return clientsOf;
    }

    public List<Integer> facilitiesOf(int customer) {
        return facilitiesOf.get(customer);
    }

    public List<List<Integer>> facilitiesOf() {
        return facilitiesOf;
    }

    public int incompCount(int customer, int facility) {
        return incompCount[customer][facility];
    }

    public int[][] incompCount() {
        return incompCount;
    }

    public double fixedCostValue() {
        return fixedCost;
    }

    public double fixedCost() {
        return fixedCost;
    }

    public double transportCostValue() {
        return transportCost;
    }

    public double transportCost() {
        return transportCost;
    }

    public double totalCostValue() {
        return totalCost;
    }

    public double totalCost() {
        return totalCost;
    }

    public int incompatibilityCount(ProblemInstance instance) {
        int count = 0;
        for (int i = 0; i < x.length; i++) {
            for (int k = i + 1; k < x.length; k++) {
                if (!instance.incompatible(i, k)) {
                    continue;
                }
                for (int j = 0; j < x[i].length; j++) {
                    if (x[i][j] > 0.0 && x[k][j] > 0.0) {
                        count++;
                    }
                }
            }
        }
        return count;
    }

    public double totalCost(ProblemInstance instance) {
        double fixed = 0.0;
        for (int j = 0; j < open.length; j++) {
            if (open[j]) {
                fixed += instance.fixedCost(j);
            }
        }
        double transport = 0.0;
        for (int i = 0; i < x.length; i++) {
            for (int j = 0; j < x[i].length; j++) {
                if (x[i][j] > 0.0) {
                    transport += instance.serviceCost(j, i) * x[i][j];
                }
            }
        }
        fixedCost = fixed;
        transportCost = transport;
        totalCost = fixedCost + transportCost;
        return totalCost;
    }

    public String assignmentSummary() {
        StringBuilder sb = new StringBuilder();
        for (int c = 0; c < demandServed.length; c++) {
            if (c > 0) {
                sb.append(' ');
            }
            sb.append("C").append(c).append("->[ ");
            boolean first = true;
            for (int j = 0; j < x[c].length; j++) {
                double q = x[c][j];
                if (q <= 0.0) {
                    continue;
                }
                if (!first) {
                    sb.append(", ");
                }
                first = false;
                sb.append("F").append(j).append(":").append((int) q);
            }
            if (first) {
                sb.append("none");
            }
            sb.append(" ]");
        }
        return sb.toString();
    }
}
