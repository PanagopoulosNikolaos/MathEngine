#include <gtk/gtk.h> // 
#include <string.h>
#include <stdlib.h>
#include "calculator_logic.h"

typedef struct {
    GtkWidget *window;
    GtkWidget *entry;
    GtkWidget *grid;
    Calculator *calc;
} CalculatorApp;

/* Forward declarations */
static void on_button_pressed(GtkWidget *widget, gpointer data);
static void on_equals_pressed(GtkWidget *widget, gpointer data);
static void on_clear_pressed(GtkWidget *widget, gpointer data);
static void on_backspace_pressed(GtkWidget *widget, gpointer data);
static void update_display(CalculatorApp *app);

static void update_display(CalculatorApp *app) {
    const char *display_text = calculator_get_display(app->calc);
    gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(app->entry)), display_text, -1);
}

static void append_to_entry(GtkEntry *entry, const char *text) {
    GtkEntryBuffer *buffer = gtk_entry_get_buffer(entry);
    const char *current_text = gtk_entry_buffer_get_text(buffer);
    if (strcmp(current_text, "0") == 0 && strcmp(text, ".") != 0) {
        gtk_entry_buffer_set_text(buffer, text, -1);
    } else {
        gtk_entry_buffer_insert_text(buffer, gtk_entry_buffer_get_length(buffer), text, -1);
    }
}

static void on_deg_rad_pressed(GtkWidget *widget, gpointer data) {
    CalculatorApp *app = (CalculatorApp *)data;
    calculator_toggle_angle_mode(app->calc);
    const char *label = (app->calc->angle_mode == DEG) ? "DEG" : "RAD";
    gtk_button_set_label(GTK_BUTTON(widget), label);
}

typedef struct {
    const char *label;
    char mapped_char;
} FunctionMapping;

// Sorted function mappings for binary search
static const FunctionMapping func_mappings[] = {
    {"!", '!'},
    {"+", '+'},
    {"-", '-'},
    {".", '.'},
    {"1/x", 'R'},
    {"C", 'C'},  // Clear button
    {"cos", 'c'},
    {"cos⁻¹", 'C'},
    {"e", 'e'},
    {"e^x", 'E'},
    {"ln", 'l'},
    {"log", 'L'},
    {"sin", 's'},
    {"sin⁻¹", 'S'},
    {"tan", 't'},
    {"tan⁻¹", 'T'},
    {"x^y", '^'},
    {"÷", '/'},
    {"×", '*'},
    {"−", '-'},
    {"√", 'q'},
    {"π", 'p'},
    {"+/−", 'N'},
    {NULL, '\0'}
};

// Binary search function for efficient lookup of function mappings
static char lookup_function_mapping(const char *label) {
    int left = 0;
    int right = sizeof(func_mappings) / sizeof(func_mappings[0]) - 1;  // Exclude the NULL terminator
    
    while (left <= right) {
        int mid = left + (right - left) / 2;
        int cmp = strcmp(label, func_mappings[mid].label);
        
        if (cmp == 0) {
            return func_mappings[mid].mapped_char;
        } else if (cmp < 0) {
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }
    
    return '\0';  // Not found
}

static void on_button_pressed(GtkWidget *widget, gpointer data) {
    CalculatorApp *app = (CalculatorApp *)data;
    const char *label = gtk_button_get_label(GTK_BUTTON(widget));
    char mapped_char = lookup_function_mapping(label);

    if (mapped_char == '\0') {
        mapped_char = label[0];
    }

    char str[2] = {mapped_char, '\0'};
    if (str[0] != '\0') {
        append_to_entry(GTK_ENTRY(app->entry), str);
    }
}

static void on_equals_pressed(GtkWidget *widget G_GNUC_UNUSED, gpointer data) {
    CalculatorApp *app = (CalculatorApp *)data;
    const char *expression = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(app->entry)));
    calculator_evaluate(app->calc, expression);
    update_display(app);
}

static void on_clear_pressed(GtkWidget *widget G_GNUC_UNUSED, gpointer data) {
    CalculatorApp *app = (CalculatorApp *)data;
    calculator_clear(app->calc);
    update_display(app);
}

