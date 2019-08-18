/*__      ___
 / /_____/ _ |_______ ____ ____
/ __/ __/ __ / __/ -_) __// __/
\__/_/ /_/ |_\__/\__/_/ (_)__/
 * "Hope is soooooooooo amazing and great!" ~ Hope O'Malley 2019
 *
 * Drew O'Malley
 */

#include <qmisc/macros.h>

#include <unistd.h>
#include <stdio.h>
#include <stdint.h>

#pragma pack(push, 1) // exact fit - no padding
typedef struct {
	uint8_t r,g,b;
} v3u_t;
#pragma pack(pop)

static void
write_test_image(v3u_t *pixels, size_t width, size_t height) {
	qassert(pixels);
	enum { SQUARE_SIZE = 16 };

	static v3u_t cboard[2] = { {0xAA,0xAA,0xAA},{0xCC,0xCC,0xCC} };


	for (int y=0, yN=height; y<yN; ++y) {
		for (int x=0, xN=width; x<xN; ++x) {
			if (!(x/SQUARE_SIZE) && !(y/SQUARE_SIZE)){
				pixels[x+y*xN].r = 0xCC;
				pixels[x+y*xN].g = 0x00;
				pixels[x+y*xN].b = 0x00;
			} else if (x/SQUARE_SIZE == (xN-1)/SQUARE_SIZE && !(y/SQUARE_SIZE)){
				pixels[x+y*xN].r = 0x00;
				pixels[x+y*xN].g = 0xCC;
				pixels[x+y*xN].b = 0x00;
			} else if (!(x/SQUARE_SIZE) && y/SQUARE_SIZE == (yN-1)/SQUARE_SIZE){
				pixels[x+y*xN].r = 0x00;
				pixels[x+y*xN].g = 0x00;
				pixels[x+y*xN].b = 0xCC;
			} else if (x/SQUARE_SIZE == (xN-1)/SQUARE_SIZE && y/SQUARE_SIZE == (yN-1)/SQUARE_SIZE){
				pixels[x+y*xN].r = 0x00;
				pixels[x+y*xN].g = 0x00;
				pixels[x+y*xN].b = 0x00;
			} else { /* not a corner, checkerboard */
				pixels[x+y*xN] = cboard[(x/SQUARE_SIZE+y/SQUARE_SIZE)%2];
			}
		}
	}
}

int
main (int argc, char *argv[]) { (void)argc;  (void)argv;
	enum { WIDTH = 120, SQUARE_SIZE = 16 };
	if (isatty(fileno(stdout))) {
		puts("output should be piped to a file, as output is binary");
		return 1;
	}
	freopen(0, "wb", stdout);

	v3u_t buffer[WIDTH*WIDTH];
	write_test_image((v3u_t *)&buffer, WIDTH, WIDTH);

	printf("P6\t%d\t%d\t255\t", WIDTH, WIDTH);
	for (int y=0, yN=WIDTH; y<yN; ++y) {
		fwrite(buffer, sizeof(buffer), 1, stdout);
	}
	fflush(stdout);

	return 0;
}
