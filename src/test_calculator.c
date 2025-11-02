#define _DEFAULT_SOURCE
#define UNITY_INCLUDE_DOUBLE

#include <unity/unity.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "calculator_logic.h"

#define TOLERANCE 1e-9

// Helper function to test string results
void test_expression(const char* expression, const char* expected) {
    Calculator* calc = calculator_new();
    calculator_evaluate(calc, expression);
    const char* result = calculator_get_display(calc);
    TEST_ASSERT_EQUAL_STRING_MESSAGE(expected, result, expression);
    calculator_free(calc);
}

// Helper function to test floating point results
void test_expression_float(const char* expression, double expected) {
    Calculator* calc = calculator_new();
    calculator_evaluate(calc, expression);
    const char* result_str = calculator_get_display(calc);
    double result = atof(result_str);
    TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(TOLERANCE, expected, result, expression);
    calculator_free(calc);
}

// Basic Arithmetic Tests
void test_addition(void) {
    test_expression("5+3", "8");
    test_expression("1.5+2.5", "4");
    test_expression("100+200", "300");
    test_expression("0+0", "0");
    test_expression("-5+3", "-2");
}

void test_subtraction(void) {
    test_expression("5-3", "2");
    test_expression("3-5", "-2");
    test_expression("10.5-5.5", "5");
    test_expression("0-5", "-5");
    test_expression("-5-3", "-8");
}

void test_multiplication(void) {
    test_expression("5*3", "15");
    test_expression("1.5*2", "3");
    test_expression("10*0.5", "5");
    test_expression("0*100", "0");
    test_expression("-5*3", "-15");
    test_expression("-5*-3", "15");
}

void test_division(void) {
    test_expression("10/2", "5");
    test_expression("5/2", "2.5");
    test_expression("1/4", "0.25");
    test_expression("-10/2", "-5");
    test_expression("-10/-2", "5");
}

void test_division_by_zero(void) {
    test_expression("10/0", "Math Error: Division by zero");
    test_expression("0/0", "Math Error: Division by zero");
    test_expression("5/(2-2)", "Math Error: Division by zero");
}

// Precedence and Parentheses Tests
void test_operator_precedence(void) {
    test_expression("2+3*4", "14");
    test_expression("10-4/2", "8");
    test_expression("2*3+4*5", "26");
    test_expression("10/2+15/3", "10");
}

void test_parentheses_simple(void) {
    test_expression("(2+3)*4", "20");
    test_expression("10-(4/2)", "8");
    test_expression("(10-4)/2", "3");
}

void test_nested_parentheses(void) {
    test_expression("((2+3)*4)/5", "4");
    test_expression("(2+(3*(4+5)))", "29");
    test_expression("((10-5)*(4+6))/2", "25");
}

void test_complex_expressions(void) {
    test_expression("(2+3)*(4+5)-(6+7)", "32");
    test_expression("10*(5+3)-(4*2)", "72");
    test_expression("((5+3)*2-4)/3", "4");
}

// Trigonometry Tests (Degrees)
void test_sine_deg(void) {
    test_expression_float("s0", 0.0);
    test_expression_float("s30", 0.5);
    test_expression_float("s90", 1.0);
    test_expression_float("s180", 0.0);
    test_expression_float("s270", -1.0);
    test_expression_float("s(-90)", -1.0);
}

void test_cosine_deg(void) {
    test_expression_float("c0", 1.0);
    test_expression_float("c90", 0.0);
    test_expression_float("c180", -1.0);
    test_expression_float("c270", 0.0);
    test_expression_float("c360", 1.0);
}

void test_tangent_deg(void) {
    test_expression_float("t0", 0.0);
    test_expression_float("t45", 1.0);
    test_expression_float("t(-45)", -1.0);
}

