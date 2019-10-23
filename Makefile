program_name="readwrite"
debug_name="debug"
src = $(wildcard *.c)
obj = $(src:.c=.o)
CC= gcc

LDFLAGS = -lpthread -lrt

writers = 30
readers = 60

q1: A2Q1.o
	$(CC) -o $@ $^ $(LDFLAGS)
	./$@ $(writers) $(readers)

q3: A2Q3.o
	$(CC) -o $@ $^ $(LDFLAGS)
	./$@ $(writers) $(readers)

$(program_name): $(obj)
	$(CC) -o $@ $^ $(LDFLAGS)
	./$(program_name) $(writers) $(readers)
	
$(debug_name): $(obj)
	$(CC) -o $@ $^ -g $(LDFLAGS)
	gdb $(debug_name) $(writers) $(readers)

.PHONY: clean
clean:
	rm -f $(obj) $(program_name) $(debug_name)