#include <gtk/gtk.h>
#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include "calculator_logic.h"

typedef struct {
    GtkWidget *window;
    GtkWidget *entry;
    GtkWidget *grid;
    GtkWidget *preview_label;
    Calculator *calc;
    GtkCssProvider *css_provider;
    gboolean is_finalized;
} CalculatorApp;
/**
 * Represents the main calculator application state and GUI components.
 *
 * This structure holds references to the GTK window, the text entry display,
 * the layout grid, the preview output label, the calculator logic engine,
 * the CSS provider used for custom styling, and a boolean tracking
 * whether the user has just pressed Enter/equals to finalize the output.
 *
 * Functions:
 *   updateDisplay: Updates the text entry buffer with the current calculator display value.
 *   appendToEntry: Appends a character or string to the display entry buffer.
 *   onDegRadPressed: Callback for when the DEG/RAD toggle button is clicked.
 *   lookupFunctionMapping: Performs a binary search to map a button label to an internal operator char.
 *   onButtonPressed: Callback for generic button press events, appending mapped character to display.
 *   evaluateExpression: Evaluates the expression in the display entry using the calculator engine.
 *   onEntryActivate: Callback for entry widget activation (pressing Enter).
 *   onEqualsPressed: Callback for the equals button click event.
 *   onClearPressed: Callback for the clear button click event.
 *   onBackspacePressed: Callback for the backspace button click event.
 *   createButton: Factory function to instantiate a GTK button with a signal connection and CSS class.
 *   onWindowDestroy: Callback for window destruction to release allocated memory.
 *   initStyles: Sets up and applies the static CSS theme to the application.
 *   onEntryChanged: Callback for entry text change events to compute the real-time preview.
 *   updatePreview: Evaluates the expression in the background to show the dynamic preview.
 *   createCalculatorWindow: Initializes and sets up the calculator UI widgets and signals.
 */

typedef struct {
    const char *label;
    char mapped_char;
} FunctionMapping;
/**
 * Maps external button labels to internal parser representation characters.
 *
 * This structure links mathematical symbol strings displayable on buttons
 * to their corresponding one-character symbols parsed by the evaluator.
 */

// Forward declarations for internal calculator view functions
static void updateDisplay(CalculatorApp *app);
static void appendToEntry(GtkEntry *entry, const char *text);
static void onDegRadPressed(GtkWidget *widget, gpointer data);
static char lookupFunctionMapping(const char *label);
static void onButtonPressed(GtkWidget *widget, gpointer data);
static void evaluateExpression(CalculatorApp *app);
static void onEntryActivate(GtkEntry *entry, gpointer data);
static void onEqualsPressed(GtkWidget *widget, gpointer data);
static void onClearPressed(GtkWidget *widget, gpointer data);
static void onBackspacePressed(GtkWidget *widget, gpointer data);
static GtkWidget* createButton(const char *label, GCallback callback, gpointer data, const char *css_class);
static void onWindowDestroy(GtkWidget *widget, gpointer data);
static void initStyles(CalculatorApp *app);
static void onEntryChanged(GtkEditable *editable, gpointer data);
static void updatePreview(CalculatorApp *app);
static void createCalculatorWindow(GtkApplication *app_gtk, gpointer user_data);

static void updateDisplay(CalculatorApp *app) {
    /**
     * Updates the display widget with the current calculator display buffer.
     * Args:
     * app (CalculatorApp *): The application context pointer.
     * Returns:
     * void: No return value.
     */
    const char *display_text = calculatorGetDisplay(app->calc);
    gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(app->entry)), display_text, -1);
}

