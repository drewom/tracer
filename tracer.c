/*__      ___
 / /_____/ _ |_______ ____ ____
/ __/ __/ __ / __/ -_) __// __/
\__/_/ /_/ |_\__/\__/_/ (_)__/
 * "Hope is soooooooooo amazing and great!" ~ Hope O'Malley 2019
 *
 * Drew O'Malley
 */

#include <qmisc/macros.h>
#include <qmisc/tensor.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <float.h>
#include <getopt.h>

#pragma pack(push, 1) // exact fit - no padding
typedef struct {
	uint8_t r,g,b;
} rgb24_t;
#pragma pack(pop)
static rgb24_t
rgb24 (uint8_t r, uint8_t g, uint8_t b) {
	rgb24_t res = {r, g, b};
	return res;
}
static rgb24_t
rgb24_from_v3 (v3_t rgb) {
	rgb24_t res = {};
	res.r = qclamp(0, rgb.r*UINT8_MAX, UINT8_MAX);
	res.g = qclamp(0, rgb.g*UINT8_MAX, UINT8_MAX);
	res.b = qclamp(0, rgb.b*UINT8_MAX, UINT8_MAX);
	return res;
}

typedef struct {
	/* camera */
	v3_t cam_pos, cam_x, cam_y, cam_z;

	/* material */
	size_t material_count;
	v3_t  *material_colour;

	/* planes */
	size_t plane_count;
	v3_t   *plane_normal;
	float  *plane_dist;
	size_t *plane_material;

	/* spheres */
	size_t sphere_count;
	v3_t   *sphere_centre;
	float  *sphere_radius;
	size_t *sphere_material;
} scene_t;

typedef struct {
	v3_t ori, nrm;
} ray_t;

typedef struct {
	int width, height;
	FILE *out;
} settings_t;

static settings_t parse_options(int argc, char *argv[]);
static void fillout_test_image(rgb24_t *pixels, size_t width, size_t height);
static void write_image_to_ppm(FILE *to, rgb24_t *pixels, size_t width, size_t height);

static float
raycast (ray_t ray, scene_t *scene, rgb24_t *out) {
	qassert(scene); qassert(out);

	/* points on ray: ray.ori + t*ray.nrm = point_on_ray where t>=0 */
	float dist = FLT_MAX;
	for(int p=0, pN=scene->plane_count; p<pN; ++p){

		v3_t p_nrm = scene->plane_normal[p];
		float p_dst = scene->plane_dist[p];

		float divide = v3T_mult_v3(p_nrm.T, ray.nrm);
		if (divide < FLT_EPSILON && divide > -FLT_EPSILON) {
			continue;
		}
		float curr = (-p_dst - v3T_mult_v3(p_nrm.T, ray.ori)) / divide;
		if (curr > 0.f && curr < dist) {
			*out = rgb24_from_v3(scene->material_colour[scene->plane_material[p]]);

			dist = curr;
		}
	}
	for(int s=0, sN=scene->sphere_count; s<sN; ++s){
		/* sphere equation:
		 * pT*p - s.r^2 = 0,
		 * (ray.ori + t*ray.nrm)T*(ray.ori + t*ray.nrm) - s.r^2
		 * solving for t:
		 * 0 == t^2*ray.nrm.T*ray.nrm + t*2*ray.ori.T*ray.nrm + ray.ori.T*ray.ori-s.r^2
		 *          ^a term^              ^b term^              ^c term^
		 * t == -b +/- sqrt(b^2 -4ac) / 2a
		 */
		v3_t s_ori = scene->sphere_centre[s];
		float s_rad = scene->sphere_radius[s];

		ray_t r = ray;
		r.ori = v3_add(ray.ori, v3_neg(s_ori));

		/* convert r to unit circle space */
		float a =   v3T_mult_v3(r.nrm.T, r.nrm),
		      b = 2*v3T_mult_v3(r.ori.T, r.nrm),
		      c =   v3T_mult_v3(r.ori.T, r.ori)-(s_rad*s_rad);
		float sqrt_term = b*b - 4.f*a*c;
		float _2a = 2.f*a;

		if (sqrt_term < FLT_EPSILON) {
			continue; /* ray misses sphere */
		}
		qassert(_2a > FLT_EPSILON || _2a < -FLT_EPSILON); /* ray normal was zero? */

		/* treat spheres as one sided, if you want to draw inside then invert radius */
		float curr = -b - sqrt(sqrt_term);
		curr /= _2a;
		if (curr > 0.f && curr < dist) {
			*out = rgb24_from_v3(scene->material_colour[scene->sphere_material[s]]);

			dist = curr;
		}
	}
	return dist;
}

