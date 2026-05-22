#ifndef CALCULATOR_LOGIC_H
#define CALCULATOR_LOGIC_H

#include <stddef.h>

#define MAX_STACK_SIZE 100
#define DISPLAY_BUFFER_SIZE 256
#define MAX_FUNCTION_NAME_LENGTH 10

typedef enum {
    ERROR_NONE,
    ERROR_SYNTAX,
    ERROR_MATH_DIV_ZERO,
    ERROR_MATH_DOMAIN,
    ERROR_STACK_OVERFLOW
} ErrorType;

typedef enum {
    DEG,
    RAD
} AngleMode;

typedef struct {
    double items[MAX_STACK_SIZE];
    int top;
} NumberStack;
/**
 * Represents a stack of double-precision floating-point numbers.
 *
 * This structure is used to hold operands during math calculation.
 *
 * Functions:
 *   nsPush: Pushes a double value onto the number stack.
 *   nsPop: Pops a double value from the number stack.
 */

// Stack for operators (chars)
typedef struct {
    char items[MAX_STACK_SIZE];
    int top;
    int total_pushed;
} OperatorStack;
/**
 * Represents a stack of operator characters.
 *
 * This structure is used to hold operators during math calculation.
 *
 * Functions:
 *   osPush: Pushes an operator character onto the operator stack.
 *   osPop: Pops an operator character from the operator stack.
 *   osPeek: Peeks at the top operator of the operator stack.
 */

typedef struct {
    char buffer[DISPLAY_BUFFER_SIZE];
    AngleMode angle_mode;
    NumberStack numbers;
    OperatorStack operators;
    ErrorType error;
} Calculator;
/**
 * Represents the core calculator model.
 *
 * This structure manages the evaluation state, including the display buffer,
 * the active angle mode, the operands and operators stacks, and any active error state.
 *
 * Functions:
 *   calculatorNew: Instantiates a new Calculator instance.
 *   calculatorFree: Deallocates a Calculator instance.
 *   calculatorClear: Resets the state of a Calculator instance.
 *   calculatorToggleAngleMode: Toggles between DEG and RAD angle mode.
 *   calculatorGetAngleMode: Gets the current angle mode.
 *   calculatorGetDisplay: Returns the display buffer.
 *   calculatorEvaluate: Parses and evaluates a mathematical expression.
 */

Calculator* calculatorNew(void);
void calculatorFree(Calculator* calc);

void calculatorEvaluate(Calculator* calc, const char* expression);
void calculatorClear(Calculator* calc);
void calculatorToggleAngleMode(Calculator* calc);
AngleMode calculatorGetAngleMode(const Calculator* calc);

const char* calculatorGetDisplay(const Calculator* calc);

#endif
