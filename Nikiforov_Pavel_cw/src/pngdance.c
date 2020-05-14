#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "structs.h"

#define PNG_DEBUG 3
#include <png.h>

struct Png{
    int width, height;
    png_byte color_type;
    png_byte bit_depth;

    png_structp png_ptr;
    png_infop info_ptr;
    int number_of_passes;
    png_bytep *row_pointers;
};

void read_png_file(char *file_name, struct Png *image) {
    int x,y;
    char header[8];    // 8 is the maximum size that can be checked

    /* open file and test for it being a png */
    FILE *fp = fopen(file_name, "rb");
    if (!fp){
        printf("Error: file could not be opened\n");
        return;
    }

    fread(header, 1, 8, fp);
    if (png_sig_cmp((png_const_bytep) header, 0, 8)){
        printf("Error: file is not recognized as a PNG\n");
        return;
    }

    /* initialize stuff */
    image->png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!image->png_ptr){
        printf("Error: png_create_read_struct failed\n");
        return;
    }

    image->info_ptr = png_create_info_struct(image->png_ptr);
    if (!image->info_ptr){
        printf("Error: png_create_info_struct failed\n");
        return;
    }

    if (setjmp(png_jmpbuf(image->png_ptr))){
        printf("Error: error during init_io\n");
        return;
    }

    png_init_io(image->png_ptr, fp);
    png_set_sig_bytes(image->png_ptr, 8);

    png_read_info(image->png_ptr, image->info_ptr);

    image->width = png_get_image_width(image->png_ptr, image->info_ptr);
    image->height = png_get_image_height(image->png_ptr, image->info_ptr);
    image->color_type = png_get_color_type(image->png_ptr, image->info_ptr);
    image->bit_depth = png_get_bit_depth(image->png_ptr, image->info_ptr);

    image->number_of_passes = png_set_interlace_handling(image->png_ptr);
    png_read_update_info(image->png_ptr, image->info_ptr);

    /* read file */
    if (setjmp(png_jmpbuf(image->png_ptr))){
        printf("Error: error during read_image\n");
        return;
    }

    image->row_pointers = (png_bytep *) malloc(sizeof(png_bytep) * image->height);
    for (y = 0; y < image->height; y++)
        image->row_pointers[y] = (png_byte *) malloc(png_get_rowbytes(image->png_ptr, image->info_ptr));

    png_read_image(image->png_ptr, image->row_pointers);

    fclose(fp);
}


void write_png_file(char *file_name, struct Png *image) {
    int x,y;
    /* create file */
    FILE *fp = fopen(file_name, "wb");
    if (!fp){
        printf("Error: file could not be opened\n");
        return;
    }

    /* initialize stuff */
    image->png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!image->png_ptr){
        printf("Error: png_create_write_struct failed\n");
        return;
    }

    image->info_ptr = png_create_info_struct(image->png_ptr);
    if (!image->info_ptr){
        printf("Error: png_create_info_struct failed\n");
        return;
    }

    if (setjmp(png_jmpbuf(image->png_ptr))){
        printf("Error: error during init_io\n");
        return;
    }

    png_init_io(image->png_ptr, fp);


    /* write header */
    if (setjmp(png_jmpbuf(image->png_ptr))){
        printf("Error: error during writing header\n");
        return;
    }

    png_set_IHDR(image->png_ptr, image->info_ptr, image->width, image->height,
                 image->bit_depth, image->color_type, PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    png_write_info(image->png_ptr, image->info_ptr);


    /* write bytes */
    if (setjmp(png_jmpbuf(image->png_ptr))){
        printf("Error: error during writing bytes\n");
        return;
    }

    png_write_image(image->png_ptr, image->row_pointers);


    /* end write */
    if (setjmp(png_jmpbuf(image->png_ptr))){
        printf("Error handling: error during end of write\n");
        return;
    }

    png_write_end(image->png_ptr, NULL);

    /* cleanup heap allocation */
    for (y = 0; y < image->height; y++)
        free(image->row_pointers[y]);
    free(image->row_pointers);

    fclose(fp);
}


