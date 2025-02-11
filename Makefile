
CPPFLAGS :=
CXXFLAGS := -std=c++20 -Werror -Wall -Wno-error=unused-variable

.PHONY: dirs

all: dirs compile1 echoserver echodemo
dirs:; @mkdir -p b/examples
clean:; rm -rf b ./compile1 ./echoserver ./echodemo

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

echodemo: b/examples/echodemo.o b/forkpipes3.o
	@g++ -o $@ $(LDFLAGS) $^
