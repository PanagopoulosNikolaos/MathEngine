.PHONY: all build test run clean

all: build

build:
	$(MAKE) -C src

test:
	$(MAKE) -C src test

run:
	$(MAKE) -C src run

clean:
	$(MAKE) -C src clean