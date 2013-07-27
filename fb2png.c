#include <fcntl.h>
#include <png.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

int main(int argc, char *argv[])
{
    char *fbdevice = "/dev/fb0";
    char *pngname = "fb.png";

    int opt = 0;

    //--------------------------------------------------------------------

    while ((opt = getopt(argc, argv, "d:p:")) != -1)
    {
        switch (opt)
        {
        case 'd':

            fbdevice = optarg;
            break;

        case 'p':

            pngname = optarg;
            break;

        default:

            fprintf(stderr, "Usage: %s [-d device] [-p pngname]\n",
                    argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    //--------------------------------------------------------------------

    int fbfd = open(fbdevice, O_RDWR);

    if (fbfd == -1)
    {
        perror("Error: cannot open framebuffer");
        exit(EXIT_FAILURE);
    }

    struct fb_fix_screeninfo finfo;

    if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1)
    {
        perror("Error: reading framebuffer fixed information");
        exit(EXIT_FAILURE);
    }

    struct fb_var_screeninfo vinfo;

    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1)
    {
        perror("Error: reading framebuffer variable information");
        exit(EXIT_FAILURE);
    }

    if ((vinfo.bits_per_pixel != 16) &&
        (vinfo.bits_per_pixel != 24) &&
        (vinfo.bits_per_pixel != 32))
    {
        fprintf(stderr, "Error: framebuffer must be either 16, ");
		fprintf(stderr, "24 or 32 bits per pixel\n");
        exit(EXIT_FAILURE);
    }

    uint8_t *fbp = mmap(0,
						finfo.smem_len,
						PROT_READ | PROT_WRITE,
						MAP_SHARED,
						fbfd,
						0);

    if ((int)fbp == -1)
    {
        perror("Error: failed to map framebuffer device to memory");
        exit(EXIT_FAILURE);
    }

    //--------------------------------------------------------------------

    png_structp png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
												  NULL,
												  NULL,
												  NULL);

    if (png_ptr == NULL)
    {
        fprintf(stderr, "Error: could not allocate PNG write struct\n");
        exit(EXIT_FAILURE);
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);

    if (info_ptr == NULL)
    {
        fprintf(stderr, "Error: could not allocate PNG info struct\n");
        exit(EXIT_FAILURE);
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        fprintf(stderr, "Error: creating PNG\n");
        exit(EXIT_FAILURE);
    }

    FILE *pngfp = fopen(pngname, "wb");

    if (pngfp == NULL)
    {
        fprintf(stderr, "Error: Unable to create %s\n", pngname);
        exit(EXIT_FAILURE);
    }

    png_init_io(png_ptr, pngfp);

    png_set_IHDR(
        png_ptr,
        info_ptr,
        vinfo.xres,
        vinfo.yres,
        8,
        PNG_COLOR_TYPE_RGB,
        PNG_INTERLACE_NONE,
        PNG_COMPRESSION_TYPE_BASE,
        PNG_FILTER_TYPE_BASE);

    png_write_info(png_ptr, info_ptr);

    png_bytep png_buffer = malloc(vinfo.xres * 3 * sizeof(png_byte));

    if (png_buffer == NULL)
    {
        fprintf(stderr, "Unable to allocate buffer\n");
        exit(EXIT_FAILURE);
    }

    //--------------------------------------------------------------------

    int r_mask = (1 << vinfo.red.length) - 1;
    int g_mask = (1 << vinfo.green.length) - 1;
    int b_mask = (1 << vinfo.blue.length) - 1;

	int bytes_per_pixel = vinfo.bits_per_pixel / 8;

	int y = 0;

    for (y = 0; y < vinfo.yres; y++)
    {
        int x;

        for (x = 0; x < vinfo.xres; x++)
        {
            int pb_offset = 3 * x;

            long int fb_offset = (x + vinfo.xoffset) * (bytes_per_pixel)
                               + (y + vinfo.yoffset) * finfo.line_length;

            if (vinfo.bits_per_pixel == 16)
            {
                uint16_t pixel = *((uint16_t *)(fbp + fb_offset));

                int r = (pixel >> vinfo.red.offset) & r_mask;
                int g = (pixel >> vinfo.green.offset) & g_mask;
                int b = (pixel >> vinfo.blue.offset) & b_mask;

                png_buffer[pb_offset] = (r * 0xFF) / r_mask;
                png_buffer[pb_offset + 1] = (g * 0xFF)  / g_mask;
                png_buffer[pb_offset + 2] = (b * 0xFF)  / b_mask;
            }
            else
            {
				uint8_t *pixels = fbp + fb_offset;

                png_buffer[pb_offset] = *(pixels+(vinfo.red.offset/8));
                png_buffer[pb_offset+1] = *(pixels+(vinfo.green.offset/8));
                png_buffer[pb_offset+2] = *(pixels+(vinfo.blue.offset/8));
            }
        }

        png_write_row(png_ptr, png_buffer);
    }

    //--------------------------------------------------------------------

    free(png_buffer);
    png_buffer = NULL;
    png_write_end(png_ptr, NULL);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(pngfp);

    //--------------------------------------------------------------------

    munmap(fbp, finfo.smem_len);
    close(fbfd);

    //--------------------------------------------------------------------

    return 0;
}
