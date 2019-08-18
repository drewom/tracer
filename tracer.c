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
#include <stdlib.h>
#include <stdint.h>

#pragma pack(push, 1) // exact fit - no padding
typedef struct {
	uint8_t r,g,b;
} rgb24_t;
#pragma pack(pop)

static void fillout_test_image(rgb24_t *pixels, size_t width, size_t height);
static void write_test_image_out(FILE *to, rgb24_t *pixels, size_t width, size_t height);

int
main (int argc, char *argv[]) { (void)argc;  (void)argv;
	enum { WIDTH = 120, SQUARE_SIZE = 16 };
	if (isatty(fileno(stdout))) {
		puts("output should be piped to a file, as output is binary");
		return 1;
	}

	rgb24_t *buffer = malloc(sizeof(rgb24_t)*WIDTH*WIDTH);
	fillout_test_image(buffer, WIDTH, WIDTH);
	write_test_image_out(stdout, buffer, WIDTH, WIDTH);

	return 0;
}

static void
fillout_test_image(rgb24_t *pixels, size_t width, size_t height) {
	qassert(pixels);
	enum { SQUARE_SIZE = 16 };

	static rgb24_t cboard[2] = { {0xAA,0xAA,0xAA},{0xCC,0xCC,0xCC} };
	static rgb24_t corners[4] = { {0xCC, 0x00, 0x00}, {0x00, 0xCC, 0x00}, {0x00, 0x00, 0xCC}, {} };


	for (int y=0, yN=height; y<yN; ++y) {
		for (int x=0, xN=width; x<xN; ++x) {
			int top    = !(x/SQUARE_SIZE),
				 bottom = x/SQUARE_SIZE == (xN-1)/SQUARE_SIZE,
			    left   = !(y/SQUARE_SIZE),
				 right  = y/SQUARE_SIZE == (yN-1)/SQUARE_SIZE;
			if((top || bottom) && (left || right)){
				pixels[x+y*xN] = corners[bottom*2|right];
			} else { /* not a corner, checkerboard */
				pixels[x+y*xN] = cboard[(x/SQUARE_SIZE+y/SQUARE_SIZE)%2];
			}
		}
	}
}

static void write_test_image_out(FILE *to, rgb24_t *pixels, size_t width, size_t height) {
	freopen(0, "wb", stdout); /* Ensure this stream is open in binary mode */

	printf("P6\t%ld\t%ld\t255\t", width, height);
	fwrite(pixels, width*height*sizeof(rgb24_t), 1, to);
	fflush(to);
}
