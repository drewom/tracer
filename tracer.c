/*__      ___
 / /_____/ _ |_______ ____ ____
/ __/ __/ __ / __/ -_) __// __/
\__/_/ /_/ |_\__/\__/_/ (_)__/
 * "Hope is soooooooooo amazing and great!" ~ Hope O'Malley 2019
 *
 * Drew O'Malley
 */

#include <unistd.h>
#include <stdio.h>
#include <stdint.h>

int
main (int argc, char *argv[]) { (void)argc;  (void)argv;

	enum {
		WIDTH = 128
	};
	if (isatty(fileno(stdout))) {
		puts("pipe output to the terminal");
		return 1;
	}
	freopen(0, "wb", stdout);

	char buffer[3*WIDTH*WIDTH];

	for (int y=0, yN=WIDTH; y<yN; ++y) {
		for (int x=0, xN=WIDTH; x<xN; ++x) {
			int squ_type= ((x/8+y/8)*3)%2;
			buffer[3*(x+y*xN)+0] = squ_type ?0xAA :0xCC;
			buffer[3*(x+y*xN)+1] = squ_type ?0xAA :0xCC;
			buffer[3*(x+y*xN)+2] = squ_type ?0xAA :0xCC;
		}
	}
	printf("P6\t%d\t%d\t255\t", WIDTH, WIDTH);
	for (int y=0, yN=WIDTH; y<yN; ++y) {
		fwrite(buffer, sizeof(buffer), 1, stdout);
//		putc('\n', stdout);
	}
	fflush(stdout);

	return 0;
}
