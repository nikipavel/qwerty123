#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include "pngdance.h"
#include "structs.h"

Color color_pick(char* color_name);
int component_pick(char* component);


int component_pick(char* component) {
	if (strcmp(component, "Red") == 0) return 0;
	if (strcmp(component, "Green") == 0) return 1;
	return 2;
}

Color color_pick(char* color_name) {
	Color color = {0, 0, 0};
	if (strcmp(color_name, "White") == 0) {
		color.red = 255;
		color.green = 255;
		color.blue = 255;
	} else if (strcmp(color_name, "Red") == 0) {
		color.red = 255;
	} else if (strcmp(color_name, "Blue") == 0) {
		color.blue = 255;
	} else if (strcmp(color_name, "Green") == 0) {
		color.green = 255;
	}else if (strcmp(color_name, "Cyan") == 0) {
		color.green = 255;
		color.blue = 255;
	}else if (strcmp(color_name, "Magenta") == 0) {
		color.red = 255;
		color.blue = 255;
	}else if (strcmp(color_name, "Yellow") == 0) {
		color.red = 255;
		color.green = 255;
	}
	return color;
}


void printHelp(){
	printf("		PNGEDITOR\n");
	printf("Change RGB-component value:\n");
	printf("--filter -c <component> -v <value> <file> - change component's value\n");
	printf("Change some color:\n");
	printf("--color --change <color> --to <color> <file>\n");
	printf("Crop image to N*M parts by line:\n");
	printf("--crop --vert <vertical> --hor <horizontal> --width <line> --linecolor <color> <file>\n");
	printf("Arguments:\n");
	printf("<file> - PNG image file\n");
	printf("<sign>:\n");
	printf("<color>:\n");
	printf("	Black\n");
	printf("	Blue\n");
	printf("	Green\n");
	printf("	Cyan\n");
	printf("	Red\n");
	printf("	Magenta\n");
	printf("	Yellow\n");
	printf("	White\n");
	printf("<component>:\n");
	printf("	Red\n");
	printf("	Green\n");
	printf("	Blue\n");
	printf("<value>:\n");
	printf("	integer in range 0-255 for component's value\n");
	printf("<vertical> <horizontal>:\n");
	printf("	unsigned integers > 0\n");
	printf("<line>:\n");
	printf("	integer > 0\n");
	printf("-h -? --help - help\n");
}

int main(int argc, char* argv[]){
	char *opts = "c:v:h?";
	int colorFlag = 0;
	int filterFlag = 0;
	int cropFlag = 0;
	struct globalArgs_t arguments = {
		(char*) malloc(10*sizeof(char)),
		-1,
		NULL,
		(char*) malloc(10*sizeof(char)),
		(char*) malloc(10*sizeof(char)),
		-1,
		-1,
		-1,
		(char*) malloc(10*sizeof(char))
	};
	struct option longOpts[]={
		{"filter", no_argument, &filterFlag, 'f'},
		{"color", no_argument, &colorFlag, 1},
		{"change", required_argument, NULL, 2},
		{"to", required_argument, NULL, 3},
		{"vert", required_argument, NULL, 4},
		{"hor", required_argument, NULL, 5},
		{"width", required_argument, NULL, 6},
		{"linecolor", required_argument, NULL, 7},
		{"help", no_argument, NULL, 'h'},
		{"crop", no_argument, &cropFlag, 8},
		{ NULL, 0, NULL, 0}
	};
	int opt;
	int value;
	int longIndex;
	int inputErrorFlag = 0;
	ProcessedArgs pr_args;
	char colors[8][10] = {"Red", "Green", "Blue", "Black", "White", "Cyan", "Yellow", "Magenta"};
	opt = getopt_long(argc, argv, opts, longOpts, &longIndex);
	while(opt!=-1){
		int color = 0;
		switch(opt){
			case 'v':
				value = atoi(optarg);
				if (value < 0 || value > 255) {
					inputErrorFlag = 1;
					printf("Error: wrong value input\n");
				} else {
					arguments.value = value;
				}
				break;
			case 'c':
				for (int i = 0; i < 3; i++) 
					if (strcmp(optarg, colors[i]) == 0) color = 1;
				if (color) {
					strcpy(arguments.component, optarg);
				} else {
					printf("Error: there is no such component!\n");
					inputErrorFlag = 1;
				}
				break;
			case 2:
			case 3:
			case 7:
				for (int i = 0; i < 8; i++) 
					if (strcmp(optarg, colors[i]) == 0) color = 1;
				if (color) {
					if (opt == 2) strcpy(arguments.change, optarg);
					if (opt == 3) strcpy(arguments.to, optarg);
					if (opt == 7) strcpy(arguments.line_color, optarg);
				} else {
					printf("Error: there is no such color as %s!\n", optarg);
					inputErrorFlag = 1;
				}
				break;
			case 4:
			case 5:
			case 6:
				value = atoi(optarg);
				if (value < 0) {
					inputErrorFlag = 1;
					printf("Error: wrong value input\n");
				} else {
					if (opt == 4) arguments.vert = value;
					if (opt == 5) arguments.hor = value;
					if (opt == 6) arguments.line_width = value;
				}
				break;
			case 'h':
			case '?':
				printHelp();
				return 0;
		}
		longIndex = 0;
		opt = getopt_long(argc, argv, opts, longOpts, &longIndex);
		if (inputErrorFlag) break;
	}
	FILE *fp = fopen(argv[argc - 1], "r");
	if (fp == NULL) {
		printf("Error: couldn't open png file\n");
		return 1;
	} else {
		pr_args.image = (char*) malloc(100*sizeof(char));
		strcpy(pr_args.image, argv[argc-1]);
	}
	if (filterFlag) {
		if (strlen(arguments.component) > 0 && arguments.value != -1) {
			pr_args.component = component_pick(arguments.component);
			pr_args.value = arguments.value;
			image_processing(pr_args, COMPONENT_VALUE_CHANGE);
			printf("Filter succsesfully applied\n");
		} else {
			printf("Filter impossible\n");
			return 1;
		}
	}
	if (colorFlag) {
		if (strlen(arguments.change) > 0 && strlen(arguments.to) > 0) {
			pr_args.from = color_pick(arguments.change);
			pr_args.to = color_pick(arguments.to);
			image_processing(pr_args, COLOR_CHANGE);
			printf("Color changing succeed\n");
		} else {
			printf("Color changing impossible\n");
			return 1;
		}
	}
	if (cropFlag) {
		if (strlen(arguments.line_color) > 0 && arguments.line_width != -1 
		&& arguments.hor != -1 && arguments.vert != -1) {
			pr_args.hor = arguments.hor;
			pr_args.vert = arguments.vert;
			pr_args.line_color = color_pick(arguments.line_color);
			pr_args.line_width = arguments.line_width;
			image_processing(pr_args, CROP_BY_LINE);
		} else {
			printf("Crop is impossible\n");
			return 1;
		}
	}
	return 0;
}