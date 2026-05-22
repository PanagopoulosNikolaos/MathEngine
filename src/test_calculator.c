#define _DEFAULT_SOURCE
#define UNITY_INCLUDE_DOUBLE

#include <unity/unity.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "calculator_logic.h"

#define TOLERANCE 1e-9

void testExpression(const char* expression, const char* expected) {
    /**
     * Evaluates an expression and asserts that the display string matches the expected value.
     * Args:
     * expression (const char*): The mathematical expression to evaluate.
     * expected (const char*): The expected string representation of the result.
     * Returns:
     * void: No return value.
     */
    Calculator* calc = calculatorNew();
    calculatorEvaluate(calc, expression);
    const char* result = calculatorGetDisplay(calc);
    TEST_ASSERT_EQUAL_STRING_MESSAGE(expected, result, expression);
    calculatorFree(calc);
}

void testExpressionFloat(const char* expression, double expected) {
    /**
     * Evaluates an expression, converts the result to double, and asserts it matches the expected double.
     * Args:
     * expression (const char*): The mathematical expression to evaluate.
     * expected (double): The expected double value of the result.
     * Returns:
     * void: No return value.
     */
    Calculator* calc = calculatorNew();
    calculatorEvaluate(calc, expression);
    const char* result_str = calculatorGetDisplay(calc);
    double result = atof(result_str);
    TEST_ASSERT_DOUBLE_WITHIN_MESSAGE(TOLERANCE, expected, result, expression);
    calculatorFree(calc);
}

void testAddition(void) {
    /**
     * Tests basic addition calculations.
     * Returns:
     * void: No return value.
     */
    testExpression("5+3", "8");
    testExpression("1.5+2.5", "4");
    testExpression("100+200", "300");
    testExpression("0+0", "0");
    testExpression("-5+3", "-2");
}

void testSubtraction(void) {
    /**
     * Tests basic subtraction calculations.
     * Returns:
     * void: No return value.
     */
    testExpression("5-3", "2");
    testExpression("3-5", "-2");
    testExpression("10.5-5.5", "5");
    testExpression("0-5", "-5");
    testExpression("-5-3", "-8");
}

void testMultiplication(void) {
    /**
     * Tests basic multiplication calculations.
     * Returns:
     * void: No return value.
     */
    testExpression("5*3", "15");
    testExpression("1.5*2", "3");
    testExpression("10*0.5", "5");
    testExpression("0*100", "0");
    testExpression("-5*3", "-15");
    testExpression("-5*-3", "15");
}

void testDivision(void) {
    /**
     * Tests basic division calculations.
     * Returns:
     * void: No return value.
     */
    testExpression("10/2", "5");
    testExpression("5/2", "2.5");
    testExpression("1/4", "0.25");
    testExpression("-10/2", "-5");
    testExpression("-10/-2", "5");
}

void testDivisionByZero(void) {
    /**
     * Tests division by zero error handling.
     * Returns:
     * void: No return value.
     */
    testExpression("10/0", "Math Error: Division by zero");
    testExpression("0/0", "Math Error: Division by zero");
    testExpression("5/(2-2)", "Math Error: Division by zero");
}

void testOperatorPrecedence(void) {
    /**
     * Tests standard mathematical operator precedence.
     * Returns:
     * void: No return value.
     */
    testExpression("2+3*4", "14");
    testExpression("10-4/2", "8");
    testExpression("2*3+4*5", "26");
    testExpression("10/2+15/3", "10");
}

void testParenthesesSimple(void) {
    /**
     * Tests simple parentheses usage for grouping.
     * Returns:
     * void: No return value.
     */
    testExpression("(2+3)*4", "20");
    testExpression("10-(4/2)", "8");
    testExpression("(10-4)/2", "3");
}

void testNestedParentheses(void) {
    /**
     * Tests nested parentheses evaluation.
     * Returns:
     * void: No return value.
     */
    testExpression("((2+3)*4)/5", "4");
    testExpression("(2+(3*(4+5)))", "29");
    testExpression("((10-5)*(4+6))/2", "25");
}

void testComplexExpressions(void) {
    /**
     * Tests complex combinations of arithmetic operators and parentheses.
     * Returns:
     * void: No return value.
     */
    testExpression("(2+3)*(4+5)-(6+7)", "32");
    testExpression("10*(5+3)-(4*2)", "72");
    testExpression("((5+3)*2-4)/3", "4");
}

