obj = main.o
CFLAGS=-std=c99
main: $(obj)
	gcc $(CFLAGS) -o main main.o -lm
clean:
	rm main $(obj)