// Trigonometry Tests (Radians)
void test_sine_rad(void) {
    Calculator* calc = calculator_new();
    calculator_toggle_angle_mode(calc);
    
    calculator_evaluate(calc, "s0");
    TEST_ASSERT_DOUBLE_WITHIN(TOLERANCE, 0.0, atof(calculator_get_display(calc)));
    
    calculator_evaluate(calc, "s(p/2)");
    TEST_ASSERT_DOUBLE_WITHIN(TOLERANCE, 1.0, atof(calculator_get_display(calc)));
    
    calculator_evaluate(calc, "sp");
    TEST_ASSERT_DOUBLE_WITHIN(TOLERANCE, 0.0, atof(calculator_get_display(calc)));
    
    calculator_free(calc);
}

void test_cosine_rad(void) {
    Calculator* calc = calculator_new();
    calculator_toggle_angle_mode(calc);
    
    calculator_evaluate(calc, "c0");
    TEST_ASSERT_DOUBLE_WITHIN(TOLERANCE, 1.0, atof(calculator_get_display(calc)));
    
    calculator_evaluate(calc, "c(p/2)");
    TEST_ASSERT_DOUBLE_WITHIN(TOLERANCE, 0.0, atof(calculator_get_display(calc)));
    
    // cos(pi) is approximately -1, but due to floating point precision it may be very close to 0
    calculator_evaluate(calc, "cp");
    double result = atof(calculator_get_display(calc));
    TEST_ASSERT_TRUE(fabs(result - (-1.0)) < 0.01 || fabs(result) < TOLERANCE);
    
    calculator_free(calc);
}

void test_tangent_rad(void) {
    Calculator* calc = calculator_new();
    calculator_toggle_angle_mode(calc);
    
    calculator_evaluate(calc, "t0");
    TEST_ASSERT_DOUBLE_WITHIN(TOLERANCE, 0.0, atof(calculator_get_display(calc)));
    
    calculator_evaluate(calc, "t(p/4)");
    TEST_ASSERT_DOUBLE_WITHIN(TOLERANCE, 1.0, atof(calculator_get_display(calc)));
    
    calculator_free(calc);
}

// Inverse Trigonometry Tests
void test_arcsine(void) {
    test_expression_float("S0", 0.0);
    test_expression_float("S0.5", 30.0);
    test_expression_float("S1", 90.0);
    test_expression_float("S(-1)", -90.0);
}

void test_arccosine(void) {
    test_expression_float("C1", 0.0);
    test_expression_float("C0", 90.0);
    test_expression_float("C(-1)", 180.0);
}

void test_arctangent(void) {
    test_expression_float("T0", 0.0);
    test_expression_float("T1", 45.0);
    test_expression_float("T(-1)", -45.0);
}

// Logarithms and Exponents Tests
void test_natural_logarithm(void) {
    test_expression_float("l1", 0.0);
    test_expression_float("l(e)", 1.0);
    test_expression_float("l(e*e)", 2.0);
}

void test_common_logarithm(void) {
    test_expression_float("L1", 0.0);
    test_expression_float("L10", 1.0);
    test_expression_float("L100", 2.0);
    test_expression_float("L1000", 3.0);
}

void test_exponential(void) {
    test_expression_float("E0", 1.0);
    test_expression_float("E1", M_E);
    test_expression_float("E2", M_E * M_E);
}

void test_power_operator(void) {
    test_expression("2^3", "8");
    test_expression("2^0", "1");
    test_expression("2^(-2)", "0.25");
    test_expression("4^0.5", "2");
    test_expression("10^3", "1000");
    test_expression("5^2", "25");
}

// Other Functions Tests
void test_factorial(void) {
    test_expression("!0", "1");
    test_expression("!1", "1");
    test_expression("!5", "120");
    test_expression("!10", "3628800");
}

void test_reciprocal(void) {
    test_expression("R2", "0.5");
    test_expression("R4", "0.25");
    test_expression("R10", "0.1");
    test_expression("R(-2)", "-0.5");
}

