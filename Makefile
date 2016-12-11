default: main

main:
	gcc main.c -o main -Wall -Wextra -Werror -O3

debug:
	gcc main.c -o main-d -Wall -Wextra -Werror -g -g3

clean:
	rm main main-d
