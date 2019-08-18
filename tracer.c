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
#include <getopt.h>

#pragma pack(push, 1) // exact fit - no padding
typedef struct {
	uint8_t r,g,b;
} rgb24_t;
#pragma pack(pop)

typedef struct {
	int width, height;
	FILE *out;
} settings_t;

static settings_t parse_options(int argc, char *argv[]);
static void fillout_test_image(rgb24_t *pixels, size_t width, size_t height);
static void write_test_image_out(FILE *to, rgb24_t *pixels, size_t width, size_t height);

int
main (int argc, char *argv[]) {

	/* parse settings, get image buffer and fill it with test background */
	settings_t settings = parse_options(argc, argv);
	rgb24_t *buffer = malloc(sizeof(rgb24_t)*settings.width*settings.height);
	fillout_test_image(buffer, settings.width, settings.height);

	write_test_image_out(settings.out, buffer, settings.width, settings.height);
	return 0;
}

static void
fillout_test_image(rgb24_t *pixels, size_t width, size_t height) {
	qassert(pixels);
	enum { SQUARE_SIZE = 8 };

	static rgb24_t cboard[2] = { {0xAA,0xAA,0xAA},{0xCC,0xCC,0xCC} };
	static rgb24_t corners[4] = { {0x90, 0x00, 0x00}, {0x00, 0x90, 0x00}, {0x00, 0x00, 0x90}, {} };

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
	qassert(to);
	freopen(0, "wb", to); /* Ensure this stream is open in binary mode */

	fprintf(to, "P6\t%ld\t%ld\t255\t", width, height);
	fwrite(pixels, width*height*sizeof(rgb24_t), 1, to);
	fflush(to);
}
static settings_t parse_options(int argc, char *argv[]) {
	settings_t result = { 800, 600, stdout };
#if DEBUG
	char *_argv[] = {"", "-o", "foo.ppm", "-w", "320", "-h", "256", 0};
	argv = _argv; argc = 7;
#endif

	int option;
	do {
		static struct option details[] = {
			{"width",  required_argument, 0, 'w'},
			{"height", required_argument, 0, 'h'},
			{"output", required_argument, 0, 'o'},
			{}};
      int index = 0;
		option = getopt_long(argc, argv, "w:h:o:", details, &index);
		switch(option){
			case -1 :{}break;
			case 'w':{ result.width  = atoi(optarg);         }break;
			case 'h':{ result.height = atoi(optarg);         }break;
			case 'o':{ result.out    = fopen(optarg, "w+b"); }break;
			case '?': default :{ abort();                    }break;
		}
	} while(option != -1);

	if (isatty(fileno(result.out))) {
		fputs("use -o flag or file redirection, output is binary\n", stderr);
		abort();
	}
	return result;
}