void change_component(struct Png *image, int component, int value) {
    int x,y;
    if (png_get_color_type(image->png_ptr, image->info_ptr) == PNG_COLOR_TYPE_RGB){
        printf("Error: input file is PNG_COLOR_TYPE_RGB but must be PNG_COLOR_TYPE_RGBA\n");
        return;
    }

    if (png_get_color_type(image->png_ptr, image->info_ptr) != PNG_COLOR_TYPE_RGBA){
        printf("Eerror: color_type of input file must be PNG_COLOR_TYPE_RGBA\n");
        return;
    }

    for (y = 0; y < image->height; y++) {
        png_byte *row = image->row_pointers[y];
        for (x = 0; x < image->width; x++) {
            png_byte *ptr = &(row[x * 4]);
            ptr[component] = value;
        }
    }
}

void draw_line(struct Png *image, int vert, int hor, int width, Color color) {
    int x,y;
    if (png_get_color_type(image->png_ptr, image->info_ptr) == PNG_COLOR_TYPE_RGB){
        printf("Error: input file is PNG_COLOR_TYPE_RGB but must be PNG_COLOR_TYPE_RGBA\n");
        return;
    }

    if (png_get_color_type(image->png_ptr, image->info_ptr) != PNG_COLOR_TYPE_RGBA){
        printf("Error: color_type of input file must be PNG_COLOR_TYPE_RGBA\n");
        return;
    }
    int each_x = image->width/hor;
    int each_y = image->height/vert;
    int x_start, x_end, y_start, y_end;
    if (width * hor > image->width || width * vert > image->height) {
        printf("Error: too much pieces or too wide line\n");
        return;
    }
    for (y = 0; y < image->height; y++) {
        png_byte *row = image->row_pointers[y];
        for (x = 0; x < image->width; x++) {
            png_byte *ptr = &(row[x * 4]);
            x_start = each_x - width/2;
            x_end = each_x + width/2;
            y_start = each_y - width/2;
            y_end = each_y + width/2;
            if (x >= x_start && x <= x_end && x_end < image->width) {
                ptr[0] = color.red;
                ptr[1] = color.green;
                ptr[2] = color.blue;
            }
            if (y >= y_start && y <= y_end && y_end < image->height) {
                ptr[0] = color.red;
                ptr[1] = color.green;
                ptr[2] = color.blue;
            }
            if (x == x_end + 1) each_x += image->width/hor;
            if (y == y_end + 1) each_y += image->height/vert;
        }
        each_x = image->width/hor;
    }
}

void change_color(struct Png *image, Color color, Color colorto) {
    int x,y;
    if (png_get_color_type(image->png_ptr, image->info_ptr) == PNG_COLOR_TYPE_RGB){
        printf("Error: input file is PNG_COLOR_TYPE_RGB but must be PNG_COLOR_TYPE_RGBA\n");
        return;
    }

    if (png_get_color_type(image->png_ptr, image->info_ptr) != PNG_COLOR_TYPE_RGBA){
        printf("Error: color_type of input file must be PNG_COLOR_TYPE_RGBA\n");
        return;
    }

    for (y = 0; y < image->height; y++) {
        png_byte *row = image->row_pointers[y];
        for (x = 0; x < image->width; x++) {
            png_byte *ptr = &(row[x * 4]);
            if (ptr[0] == color.red && ptr[1] == color.green && ptr[2] == color.blue) {
                ptr[0] = colorto.red;
                ptr[1] = colorto.green;
                ptr[2] = colorto.blue;
            }   
        }
    }
}


int image_processing(ProcessedArgs arguments, int operation) {
    struct Png image;
    read_png_file(arguments.image, &image);
    if (operation == COMPONENT_VALUE_CHANGE) {
        change_component(&image, arguments.component, arguments.value);
    } else if (operation == COLOR_CHANGE) {
        change_color(&image, arguments.from, arguments.to);
    } else if (operation == CROP_BY_LINE) {
        draw_line(&image, arguments.vert, arguments.hor, arguments.line_width, arguments.line_color);
    }
    write_png_file(arguments.image, &image);
    return 0;
}