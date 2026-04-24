package com.example;

import java.util.Arrays;
import java.util.Comparator;
import java.util.Optional;

/**
 * Algoritmo voraz constructivo ajustado al pseudocodigo MS-CFLP-CI.
 */
public class ConstructiveSolver {

    private static final int SLACK_K = 5;

    public Optional<Solution> build(ProblemInstance instance) {
        Solution solution = new Solution(instance.facilityCount(), instance.customerCount());

        // Fase 1: selección de instalaciones.
        int[] fOpen = faseSeleccionInstalaciones(instance, solution);
        if (fOpen.length == 0) {
            return Optional.empty();
        }

        int[] r_j = new int[instance.facilityCount()];
        for (int j = 0; j < instance.facilityCount(); j++) {
            r_j[j] = instance.capacity(j);
        }

        // Fase 2: asignación de clientes.
        for (int i = 0; i < instance.customerCount(); i++) {
            int d_i = instance.demand(i);
            final int cliente = i;

            Integer[] l_i = Arrays.stream(fOpen).boxed()
                .sorted(Comparator.comparingDouble(j -> instance.serviceCost(j, cliente)))
                    .toArray(Integer[]::new);

            int k = 0;
            while (d_i > 0 && k < l_i.length) {
                int j = l_i[k++];
                if (!esCompatibleEnInstalacion(solution, instance, i, j)) {
                    continue;
                }

                int q = Math.min(d_i, r_j[j]);
                if (q <= 0) {
                    continue;
                }

                solution.asignarCantidadXij(i, j, q);
                r_j[j] -= q;
                d_i -= q;
            }

            if (d_i > 0) {
                return Optional.empty();
            }
        }

        // Cálculo de costes agregados.
        solution.totalCost(instance);
        return Optional.of(solution);
    }

    private int[] faseSeleccionInstalaciones(ProblemInstance instance, Solution solution) {
        Integer[] facilities = new Integer[instance.facilityCount()];
        for (int j = 0; j < facilities.length; j++) {
            facilities[j] = j;
        }
        Arrays.sort(facilities, Comparator.comparingDouble(instance::fixedCost));

        int d_total = Arrays.stream(instance.demands()).sum();
        int capacidadAcumulada = 0;
        int tamanoFopen = 0;

        // Selección base: abrir instalaciones hasta cubrir la demanda total.
        while (tamanoFopen < facilities.length && capacidadAcumulada < d_total) {
            int j = facilities[tamanoFopen++];
            solution.abrirInstalacion(j);
            capacidadAcumulada += instance.capacity(j);
        }

        if (capacidadAcumulada < d_total) {
            return new int[0];
        }

        // Holgura k = 5 instalaciones extra.
        int f_extra = 0;
        while (tamanoFopen < facilities.length && f_extra < SLACK_K) {
            int j = facilities[tamanoFopen++];
            solution.abrirInstalacion(j);
            f_extra++;
        }

        int[] fOpen = new int[tamanoFopen];
        for (int i = 0; i < tamanoFopen; i++) {
            fOpen[i] = facilities[i];
        }
        return fOpen;
    }

    private static boolean esCompatibleEnInstalacion(Solution solution, ProblemInstance instance, int i, int j) {
        for (int otroCliente = 0; otroCliente < instance.customerCount(); otroCliente++) {
            if (otroCliente == i) {
                continue;
            }
            if (!solution.clienteEnInstalacion(otroCliente, j)) {
                continue;
            }
            if (instance.incompatible(i, otroCliente)) {
                return false;
            }
        }
        return true;
    }

    static boolean canAssign(Solution solution, ProblemInstance instance, int customer, int facility) {
        int newLoad = solution.load(facility) + instance.demand(customer);
        if (newLoad > instance.capacity(facility)) {
            return false;
        }

        for (int other = 0; other < instance.customerCount(); other++) {
            if (other == customer) {
                continue;
            }
            if (solution.clienteEnInstalacion(other, facility) && instance.incompatible(customer, other)) {
                return false;
            }
        }

        return true;
    }
}
