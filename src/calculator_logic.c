#include "calculator_logic.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
#ifndef M_E
#define M_E 2.71828182845904523536
#endif

// Function prototypes for stack operations
int ns_push(NumberStack* s, double item);
double ns_pop(NumberStack* s);
int os_push(OperatorStack* s, char item);
char os_pop(OperatorStack* s);
char os_peek(OperatorStack* s);
int get_precedence(char op);
void apply_operator(Calculator* calc, char op);
double factorial(double n);
int is_right_associative(char op);

Calculator* calculator_new(void) {
    Calculator* calc = (Calculator*)malloc(sizeof(Calculator));
    if (calc) {
        strcpy(calc->buffer, "0");
        calc->angle_mode = DEG;
        calc->error = ERROR_NONE;
        calc->numbers.top = -1;
        calc->operators.top = -1;
        calc->operators.total_pushed = 0;
    }
    return calc;
}

void calculator_free(Calculator* calc) {
    if (calc) {
        free(calc);
    }
}

void calculator_clear(Calculator* calc) {
    strcpy(calc->buffer, "0");
    calc->error = ERROR_NONE;
}

void calculator_toggle_angle_mode(Calculator* calc) {
    if (calc->angle_mode == DEG) {
        calc->angle_mode = RAD;
    } else {
        calc->angle_mode = DEG;
    }
}

AngleMode calculator_get_angle_mode(const Calculator* calc) {
    return calc ? calc->angle_mode : DEG;
}

const char* calculator_get_display(const Calculator* calc) {
    return calc->buffer;
}

void calculator_evaluate(Calculator* calc, const char* expression) {
    calc->numbers.top = -1;
    calc->operators.top = -1;
    calc->operators.total_pushed = 0;
    calc->error = ERROR_NONE;

    const char* p = expression;

    while (*p) {
        if (isspace((unsigned char)*p)) { p++; continue; }
        if (isdigit((unsigned char)*p) || *p == '.') {
            char* end;
            double num = strtod(p, &end);
            if (end == p) {
                calc->error = ERROR_SYNTAX;
                snprintf(calc->buffer, sizeof(calc->buffer), "Syntax Error: Invalid expression");
                return;
            }
            if (!ns_push(&calc->numbers, num)) {
                calc->error = ERROR_STACK_OVERFLOW;
                break;
            }
            p = end;
            continue;
        } else if (*p == 'p') {
            if (!ns_push(&calc->numbers, M_PI)) {
                calc->error = ERROR_STACK_OVERFLOW;
                break;
            }
        } else if (*p == 'e') {
            if (!ns_push(&calc->numbers, M_E)) {
                calc->error = ERROR_STACK_OVERFLOW;
                break;
            }
        } else if (*p == '(') {
            if (!os_push(&calc->operators, *p)) {
                calc->error = ERROR_STACK_OVERFLOW;
                break;
            }
        } else if (*p == ')') {
            while (calc->operators.top != -1 && os_peek(&calc->operators) != '(') {
                apply_operator(calc, os_pop(&calc->operators));
                if (calc->error != ERROR_NONE) break;
            }
            if (calc->error != ERROR_NONE) break;

            if (calc->operators.top != -1) {
                os_pop(&calc->operators); // Pop '('
            } else {
                calc->error = ERROR_SYNTAX;
                snprintf(calc->buffer, sizeof(calc->buffer), "Syntax Error: Mismatched parentheses");
                return;
            }
        } else if (isalpha((unsigned char)*p)) {
            char func[MAX_FUNCTION_NAME_LENGTH];
            int i = 0;
            while (isalpha((unsigned char)*p) && i < MAX_FUNCTION_NAME_LENGTH - 1) {
                func[i++] = *p++;
            }
            func[i] = '\0';
            if (!os_push(&calc->operators, func[0])) { // Simple mapping NOT recommended to be changed later on
                calc->error = ERROR_STACK_OVERFLOW;
                break;
            }
            p--; // Decrement to handle the next character correctly
        } else { // Operator
            while (calc->operators.top != -1) {
                char top_op = os_peek(&calc->operators);
                int top_prec = get_precedence(top_op);
                int curr_prec = get_precedence(*p);
                if (top_prec > curr_prec || (top_prec == curr_prec && !is_right_associative(*p))) {
                    apply_operator(calc, os_pop(&calc->operators));
                    if (calc->error != ERROR_NONE) break;
                } else {
                    break;
                }
            }
            if (calc->error != ERROR_NONE) break;
            if (!os_push(&calc->operators, *p)) {
                calc->error = ERROR_STACK_OVERFLOW;
                break;
            }
        }
        p++;
    }

    while (calc->operators.top != -1 && calc->error == ERROR_NONE) {
        if (os_peek(&calc->operators) == '(') {
            calc->error = ERROR_SYNTAX;
            break;
        }
        apply_operator(calc, os_pop(&calc->operators));
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
        double val = ns_pop(&calc->numbers);
        if (isnan(val)) {
             if (calc->error == ERROR_MATH_DIV_ZERO) {
                snprintf(calc->buffer, sizeof(calc->buffer), "Math Error: Division by zero");
            } else {
                snprintf(calc->buffer, sizeof(calc->buffer), "Math Error: Domain error (e.g., sqrt(-1))");
            }
        } else if (!isfinite(val)) {
            snprintf(calc->buffer, sizeof(calc->buffer), "Error: Overflow");
        } else {
            snprintf(calc->buffer, sizeof(calc->buffer), "%.10g", val);
        }
    } else if (calc->error == ERROR_NONE) {
        snprintf(calc->buffer, sizeof(calc->buffer), "Syntax Error: Invalid expression");
    }
}

