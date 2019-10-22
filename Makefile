program_name="readwrite"
debug_name="debug"
src = $(wildcard *.c)
obj = $(src:.c=.o)
CC= gcc

LDFLAGS = -lpthread -lrt

writers = 1
readers = 1

$(program_name): $(obj)
	$(CC) -o $@ $^ $(LDFLAGS)
    # $(CC) -o $@ $^ $(LDFLAGS)
	./$(program_name) $(writers) $(readers)
	
$(debug_name): $(obj)
	$(CC) -o $@ $^ -g $(LDFLAGS)
	gdb $(debug_name) $(writers) $(readers)

.PHONY: clean
clean:
	rm -f $(obj) $(program_name) $(debug_name)