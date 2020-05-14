#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "structs.h"
#pragma once

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

void read_png_file(char *file_name, struct Png *image);
void write_png_file(char *file_name, struct Png *image);
void process_file(struct Png *image);
int image_processing(ProcessedArgs arguments, int operation);
void change_component(struct Png *image, int component, int value);
void change_color(struct Png *image, Color color, Color colorto);
void draw_line(struct Png *image, int n, int m, int width, Color color);