/*
  
*/

#include <stdio.h>
#include <stdlib.h>

#include "png.h"        /* libpng header; includes zlib.h */
#include "readpng.h"    /* typedefs, common macros, public prototypes */

#define PROGNAME "empty-image"

int main( int argc, char **argv ) {

    int x, y;    

    char * filename;
    FILE * infile;
    int rc;
    unsigned long image_width, image_height, image_rowbytes, image_depth;
    int image_channels;
    unsigned char *image_data;

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
    
    /*
      printf("  %s is %lu by %lu\n", filename, image_width, image_height );
      printf("  %s has depth: %lu\n", filename, image_depth );
    */
    
    image_data = readpng_get_image(1.0, &image_channels, &image_rowbytes);
    
    /*
      printf("  %s has row_bytes: %lu\n", filename, image_rowbytes );
    */
    
    
    for( x = 0; x < image_width; ++x ) {
        for( y = 0; y < image_height; ++y ) {
            unsigned char r, g, b;
            r = image_data[y*image_rowbytes+(x*3)+0];
            g = image_data[y*image_rowbytes+(x*3)+1];
            b = image_data[y*image_rowbytes+(x*3)+2];
            if( (r<254)||(g<254)||(b<254) ) {
                /* printf("Got a non-white pixel (%d,%d,%d) at (%d,%d)\n",
                   r, g, b, x, y); */
                printf("%lu %lu\n",image_width,image_height);
                return -1;
            }
        }
    }
    readpng_cleanup(TRUE);
    fclose(infile);    

    return 0;
    
}

