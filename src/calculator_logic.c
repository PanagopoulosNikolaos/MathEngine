#include "calculator_logic.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <stdbool.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_E
#define M_E 2.71828182845904523536
#endif

// Prototypes for stack operations
int nsPush(NumberStack* s, double item);
double nsPop(NumberStack* s, Calculator* calc);
int osPush(OperatorStack* s, char item);
char osPop(OperatorStack* s);
char osPeek(OperatorStack* s);
int getPrecedence(char op);
void applyOperator(Calculator* calc, char op);
double factorial(double n, Calculator* calc);
int isRightAssociative(char op);

typedef struct {
    const char *name;
    char mapped_char;
} FunctionNameMapping;

static const FunctionNameMapping function_name_map[] = {
    {"sin", 's'},
    {"cos", 'c'},
    {"tan", 't'},
    {"sqrt", 'q'},
    {"ln", 'l'},
    {"log", 'L'},
    {"exp", 'E'},
    {NULL, '\0'}
};

static int isFunctionCode(char c) {
    /**
     * Checks if a single character is a recognized internal function operator code.
     * Args:
     * c (char): The character to check.
     * Returns:
     * int: Non-zero if the character is a valid function code.
     */
    switch (c) {
        case 's': case 'c': case 't': case 'S': case 'C': case 'T':
        case 'l': case 'L': case 'q': case '!': case 'E': case 'R': case 'N':
            return 1;
        default:
            return 0;
    }
}

static const char* recognized_function_names[] = {
    "sin", "cos", "tan", "sqrt", "ln", "log", "exp", NULL
};