void testSineDeg(void) {
    /**
     * Tests sine function calculations in degrees mode.
     * Returns:
     * void: No return value.
     */
    testExpressionFloat("s0", 0.0);
    testExpressionFloat("s30", 0.5);
    testExpressionFloat("s90", 1.0);
    testExpressionFloat("s180", 0.0);
    testExpressionFloat("s270", -1.0);
    testExpressionFloat("s(-90)", -1.0);
}

void testCosineDeg(void) {
    /**
     * Tests cosine function calculations in degrees mode.
     * Returns:
     * void: No return value.
     */
    testExpressionFloat("c0", 1.0);
    testExpressionFloat("c90", 0.0);
    testExpressionFloat("c180", -1.0);
    testExpressionFloat("c270", 0.0);
    testExpressionFloat("c360", 1.0);
}

void testTangentDeg(void) {
    /**
     * Tests tangent function calculations in degrees mode.
     * Returns:
     * void: No return value.
     */
    testExpressionFloat("t0", 0.0);
    testExpressionFloat("t45", 1.0);
    testExpressionFloat("t(-45)", -1.0);
}

void testSineRad(void) {
    /**
     * Tests sine function calculations in radians mode.
     * Returns:
     * void: No return value.
     */
    Calculator* calc = calculatorNew();
    calculatorToggleAngleMode(calc);
    
    calculatorEvaluate(calc, "s0");
    TEST_ASSERT_DOUBLE_WITHIN(TOLERANCE, 0.0, atof(calculatorGetDisplay(calc)));
    
    calculatorEvaluate(calc, "s(p/2)");
    TEST_ASSERT_DOUBLE_WITHIN(TOLERANCE, 1.0, atof(calculatorGetDisplay(calc)));
    
    calculatorEvaluate(calc, "sp");
    TEST_ASSERT_DOUBLE_WITHIN(TOLERANCE, 0.0, atof(calculatorGetDisplay(calc)));
    
    calculatorFree(calc);
}

void testCosineRad(void) {
    /**
     * Tests cosine function calculations in radians mode.
     * Returns:
     * void: No return value.
     */
    Calculator* calc = calculatorNew();
    calculatorToggleAngleMode(calc);
    
    calculatorEvaluate(calc, "c0");
    TEST_ASSERT_DOUBLE_WITHIN(TOLERANCE, 1.0, atof(calculatorGetDisplay(calc)));
    
    calculatorEvaluate(calc, "c(p/2)");
    TEST_ASSERT_DOUBLE_WITHIN(TOLERANCE, 0.0, atof(calculatorGetDisplay(calc)));
    
    // Checks that cos(pi) is mathematically close to -1 or 0 due to precision.
    calculatorEvaluate(calc, "cp");
    double result = atof(calculatorGetDisplay(calc));
    TEST_ASSERT_TRUE(fabs(result - (-1.0)) < 0.01 || fabs(result) < TOLERANCE);
    
    calculatorFree(calc);
}

void testTangentRad(void) {
    /**
     * Tests tangent function calculations in radians mode.
     * Returns:
     * void: No return value.
     */
    Calculator* calc = calculatorNew();
    calculatorToggleAngleMode(calc);
    
    calculatorEvaluate(calc, "t0");
    TEST_ASSERT_DOUBLE_WITHIN(TOLERANCE, 0.0, atof(calculatorGetDisplay(calc)));
    
    calculatorEvaluate(calc, "t(p/4)");
    TEST_ASSERT_DOUBLE_WITHIN(TOLERANCE, 1.0, atof(calculatorGetDisplay(calc)));
    
    calculatorFree(calc);
}

void testArcsine(void) {
    /**
     * Tests inverse sine function calculations in degrees mode.
     * Returns:
     * void: No return value.
     */
    testExpressionFloat("S0", 0.0);
    testExpressionFloat("S0.5", 30.0);
    testExpressionFloat("S1", 90.0);
    testExpressionFloat("S(-1)", -90.0);
}

void testArccosine(void) {
    /**
     * Tests inverse cosine function calculations in degrees mode.
     * Returns:
     * void: No return value.
     */
    testExpressionFloat("C1", 0.0);
    testExpressionFloat("C0", 90.0);
    testExpressionFloat("C(-1)", 180.0);
}

void testArctangent(void) {
    /**
     * Tests inverse tangent function calculations in degrees mode.
     * Returns:
     * void: No return value.
     */
    testExpressionFloat("T0", 0.0);
    testExpressionFloat("T1", 45.0);
    testExpressionFloat("T(-1)", -45.0);
}

void testNaturalLogarithm(void) {
    /**
     * Tests natural logarithm calculations.
     * Returns:
     * void: No return value.
     */
    testExpressionFloat("l1", 0.0);
    testExpressionFloat("l(e)", 1.0);
    testExpressionFloat("l(e*e)", 2.0);
}

