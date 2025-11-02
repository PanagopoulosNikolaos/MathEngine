CC = gcc
CFLAGS = $(shell pkg-config --cflags gtk4) -Wall -Wextra -O2
LDFLAGS = $(shell pkg-config --libs gtk4) -lm

TARGET = calculator
SOURCES = calculator.c calculator_logic.c
OBJECTS = $(SOURCES:.c=.o)

TEST_TARGET = test_calculator
TEST_SOURCES = test_calculator.c calculator_logic.c

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

$(TEST_TARGET): $(TEST_SOURCES)
	$(CC) $(TEST_SOURCES) -o $(TEST_TARGET) -lm


%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET) $(TEST_TARGET)

run: $(TARGET)
	./$(TARGET)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

.PHONY: all clean run test
