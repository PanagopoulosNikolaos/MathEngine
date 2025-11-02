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

// Function prototypes for stack operations
int ns_push(NumberStack* s, double item);
double ns_pop(NumberStack* s, Calculator* calc);
int os_push(OperatorStack* s, char item);
char os_pop(OperatorStack* s);
char os_peek(OperatorStack* s);
int get_precedence(char op);
void apply_operator(Calculator* calc, char op);
double factorial(double n, Calculator* calc);
int is_right_associative(char op);

typedef enum {
    TOKEN_NONE,
    TOKEN_NUMBER,
    TOKEN_OPERATOR,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_FUNCTION,
    TOKEN_CONSTANT
} TokenType;

static int needs_implicit_multiplication(TokenType prev, TokenType current) {
    if (prev == TOKEN_NONE) {
        return 0;
    }

    int prev_is_value = (prev == TOKEN_NUMBER || prev == TOKEN_RPAREN || prev == TOKEN_CONSTANT);
    int current_is_value = (current == TOKEN_LPAREN || current == TOKEN_NUMBER || current == TOKEN_CONSTANT || current == TOKEN_FUNCTION);

    return prev_is_value && current_is_value;
}

static int process_operator_token(Calculator* calc, char op) {
    while (calc->operators.top != -1) {
        char top_op = os_peek(&calc->operators);
        int top_prec = get_precedence(top_op);
        int curr_prec = get_precedence(op);

        if (top_prec > curr_prec || (top_prec == curr_prec && !is_right_associative(op))) {
            apply_operator(calc, os_pop(&calc->operators));
            if (calc->error != ERROR_NONE) {
                return 0;
            }
        } else {
            break;
        }
    }

    if (!os_push(&calc->operators, op)) {
        calc->error = ERROR_STACK_OVERFLOW;
        return 0;
    }

    return 1;
}

static int insert_implicit_multiplication(Calculator* calc, TokenType* prev_token, TokenType current_token) {
    if (needs_implicit_multiplication(*prev_token, current_token)) {
        if (!process_operator_token(calc, '*')) {
            return 0;
        }
        *prev_token = TOKEN_OPERATOR;
    }
    return 1;
}

