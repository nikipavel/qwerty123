#define COLOR_CHANGE  1
#define COMPONENT_VALUE_CHANGE  2
#define CROP_BY_LINE  3
#pragma once


typedef struct{
	int red;
	int green;
	int blue;
} Color;

struct globalArgs_t {
    char* component;                
    int value;             
    FILE* image;
    char* change;
    char* to;
    int vert;
    int hor;
	int line_width;
	char* line_color;
} globalArgs;

typedef struct {
	Color from;
	Color to;
	char* image;
	int component;
	int value;
	int vert;
	int hor;
	int line_width;
	Color line_color;
} ProcessedArgs;