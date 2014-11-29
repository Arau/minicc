CXX=clang++

OBJECTS=main.o test.o input.o parser.o ast.o token.o value.o \
   prettypr.o astpr.o interpreter.o stepper.o walker.o translator.o \
   types.o type_checker.o flowcontrol.o

SRCS=$(OBJECTS:.o=.cc)

CXXFLAGS=-std=c++11

all: minicc

debug: CXXFLAGS += -g3 -O0 -D_GLIBCXX_DEBUG -g

depend: .depend

.depend: $(SRCS)
	@rm -f ./.depend
	@echo "Computing dependencies..."
	@$(CC) $(CFLAGS) -MM $^>>./.depend;

-include .depend

debug: CXXFLAGS += -g3 -O0
debug: minicc

release: CXXFLAGS += -O3
release: minicc

minicc: .depend $(OBJECTS)
	$(CXX) -o minicc $(OBJECTS)

clean:
	rm -f .depend minicc $(OBJECTS)
