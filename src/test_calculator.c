#define _DEFAULT_SOURCE
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include "calculator_logic.h"

#define TOLERANCE 1e-9

void test_expression(const char* expression, const char* expected) {
    Calculator* calc = calculator_new();
    calculator_evaluate(calc, expression);
    const char* result = calculator_get_display(calc);
    printf("Testing: %s, Expected: %s, Got: %s\n", expression, expected, result);
    assert(strcmp(result, expected) == 0);
    calculator_free(calc);
}

void test_expression_float(const char* expression, double expected) {
    Calculator* calc = calculator_new();
    calculator_evaluate(calc, expression);
    const char* result_str = calculator_get_display(calc);
    double result = atof(result_str);
    printf("Testing: %s, Expected: %f, Got: %f\n", expression, expected, result);
    assert(fabs(result - expected) < TOLERANCE);
    calculator_free(calc);
}

void test_basic_arithmetic() {
    printf("\n--- Basic Arithmetic ---\n");
    test_expression("5+3", "8");
    test_expression("1.5+2.5", "4");
    test_expression("100+200", "300");
    test_expression("5-3", "2");
    test_expression("3-5", "-2");
    test_expression("10.5-5.5", "5");
    test_expression("5*3", "15");
    test_expression("1.5*2", "3");
    test_expression("10*0.5", "5");
    test_expression("10/2", "5");
    test_expression("5/2", "2.5");
}

void test_precedence_and_parentheses() {
    printf("\n--- Precedence and Parentheses ---\n");
    test_expression("2+3*4", "14");
    test_expression("10-4/2", "8");
    test_expression("(2+3)*4", "20");
    test_expression("10-(4/2)", "8");
    test_expression("((2+3)*4)/5", "4");
}

void test_trigonometry() {
    printf("\n--- Trigonometry (DEG) ---\n");
    test_expression_float("s90", 1.0);
    test_expression_float("c0", 1.0);
    test_expression_float("t45", 1.0);
    test_expression_float("s0", 0.0);
    test_expression_float("c90", 0.0);
    test_expression_float("t0", 0.0);

    printf("\n--- Trigonometry (RAD) ---\n");
    Calculator* calc = calculator_new();
    calculator_toggle_angle_mode(calc);
    calculator_evaluate(calc, "s(p/2)");
    assert(fabs(atof(calculator_get_display(calc)) - 1.0) < TOLERANCE);
    calculator_evaluate(calc, "c0");
    assert(fabs(atof(calculator_get_display(calc)) - 1.0) < TOLERANCE);
    calculator_evaluate(calc, "t(p/4)");
    assert(fabs(atof(calculator_get_display(calc)) - 1.0) < TOLERANCE);
    calculator_free(calc);
}

void test_inverse_trigonometry() {
    printf("\n--- Inverse Trigonometry (DEG) ---\n");
    test_expression_float("S1", 90.0);
    test_expression_float("C1", 0.0);
    test_expression_float("T1", 45.0);
}

void test_logarithms_and_exponents() {
    printf("\n--- Logarithms and Exponents ---\n");
    test_expression_float("l(e)", 1.0);
    test_expression_float("L10", 1.0);
    test_expression_float("E1", M_E);
    test_expression("2^3", "8");
    test_expression("4^0.5", "2");
}

void test_other_functions() {
    printf("\n--- Other Functions ---\n");
    test_expression("!5", "120");
    test_expression("R4", "0.25");
    test_expression("N5", "-5");
    test_expression("10%3", "1");
    test_expression_float("q16", 4.0);
}

void test_constants() {
    printf("\n--- Constants ---\n");
    test_expression_float("p", M_PI);
    test_expression_float("e", M_E);
}

void test_edge_cases() {
    printf("\n--- Edge Cases ---\n");
    test_expression("10/0", "Math Error: Division by zero");
    test_expression("q(-1)", "Math Error: Domain error (e.g., sqrt(-1))");
    test_expression("l0", "Math Error: Domain error (e.g., sqrt(-1))");
    test_expression("L-1", "Math Error: Domain error (e.g., sqrt(-1))");
    test_expression("!(-1)", "Math Error: Domain error (e.g., sqrt(-1))");
    test_expression("1+2.3.4", "Syntax Error: Invalid expression");
    test_expression("(2+3", "Syntax Error: Mismatched parentheses");
    test_expression("(2+3))", "Syntax Error: Mismatched parentheses");

    char long_expr[1000];
    strcpy(long_expr, "1");
    for (int i = 0; i < 200; i++) {
        strcat(long_expr, "+1");
    }
    test_expression(long_expr, "Error: Operator stack overflow");
}

int main() {
    printf("Running tests...\n");
    test_basic_arithmetic();
    test_precedence_and_parentheses();
    test_trigonometry();
    test_inverse_trigonometry();
    test_logarithms_and_exponents();
    test_other_functions();
    test_constants();
    test_edge_cases();
    printf("\nAll tests passed!\n");
    return 0;
}
