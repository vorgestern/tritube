
CPPFLAGS :=
CXXFLAGS := -std=c++20 -Werror -Wall -Wno-error=unused-variable

.PHONY: dirs

all: dirs compile1 echoserver echodemo libtritube.a
dirs:; @mkdir -p b/examples
clean:; rm -rf b ./compile1 ./echoserver ./echodemo libtritube.a

b/%.o: src/%.cpp src/forkpipes.h
	@echo compile $<
	@g++ -c $(CPPFLAGS) $(CXXFLAGS) -o $@ $<

b/examples/%.o: examples/%.cpp src/forkpipes.h
	@echo examples $<
	g++ -c $(CPPFLAGS) $(CXXFLAGS) -I src -o $@ $<

compile1: b/examples/compile1.o libtritube.a
	@g++ -o $@ $(LDFLAGS) $^

echoserver: b/examples/echoserver.o libtritube.a
	@g++ -o $@ $(LDFLAGS) $^

echodemo: b/examples/echodemo.o libtritube.a
	@g++ -o $@ $(LDFLAGS) $^

b/forkpipes.o: src/forkpipes.cpp src/forkpipes.h
	@g++ -o $@ -c $(CXXFLAGS) $<
b/forkpipes3.o: src/forkpipes3.cpp src/forkpipes.h
	@g++ -o $@ -c $(CXXFLAGS) $<

libtritube.a: b/forkpipes.o b/forkpipes3.o
	@echo $@
	@ar -crs $@ $^
