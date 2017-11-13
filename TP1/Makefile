HEADERS = 
DERS = 
OBJECTS = test.o

default: test

%.o: %.c $(HEADERS)
	gcc -c $< -o $@

test: $(OBJECTS)
	gcc $(OBJECTS) -o $@

clean:
	-rm -f $(OBJECTS)
	-rm -f test