static void appendToEntry(GtkEntry *entry, const char *text) {
    /**
     * Appends the given text to the display entry widget.
     * Args:
     * entry (GtkEntry *): The target text entry widget.
     * text (const char *): The text string to append.
     * Returns:
     * void: No return value.
     */
    GtkEntryBuffer *buffer = gtk_entry_get_buffer(entry);
    const char *current_text = gtk_entry_buffer_get_text(buffer);
    if (strcmp(current_text, "0") == 0 && strcmp(text, ".") != 0) {
        gtk_entry_buffer_set_text(buffer, text, -1);
    } else {
        gtk_entry_buffer_insert_text(buffer, gtk_entry_buffer_get_length(buffer), text, -1);
    }
}

static void onDegRadPressed(GtkWidget *widget, gpointer data) {
    /**
     * Toggles the angle mode between DEG and RAD and updates the button label.
     * Args:
     * widget (GtkWidget *): The DEG/RAD toggle button widget.
     * data (gpointer): The pointer to the CalculatorApp context.
     * Returns:
     * void: No return value.
     */
    CalculatorApp *app = (CalculatorApp *)data;
    calculatorToggleAngleMode(app->calc);
    const char *label = (app->calc->angle_mode == DEG) ? "DEG" : "RAD";
    gtk_button_set_label(GTK_BUTTON(widget), label);
}

// Holds sorted function mappings to enable fast binary search lookup of operators.
static const FunctionMapping func_mappings[] = {
    {"!", '!'},
    {"+", '+'},
    {"-", '-'},
    {".", '.'},
    {"1/x", 'R'},
    {"C", 'C'}, // Maps the clear key command to its internal equivalent.
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

static char lookupFunctionMapping(const char *label) {
    /**
     * Performs a binary search to find the character mapping for a given label.
     * Args:
     * label (const char *): The button label string.
     * Returns:
     * char: The mapped single character representation of the function.
     */
    int left = 0;
    int right = (sizeof(func_mappings) / sizeof(func_mappings[0])) - 2; // Excludes the terminating NULL entry to avoid out-of-bounds comparison.
    
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
    
    return '\0'; // Returns null character when no corresponding function mapping is found.
}

static void onButtonPressed(GtkWidget *widget, gpointer data) {
    /**
     * Handles generic button clicks by looking up and appending the character.
     * Args:
     * widget (GtkWidget *): The clicked button widget.
     * data (gpointer): The pointer to the CalculatorApp context.
     * Returns:
     * void: No return value.
     */
    CalculatorApp *app = (CalculatorApp *)data;
    const char *label = gtk_button_get_label(GTK_BUTTON(widget));
    char mapped_char = lookupFunctionMapping(label);

    if (mapped_char == '\0') {
        mapped_char = label[0];
    }

    if (app->is_finalized) {
        // Reset finalized state
        app->is_finalized = FALSE;
        gtk_widget_remove_css_class(app->preview_label, "output-mode");
        gtk_widget_add_css_class(app->preview_label, "preview-mode");

        // If it is an operator, chain the calculation
        if (mapped_char == '+' || mapped_char == '-' || mapped_char == '*' || mapped_char == '/' || mapped_char == '^' || mapped_char == '%' || mapped_char == '!') {
            const char *prev_result = calculatorGetDisplay(app->calc);
            gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(app->entry)), prev_result, -1);
        } else {
            // Otherwise, start a new calculation
            gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(app->entry)), "", -1);
        }
    }

    char str[2] = {mapped_char, '\0'};
    if (str[0] != '\0') {
        appendToEntry(GTK_ENTRY(app->entry), str);
    }
}

static void evaluateExpression(CalculatorApp *app) {
    /**
     * Triggers evaluation of the expression currently in the display entry.
     * Args:
     * app (CalculatorApp *): The application context pointer.
     * Returns:
     * void: No return value.
     */
    const char *expression = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(app->entry)));
    
    // Evaluate for real
    calculatorEvaluate(app->calc, expression);
    
    // Set the result in the preview label
    gtk_label_set_text(GTK_LABEL(app->preview_label), app->calc->buffer);
    
    // Mark as finalized and style accordingly
    app->is_finalized = TRUE;
    gtk_widget_remove_css_class(app->preview_label, "preview-mode");
    gtk_widget_add_css_class(app->preview_label, "output-mode");
}