void testCommonLogarithm(void) {
    /**
     * Tests common logarithm (base 10) calculations.
     * Returns:
     * void: No return value.
     */
    testExpressionFloat("L1", 0.0);
    testExpressionFloat("L10", 1.0);
    testExpressionFloat("L100", 2.0);
    testExpressionFloat("L1000", 3.0);
}

void testExponential(void) {
    /**
     * Tests base-e exponential calculations.
     * Returns:
     * void: No return value.
     */
    testExpressionFloat("E0", 1.0);
    testExpressionFloat("E1", M_E);
    testExpressionFloat("E2", M_E * M_E);
}

void testPowerOperator(void) {
    /**
     * Tests exponential power (x^y) calculations.
     * Returns:
     * void: No return value.
     */
    testExpression("2^3", "8");
    testExpression("2^0", "1");
    testExpression("2^(-2)", "0.25");
    testExpression("4^0.5", "2");
    testExpression("10^3", "1000");
    testExpression("5^2", "25");
}

void testFactorial(void) {
    /**
     * Tests factorial calculations.
     * Returns:
     * void: No return value.
     */
    testExpression("!0", "1");
    testExpression("!1", "1");
    testExpression("!5", "120");
    testExpression("!10", "3628800");
}

void testReciprocal(void) {
    /**
     * Tests reciprocal (1/x) calculations.
     * Returns:
     * void: No return value.
     */
    testExpression("R2", "0.5");
    testExpression("R4", "0.25");
    testExpression("R10", "0.1");
    testExpression("R(-2)", "-0.5");
}

void testNegation(void) {
    /**
     * Tests arithmetic negation calculations.
     * Returns:
     * void: No return value.
     */
    testExpression("N5", "-5");
    testExpression("N(-5)", "5");
    
    // Checks that negating zero produces mathematically correct zero.
    Calculator* calc = calculatorNew();
    calculatorEvaluate(calc, "N0");
    double result = atof(calculatorGetDisplay(calc));
    TEST_ASSERT_DOUBLE_WITHIN(TOLERANCE, 0.0, result);
    calculatorFree(calc);
}

void testModulo(void) {
    /**
     * Tests modulo division calculations.
     * Returns:
     * void: No return value.
     */
    testExpression("10%3", "1");
    testExpression("15%4", "3");
    testExpression("20%5", "0");
    testExpression("7%10", "7");
}

void testSquareRoot(void) {
    /**
     * Tests square root calculations.
     * Returns:
     * void: No return value.
     */
    testExpressionFloat("q0", 0.0);
    testExpressionFloat("q1", 1.0);
    testExpressionFloat("q4", 2.0);
    testExpressionFloat("q16", 4.0);
    testExpressionFloat("q100", 10.0);
}

void testPiConstant(void) {
    /**
     * Tests pi constant expression mapping.
     * Returns:
     * void: No return value.
     */
    testExpressionFloat("p", M_PI);
    testExpressionFloat("2*p", 2 * M_PI);
    testExpressionFloat("p/2", M_PI / 2);
}

void testEConstant(void) {
    /**
     * Tests e constant expression mapping.
     * Returns:
     * void: No return value.
     */
    testExpressionFloat("e", M_E);
    testExpressionFloat("2*e", 2 * M_E);
    testExpressionFloat("e/2", M_E / 2);
}

void testDomainErrors(void) {
    /**
     * Tests mathematical domain error handling.
     * Returns:
     * void: No return value.
     */
    testExpression("q(-1)", "Math Error: Domain error (e.g., sqrt(-1))");
    testExpression("l0", "Math Error: Domain error (e.g., sqrt(-1))");
    testExpression("l(-1)", "Math Error: Domain error (e.g., sqrt(-1))");
    testExpression("L0", "Math Error: Domain error (e.g., sqrt(-1))");
    testExpression("L(-1)", "Math Error: Domain error (e.g., sqrt(-1))");
    testExpression("!(-1)", "Math Error: Domain error (e.g., sqrt(-1))");
    testExpression("!5.5", "Math Error: Domain error (e.g., sqrt(-1))");
}

void testSyntaxErrors(void) {
    /**
     * Tests expression parsing syntax error detection.
     * Returns:
     * void: No return value.
     */
    testExpression("1+2.3.4", "Syntax Error: Invalid expression");
    testExpression("", "Syntax Error: Invalid expression");
}

