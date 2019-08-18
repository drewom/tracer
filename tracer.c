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
	enum { WIDTH = 120, SQUARE_SIZE = 16 };
	if (isatty(fileno(stdout))) {
		puts("pipe output to the terminal");
		return 1;
	}
	freopen(0, "wb", stdout);

#pragma pack(push, 1) // exact fit - no padding
	struct { unsigned char r,g,b; } buffer[WIDTH*WIDTH];
#pragma pack(pop)

	for (int y=0, yN=WIDTH; y<yN; ++y) {
		for (int x=0, xN=WIDTH; x<xN; ++x) {
			int squ_type= (x/SQUARE_SIZE+y/SQUARE_SIZE)%2;
			if (!(x/SQUARE_SIZE) && !(y/SQUARE_SIZE)){
				buffer[x+y*xN].r = 0xCC;
				buffer[x+y*xN].g = 0x00;
				buffer[x+y*xN].b = 0x00;
			} else if (x/SQUARE_SIZE == (xN-1)/SQUARE_SIZE && !(y/SQUARE_SIZE)){
				buffer[x+y*xN].r = 0x00;
				buffer[x+y*xN].g = 0xCC;
				buffer[x+y*xN].b = 0x00;
			} else if (!(x/SQUARE_SIZE) && y/SQUARE_SIZE == (yN-1)/SQUARE_SIZE){
				buffer[x+y*xN].r = 0x00;
				buffer[x+y*xN].g = 0x00;
				buffer[x+y*xN].b = 0xCC;
			} else if (x/SQUARE_SIZE == (xN-1)/SQUARE_SIZE && y/SQUARE_SIZE == (yN-1)/SQUARE_SIZE){
				buffer[x+y*xN].r = 0x00;
				buffer[x+y*xN].g = 0x00;
				buffer[x+y*xN].b = 0x00;
			} else {
				buffer[x+y*xN].r = squ_type ?0xAA :0xCC;
				buffer[x+y*xN].g = squ_type ?0xAA :0xCC;
				buffer[x+y*xN].b = squ_type ?0xAA :0xCC;
			}
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
