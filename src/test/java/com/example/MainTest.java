package com.example;

import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.*;

class MainTest {

    @Test
    void noArgs_returnsUsage() {
        String result = Main.run(new String[]{});
        assertTrue(result.contains("Usage:"), "Should contain 'Usage:' when no args given");
    }

    @Test
    void greet_returnsExpectedOutput() {
        String result = Main.run(new String[]{"greet", "Diego"});
        assertEquals("Hello, Diego!", result);
    }

    @Test
    void demo_producesFeasibleSolution() {
        String result = Main.run(new String[]{"demo"});
        assertTrue(result.contains("Feasible: true"));
        assertTrue(result.contains("Cost:"));
        assertTrue(result.contains("Assignment:"));
    }
}