// Stack implementations
int ns_push(NumberStack* s, double item) {
    if (s->top < MAX_STACK_SIZE - 1) {
        s->items[++s->top] = item;
        return 1;
    }
    return 0;
}

double ns_pop(NumberStack* s) {
    if (s->top > -1) {
        return s->items[s->top--];
    }
    return 0.0; // Should handle error
}

int os_push(OperatorStack* s, char item) {
    if (s->top >= MAX_STACK_SIZE - 1) {
        return 0;
    }
    if (s->total_pushed >= MAX_STACK_SIZE) {
        return 0;
    }
    s->items[++s->top] = item;
    s->total_pushed++;
    return 1;
}

char os_pop(OperatorStack* s) {
    if (s->top > -1) {
        return s->items[s->top--];
    }
    return '\0'; // Should handle error
}

char os_peek(OperatorStack* s) {
    if (s->top > -1) {
        return s->items[s->top];
    }
    return '\0'; // Should handle error
}

int get_precedence(char op) {
    switch (op) {
        case '+': case '-': return 1;
        case '*': case '/': case '%': return 2;
        case '^': return 3; // exponent should be right-associative
        case 's': case 'c': case 't': case 'l': case 'L': case 'q': case '!': case 'S': case 'C': case 'T': case 'E': case 'R': case 'N': return 4;
        default: return 0;
    }
}
int is_right_associative(char op) {
    return op == '^';
}

void apply_operator(Calculator* calc, char op) {
    double a, b;
    NumberStack* numbers = &calc->numbers;
    AngleMode angle_mode = calc->angle_mode;

    switch (op) {
        case '+': b = ns_pop(numbers); a = ns_pop(numbers); ns_push(numbers, a + b); break;
        case '-': b = ns_pop(numbers); a = ns_pop(numbers); ns_push(numbers, a - b); break;
        case '*': b = ns_pop(numbers); a = ns_pop(numbers); ns_push(numbers, a * b); break;
        case '/': 
            b = ns_pop(numbers); a = ns_pop(numbers); 
            if (b == 0.0) {
                calc->error = ERROR_MATH_DIV_ZERO;
                ns_push(numbers, NAN);
            } else {
                ns_push(numbers, a / b);
            }
            break;
        case '%': 
            b = ns_pop(numbers); a = ns_pop(numbers); 
            if (b == 0.0) {
                calc->error = ERROR_MATH_DIV_ZERO;
                ns_push(numbers, NAN);
            }
            else {
                ns_push(numbers, fmod(a, b));
            }
            break;
        case '^': b = ns_pop(numbers); a = ns_pop(numbers); ns_push(numbers, pow(a, b)); break;

        case 's': a = ns_pop(numbers); ns_push(numbers, angle_mode == DEG ? sin(a * M_PI / 180.0) : sin(a)); break;
        case 'c': a = ns_pop(numbers); ns_push(numbers, angle_mode == DEG ? cos(a * M_PI / 180.0) : cos(a)); break;
        case 't': a = ns_pop(numbers); ns_push(numbers, angle_mode == DEG ? tan(a * M_PI / 180.0) : tan(a)); break;

        case 'S': a = ns_pop(numbers); ns_push(numbers, angle_mode == DEG ? asin(a) * 180.0 / M_PI : asin(a)); break;
        case 'C': a = ns_pop(numbers); ns_push(numbers, angle_mode == DEG ? acos(a) * 180.0 / M_PI : acos(a)); break;
        case 'T': a = ns_pop(numbers); ns_push(numbers, angle_mode == DEG ? atan(a) * 180.0 / M_PI : atan(a)); break;

        case 'l': 
            a = ns_pop(numbers); 
            if (a <= 0.0) {
                calc->error = ERROR_MATH_DOMAIN;
                ns_push(numbers, NAN);
            } else {
                ns_push(numbers, log(a));
            }
            break;
        case 'L': 
            a = ns_pop(numbers); 
            if (a <= 0.0) {
                calc->error = ERROR_MATH_DOMAIN;
                ns_push(numbers, NAN);
            } else {
                ns_push(numbers, log10(a));
            }
            break;
        case 'q': 
            a = ns_pop(numbers); 
            if (a < 0.0) {
                calc->error = ERROR_MATH_DOMAIN;
                ns_push(numbers, NAN);
            }
            else {
                ns_push(numbers, sqrt(a));
            }
            break;
        case '!': a = ns_pop(numbers); ns_push(numbers, factorial(a)); break;
        case 'E': a = ns_pop(numbers); ns_push(numbers, exp(a)); break;
        case 'R': 
            a = ns_pop(numbers); 
            if (a == 0.0) {
                calc->error = ERROR_MATH_DIV_ZERO;
                ns_push(numbers, NAN);
            } else {
                ns_push(numbers, 1.0 / a);
            }
            break;
        case 'N': a = ns_pop(numbers); ns_push(numbers, -a); break;
        default: break;
    }
}

double factorial(double n) {
    if (n < 0 || floor(n) != n) return NAN; // Only defined for non-negative integers
    if (n == 0) return 1;
    double result = 1;
    int ni = (int)n;
    for (int i = 1; i <= ni; i++) {
        result *= i;
        if (!isfinite(result)) return NAN;
    }
    return result;
}
