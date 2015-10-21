#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>

#include <sys/fbio.h>

struct fb_instance {
	int fd;
	struct fbtype fb;
	void *mmap;
	int mmap_len;
};

static int
fb_get_type(struct fb_instance *fb)
{
	int ret;

	ret = ioctl(fb->fd, FBIOGTYPE, &fb->fb, sizeof(fb->fb));
	if (ret < 0) {
		warn("%s: FBIOGTYPE", __func__);
		return (-1);
	}
	printf("%s: type=%d, height=%d, width=%d, depth=%d, cmsize=%d, size=%d\n",
	    __func__,
	    fb->fb.fb_type,
	    fb->fb.fb_height,
	    fb->fb.fb_width,
	    fb->fb.fb_depth,
	    fb->fb.fb_cmsize,
	    fb->fb.fb_size);
	return (0);
}

static int
fb_mmap(struct fb_instance *fb)
{

	fb->mmap = mmap(NULL, fb->fb.fb_size, PROT_READ | PROT_WRITE, MAP_SHARED, fb->fd, 0);
	if (fb->mmap == MAP_FAILED) {
		warn("%s: mmap", __func__);
		return (-1);
	}
	return (0);
}

static void
fb_close(struct fb_instance *fb)
{

	if (fb->mmap) {
		munmap(fb->mmap, fb->mmap_len);
		fb->mmap = NULL;
	}

	if (fb->fd > 0)
		close(fb->fd);
}

static void
fb_fill(struct fb_instance *fb, int rgb)
{
	int x, y;
	int stride = FBTYPE_GET_STRIDE(&fb->fb);
	int offset, offset_start;
	uint32_t *fbd = fb->mmap;
	int color = 0;

	offset = 0;
	/* XXX hard-coded for 24 bit RGB for now */
	for (y = 0; y < fb->fb.fb_height; y++) {
		offset_start = offset;
		for (x = 0; x < fb->fb.fb_width; x++) {
			fbd[offset] = rgb;
			offset++;
		}
		/* XXX stride is in bytes.. */
		offset = offset_start + (stride / sizeof(uint32_t));
	}
}

int
main(int argc, const char *argv[])
{
	struct fb_instance fb;

	memset(&fb, '\0', sizeof(fb));

	fb.fd = open("/dev/fb0", O_RDWR);
	if (fb.fd < 0) {
		err(1, "mmap /dev/fb0");
	}

	/* Get the framebuffer setup */
	if (fb_get_type(&fb) != 0)
		goto finish;

	/* Mmap it */
	if (fb_mmap(&fb) != 0)
		goto finish;

	fb_fill(&fb, 0);
	fb_fill(&fb, 0xaaaaaaaa);
	fb_fill(&fb, 0x0);

finish:
	fb_close(&fb);
	exit(0);
}
