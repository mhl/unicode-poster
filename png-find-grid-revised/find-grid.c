/*
  
*/

#include <stdio.h>
#include <stdlib.h>

#include "png.h"        /* libpng header; includes zlib.h */
#include "readpng.h"    /* typedefs, common macros, public prototypes */

#define PROGNAME "find-grid"

int significant_horizontal_line( const unsigned char * image_data,
                                 int y,
                                 unsigned long image_width,
                                 unsigned long pitch,
                                 unsigned long minimum_width)
{
    int run_length = 0, x;
    
    for( x = 0; x < image_width; ++x ) {

        int non_white = (image_data[y*pitch+(x*3)] < 0xFE);

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
        
        int non_white = (image_data[y*pitch+(x*3)] < 0xFE);

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

    int last_in_each_cell_x[256];
    int first_in_each_cell_x[256];
    int last_in_each_cell_y[256];
    int first_in_each_cell_y[256];

    int i_last_x;
    int i_first_x;
    int i_last_y;
    int i_first_y;

    int cell_width, cell_height;

    int crossing_grid_boundary;

    int x, y;    

    char * filename, * minimum_box_width_s, * minimum_box_height_s;
    unsigned long minimum_box_width, minimum_box_height;
    FILE * infile;
    int rc;
    unsigned long image_width, image_height, image_rowbytes, image_depth;
    int image_channels;
    unsigned char *image_data;

    if( argc != 4 ) {
        printf("Usage: find-grid <png-file-name> <minimum-box-width> <minimum-box-height>\n");
        return -1;
    }
    
    filename = argv[1];
    minimum_box_width_s = argv[2];
    minimum_box_height_s = argv[3];

    // atoi is bad, of course...

    minimum_box_width = (unsigned long) atol(argv[2]);
    minimum_box_height = (unsigned long) atol(argv[3]);

    // printf("%lu x %lu\n",minimum_box_width,minimum_box_height);

    if( minimum_box_width <= 0 ) {
        printf( "minimum_box_width must be an integer and greater than or equal to zero.\n");
        return -1;
    }

    if( minimum_box_height <= 0 ) {
        printf( "minimum_box_height must be an integer and greater than or equal to zero.\n");
        return -1;
    }
               
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

    i_last_x = 0;
    i_first_x = 0;

    crossing_grid_boundary = 0;

    for( x = 0; x < image_width; ++ x ) {
        
        int significant_line = significant_vertical_line( image_data,
                                                          x,
                                                          image_height,
                                                          image_rowbytes,
                                                          minimum_box_height );

        /*
        if( significant_line )
            printf("Significant vertical line at x = %d\n",x);
        */

        if( crossing_grid_boundary && ! significant_line ) {
            first_in_each_cell_x[i_first_x++] = x;
            crossing_grid_boundary = 0;
        } else if( (! crossing_grid_boundary) && significant_line ) {
            last_in_each_cell_x[i_last_x++] = x - 1;
            crossing_grid_boundary = 1;
        }

    }

    cell_width = i_last_x - 1;
   
    crossing_grid_boundary = 0;

    i_last_y = 0;
    i_first_y = 0;

    printf( "image height is: %lu\n", image_height );

    for( y = 0; y < image_height; ++y ) {
        
        int significant_line = significant_horizontal_line( image_data,
                                                            y,
                                                            image_width,
                                                            image_rowbytes,
                                                            minimum_box_width );

        /*
        if( significant_line )
            printf("Significant horizontal line at y = %d\n",y);
        */
        
        if( crossing_grid_boundary && ! significant_line ) {
            first_in_each_cell_y[i_first_y++] = y;
            crossing_grid_boundary = 0;
        } else if( (! crossing_grid_boundary) && significant_line ) {
            last_in_each_cell_y[i_last_y++] = y - 1;
            crossing_grid_boundary = 1;
        }

    }

    cell_height = i_last_y - 1;

    /*
    printf( "Cell width: %d\n", cell_width );
    printf( "Cell height: %d\n", cell_height );
    */
    
    readpng_cleanup(TRUE);
    fclose(infile);

    printf( " last_in_each_cell_x:" );
    for( i_last_x = 1; i_last_x <= cell_width; ++ i_last_x )
        printf( " %6d", last_in_each_cell_x[i_last_x] );
    printf( "\n" );
        
    printf( " last_in_each_cell_y:" );
    for( i_last_y = 1; i_last_y <= cell_height; ++ i_last_y )
        printf( " %6d", last_in_each_cell_y[i_last_y] );
    printf( "\n" );
        
    printf( "first_in_each_cell_x:" );
    for( i_first_x = 0; i_first_x < cell_width; ++ i_first_x )
        printf( " %6d", first_in_each_cell_x[i_first_x] );
    printf( "\n" );
        
    printf( "first_in_each_cell_y:" );
    for( i_first_y = 0; i_first_y < cell_height; ++ i_first_y )
        printf( " %6d", first_in_each_cell_y[i_first_y] );
    printf( "\n" );

    return 0;
    
}