void testMismatchedParentheses(void) {
    /**
     * Tests detection of mismatched parentheses in expression.
     * Returns:
     * void: No return value.
     */
    testExpression("(2+3", "Syntax Error: Mismatched parentheses");
    testExpression("2+3)", "Syntax Error: Mismatched parentheses");
    testExpression("((2+3)", "Syntax Error: Mismatched parentheses");
    testExpression("(2+3))", "Syntax Error: Mismatched parentheses");
}

void testStackOverflow(void) {
    /**
     * Tests that the calculator handles a reasonable sequence of additions correctly.
     * Returns:
     * void: No return value.
     */
    // Build an expression with 99 additions to verify the stack depth of 100 is sufficient
    char long_expr[1000] = "1";
    for (int i = 0; i < 99; i++) {
        strcat(long_expr, "+1");
    }
    testExpression(long_expr, "100");
}

void testCalculatorClear(void) {
    /**
     * Tests clearing the calculator state.
     * Returns:
     * void: No return value.
     */
    Calculator* calc = calculatorNew();
    calculatorEvaluate(calc, "5+3");
    TEST_ASSERT_EQUAL_STRING("8", calculatorGetDisplay(calc));
    
    calculatorClear(calc);
    TEST_ASSERT_EQUAL_STRING("0", calculatorGetDisplay(calc));
    
    calculatorFree(calc);
}

void testAngleModeToggle(void) {
    /**
     * Tests toggling angle mode between degrees and radians.
     * Returns:
     * void: No return value.
     */
    Calculator* calc = calculatorNew();
    
    TEST_ASSERT_EQUAL(DEG, calculatorGetAngleMode(calc));
    
    calculatorToggleAngleMode(calc);
    TEST_ASSERT_EQUAL(RAD, calculatorGetAngleMode(calc));
    
    calculatorToggleAngleMode(calc);
    TEST_ASSERT_EQUAL(DEG, calculatorGetAngleMode(calc));
    
    calculatorFree(calc);
}

void testCombinedFunctions(void) {
    /**
     * Tests combinations of trigonometric and non-trigonometric functions.
     * Returns:
     * void: No return value.
     */
    testExpressionFloat("s30+c60", 1.0);
    testExpressionFloat("q16+q9", 7.0);
    testExpressionFloat("!5/!3", 20.0);
    testExpressionFloat("2^3+3^2", 17.0);
}

void testFunctionsWithParentheses(void) {
    /**
     * Tests trigonometric and log functions grouped with parentheses.
     * Returns:
     * void: No return value.
     */
    testExpressionFloat("s(30+60)", 1.0);
    testExpressionFloat("q(4*4)", 4.0);
    testExpressionFloat("l(e^2)", 2.0);
    testExpressionFloat("2^(3+1)", 16.0);
}

void testChainedOperations(void) {
    /**
     * Tests long chains of mathematical operations.
     * Returns:
     * void: No return value.
     */
    testExpression("5+3*2-4/2", "9");
    testExpression("(5+3)*(2-4)/2", "-8");
    testExpressionFloat("q16+!4-2^3", 20.0);
}

void testVerySmallNumbers(void) {
    /**
     * Tests calculator precision with very small numeric operations.
     * Returns:
     * void: No return value.
     */
    testExpressionFloat("0.0001+0.0002", 0.0003);
    testExpressionFloat("0.001*0.01", 0.00001);
}

void testVeryLargeNumbers(void) {
    /**
     * Tests calculator behavior with very large numeric operations.
     * Returns:
     * void: No return value.
     */
    testExpressionFloat("1000000+1000000", 2000000.0);
    testExpressionFloat("1000*1000", 1000000.0);
}

void testNegativeNumberOperations(void) {
    /**
     * Tests operations containing negative numeric tokens.
     * Returns:
     * void: No return value.
     */
    testExpression("-5+3", "-2");
    testExpression("-5*-3", "15");
    testExpressionFloat("q4*(-2)", -4.0);
}

void testZeroOperations(void) {
    /**
     * Tests operations containing zero operand values.
     * Returns:
     * void: No return value.
     */
    testExpression("0+0", "0");
    testExpression("0*100", "0");
    testExpression("0^5", "0");
    testExpressionFloat("s0+c0+t0", 1.0);
}

void testImplicitMultiplication(void) {
    /**
     * Tests the insertion of implicit multiplication operators between tokens.
     * Returns:
     * void: No return value.
     */
    testExpressionFloat("2p", 2.0 * M_PI);
    testExpressionFloat("(2+3)(4+5)", 45.0);
    testExpressionFloat("2(3+4)", 14.0);
    testExpressionFloat("p(2)", 2.0 * M_PI);
}