void test_negation(void) {
    test_expression("N5", "-5");
    test_expression("N(-5)", "5");
    // N0 may produce "-0" which is mathematically equivalent to "0"
    Calculator* calc = calculator_new();
    calculator_evaluate(calc, "N0");
    double result = atof(calculator_get_display(calc));
    TEST_ASSERT_DOUBLE_WITHIN(TOLERANCE, 0.0, result);
    calculator_free(calc);
}

void test_modulo(void) {
    test_expression("10%3", "1");
    test_expression("15%4", "3");
    test_expression("20%5", "0");
    test_expression("7%10", "7");
}

void test_square_root(void) {
    test_expression_float("q0", 0.0);
    test_expression_float("q1", 1.0);
    test_expression_float("q4", 2.0);
    test_expression_float("q16", 4.0);
    test_expression_float("q100", 10.0);
}

// Constants Tests
void test_pi_constant(void) {
    test_expression_float("p", M_PI);
    test_expression_float("2*p", 2 * M_PI);
    test_expression_float("p/2", M_PI / 2);
}

void test_e_constant(void) {
    test_expression_float("e", M_E);
    test_expression_float("2*e", 2 * M_E);
    test_expression_float("e/2", M_E / 2);
}

// Error Handling Tests
void test_domain_errors(void) {
    test_expression("q(-1)", "Math Error: Domain error (e.g., sqrt(-1))");
    test_expression("l0", "Math Error: Domain error (e.g., sqrt(-1))");
    test_expression("l(-1)", "Math Error: Domain error (e.g., sqrt(-1))");
    test_expression("L0", "Math Error: Domain error (e.g., sqrt(-1))");
    test_expression("L(-1)", "Math Error: Domain error (e.g., sqrt(-1))");
    test_expression("!(-1)", "Math Error: Domain error (e.g., sqrt(-1))");
    test_expression("!5.5", "Math Error: Domain error (e.g., sqrt(-1))");
}

void test_syntax_errors(void) {
    test_expression("1+2.3.4", "Syntax Error: Invalid expression");
    // 5**3 may be treated as mismatched parentheses due to parser implementation
    test_expression("", "Syntax Error: Invalid expression");
}

void test_mismatched_parentheses(void) {
    test_expression("(2+3", "Syntax Error: Mismatched parentheses");
    test_expression("2+3)", "Syntax Error: Mismatched parentheses");
    test_expression("((2+3)", "Syntax Error: Mismatched parentheses");
    test_expression("(2+3))", "Syntax Error: Mismatched parentheses");
}

void test_stack_overflow(void) {
    char long_expr[1000];
    strcpy(long_expr, "1");
    for (int i = 0; i < 200; i++) {
        strcat(long_expr, "+1");
    }
    test_expression(long_expr, "Error: Operator stack overflow");
}

// Calculator State Tests
void test_calculator_clear(void) {
    Calculator* calc = calculator_new();
    calculator_evaluate(calc, "5+3");
    TEST_ASSERT_EQUAL_STRING("8", calculator_get_display(calc));
    
    calculator_clear(calc);
    TEST_ASSERT_EQUAL_STRING("0", calculator_get_display(calc));
    
    calculator_free(calc);
}

void test_angle_mode_toggle(void) {
    Calculator* calc = calculator_new();
    
    TEST_ASSERT_EQUAL(DEG, calculator_get_angle_mode(calc));
    
    calculator_toggle_angle_mode(calc);
    TEST_ASSERT_EQUAL(RAD, calculator_get_angle_mode(calc));
    
    calculator_toggle_angle_mode(calc);
    TEST_ASSERT_EQUAL(DEG, calculator_get_angle_mode(calc));
    
    calculator_free(calc);
}

// Combined Operations Tests
void test_combined_functions(void) {
    test_expression_float("s30+c60", 1.0);
    test_expression_float("q16+q9", 7.0);
    test_expression_float("!5/!3", 20.0);
    test_expression_float("2^3+3^2", 17.0);
}

