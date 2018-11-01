.PHONY: all clean

CFLAGS+=
LDFLAGS+=-lm     # link to math library

TARGET=train test

all: $(TARGET)

train:
	g++ -O2 train.cpp -o train

test:
	g++ -O2 test.cpp -o test

clean:
	$(RM) $(TARGET)   # type make clean to remove the compiled file