static int isValidFunctionName(const char *name) {
    /**
     * Checks if a string is a recognized multi-character function name or single-char code.
     * Args:
     * name (const char*): The function name string.
     * Returns:
     * int: Non-zero if recognized, zero otherwise.
     */
    if (name[0] != '\0' && name[1] == '\0') {
        return isFunctionCode(name[0]);
    }
    for (int i = 0; recognized_function_names[i] != NULL; i++) {
        if (strcmp(name, recognized_function_names[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static char lookupFunctionName(const char *name) {
    /**
     * Maps a function name or single-char code to its internal operator character.
     * Args:
     * name (const char*): The function name or single-char code.
     * Returns:
     * char: The internal operator character, or '\0' if not found.
     */
    if (name[0] != '\0' && name[1] == '\0') {
        return name[0];
    }
    for (int i = 0; function_name_map[i].name != NULL; i++) {
        if (strcmp(name, function_name_map[i].name) == 0) {
            return function_name_map[i].mapped_char;
        }
    }
    return '\0';
}

typedef enum {
    TOKEN_NONE,
    TOKEN_NUMBER,
    TOKEN_OPERATOR,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_FUNCTION,
    TOKEN_CONSTANT
} TokenType;

static int needsImplicitMultiplication(TokenType prev, TokenType current) {
    /**
     * Determines whether implicit multiplication should be inserted between two token types.
     * Args:
     * prev (TokenType): The previous token type.
     * current (TokenType): The current token type.
     * Returns:
     * int: Non-zero if implicit multiplication is needed, zero otherwise.
     */
    if (prev == TOKEN_NONE) {
        return 0;
    }

    int prev_is_value = (prev == TOKEN_NUMBER || prev == TOKEN_RPAREN || prev == TOKEN_CONSTANT);
    int current_is_value = (current == TOKEN_LPAREN || current == TOKEN_NUMBER || current == TOKEN_CONSTANT || current == TOKEN_FUNCTION);

    return prev_is_value && current_is_value;
}

static int processOperatorToken(Calculator* calc, char op) {
    /**
     * Processes an operator token by applying higher or equal precedence operators from the stack.
     * Args:
     * calc (Calculator*): The calculator context pointer.
     * op (char): The operator character to process.
     * Returns:
     * int: Non-zero if successfully processed, zero otherwise.
     */
    while (calc->operators.top != -1) {
        char top_op = osPeek(&calc->operators);
        int top_prec = getPrecedence(top_op);
        int curr_prec = getPrecedence(op);

        if (top_prec > curr_prec || (top_prec == curr_prec && !isRightAssociative(op))) {
            applyOperator(calc, osPop(&calc->operators));
            if (calc->error != ERROR_NONE) {
                return 0;
            }
        } else {
            break;
        }
    }

    if (!osPush(&calc->operators, op)) {
        calc->error = ERROR_STACK_OVERFLOW;
        return 0;
    }

    return 1;
}

static int insertImplicitMultiplication(Calculator* calc, TokenType* prev_token, TokenType current_token) {
    /**
     * Inserts an implicit multiplication operator if required by the token sequence.
     * Args:
     * calc (Calculator*): The calculator context pointer.
     * prev_token (TokenType*): A pointer to the previous token type, which may be updated.
     * current_token (TokenType): The current token type.
     * Returns:
     * int: Non-zero if successfully inserted or not needed, zero on error.
     */
    if (needsImplicitMultiplication(*prev_token, current_token)) {
        if (!processOperatorToken(calc, '*')) {
            return 0;
        }
        *prev_token = TOKEN_OPERATOR;
    }
    return 1;
}

static void formatResult(char* buffer, size_t size, double value) {
    /**
     * Formats a double value into a string representation in the display buffer.
     * Args:
     * buffer (char*): The target string buffer.
     * size (size_t): The maximum size of the buffer.
     * value (double): The numeric value to format.
     * Returns:
     * void: No return value.
     */
    double abs_val = fabs(value);
    if (abs_val != 0.0 && (abs_val >= 1e10 || abs_val < 1e-6)) {
        snprintf(buffer, size, "%.10e", value);
    } else {
        snprintf(buffer, size, "%.10g", value);
    }
}

Calculator* calculatorNew(void) {
    /**
     * Creates and initializes a new Calculator instance.
     * Returns:
     * Calculator*: A pointer to the newly allocated Calculator structure.
     */
    Calculator* calc = (Calculator*)malloc(sizeof(Calculator));
    if (calc) {
        strcpy(calc->buffer, "0");
        calc->angle_mode = DEG;
        calc->error = ERROR_NONE;
        calc->numbers.top = -1;
        calc->operators.top = -1;
    }
    return calc;
}

void calculatorFree(Calculator* calc) {
    /**
     * Frees the memory allocated for a Calculator instance.
     * Args:
     * calc (Calculator*): The calculator context pointer to free.
     * Returns:
     * void: No return value.
     */
    if (calc) {
        free(calc);
    }
}

void calculatorClear(Calculator* calc) {
    /**
     * Resets the display buffer and error state of the calculator.
     * Args:
     * calc (Calculator*): The calculator context pointer.
     * Returns:
     * void: No return value.
     */
    strcpy(calc->buffer, "0");
    calc->error = ERROR_NONE;
}

void calculatorToggleAngleMode(Calculator* calc) {
    /**
     * Toggles the calculator angle mode between degrees and radians.
     * Args:
     * calc (Calculator*): The calculator context pointer.
     * Returns:
     * void: No return value.
     */
    if (calc->angle_mode == DEG) {
        calc->angle_mode = RAD;
    } else {
        calc->angle_mode = DEG;
    }
}

AngleMode calculatorGetAngleMode(const Calculator* calc) {
    /**
     * Retrieves the current angle mode of the calculator.
     * Args:
     * calc (const Calculator*): The calculator context pointer.
     * Returns:
     * AngleMode: The current angle mode (DEG or RAD).
     */
    return calc ? calc->angle_mode : DEG;
}

const char* calculatorGetDisplay(const Calculator* calc) {
    /**
     * Retrieves the current string representation of the calculator display.
     * Args:
     * calc (const Calculator*): The calculator context pointer.
     * Returns:
     * const char*: The display string buffer.
     */
    return calc->buffer;
}

void calculatorEvaluate(Calculator* calc, const char* expression) {
    /**
     * Evaluates a mathematical expression and updates the calculator display with the result.
     * Args:
     * calc (Calculator*): The calculator context pointer.
     * expression (const char*): The string expression to evaluate.
     * Returns:
     * void: No return value.
     */
    calc->numbers.top = -1;
    calc->operators.top = -1;
    calc->error = ERROR_NONE;

    TokenType prev_token = TOKEN_NONE;
    const char* p = expression;

    while (*p) {
        if (isspace((unsigned char)*p)) {
            p++;
            continue;
        }

        if (isdigit((unsigned char)*p) || *p == '.' || ((*p == '+' || *p == '-') && (prev_token == TOKEN_NONE || prev_token == TOKEN_OPERATOR || prev_token == TOKEN_LPAREN || prev_token == TOKEN_FUNCTION) && (isdigit((unsigned char)*(p + 1)) || *(p + 1) == '.'))) {
            // Rejects consecutive decimal points or decimal point immediately following a value to avoid syntax ambiguity.
            if (*p == '.' && (prev_token == TOKEN_NUMBER || prev_token == TOKEN_RPAREN || prev_token == TOKEN_CONSTANT)) {
                calc->error = ERROR_SYNTAX;
                snprintf(calc->buffer, sizeof(calc->buffer), "Syntax Error: Invalid expression");
                return;
            }
            if (!insertImplicitMultiplication(calc, &prev_token, TOKEN_NUMBER)) {
                break;
            }

            const char* start = p;

            // Attempts to parse a double precision number with an optional leading sign.
            char* end;
            double num = strtod(start, &end);
            if (end == start) {
                calc->error = ERROR_SYNTAX;
                snprintf(calc->buffer, sizeof(calc->buffer), "Syntax Error: Invalid expression");
                return;
            }
            // Checks if the token immediately following a parsed number is a decimal point to reject invalid floating point formats.
            if (*end == '.') {
                calc->error = ERROR_SYNTAX;
                snprintf(calc->buffer, sizeof(calc->buffer), "Syntax Error: Invalid expression");
                return;
            }

            if (!nsPush(&calc->numbers, num)) {
                calc->error = ERROR_STACK_OVERFLOW;
                break;
            }
            p = end;
            prev_token = TOKEN_NUMBER;
            continue;
        } else if (*p == 'p') {
            if (!insertImplicitMultiplication(calc, &prev_token, TOKEN_CONSTANT)) {
                break;
            }
            if (!nsPush(&calc->numbers, M_PI)) {
                calc->error = ERROR_STACK_OVERFLOW;
                break;
            }
            prev_token = TOKEN_CONSTANT;
        } else if (*p == 'e') {
            if (!insertImplicitMultiplication(calc, &prev_token, TOKEN_CONSTANT)) {
                break;
            }
            if (!nsPush(&calc->numbers, M_E)) {
                calc->error = ERROR_STACK_OVERFLOW;
                break;
            }
            prev_token = TOKEN_CONSTANT;
        } else if (*p == '(') {
            if (!insertImplicitMultiplication(calc, &prev_token, TOKEN_LPAREN)) {
                break;
            }
            if (!osPush(&calc->operators, *p)) {
                calc->error = ERROR_STACK_OVERFLOW;
                break;
            }
            prev_token = TOKEN_LPAREN;
        } else if (*p == ')') {
            while (calc->operators.top != -1 && osPeek(&calc->operators) != '(') {
                applyOperator(calc, osPop(&calc->operators));
                if (calc->error != ERROR_NONE) {
                    break;
                }
            }
            if (calc->error != ERROR_NONE) {
                break;
            }

            if (calc->operators.top != -1) {
                osPop(&calc->operators);
            } else {
                calc->error = ERROR_SYNTAX;
                snprintf(calc->buffer, sizeof(calc->buffer), "Syntax Error: Mismatched parentheses");
                return;
            }
            prev_token = TOKEN_RPAREN;
        } else if (isalpha((unsigned char)*p)) {
            if (!insertImplicitMultiplication(calc, &prev_token, TOKEN_FUNCTION)) {
                break;
            }
            char func[MAX_FUNCTION_NAME_LENGTH];
            int i = 0;
            while (isalpha((unsigned char)*p) && i < MAX_FUNCTION_NAME_LENGTH - 1) {
                func[i++] = *p++;
            }
            func[i] = '\0';
            if (!isValidFunctionName(func)) {
                calc->error = ERROR_SYNTAX;
                snprintf(calc->buffer, sizeof(calc->buffer), "Syntax Error: Unknown function '%s'", func);
                return;
            }
            char mapped_op = lookupFunctionName(func);
            if (mapped_op == '\0') {
                mapped_op = func[0];
            }
            if (!osPush(&calc->operators, mapped_op)) {
                calc->error = ERROR_STACK_OVERFLOW;
                break;
            }
            p--;
            prev_token = TOKEN_FUNCTION;
        } else {
            if (!processOperatorToken(calc, *p)) {
                break;
            }
            prev_token = TOKEN_OPERATOR;
        }
        p++;
    }

    while (calc->operators.top != -1 && calc->error == ERROR_NONE) {
        if (osPeek(&calc->operators) == '(') {
            calc->error = ERROR_SYNTAX;
            break;
        }
        applyOperator(calc, osPop(&calc->operators));
    }

    if (calc->error != ERROR_NONE) {
        if (calc->error == ERROR_MATH_DIV_ZERO) {
            snprintf(calc->buffer, sizeof(calc->buffer), "Math Error: Division by zero");
        } else if (calc->error == ERROR_MATH_DOMAIN) {
            snprintf(calc->buffer, sizeof(calc->buffer), "Math Error: Domain error (e.g., sqrt(-1))");
        } else if (calc->error == ERROR_STACK_OVERFLOW) {
            snprintf(calc->buffer, sizeof(calc->buffer), "Error: Operator stack overflow");
        } else if (calc->error == ERROR_SYNTAX) {
            snprintf(calc->buffer, sizeof(calc->buffer), "Syntax Error: Mismatched parentheses");
        }
        return;
    }

    if (calc->numbers.top == 0) {
        double val = nsPop(&calc->numbers, calc);
        if (isnan(val)) {
            if (calc->error == ERROR_MATH_DIV_ZERO) {
                snprintf(calc->buffer, sizeof(calc->buffer), "Math Error: Division by zero");
            } else {
                snprintf(calc->buffer, sizeof(calc->buffer), "Math Error: Domain error (e.g., sqrt(-1))");
            }
        } else if (!isfinite(val)) {
            snprintf(calc->buffer, sizeof(calc->buffer), "Error: Overflow");
        } else {
            formatResult(calc->buffer, sizeof(calc->buffer), val);
        }
    } else if (calc->error == ERROR_NONE) {
        snprintf(calc->buffer, sizeof(calc->buffer), "Syntax Error: Invalid expression");
    }
}

int nsPush(NumberStack* s, double item) {
    /**
     * Pushes a double value onto the number stack.
     * Args:
     * s (NumberStack*): The target number stack.
     * item (double): The value to push.
     * Returns:
     * int: Non-zero if successful, zero if the stack is full.
     */
    if (s->top < MAX_STACK_SIZE - 1) {
        s->items[++s->top] = item;
        return 1;
    }
    return 0;
}

double nsPop(NumberStack* s, Calculator* calc) {
    /**
     * Pops a double value from the number stack.
     * Args:
     * s (NumberStack*): The target number stack.
     * calc (Calculator*): The calculator context pointer used to set error state.
     * Returns:
     * double: The popped value, or 0.0 if the stack is empty.
     */
    if (s->top > -1) {
        return s->items[s->top--];
    }
    if (calc) {
        calc->error = ERROR_SYNTAX;
    }
    return 0.0;
}

int osPush(OperatorStack* s, char item) {
    /**
     * Pushes an operator character onto the operator stack.
     * Args:
     * s (OperatorStack*): The target operator stack.
     * item (char): The operator character to push.
     * Returns:
     * int: Non-zero if successful, zero if stack is full.
     */
    if (s->top >= MAX_STACK_SIZE - 1) {
        return 0;
    }
    s->items[++s->top] = item;
    return 1;
}

char osPop(OperatorStack* s) {
    /**
     * Pops an operator character from the operator stack.
     * Args:
     * s (OperatorStack*): The target operator stack.
     * Returns:
     * char: The popped operator character, or null character if empty.
     */
    if (s->top > -1) {
        return s->items[s->top--];
    }
    return '\0';
}

char osPeek(OperatorStack* s) {
    /**
     * Peeks at the top operator character without removing it.
     * Args:
     * s (OperatorStack*): The target operator stack.
     * Returns:
     * char: The top operator character, or null character if empty.
     */
    if (s->top > -1) {
        return s->items[s->top];
    }
    return '\0';
}

int getPrecedence(char op) {
    /**
     * Returns the precedence level of a given operator.
     * Args:
     * op (char): The operator character.
     * Returns:
     * int: The precedence level (higher is greater precedence).
     */
    switch (op) {
        case '+': case '-': return 1;
        case '*': case '/': case '%': return 2;
        case '^': return 3;
        case 's': case 'c': case 't': case 'l': case 'L': case 'q': case '!': case 'S': case 'C': case 'T': case 'E': case 'R': case 'N': return 4;
        default: return 0;
    }
}

int isRightAssociative(char op) {
    /**
     * Determines if a given operator is right-associative.
     * Args:
     * op (char): The operator character.
     * Returns:
     * int: Non-zero if right-associative, zero otherwise.
     */
    return op == '^';
}

void applyOperator(Calculator* calc, char op) {
    /**
     * Applies the given operator to the top values on the number stack.
     * Args:
     * calc (Calculator*): The calculator context pointer.
     * op (char): The operator character to apply.
     * Returns:
     * void: No return value.
     */
    double a, b;
    NumberStack* numbers = &calc->numbers;
    AngleMode angle_mode = calc->angle_mode;

    switch (op) {
        case '+': b = nsPop(numbers, calc); a = nsPop(numbers, calc); nsPush(numbers, a + b); break;
        case '-': b = nsPop(numbers, calc); a = nsPop(numbers, calc); nsPush(numbers, a - b); break;
        case '*': b = nsPop(numbers, calc); a = nsPop(numbers, calc); nsPush(numbers, a * b); break;
        case '/': 
            b = nsPop(numbers, calc); a = nsPop(numbers, calc); 
            if (b == 0.0) {
                calc->error = ERROR_MATH_DIV_ZERO;
                nsPush(numbers, NAN);
            } else {
                nsPush(numbers, a / b);
            }
            break;
        case '%': 
            b = nsPop(numbers, calc); a = nsPop(numbers, calc); 
            if (b == 0.0) {
                calc->error = ERROR_MATH_DIV_ZERO;
                nsPush(numbers, NAN);
            }
            else {
                nsPush(numbers, fmod(a, b));
            }
            break;
        case '^': b = nsPop(numbers, calc); a = nsPop(numbers, calc); nsPush(numbers, pow(a, b)); break;

        case 's': 
            if (numbers->top < 0) {
                calc->error = ERROR_SYNTAX;
                nsPush(numbers, NAN);
                return;
            }
            a = nsPop(numbers, calc); 
            nsPush(numbers, angle_mode == DEG ? sin(a * M_PI / 180.0) : sin(a)); 
            break;
        case 'c': 
            if (numbers->top < 0) {
                calc->error = ERROR_SYNTAX;
                nsPush(numbers, NAN);
                return;
            }
            a = nsPop(numbers, calc); 
            nsPush(numbers, angle_mode == DEG ? cos(a * M_PI / 180.0) : cos(a)); 
            break;
        case 't': 
            if (numbers->top < 0) {
                calc->error = ERROR_SYNTAX;
                nsPush(numbers, NAN);
                return;
            }
            a = nsPop(numbers, calc); 
            nsPush(numbers, angle_mode == DEG ? tan(a * M_PI / 180.0) : tan(a)); 
            break;

        case 'S': 
            if (numbers->top < 0) {
                calc->error = ERROR_SYNTAX;
                nsPush(numbers, NAN);
                return;
            }
            a = nsPop(numbers, calc); 
            nsPush(numbers, angle_mode == DEG ? asin(a) * 180.0 / M_PI : asin(a)); 
            break;
        case 'C': 
            if (numbers->top < 0) {
                calc->error = ERROR_SYNTAX;
                nsPush(numbers, NAN);
                return;
            }
            a = nsPop(numbers, calc); 
            nsPush(numbers, angle_mode == DEG ? acos(a) * 180.0 / M_PI : acos(a)); 
            break;
        case 'T': 
            if (numbers->top < 0) {
                calc->error = ERROR_SYNTAX;
                nsPush(numbers, NAN);
                return;
            }
            a = nsPop(numbers, calc); 
            nsPush(numbers, angle_mode == DEG ? atan(a) * 180.0 / M_PI : atan(a)); 
            break;

        case 'l': 
            if (numbers->top < 0) {
                calc->error = ERROR_SYNTAX;
                nsPush(numbers, NAN);
                return;
            }
            a = nsPop(numbers, calc); 
            if (a <= 0.0) {
                calc->error = ERROR_MATH_DOMAIN;
                nsPush(numbers, NAN);
            } else {
                nsPush(numbers, log(a));
            }
            break;
        case 'L': 
            if (numbers->top < 0) {
                calc->error = ERROR_SYNTAX;
                nsPush(numbers, NAN);
                return;
            }
            a = nsPop(numbers, calc); 
            if (a <= 0.0) {
                calc->error = ERROR_MATH_DOMAIN;
                nsPush(numbers, NAN);
            } else {
                nsPush(numbers, log10(a));
            }
            break;
        case 'q': 
            if (numbers->top < 0) {
                calc->error = ERROR_SYNTAX;
                nsPush(numbers, NAN);
                return;
            }
            a = nsPop(numbers, calc); 
            if (a < 0.0) {
                calc->error = ERROR_MATH_DOMAIN;
                nsPush(numbers, NAN);
            }
            else {
                nsPush(numbers, sqrt(a));
            }
            break;
        case '!': 
            if (numbers->top < 0) {
                calc->error = ERROR_SYNTAX;
                nsPush(numbers, NAN);
                return;
            }
            a = nsPop(numbers, calc); 
            nsPush(numbers, factorial(a, calc)); 
            break;
        case 'E': 
            if (numbers->top < 0) {
                calc->error = ERROR_SYNTAX;
                nsPush(numbers, NAN);
                return;
            }
            a = nsPop(numbers, calc); 
            nsPush(numbers, exp(a)); 
            break;
        case 'R': 
            if (numbers->top < 0) {
                calc->error = ERROR_SYNTAX;
                nsPush(numbers, NAN);
                return;
            }
            a = nsPop(numbers, calc); 
            if (a == 0.0) {
                calc->error = ERROR_MATH_DIV_ZERO;
                nsPush(numbers, NAN);
            } else {
                nsPush(numbers, 1.0 / a);
            }
            break;
        case 'N': 
            if (numbers->top < 0) {
                calc->error = ERROR_SYNTAX;
                nsPush(numbers, NAN);
                return;
            }
            a = nsPop(numbers, calc); 
            nsPush(numbers, -a); 
            break;
        default: 
            calc->error = ERROR_SYNTAX;
            snprintf(calc->buffer, sizeof(calc->buffer), "Syntax Error: Unknown operator");
            break;
    }
}

double factorial(double n, Calculator* calc) {
    /**
     * Computes the factorial of a given double value.
     * Args:
     * n (double): The non-negative integer value.
     * calc (Calculator*): The calculator context pointer to record domain errors.
     * Returns:
     * double: The factorial result, or NAN on error.
     */
    if (n < 0 || floor(n) != n) {
        calc->error = ERROR_MATH_DOMAIN;
        return NAN;
    }
    if (n == 0) return 1;
    double result = 1;
    int ni = (int)n;
    for (int i = 1; i <= ni; i++) {
        result *= i;
        if (!isfinite(result)) {
            calc->error = ERROR_MATH_DOMAIN;
            return NAN;
        }
    }
    return result;
}