static void onEntryActivate(GtkEntry *entry G_GNUC_UNUSED, gpointer data) {
    /**
     * Callback for entry activation (e.g. pressing Enter) to trigger evaluation.
     * Args:
     * entry (GtkEntry *): The display entry widget.
     * data (gpointer): The pointer to the CalculatorApp context.
     * Returns:
     * void: No return value.
     */
    evaluateExpression((CalculatorApp *)data);
}

static void onEqualsPressed(GtkWidget *widget G_GNUC_UNUSED, gpointer data) {
    /**
     * Callback for the equals button click to trigger evaluation.
     * Args:
     * widget (GtkWidget *): The equals button widget.
     * data (gpointer): The pointer to the CalculatorApp context.
     * Returns:
     * void: No return value.
     */
    evaluateExpression((CalculatorApp *)data);
}

static void onClearPressed(GtkWidget *widget G_GNUC_UNUSED, gpointer data) {
    /**
     * Callback for the clear button click to reset the calculator state.
     * Args:
     * widget (GtkWidget *): The clear button widget.
     * data (gpointer): The pointer to the CalculatorApp context.
     * Returns:
     * void: No return value.
     */
    CalculatorApp *app = (CalculatorApp *)data;
    calculatorClear(app->calc);
    gtk_entry_buffer_set_text(gtk_entry_get_buffer(GTK_ENTRY(app->entry)), "0", -1);
    gtk_label_set_text(GTK_LABEL(app->preview_label), "");
    app->is_finalized = FALSE;
    gtk_widget_remove_css_class(app->preview_label, "output-mode");
    gtk_widget_add_css_class(app->preview_label, "preview-mode");
}

static void onBackspacePressed(GtkWidget *widget G_GNUC_UNUSED, gpointer data) {
    /**
     * Callback for backspace button click to remove the last character.
     * Args:
     * widget (GtkWidget *): The backspace button widget.
     * data (gpointer): The pointer to the CalculatorApp context.
     * Returns:
     * void: No return value.
     */
    CalculatorApp *app = (CalculatorApp *)data;
    if (app->is_finalized) {
        app->is_finalized = FALSE;
        gtk_widget_remove_css_class(app->preview_label, "output-mode");
        gtk_widget_add_css_class(app->preview_label, "preview-mode");
    }

    GtkEntryBuffer *buffer = gtk_entry_get_buffer(GTK_ENTRY(app->entry));
    guint length = gtk_entry_buffer_get_length(buffer);
    if (length > 1) {
        gtk_entry_buffer_delete_text(buffer, length - 1, 1);
    } else if (length == 1) {
        gtk_entry_buffer_set_text(buffer, "0", -1);
    }
}

static GtkWidget* createButton(const char *label, GCallback callback, gpointer data, const char *css_class) {
    /**
     * Creates a GtkButton widget, sets its label, callback signal, and CSS class.
     * Args:
     * label (const char *): The label text of the button.
     * callback (GCallback): The click event handler function pointer.
     * data (gpointer): The user data pointer passed to the callback.
     * css_class (const char *): The CSS class name to style the button.
     * Returns:
     * GtkWidget *: The newly created button widget pointer.
     */
    GtkWidget *button = gtk_button_new_with_label(label);
    g_signal_connect(button, "clicked", callback, data);
    gtk_widget_add_css_class(button, css_class);
    return button;
}

static void onWindowDestroy(GtkWidget *widget G_GNUC_UNUSED, gpointer data) {
    /**
     * Frees resources when the main window is destroyed.
     * Args:
     * widget (GtkWidget *): The main window widget.
     * data (gpointer): The pointer to the CalculatorApp context.
     * Returns:
     * void: No return value.
     */
    CalculatorApp *app = (CalculatorApp *)data;
    calculatorFree(app->calc);
    g_object_unref(app->css_provider);
    free(app);
}

