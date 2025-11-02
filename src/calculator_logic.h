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

// Stack for operators (chars)
typedef struct {
    char items[MAX_STACK_SIZE];
    int top;
    int total_pushed;
} OperatorStack;

typedef struct {
    char buffer[DISPLAY_BUFFER_SIZE];
    AngleMode angle_mode;
    NumberStack numbers;
    OperatorStack operators;
    ErrorType error;
} Calculator;

Calculator* calculator_new(void);
void calculator_free(Calculator* calc);

void calculator_evaluate(Calculator* calc, const char* expression);
void calculator_clear(Calculator* calc);
void calculator_toggle_angle_mode(Calculator* calc);
AngleMode calculator_get_angle_mode(const Calculator* calc);

const char* calculator_get_display(const Calculator* calc);

#endif
