
CPPFLAGS :=
CXXFLAGS := -std=c++20 -Werror -Wall

.PHONY: dirs

all: dirs compile1 echoserver
dirs:; @mkdir -p b/examples
clean:; rm -rf b ./compile1

b/%.o: src/%.cpp src/forkpipes.h
	@echo compile $<
	@g++ -c $(CPPFLAGS) $(CXXFLAGS) -o $@ $<

b/examples/%.o: examples/%.cpp src/forkpipes.h
	@echo examples $<
	g++ -c $(CPPFLAGS) $(CXXFLAGS) -I src -o $@ $<

compile1: b/examples/compile1.o b/forkpipes.o b/forkpipes3.o
	@g++ -o $@ $(LDFLAGS) $^

echoserver: b/examples/echoserver.o b/forkpipes.o b/forkpipes3.o
	@g++ -o $@ $(LDFLAGS) $^
