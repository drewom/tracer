/*__      ___
 / /_____/ _ |_______ ____ ____
/ __/ __/ __ / __/ -_) __// __/
\__/_/ /_/ |_\__/\__/_/ (_)__/
 * "Hope is soooooooooo amazing and great!" ~ Hope O'Malley 2019
 *
 * Drew O'Malley
 */

int
main (int argc, char *argv[]) { (void)argc;  (void)argv;
	uint32_t buffer[512*512];

	for (int y=0, yN=512; y<yN; ++y) {
		for (int x=0, xN=512; x<xN; ++x) {
			buffer[x+y*xN] = (x/4+y/4)%2 ?0xAAAAAAAA :0xCCCCCCCC;
		}
	}

	return 0;
}
