/*
  
*/

#include <stdio.h>
#include <stdlib.h>

#include "png.h"        /* libpng header; includes zlib.h */
#include "readpng.h"    /* typedefs, common macros, public prototypes */

#define PROGNAME "png-size"

int significant_horizontal_line( const unsigned char * image_data,
                                 int y,
                                 unsigned long image_width,
                                 unsigned long pitch,
                                 unsigned long minimum_width)
{
    int run_length = 0, x;
    
    for( x = 0; x < image_width; ++x ) {

        int non_white = image_data[y*pitch+(x*3)] != 0xFF;

        if( non_white )
            ++ run_length;
        else
            run_length = 0;

        if( run_length > minimum_width )
            return 1;

    }

    return 0;
}


int significant_vertical_line( const unsigned char * image_data,
                               int x,
                               unsigned long image_height,
                               unsigned long pitch,
                               unsigned long minimum_height)
{
    int run_length = 0, y;
    
    for( y = 0; y < image_height; ++y ) {
        
        int non_white = image_data[y*pitch+(x*3)] != 0xFF;

        if( non_white )
            ++ run_length;
        else
            run_length = 0;

        if( run_length > minimum_height )
            return 1;

    }

    return 0;
}

int main( int argc, char **argv ) {

    char * filename;
    FILE * infile;
    int rc;
    unsigned long image_width, image_height, image_depth;

    if( argc != 2 ) {
        printf("Usage: find-grid <png-file-name>\n");
        return -1;
    }
    
    filename = argv[1];

    infile = fopen(filename, "rb");
    
    if( !infile ) {
        fprintf(stderr, PROGNAME ":  can't open PNG file [%s]\n", filename);
        return -1;
        
    }

    if ((rc = readpng_init(infile, &image_width, &image_height, &image_depth)) != 0) {
        switch (rc) {
        case 1:
            fprintf(stderr, PROGNAME
                    ":  [%s] is not a PNG file: incorrect signature\n",
                    filename);
            return -1;
            case 2:
                fprintf(stderr, PROGNAME
                        ":  [%s] has bad IHDR (libpng longjmp)\n",
                        filename);
                return -1;
        case 4:
            fprintf(stderr, PROGNAME ":  insufficient memory\n");
            return -1;
        default:
            fprintf(stderr, PROGNAME
                    ":  unknown readpng_init() error\n");
            return -1;
        }
    }

    printf("%lux%lu",image_width,image_height);
    
    readpng_cleanup(TRUE);
    fclose(infile);

    return 0;
    
}