int
main (int argc, char *argv[]) {

	/* parse settings, get image buffer and fill it with test background */
	settings_t settings = parse_options(argc, argv);
	rgb24_t *buffer = malloc(sizeof(rgb24_t)*settings.width*settings.height);
	fillout_test_image(buffer, settings.width, settings.height);

	/* Raytracer start here */
	scene_t scene = {};
	scene.cam_pos = v3(0.f, 4.f, 10.f);
	scene.cam_z = v3_norm_or_zero(scene.cam_pos);
	scene.cam_x = v3_norm_or_zero(v3_cross(v3(0,1,0), scene.cam_pos));
	scene.cam_y = v3_norm_or_zero(v3_cross(scene.cam_y, scene.cam_z));

	v3_t material_colours[] = { v3(.3f,.3f,.2f), v3(.2f,.2f,.4f), v3(.1f,.5f,.4f), v3(.1f, .8f, .5f), v3(.6f, .1f, .5f), v3(.3f, .5f, .8f),  };
	v3_t plane_normals[] = {v3(0,1.f,0), v3_norm_or_zero(v3(-1.f, 4.f, 0)), v3_norm_or_zero(v3(1.f, 3.f, 0))};
	float plane_dist[] = {0.f, 4.f, 8.f};
	size_t plane_materials[] = {0, 1, 2};
	v3_t  sphere_centres[] = {v3(0,3.f,3),v3(5.f,0,3),v3(-4.f,3.f,6.f)};
	float sphere_radii[] = {3.f, 2.f, 1.f};
	size_t sphere_materials[] = {3, 4, 5};

	scene.material_colour = material_colours;
	scene.material_count = qcountof(material_colours);

	scene.plane_normal = plane_normals;
	scene.plane_dist = plane_dist;
	scene.plane_material = plane_materials;
	scene.plane_count = qcountof(plane_normals);

	scene.sphere_centre = sphere_centres;
	scene.sphere_radius = sphere_radii;
	scene.sphere_material = sphere_materials;
	scene.sphere_count = qcountof(sphere_centres);

	float aspect_ratio_u = 1.f,
	      aspect_ratio_v = 1.f;
	if (settings.width > settings.height) {
		aspect_ratio_u = (float)settings.width / (float)settings.height;
	} else {
		aspect_ratio_v = (float)settings.height / (float)settings.width;
	}

	for (int y=0, yN=settings.height; y<yN; ++y) {
		float nearplane_v = +aspect_ratio_v - 2.f*aspect_ratio_v*((float)y/(float)yN);
		for (int x=0, xN=settings.width; x<xN; ++x) {
			float nearplane_u = -aspect_ratio_u + 2.f*aspect_ratio_u*((float)x/(float)xN);

			ray_t ray = {};
			ray.ori = scene.cam_pos;
			ray.nrm = v3_neg(scene.cam_z);
			ray.nrm.x += nearplane_u;
			ray.nrm.y += nearplane_v;
			raycast(ray, &scene, &buffer[x+y*xN]);
		}
	}

	/* Write out image */
	write_image_to_ppm(settings.out, buffer, settings.width, settings.height);
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
			} else { /* not a corner, so checker board */
				pixels[x+y*xN] = cboard[(x/SQUARE_SIZE+y/SQUARE_SIZE)%2];
			}
		}
	}
}

static void write_image_to_ppm(FILE *to, rgb24_t *pixels, size_t width, size_t height) {
	qassert(to);
	freopen(0, "wb", to); /* Ensure this stream is open in binary mode */

	fprintf(to, "P6\t%ld\t%ld\t255\t", width, height);
	fwrite(pixels, width*height*sizeof(rgb24_t), 1, to);
	fflush(to);
}
static settings_t parse_options(int argc, char *argv[]) {
	settings_t result = { 800, 600, stdout };
#if DEBUG
	char *_argv[] = {"", "-o", "foo.ppm", "-w", "420", "-h", "256", 0};
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
