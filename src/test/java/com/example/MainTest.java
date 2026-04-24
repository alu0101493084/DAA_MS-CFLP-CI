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
    void helpFlag_returnsUsage() {
        String result = Main.run(new String[]{"--help"});
        assertTrue(result.contains("Usage:"), "Should contain 'Usage:' for --help");
    }

    @Test
    void greetWithName_returnsGreeting() {
        String result = Main.run(new String[]{"greet", "Alice"});
        assertEquals("Hello, Alice!", result);
    }

    @Test
    void greetWithoutName_returnsError() {
        String result = Main.run(new String[]{"greet"});
        assertTrue(result.startsWith("Error:"), "Should return error when name is missing");
    }

    @Test
    void unknownCommand_returnsError() {
        String result = Main.run(new String[]{"unknown"});
        assertTrue(result.startsWith("Unknown command:"), "Should report unknown command");
    }
}
