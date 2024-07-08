#invoke make inside following directories and in this order: loader, launch, fib
#move the lib_simpleloader.so and launch binaries inside bin directory
#Provide the command for cleanup
all:
	gcc -m32 -no-pie -nostdlib -o sum sum.c
	gcc -m32 -o loader loader.c

execute:
	./loader ./sum

clean: 
	-@rm -f sum loader