static void initStyles(CalculatorApp *app) {
    /**
     * Initializes and loads the static CSS stylesheet.
     * Args:
     * app (CalculatorApp *): The application context pointer.
     * Returns:
     * void: No return value.
     */
    app->css_provider = gtk_css_provider_new();
    const char *css = 
        "window { "
        "  background-color: #1c2128; "
        "} "
        "box { "
        "  background-color: #1c2128; "
        "} "
        "entry { "
        "  background-color: #22272e; "
        "  border: 1px solid #444c56; "
        "  border-radius: 6px; "
        "  color: #f0f6fc; "
        "  font-family: 'DejaVu Sans Mono', monospace; "
        "  font-size: 20px; "
        "  font-weight: bold; "
        "  padding: 10px; "
        "} "
        "entry:focus { "
        "  border-color: #388bfd; "
        "} "
        ".display-preview { "
        "  font-family: 'DejaVu Sans Mono', monospace; "
        "  font-size: 16px; "
        "  margin-right: 5px; "
        "  margin-top: 4px; "
        "  margin-bottom: 8px; "
        "} "
        ".preview-mode { "
        "  color: #768390; "
        "  font-weight: normal; "
        "} "
        ".output-mode { "
        "  color: #57ab5a; "
        "  font-weight: bold; "
        "} "
        "button { "
        "  font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Helvetica, Arial, sans-serif; "
        "  font-size: 16px; "
        "  font-weight: 600; "
        "  border-radius: 6px; "
        "  border: 1px solid #444c56; "
        "  padding: 10px; "
        "} "
        "button.btn-digit { "
        "  background-color: #2d333b; "
        "  color: #adbac7; "
        "} "
        "button.btn-digit:hover { "
        "  background-color: #373e47; "
        "  border-color: #768390; "
        "} "
        "button.btn-digit:active { "
        "  background-color: #444c56; "
        "} "
        "button.btn-function { "
        "  background-color: #22272e; "
        "  color: #768390; "
        "} "
        "button.btn-function:hover { "
        "  background-color: #2d333b; "
        "  color: #adbac7; "
        "  border-color: #768390; "
        "} "
        "button.btn-function:active { "
        "  background-color: #373e47; "
        "} "
        "button.btn-operator { "
        "  background-color: #2b303b; "
        "  color: #539bf5; "
        "  border-color: #444c56; "
        "} "
        "button.btn-operator:hover { "
        "  background-color: #373e47; "
        "  border-color: #539bf5; "
        "} "
        "button.btn-operator:active { "
        "  background-color: #444c56; "
        "} "
        "button.btn-equals { "
        "  background-color: #347d39; "
        "  color: #ffffff; "
        "  border-color: #46954a; "
        "} "
        "button.btn-equals:hover { "
        "  background-color: #46954a; "
        "  border-color: #539bf5; "
        "} "
        "button.btn-equals:active { "
        "  background-color: #2b6530; "
        "} ";

    gtk_css_provider_load_from_string(app->css_provider, css);
    gtk_style_context_add_provider_for_display(
        gdk_display_get_default(),
        GTK_STYLE_PROVIDER(app->css_provider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );
}

static void onEntryChanged(GtkEditable *editable G_GNUC_UNUSED, gpointer data) {
    /**
     * Handles text changes in the entry to update the real-time preview.
     * Args:
     * editable (GtkEditable *): The entry widget's editable interface.
     * data (gpointer): The pointer to the CalculatorApp context.
     * Returns:
     * void: No return value.
     */
    CalculatorApp *app = (CalculatorApp *)data;
    
    if (app->is_finalized) {
        app->is_finalized = FALSE;
        gtk_widget_remove_css_class(app->preview_label, "output-mode");
        gtk_widget_add_css_class(app->preview_label, "preview-mode");
    }
    
    updatePreview(app);
}

static void updatePreview(CalculatorApp *app) {
    /**
     * Evaluates the expression in the background to show the dynamic preview.
     * Args:
     * app (CalculatorApp *): The application context pointer.
     * Returns:
     * void: No return value.
     */
    const char *expression = gtk_entry_buffer_get_text(gtk_entry_get_buffer(GTK_ENTRY(app->entry)));
    
    if (strlen(expression) == 0 || strcmp(expression, "0") == 0) {
        gtk_label_set_text(GTK_LABEL(app->preview_label), "");
        return;
    }

    char saved_buffer[DISPLAY_BUFFER_SIZE];
    memcpy(saved_buffer, app->calc->buffer, DISPLAY_BUFFER_SIZE);
    ErrorType saved_error = app->calc->error;

    calculatorEvaluate(app->calc, expression);

    if (app->calc->error == ERROR_NONE) {
        gtk_label_set_text(GTK_LABEL(app->preview_label), app->calc->buffer);
    } else {
        gtk_label_set_text(GTK_LABEL(app->preview_label), "");
    }

    memcpy(app->calc->buffer, saved_buffer, DISPLAY_BUFFER_SIZE);
    app->calc->error = saved_error;
}

static void createCalculatorWindow(GtkApplication *app_gtk, gpointer user_data G_GNUC_UNUSED) {
    /**
     * Sets up the main window and UI grid of the calculator.
     * Args:
     * app_gtk (GtkApplication *): The GTK application object.
     * user_data (gpointer): The user data passed to the callback.
     * Returns:
     * void: No return value.
     */
    CalculatorApp *app = (CalculatorApp *)calloc(1, sizeof(CalculatorApp));
    if (!app) {
        g_warning("Failed to allocate CalculatorApp");
        return;
    }

    app->calc = calculatorNew();
    if (!app->calc) {
        g_warning("Failed to allocate Calculator");
        free(app);
        return;
    }

    app->window = gtk_application_window_new(app_gtk);
    if (!app->window) {
        g_warning("Failed to create application window");
        calculatorFree(app->calc);
        free(app);
        return;
    }
    gtk_window_set_title(GTK_WINDOW(app->window), "Calculator");
    gtk_window_set_default_size(GTK_WINDOW(app->window), 400, 550);
    gtk_window_set_resizable(GTK_WINDOW(app->window), TRUE);
    g_signal_connect(app->window, "destroy", G_CALLBACK(onWindowDestroy), app);

    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_top(vbox, 10);
    gtk_widget_set_margin_bottom(vbox, 10);
    gtk_widget_set_margin_start(vbox, 10);
    gtk_widget_set_margin_end(vbox, 10);
    gtk_window_set_child(GTK_WINDOW(app->window), vbox);

    // Configures the editable text display widget for input/output visualization.
    app->entry = gtk_entry_new();
    gtk_widget_set_halign(app->entry, GTK_ALIGN_FILL);
    gtk_entry_set_alignment(GTK_ENTRY(app->entry), 1.0);
    gtk_editable_set_editable(GTK_EDITABLE(app->entry), TRUE);
    gtk_widget_set_can_focus(app->entry, TRUE);
    g_signal_connect(app->entry, "changed", G_CALLBACK(onEntryChanged), app);
    g_signal_connect(app->entry, "activate", G_CALLBACK(onEntryActivate), app);
    gtk_box_append(GTK_BOX(vbox), app->entry);

    app->preview_label = gtk_label_new("");
    gtk_widget_set_halign(app->preview_label, GTK_ALIGN_END);
    gtk_widget_add_css_class(app->preview_label, "display-preview");
    gtk_widget_add_css_class(app->preview_label, "preview-mode");
    gtk_box_append(GTK_BOX(vbox), app->preview_label);

    updateDisplay(app);
    gtk_widget_grab_focus(app->entry);

    // Prepares the homogeneous grid container to house calculator buttons.
    app->grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(app->grid), 8);
    gtk_grid_set_column_spacing(GTK_GRID(app->grid), 8);
    gtk_grid_set_row_homogeneous(GTK_GRID(app->grid), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID(app->grid), TRUE);
    gtk_box_append(GTK_BOX(vbox), app->grid);
    gtk_widget_set_vexpand(app->grid, TRUE);
    gtk_widget_set_hexpand(app->grid, TRUE);

    // Arranges trigonometry and angle toggles in the first row of the grid.
    GtkWidget *btn_deg_rad = createButton("DEG", G_CALLBACK(onDegRadPressed), app, "btn-function");
    GtkWidget *btn_sin = createButton("sin", G_CALLBACK(onButtonPressed), app, "btn-function");
    GtkWidget *btn_cos = createButton("cos", G_CALLBACK(onButtonPressed), app, "btn-function");
    GtkWidget *btn_tan = createButton("tan", G_CALLBACK(onButtonPressed), app, "btn-function");

    gtk_grid_attach(GTK_GRID(app->grid), btn_deg_rad, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_sin, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_cos, 2, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_tan, 3, 0, 1, 1);

    // Positions inverse trigonometry and natural log in the second row.
    GtkWidget *btn_asin = createButton("sin⁻¹", G_CALLBACK(onButtonPressed), app, "btn-function");
    GtkWidget *btn_acos = createButton("cos⁻¹", G_CALLBACK(onButtonPressed), app, "btn-function");
    GtkWidget *btn_atan = createButton("tan⁻¹", G_CALLBACK(onButtonPressed), app, "btn-function");
    GtkWidget *btn_ln = createButton("ln", G_CALLBACK(onButtonPressed), app, "btn-function");

    gtk_grid_attach(GTK_GRID(app->grid), btn_asin, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_acos, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_atan, 2, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_ln, 3, 1, 1, 1);

    // Places exponentials, square root, and log base 10 in the third row.
    GtkWidget *btn_pow = createButton("x^y", G_CALLBACK(onButtonPressed), app, "btn-function");
    GtkWidget *btn_sqrt = createButton("√", G_CALLBACK(onButtonPressed), app, "btn-function");
    GtkWidget *btn_ex = createButton("e^x", G_CALLBACK(onButtonPressed), app, "btn-function");
    GtkWidget *btn_log = createButton("log", G_CALLBACK(onButtonPressed), app, "btn-function");

    gtk_grid_attach(GTK_GRID(app->grid), btn_pow, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_sqrt, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_ex, 2, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_log, 3, 2, 1, 1);

    // Positions parenthetical controls and mathematical constants in the fourth row.
    GtkWidget *btn_lparen = createButton("(", G_CALLBACK(onButtonPressed), app, "btn-function");
    GtkWidget *btn_rparen = createButton(")", G_CALLBACK(onButtonPressed), app, "btn-function");
    GtkWidget *btn_pi = createButton("π", G_CALLBACK(onButtonPressed), app, "btn-function");
    GtkWidget *btn_e = createButton("e", G_CALLBACK(onButtonPressed), app, "btn-function");

    gtk_grid_attach(GTK_GRID(app->grid), btn_lparen, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_rparen, 1, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_pi, 2, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_e, 3, 3, 1, 1);

    // Places edit controls, percentage, and division in the fifth row.
    GtkWidget *btn_c = createButton("C", G_CALLBACK(onClearPressed), app, "btn-function");
    GtkWidget *btn_backspace = createButton("←", G_CALLBACK(onBackspacePressed), app, "btn-function");
    GtkWidget *btn_percent = createButton("%", G_CALLBACK(onButtonPressed), app, "btn-operator");
    GtkWidget *btn_divide = createButton("÷", G_CALLBACK(onButtonPressed), app, "btn-operator");
    
    gtk_grid_attach(GTK_GRID(app->grid), btn_c, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_backspace, 1, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_percent, 2, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_divide, 3, 4, 1, 1);

    // Houses digits seven through nine and multiplication in the sixth row.
    GtkWidget *btn_7 = createButton("7", G_CALLBACK(onButtonPressed), app, "btn-digit");
    GtkWidget *btn_8 = createButton("8", G_CALLBACK(onButtonPressed), app, "btn-digit");
    GtkWidget *btn_9 = createButton("9", G_CALLBACK(onButtonPressed), app, "btn-digit");
    GtkWidget *btn_multiply = createButton("×", G_CALLBACK(onButtonPressed), app, "btn-operator");
    
    gtk_grid_attach(GTK_GRID(app->grid), btn_7, 0, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_8, 1, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_9, 2, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_multiply, 3, 5, 1, 1);

    // Houses digits four through six and subtraction in the seventh row.
    GtkWidget *btn_4 = createButton("4", G_CALLBACK(onButtonPressed), app, "btn-digit");
    GtkWidget *btn_5 = createButton("5", G_CALLBACK(onButtonPressed), app, "btn-digit");
    GtkWidget *btn_6 = createButton("6", G_CALLBACK(onButtonPressed), app, "btn-digit");
    GtkWidget *btn_minus = createButton("−", G_CALLBACK(onButtonPressed), app, "btn-operator");
    
    gtk_grid_attach(GTK_GRID(app->grid), btn_4, 0, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_5, 1, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_6, 2, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_minus, 3, 6, 1, 1);

    // Houses digits one through three and addition in the eighth row.
    GtkWidget *btn_1 = createButton("1", G_CALLBACK(onButtonPressed), app, "btn-digit");
    GtkWidget *btn_2 = createButton("2", G_CALLBACK(onButtonPressed), app, "btn-digit");
    GtkWidget *btn_3 = createButton("3", G_CALLBACK(onButtonPressed), app, "btn-digit");
    GtkWidget *btn_plus = createButton("+", G_CALLBACK(onButtonPressed), app, "btn-operator");
    
    gtk_grid_attach(GTK_GRID(app->grid), btn_1, 0, 7, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_2, 1, 7, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_3, 2, 7, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_plus, 3, 7, 1, 1);

    // Places zero, decimal point, reciprocal, and equals in the ninth row.
    GtkWidget *btn_0 = createButton("0", G_CALLBACK(onButtonPressed), app, "btn-digit");
    GtkWidget *btn_dot = createButton(".", G_CALLBACK(onButtonPressed), app, "btn-digit");
    GtkWidget *btn_reciprocal = createButton("1/x", G_CALLBACK(onButtonPressed), app, "btn-function");
    GtkWidget *btn_equals = createButton("=", G_CALLBACK(onEqualsPressed), app, "btn-equals");
    
    gtk_grid_attach(GTK_GRID(app->grid), btn_0, 0, 8, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_dot, 1, 8, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_reciprocal, 2, 8, 1, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_equals, 3, 8, 1, 1);

    // Spans factorial and sign toggles across the final grid row.
    GtkWidget *btn_factorial = createButton("!", G_CALLBACK(onButtonPressed), app, "btn-function");
    GtkWidget *btn_negate = createButton("+/−", G_CALLBACK(onButtonPressed), app, "btn-function");
    gtk_grid_attach(GTK_GRID(app->grid), btn_factorial, 0, 9, 2, 1);
    gtk_grid_attach(GTK_GRID(app->grid), btn_negate, 2, 9, 2, 1);

    // Initializes styling provider to customize the user interface look.
    initStyles(app);

    gtk_widget_add_css_class(app->grid, "grid");

    gtk_window_present(GTK_WINDOW(app->window));
}

int main(int argc, char *argv[]) {
    /**
     * The main entry point of the calculator application.
     * Args:
     * argc (int): The argument count.
     * argv (char **): The array of argument strings.
     * Returns:
     * int: The application exit status code.
     */
    GtkApplication *app_gtk = gtk_application_new("com.example.calculator", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(app_gtk, "activate", G_CALLBACK(createCalculatorWindow), NULL);
    int status = g_application_run(G_APPLICATION(app_gtk), argc, argv);
    g_object_unref(app_gtk);
    return status;
}