static void on_backspace_pressed(GtkWidget *widget G_GNUC_UNUSED, gpointer data) {
    GtkEntryBuffer *buffer = gtk_entry_get_buffer(GTK_ENTRY(((CalculatorApp *)data)->entry));
    guint length = gtk_entry_buffer_get_length(buffer);
    if (length > 0) {
        gtk_entry_buffer_delete_text(buffer, length - 1, 1);
    }
}

static GtkWidget* create_button(const char *label, GCallback callback, gpointer data, const char *css_class) {
    GtkWidget *button = gtk_button_new_with_label(label);
    g_signal_connect(button, "clicked", callback, data);
    gtk_widget_add_css_class(button, css_class);
    return button;
}

static void on_window_destroy(GtkWidget *widget, gpointer data) {
    CalculatorApp *app = (CalculatorApp *)data;
    calculator_free(app->calc);
    free(app);
}

static void create_calculator_window(GtkApplication *app_gtk, gpointer user_data) {
    CalculatorApp *app = (CalculatorApp *)malloc(sizeof(CalculatorApp));
    app->calc = calculator_new();

    app->window = gtk_application_window_new(app_gtk);
    gtk_window_set_title(GTK_WINDOW(app->window), "Google Calculator");
    gtk_window_set_default_size(GTK_WINDOW(app->window), 400, 550);
    gtk_window_set_resizable(GTK_WINDOW(app->window), TRUE);
    g_signal_connect(app->window, "destroy", G_CALLBACK(on_window_destroy), app);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_top(vbox, 10);
    gtk_widget_set_margin_bottom(vbox, 10);
    gtk_widget_set_margin_start(vbox, 10);
    gtk_widget_set_margin_end(vbox, 10);
    gtk_window_set_child(GTK_WINDOW(app->window), vbox);

    /* Display */
    app->entry = gtk_entry_new();
    gtk_widget_set_halign(app->entry, GTK_ALIGN_FILL);
    gtk_widget_set_valign(app->entry, GTK_ALIGN_CENTER);
    gtk_widget_add_css_class(app->entry, "display");
    gtk_entry_set_alignment(GTK_ENTRY(app->entry), 1.0);
    gtk_editable_set_editable(GTK_EDITABLE(app->entry), FALSE);
    
    gtk_box_append(GTK_BOX(vbox), app->entry);
    gtk_widget_set_vexpand(app->entry, FALSE);

    /* Grid of buttons */
    app->grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(app->grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(app->grid), 8);
    gtk_grid_set_row_homogeneous(GTK_GRID(app->grid), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID(app->grid), TRUE);
    gtk_box_append(GTK_BOX(vbox), app->grid);
    gtk_widget_set_vexpand(app->grid, TRUE);
    gtk_widget_set_hexpand(app->grid, TRUE);

    /* Row 0: Scientific Functions */
    GtkWidget *btn_deg_rad = create_button("DEG", G_CALLBACK(on_deg_rad_pressed), app, "btn-function");
    GtkWidget *btn_sin = create_button("sin", G_CALLBACK(on_button_pressed), app, "btn-function");
    GtkWidget *btn_cos = create_button("cos", G_CALLBACK(on_button_pressed), app, "btn-function");
    GtkWidget *btn_tan = create_button("tan", G_CALLBACK(on_button_pressed), app, "btn-function");

    gtk_grid_attach(GTK_GRID(app->grid), btn_deg_rad, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_sin, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_cos, 2, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_tan, 3, 0, 1, 1);

    /* Row 1: Inverse Trig and Logs */
    GtkWidget *btn_asin = create_button("sin⁻¹", G_CALLBACK(on_button_pressed), app, "btn-function");
    GtkWidget *btn_acos = create_button("cos⁻¹", G_CALLBACK(on_button_pressed), app, "btn-function");
    GtkWidget *btn_atan = create_button("tan⁻¹", G_CALLBACK(on_button_pressed), app, "btn-function");
    GtkWidget *btn_ln = create_button("ln", G_CALLBACK(on_button_pressed), app, "btn-function");

    gtk_grid_attach(GTK_GRID(app->grid), btn_asin, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_acos, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_atan, 2, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_ln, 3, 1, 1, 1);

    /* Row 2: Powers and Roots */
    GtkWidget *btn_pow = create_button("x^y", G_CALLBACK(on_button_pressed), app, "btn-function");
    GtkWidget *btn_sqrt = create_button("√", G_CALLBACK(on_button_pressed), app, "btn-function");
    GtkWidget *btn_ex = create_button("e^x", G_CALLBACK(on_button_pressed), app, "btn-function");
    GtkWidget *btn_log = create_button("log", G_CALLBACK(on_button_pressed), app, "btn-function");

    gtk_grid_attach(GTK_GRID(app->grid), btn_pow, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_sqrt, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_ex, 2, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_log, 3, 2, 1, 1);

    /* Row 3: Parentheses and Constants */
    GtkWidget *btn_lparen = create_button("(", G_CALLBACK(on_button_pressed), app, "btn-function");
    GtkWidget *btn_rparen = create_button(")", G_CALLBACK(on_button_pressed), app, "btn-function");
    GtkWidget *btn_pi = create_button("π", G_CALLBACK(on_button_pressed), app, "btn-function");
    GtkWidget *btn_e = create_button("e", G_CALLBACK(on_button_pressed), app, "btn-function");

    gtk_grid_attach(GTK_GRID(app->grid), btn_lparen, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_rparen, 1, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_pi, 2, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_e, 3, 3, 1, 1);

    /* Row 4: C, ←, %, / */
    GtkWidget *btn_c = create_button("C", G_CALLBACK(on_clear_pressed), app, "btn-function");
    GtkWidget *btn_backspace = create_button("←", G_CALLBACK(on_backspace_pressed), app, "btn-function");
    GtkWidget *btn_percent = create_button("%", G_CALLBACK(on_button_pressed), app, "btn-operator");
    GtkWidget *btn_divide = create_button("÷", G_CALLBACK(on_button_pressed), app, "btn-operator");
    
    gtk_grid_attach(GTK_GRID(app->grid), btn_c, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_backspace, 1, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_percent, 2, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_divide, 3, 4, 1, 1);

    /* Row 5: 7, 8, 9, × */
    GtkWidget *btn_7 = create_button("7", G_CALLBACK(on_button_pressed), app, "btn-digit");
    GtkWidget *btn_8 = create_button("8", G_CALLBACK(on_button_pressed), app, "btn-digit");
    GtkWidget *btn_9 = create_button("9", G_CALLBACK(on_button_pressed), app, "btn-digit");
    GtkWidget *btn_multiply = create_button("×", G_CALLBACK(on_button_pressed), app, "btn-operator");
    
    gtk_grid_attach(GTK_GRID(app->grid), btn_7, 0, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_8, 1, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_9, 2, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_multiply, 3, 5, 1, 1);

    /* Row 6: 4, 5, 6, − */
    GtkWidget *btn_4 = create_button("4", G_CALLBACK(on_button_pressed), app, "btn-digit");
    GtkWidget *btn_5 = create_button("5", G_CALLBACK(on_button_pressed), app, "btn-digit");
    GtkWidget *btn_6 = create_button("6", G_CALLBACK(on_button_pressed), app, "btn-digit");
    GtkWidget *btn_minus = create_button("−", G_CALLBACK(on_button_pressed), app, "btn-operator");
    
    gtk_grid_attach(GTK_GRID(app->grid), btn_4, 0, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_5, 1, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_6, 2, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_minus, 3, 6, 1, 1);

    /* Row 7: 1, 2, 3, + */
    GtkWidget *btn_1 = create_button("1", G_CALLBACK(on_button_pressed), app, "btn-digit");
    GtkWidget *btn_2 = create_button("2", G_CALLBACK(on_button_pressed), app, "btn-digit");
    GtkWidget *btn_3 = create_button("3", G_CALLBACK(on_button_pressed), app, "btn-digit");
    GtkWidget *btn_plus = create_button("+", G_CALLBACK(on_button_pressed), app, "btn-operator");
    
    gtk_grid_attach(GTK_GRID(app->grid), btn_1, 0, 7, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_2, 1, 7, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_3, 2, 7, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_plus, 3, 7, 1, 1);

    /* Row 8: 0, ., 1/x, = */
    GtkWidget *btn_0 = create_button("0", G_CALLBACK(on_button_pressed), app, "btn-digit");
    GtkWidget *btn_dot = create_button(".", G_CALLBACK(on_button_pressed), app, "btn-digit");
    GtkWidget *btn_reciprocal = create_button("1/x", G_CALLBACK(on_button_pressed), app, "btn-function");
    GtkWidget *btn_equals = create_button("=", G_CALLBACK(on_equals_pressed), app, "btn-equals");
    
    gtk_grid_attach(GTK_GRID(app->grid), btn_0, 0, 8, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_dot, 1, 8, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_reciprocal, 2, 8, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_equals, 3, 8, 1, 1);

    /* Row 9: Factorial and Negate */
    GtkWidget *btn_factorial = create_button("!", G_CALLBACK(on_button_pressed), app, "btn-function");
    GtkWidget *btn_negate = create_button("+/−", G_CALLBACK(on_button_pressed), app, "btn-function");
    gtk_grid_attach(GTK_GRID(app->grid), btn_factorial, 0, 9, 2, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_negate, 2, 9, 2, 1);

    /* Apply CSS styling */
    const char *css_str = 
        "entry.display { "
        "  font-family: 'DejaVu Sans Mono', monospace; "
        "  font-weight: bold; "
        "  padding: 20px; "
        "  background-color: #f8f9fa; "
        "  border-radius: 8px; "
        "  color: #333333; "
        "  font-size: 24px;"
        "} "
        "button { "
        "  font-size: 16px; "
        "  padding: 10px; "
        "  font-weight: bold; "
        "  border-radius: 4px; "
        "  border: none; "
        "  min-width: 60px; "
        "  min-height: 40px; "
        "} "
        "button.btn-digit { "
        "  background-color: #ffffff; "
        "  color: #333333; "
        "  border: 1px solid #dadce0; "
        "} "
        "button.btn-digit:hover { "
        "  background-color: #f8f9fa; "
        "} "
        "button.btn-digit:active { "
        "  background-color: #e8eaed; "
        "} "
        "button.btn-operator { "
        "  background-color: #4285f4; "
        "  color: white; "
        "  border: none; "
        "} "
        "button.btn-operator:hover { "
        "  background-color: #3367d6; "
        "} "
        "button.btn-operator:active { "
        "  background-color: #2551b8; "
        "} "
        "button.btn-function { "
        "  background-color: #f8f9fa; "
        "  color: #3c4043; "
        "  border: 1px solid #dadce0; "
        "} "
        "button.btn-function:hover { "
        "  background-color: #f1f3f4; "
        "} "
        "button.btn-function:active { "
        "  background-color: #e8eaed; "
        "} "
        "button.btn-equals { "
        "  background-color: #34a853; "
        "  color: white; "
        "  border: none; "
        "} "
        "button.btn-equals:hover { "
        "  background-color: #2d8e47; "
        "} "
        "button.btn-equals:active { "
        "  background-color: #1f6b3e; "
        "} ";

    GtkCssProvider *css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_string(css_provider, css_str);

    gtk_widget_add_css_class(app->grid, "grid");
    gtk_widget_add_css_class(app->entry, "display");
    
    g_object_unref(css_provider);

    gtk_window_present(GTK_WINDOW(app->window));
}

int main(int argc, char *argv[]) {
    GtkApplication *app_gtk = gtk_application_new("com.example.calculator", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app_gtk, "activate", G_CALLBACK(create_calculator_window), NULL);
    int status = g_application_run(G_APPLICATION(app_gtk), argc, argv);
    g_object_unref(app_gtk);
    return status;
}
