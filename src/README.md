# Google Calculator Clone - C Implementation

A modern GTK4-based calculator application built entirely in C, mimicking Google's calculator interface with a clean, responsive design.

## Features

- **Modern GUI**: Clean, minimalist design inspired by Google Calculator
- **Responsive Layout**: Dynamic spacing and scaling - works seamlessly at any window size
- **LaTeX-style Display**: Professional monospace font (DejaVu Sans Mono) for mathematical expressions
- **Full Calculator Functionality**:
  - Basic arithmetic operations (+, −, ×, ÷)
  - Decimal point support
  - Percentage calculations
  - Square root function
  - Negation toggle (+/−)
  - Clear and backspace functionality
  - Proper operator chaining

- **Google-inspired Styling**:
  - Blue operator buttons (#4285f4)
  - Green equals button (#34a853)
  - White digit buttons with subtle borders
  - Hover and active state animations
  - Professional color palette matching Google's design

## Build Requirements

- GTK 4.0 or later
- GLib 2.0
- Cairo graphics library
- Pango text rendering
- Graphene geometry library
- GCC compiler

## Building

```bash
cd C_PROGRAMMING_II/src
make
```

## Running

```bash
./calculator
```

Or use the convenience target:

```bash
make run
```

## Cleaning Build Artifacts

```bash
make clean
```

## Project Structure

- `calculator.c` - Main GUI application and event handlers
- `calculator_logic.c` - Core calculator computation logic
- `calculator_logic.h` - Calculator logic header and data structures
- `Makefile` - Build configuration with GTK4 support

## Architecture

### Backend (calculator_logic.c)

The calculation engine is separated from the GUI:
- Maintains calculation state (accumulator, current input, operation)
- Supports function chaining (multiple operations without pressing equals)
- Handles edge cases (division by zero, precision formatting)
- Uses `double` precision for calculations

### Frontend (calculator.c)

GTK4-based interface using:
- Grid layout for responsive button arrangement
- CSS styling for consistent theming
- Signal-slot callback pattern for button interactions
- Pango attributes for large, monospace display text

## Usage

1. Click number buttons to enter values
2. Click operator buttons (+, −, ×, ÷) to perform operations
3. Press = to calculate the result
4. Use ← to delete the last digit
5. Use C to clear everything
6. Use % for percentage conversion
7. Use √ for square root
8. Use +/− to negate the current value

## Responsive Design

The calculator automatically scales:
- Button sizes adjust to window dimensions
- Display text remains readable at any size
- Grid layout maintains proper aspect ratios
- Margins scale dynamically

## Notes

- All code is written in C with standard libraries
- GTK4 provides native window decorations and theme support
- The application follows GTK best practices for modern GUI development
