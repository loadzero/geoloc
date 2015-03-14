$(shell mkdir -p bin; mkdir -p tmp)

DEPS := $(shell echo Makefile geoloc/*.cpp geoloc/*.hpp)

all: bin/geoloc bin/test

bin/geoloc: $(DEPS)
	c++ -std=c++03 -O2 -Wall -Werror geoloc/geoloc.cpp geoloc/error.cpp \
		-o bin/geoloc

bin/test: $(DEPS)
	c++ -std=c++03 -g -Wall -Werror geoloc/test.cpp geoloc/error.cpp \
		-o bin/test

.PHONY: test install uninstall clean

test: bin/test
	./bin/test

install: bin/geoloc
	./scripts/install.sh

uninstall:
	./scripts/uninstall.sh

clean:
	rm -rf bin