static void format_result(char* buffer, size_t size, double value) {
    double abs_val = fabs(value);
    if (abs_val != 0.0 && (abs_val >= 1e10 || abs_val < 1e-6)) {
        snprintf(buffer, size, "%.10e", value);
    } else {
        snprintf(buffer, size, "%.10g", value);
    }
}

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

    TokenType prev_token = TOKEN_NONE;
    const char* p = expression;

    while (*p) {
        if (isspace((unsigned char)*p)) {
            p++;
            continue;
        }

        if (isdigit((unsigned char)*p) || *p == '.' || ((*p == '+' || *p == '-') && (prev_token == TOKEN_NONE || prev_token == TOKEN_OPERATOR || prev_token == TOKEN_LPAREN || prev_token == TOKEN_FUNCTION) && (isdigit((unsigned char)*(p + 1)) || *(p + 1) == '.'))) {
            // Disallow a fractional token starting with '.' immediately after a value (e.g., "2.3.4")
            if (*p == '.' && (prev_token == TOKEN_NUMBER || prev_token == TOKEN_RPAREN || prev_token == TOKEN_CONSTANT)) {
                calc->error = ERROR_SYNTAX;
                snprintf(calc->buffer, sizeof(calc->buffer), "Syntax Error: Invalid expression");
                return;
            }
            if (!insert_implicit_multiplication(calc, &prev_token, TOKEN_NUMBER)) {
                break;
            }

            const char* start = p;

            // Parse number with optional leading sign using strtod
            char* end;
            double num = strtod(start, &end);
            if (end == start) {
                calc->error = ERROR_SYNTAX;
                snprintf(calc->buffer, sizeof(calc->buffer), "Syntax Error: Invalid expression");
                return;
            }
            // If a '.' immediately follows a completed number (e.g., "2.3.4" or "2..3"), it's invalid
            if (*end == '.') {
                calc->error = ERROR_SYNTAX;
                snprintf(calc->buffer, sizeof(calc->buffer), "Syntax Error: Invalid expression");
                return;
            }

            if (!ns_push(&calc->numbers, num)) {
                calc->error = ERROR_STACK_OVERFLOW;
                break;
            }
            p = end;
            prev_token = TOKEN_NUMBER;
            continue;
        } else if (*p == 'p') {
            if (!insert_implicit_multiplication(calc, &prev_token, TOKEN_CONSTANT)) {
                break;
            }
            if (!ns_push(&calc->numbers, M_PI)) {
                calc->error = ERROR_STACK_OVERFLOW;
                break;
            }
            prev_token = TOKEN_CONSTANT;
        } else if (*p == 'e') {
            if (!insert_implicit_multiplication(calc, &prev_token, TOKEN_CONSTANT)) {
                break;
            }
            if (!ns_push(&calc->numbers, M_E)) {
                calc->error = ERROR_STACK_OVERFLOW;
                break;
            }
            prev_token = TOKEN_CONSTANT;
        } else if (*p == '(') {
            if (!insert_implicit_multiplication(calc, &prev_token, TOKEN_LPAREN)) {
                break;
            }
            if (!os_push(&calc->operators, *p)) {
                calc->error = ERROR_STACK_OVERFLOW;
                break;
            }
            prev_token = TOKEN_LPAREN;
        } else if (*p == ')') {
            while (calc->operators.top != -1 && os_peek(&calc->operators) != '(') {
                apply_operator(calc, os_pop(&calc->operators));
                if (calc->error != ERROR_NONE) {
                    break;
                }
            }
            if (calc->error != ERROR_NONE) {
                break;
            }

            if (calc->operators.top != -1) {
                os_pop(&calc->operators);
            } else {
                calc->error = ERROR_SYNTAX;
                snprintf(calc->buffer, sizeof(calc->buffer), "Syntax Error: Mismatched parentheses");
                return;
            }
            prev_token = TOKEN_RPAREN;
        } else if (isalpha((unsigned char)*p)) {
            if (!insert_implicit_multiplication(calc, &prev_token, TOKEN_FUNCTION)) {
                break;
            }
            char func[MAX_FUNCTION_NAME_LENGTH];
            int i = 0;
            while (isalpha((unsigned char)*p) && i < MAX_FUNCTION_NAME_LENGTH - 1) {
                func[i++] = *p++;
            }
            func[i] = '\0';
            if (!os_push(&calc->operators, func[0])) {
                calc->error = ERROR_STACK_OVERFLOW;
                break;
            }
            p--;
            prev_token = TOKEN_FUNCTION;
        } else {
            if (!process_operator_token(calc, *p)) {
                break;
            }
            prev_token = TOKEN_OPERATOR;
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
        double val = ns_pop(&calc->numbers, calc);
        if (isnan(val)) {
            if (calc->error == ERROR_MATH_DIV_ZERO) {
                snprintf(calc->buffer, sizeof(calc->buffer), "Math Error: Division by zero");
            } else {
                snprintf(calc->buffer, sizeof(calc->buffer), "Math Error: Domain error (e.g., sqrt(-1))");
            }
        } else if (!isfinite(val)) {
            snprintf(calc->buffer, sizeof(calc->buffer), "Error: Overflow");
        } else {
            format_result(calc->buffer, sizeof(calc->buffer), val);
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

double ns_pop(NumberStack* s, Calculator* calc) {
    if (s->top > -1) {
        return s->items[s->top--];
    }
    if (calc) {
        calc->error = ERROR_SYNTAX;
    }
    return 0.0;
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
        case '+': b = ns_pop(numbers, calc); a = ns_pop(numbers, calc); ns_push(numbers, a + b); break;
        case '-': b = ns_pop(numbers, calc); a = ns_pop(numbers, calc); ns_push(numbers, a - b); break;
        case '*': b = ns_pop(numbers, calc); a = ns_pop(numbers, calc); ns_push(numbers, a * b); break;
        case '/': 
            b = ns_pop(numbers, calc); a = ns_pop(numbers, calc); 
            if (b == 0.0) {
                calc->error = ERROR_MATH_DIV_ZERO;
                ns_push(numbers, NAN);
            } else {
                ns_push(numbers, a / b);
            }
            break;
        case '%': 
            b = ns_pop(numbers, calc); a = ns_pop(numbers, calc); 
            if (b == 0.0) {
                calc->error = ERROR_MATH_DIV_ZERO;
                ns_push(numbers, NAN);
            }
            else {
                ns_push(numbers, fmod(a, b));
            }
            break;
        case '^': b = ns_pop(numbers, calc); a = ns_pop(numbers, calc); ns_push(numbers, pow(a, b)); break;

        case 's': 
            if (numbers->top < 0) {
                calc->error = ERROR_SYNTAX;
                ns_push(numbers, NAN);
                return;
            }
            a = ns_pop(numbers, calc); 
            ns_push(numbers, angle_mode == DEG ? sin(a * M_PI / 180.0) : sin(a)); 
            break;
        case 'c': 
            if (numbers->top < 0) {
                calc->error = ERROR_SYNTAX;
                ns_push(numbers, NAN);
                return;
            }
            a = ns_pop(numbers, calc); 
            ns_push(numbers, angle_mode == DEG ? cos(a * M_PI / 180.0) : cos(a)); 
            break;
        case 't': 
            if (numbers->top < 0) {
                calc->error = ERROR_SYNTAX;
                ns_push(numbers, NAN);
                return;
            }
            a = ns_pop(numbers, calc); 
            ns_push(numbers, angle_mode == DEG ? tan(a * M_PI / 180.0) : tan(a)); 
            break;

        case 'S': 
            if (numbers->top < 0) {
                calc->error = ERROR_SYNTAX;
                ns_push(numbers, NAN);
                return;
            }
            a = ns_pop(numbers, calc); 
            ns_push(numbers, angle_mode == DEG ? asin(a) * 180.0 / M_PI : asin(a)); 
            break;
        case 'C': 
            if (numbers->top < 0) {
                calc->error = ERROR_SYNTAX;
                ns_push(numbers, NAN);
                return;
            }
            a = ns_pop(numbers, calc); 
            ns_push(numbers, angle_mode == DEG ? acos(a) * 180.0 / M_PI : acos(a)); 
            break;
        case 'T': 
            if (numbers->top < 0) {
                calc->error = ERROR_SYNTAX;
                ns_push(numbers, NAN);
                return;
            }
            a = ns_pop(numbers, calc); 
            ns_push(numbers, angle_mode == DEG ? atan(a) * 180.0 / M_PI : atan(a)); 
            break;

        case 'l': 
            if (numbers->top < 0) {
                calc->error = ERROR_SYNTAX;
                ns_push(numbers, NAN);
                return;
            }
            a = ns_pop(numbers, calc); 
            if (a <= 0.0) {
                calc->error = ERROR_MATH_DOMAIN;
                ns_push(numbers, NAN);
            } else {
                ns_push(numbers, log(a));
            }
            break;
        case 'L': 
            if (numbers->top < 0) {
                calc->error = ERROR_SYNTAX;
                ns_push(numbers, NAN);
                return;
            }
            a = ns_pop(numbers, calc); 
            if (a <= 0.0) {
                calc->error = ERROR_MATH_DOMAIN;
                ns_push(numbers, NAN);
            } else {
                ns_push(numbers, log10(a));
            }
            break;
        case 'q': 
            if (numbers->top < 0) {
                calc->error = ERROR_SYNTAX;
                ns_push(numbers, NAN);
                return;
            }
            a = ns_pop(numbers, calc); 
            if (a < 0.0) {
                calc->error = ERROR_MATH_DOMAIN;
                ns_push(numbers, NAN);
            }
            else {
                ns_push(numbers, sqrt(a));
            }
            break;
        case '!': 
            if (numbers->top < 0) {
                calc->error = ERROR_SYNTAX;
                ns_push(numbers, NAN);
                return;
            }
            a = ns_pop(numbers, calc); 
            ns_push(numbers, factorial(a, calc)); 
            break;
        case 'E': 
            if (numbers->top < 0) {
                calc->error = ERROR_SYNTAX;
                ns_push(numbers, NAN);
                return;
            }
            a = ns_pop(numbers, calc); 
            ns_push(numbers, exp(a)); 
            break;
        case 'R': 
            if (numbers->top < 0) {
                calc->error = ERROR_SYNTAX;
                ns_push(numbers, NAN);
                return;
            }
            a = ns_pop(numbers, calc); 
            if (a == 0.0) {
                calc->error = ERROR_MATH_DIV_ZERO;
                ns_push(numbers, NAN);
            } else {
                ns_push(numbers, 1.0 / a);
            }
            break;
        case 'N': 
            if (numbers->top < 0) {
                calc->error = ERROR_SYNTAX;
                ns_push(numbers, NAN);
                return;
            }
            a = ns_pop(numbers, calc); 
            ns_push(numbers, -a); 
            break;
        default: break;
    }
}

double factorial(double n, Calculator* calc) {
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