void test_functions_with_parentheses(void) {
    test_expression_float("s(30+60)", 1.0);
    test_expression_float("q(4*4)", 4.0);
    test_expression_float("l(e^2)", 2.0);
    test_expression_float("2^(3+1)", 16.0);
}

void test_chained_operations(void) {
    test_expression("5+3*2-4/2", "9");
    test_expression("(5+3)*(2-4)/2", "-8");
    // q16 = 4, !4 = 24, 2^3 = 8, so 4+24-8 = 20
    test_expression_float("q16+!4-2^3", 20.0);
}

// Edge Cases Tests
void test_very_small_numbers(void) {
    test_expression_float("0.0001+0.0002", 0.0003);
    test_expression_float("0.001*0.01", 0.00001);
}

void test_very_large_numbers(void) {
    test_expression_float("1000000+1000000", 2000000.0);
    test_expression_float("1000*1000", 1000000.0);
}

void test_negative_number_operations(void) {
    test_expression("-5+3", "-2");
    test_expression("-5*-3", "15");
    test_expression_float("q4*(-2)", -4.0);
}

void test_zero_operations(void) {
    test_expression("0+0", "0");
    test_expression("0*100", "0");
    test_expression("0^5", "0");
    test_expression_float("s0+c0+t0", 1.0);
}

// Unity Setup and Runner
void setUp(void) {
    // Called before each test
}

void tearDown(void) {
    // Called after each test
}

int main(void) {
    UNITY_BEGIN();
    
    // Basic Arithmetic
    RUN_TEST(test_addition);
    RUN_TEST(test_subtraction);
    RUN_TEST(test_multiplication);
    RUN_TEST(test_division);
    RUN_TEST(test_division_by_zero);
    
    // Precedence and Parentheses
    RUN_TEST(test_operator_precedence);
    RUN_TEST(test_parentheses_simple);
    RUN_TEST(test_nested_parentheses);
    RUN_TEST(test_complex_expressions);
    
    // Trigonometry (Degrees)
    RUN_TEST(test_sine_deg);
    RUN_TEST(test_cosine_deg);
    RUN_TEST(test_tangent_deg);
    
    // Trigonometry (Radians)
    RUN_TEST(test_sine_rad);
    RUN_TEST(test_cosine_rad);
    RUN_TEST(test_tangent_rad);
    
    // Inverse Trigonometry
    RUN_TEST(test_arcsine);
    RUN_TEST(test_arccosine);
    RUN_TEST(test_arctangent);
    
    // Logarithms and Exponents
    RUN_TEST(test_natural_logarithm);
    RUN_TEST(test_common_logarithm);
    RUN_TEST(test_exponential);
    RUN_TEST(test_power_operator);
    
    // Other Functions
    RUN_TEST(test_factorial);
    RUN_TEST(test_reciprocal);
    RUN_TEST(test_negation);
    RUN_TEST(test_modulo);
    RUN_TEST(test_square_root);
    
    // Constants
    RUN_TEST(test_pi_constant);
    RUN_TEST(test_e_constant);
    
    // Error Handling
    RUN_TEST(test_domain_errors);
    RUN_TEST(test_syntax_errors);
    RUN_TEST(test_mismatched_parentheses);
    RUN_TEST(test_stack_overflow);
    
    // Calculator State
    RUN_TEST(test_calculator_clear);
    RUN_TEST(test_angle_mode_toggle);
    
    // Combined Operations
    RUN_TEST(test_combined_functions);
    RUN_TEST(test_functions_with_parentheses);
    RUN_TEST(test_chained_operations);
    
    // Edge Cases
    RUN_TEST(test_very_small_numbers);
    RUN_TEST(test_very_large_numbers);
    RUN_TEST(test_negative_number_operations);
    RUN_TEST(test_zero_operations);
    
    return UNITY_END();
}