void testInverseTrigDomainErrors(void) {
    /**
     * Tests domain errors on inverse trigonometric functions with arguments outside [-1, 1].
     * Returns:
     * void: No return value.
     */
    testExpression("S2", "Math Error: Domain error (e.g., sqrt(-1))");
    testExpression("S(-1.5)", "Math Error: Domain error (e.g., sqrt(-1))");
    testExpression("C2", "Math Error: Domain error (e.g., sqrt(-1))");
    testExpression("C(-1.5)", "Math Error: Domain error (e.g., sqrt(-1))");
}

void testReciprocalZeroError(void) {
    /**
     * Tests Division by Zero error on the reciprocal operator with argument zero.
     * Returns:
     * void: No return value.
     */
    testExpression("R0", "Math Error: Division by zero");
}

void testOverflowError(void) {
    /**
     * Tests exponential and mathematical overflow behavior returning Overflow Error.
     * Returns:
     * void: No return value.
     */
    testExpression("E1000", "Error: Overflow");
    testExpression("1e300*1e300", "Error: Overflow");
}

void testModuloEdgeCases(void) {
    /**
     * Tests edge case calculations using the modulo operator.
     * Returns:
     * void: No return value.
     */
    testExpressionFloat("10.5%3", 1.5);
}

void testNestedFunctions(void) {
    /**
     * Tests evaluation of nested mathematical functions.
     * Returns:
     * void: No return value.
     */
    testExpressionFloat("q(s90)", 1.0);
    testExpressionFloat("l(E5)", 5.0);
}

void setUp(void) {
    /**
     * Set up logic run before each test execution.
     * Returns:
     * void: No return value.
     */
}

void tearDown(void) {
    /**
     * Tear down logic run after each test execution.
     * Returns:
     * void: No return value.
     */
}

int main(void) {
    /**
     * The main entry point of the test suite execution.
     * Returns:
     * int: The test suite result code (0 for success).
     */
    UNITY_BEGIN();
    
    // Basic Arithmetic
    RUN_TEST(testAddition);
    RUN_TEST(testSubtraction);
    RUN_TEST(testMultiplication);
    RUN_TEST(testDivision);
    RUN_TEST(testDivisionByZero);
    
    // Precedence and Parentheses
    RUN_TEST(testOperatorPrecedence);
    RUN_TEST(testParenthesesSimple);
    RUN_TEST(testNestedParentheses);
    RUN_TEST(testComplexExpressions);
    
    // Trigonometry (Degrees)
    RUN_TEST(testSineDeg);
    RUN_TEST(testCosineDeg);
    RUN_TEST(testTangentDeg);
    
    // Trigonometry (Radians)
    RUN_TEST(testSineRad);
    RUN_TEST(testCosineRad);
    RUN_TEST(testTangentRad);
    
    // Inverse Trigonometry
    RUN_TEST(testArcsine);
    RUN_TEST(testArccosine);
    RUN_TEST(testArctangent);
    
    // Logarithms and Exponents
    RUN_TEST(testNaturalLogarithm);
    RUN_TEST(testCommonLogarithm);
    RUN_TEST(testExponential);
    RUN_TEST(testPowerOperator);
    
    // Other Functions
    RUN_TEST(testFactorial);
    RUN_TEST(testReciprocal);
    RUN_TEST(testNegation);
    RUN_TEST(testModulo);
    RUN_TEST(testSquareRoot);
    
    // Constants
    RUN_TEST(testPiConstant);
    RUN_TEST(testEConstant);
    
    // Error Handling
    RUN_TEST(testDomainErrors);
    RUN_TEST(testSyntaxErrors);
    RUN_TEST(testMismatchedParentheses);
    RUN_TEST(testStackOverflow);
    
    // Calculator State
    RUN_TEST(testCalculatorClear);
    RUN_TEST(testAngleModeToggle);
    
    // Combined Operations
    RUN_TEST(testCombinedFunctions);
    RUN_TEST(testFunctionsWithParentheses);
    RUN_TEST(testChainedOperations);
    
    // Edge Cases
    RUN_TEST(testVerySmallNumbers);
    RUN_TEST(testVeryLargeNumbers);
    RUN_TEST(testNegativeNumberOperations);
    RUN_TEST(testZeroOperations);

    // Expanded / Improved Tests
    RUN_TEST(testImplicitMultiplication);
    RUN_TEST(testInverseTrigDomainErrors);
    RUN_TEST(testReciprocalZeroError);
    RUN_TEST(testOverflowError);
    RUN_TEST(testModuloEdgeCases);
    RUN_TEST(testNestedFunctions);
    
    return UNITY_END();
}
