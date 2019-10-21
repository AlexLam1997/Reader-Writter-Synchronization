program_name="myprogram"
debug_name="debug"
src = $(wildcard *.c)
obj = $(src:.c=.o)
CC= gcc

$(program_name): $(obj)
	$(CC) -o $@ $^
    # $(CC) -o $@ $^ $(LDFLAGS)
	./$(program_name)
	
$(debug_name): $(obj)
	$(CC) -o $@ $^ -g
	gdb $(debug_name)

.PHONY: clean
clean:
	rm -f $(obj) $(program_name) $(debug_name)