/*
  
*/

#include <stdio.h>
#include <stdlib.h>

#include "png.h"        /* libpng header; includes zlib.h */
#include "readpng.h"    /* typedefs, common macros, public prototypes */

#define PROGNAME "crop-images"

typedef struct {

    char * output_filename;
    unsigned long x;
    unsigned long y;
    unsigned long width;
    unsigned long height;

} target;


int main( int argc, char **argv ) {

    target targets[512];
    int number_of_targets = 0;
    char * filename = NULL;
    char * crop_specification_filename = NULL;
    FILE * infile;
    char line_buffer[512];

    int rc;
    unsigned long image_width, image_height, image_rowbytes, image_depth;
    int image_channels;
    unsigned char *image_data;

    int i;

    if( argc != 3 ) {
        printf("Usage: crop-images <input-png-file-name> <crop-specification>\n");
        return -1;
    }
    
    filename = argv[1];
    crop_specification_filename = argv[2];

    FILE * specification_file = fopen(crop_specification_filename,"r");
    if( ! specification_file ) {
        fprintf(stderr,"Failed to open %s.\n",crop_specification_filename);
        return -1;
    }
    
    while( fgets(line_buffer,511,specification_file) ) {

        targets[number_of_targets].output_filename = malloc(512);
        if( ! targets[number_of_targets].output_filename ) {
            fprintf(stderr,"Failed to allocate 512 bytes.\n");
            return -1;
        }

        int result = sscanf( line_buffer,
                             "%s %lu %lu %lu %lu",
                             targets[number_of_targets].output_filename,
                             &(targets[number_of_targets].x),
                             &(targets[number_of_targets].y),
                             &(targets[number_of_targets].width),
                             &(targets[number_of_targets].height) );

        if( result != 5 ) {
            fprintf( stderr, "Standard input wasn't in the right input.\n" );
        }

        /*
        printf( "%d:\n", number_of_targets );
        printf( "   filename: %s\n", targets[number_of_targets].output_filename );
        printf( "   x: %lu\n", targets[number_of_targets].x );
        printf( "   y: %lu\n", targets[number_of_targets].y );
        printf( "   width: %lu\n", targets[number_of_targets].width );
        printf( "   height: %lu\n", targets[number_of_targets].height );
        */

        ++ number_of_targets;
    }
    if(fclose(specification_file)) {
        fprintf(stderr, PROGNAME ": failed to close specification file [%s]\n", crop_specification_filename);
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
    
    // printf("  %s is %lu by %lu\n", filename, image_width, image_height );
    // printf("  %s has depth: %lu\n", filename, image_depth );
    
    image_data = readpng_get_image(1.0, &image_channels, &image_rowbytes);

    // printf("  %s has row_bytes: %lu\n", filename, image_rowbytes );

    for( i = 0; i < number_of_targets; ++i ) {

        png_bytepp rows = NULL;
        png_infop info_ptr = NULL;
        unsigned long width = targets[i].width;
        unsigned long height = targets[i].height;
        unsigned long x = targets[i].x;
        unsigned long y = targets[i].y;
        unsigned int pitch;
        char * output_filename = targets[i].output_filename;
        png_structp png_ptr;
        unsigned int j, k;

        FILE * fp = fopen( output_filename, "wb" );
        if( ! fp ) {
            fprintf(stderr, PROGNAME ":  can't open PNG file [%s] for output\n",
                    output_filename);
            return -1;
        }

        rows = malloc( height * sizeof(png_bytep) );
        
        png_ptr = png_create_write_struct( PNG_LIBPNG_VER_STRING, 0, 0, 0 );
        if( ! png_ptr ) {
            fprintf(stderr, PROGNAME "Failed to allocate a png_structp\n" );
            free(rows);
            fclose(fp);
            return -1;
        }
        
        info_ptr = png_create_info_struct( png_ptr );
        if( ! info_ptr ) {
            fprintf(stderr, PROGNAME "Failed to allocate a png_infop" );
            png_destroy_write_struct( &png_ptr, 0 );
            free(rows);
            fclose( fp );
            return -1;
        }

        if( setjmp( png_ptr->jmpbuf ) ) {
            fprintf(stderr, PROGNAME "Failed to setjmp\n" );
            png_destroy_write_struct( &png_ptr, 0 );
            free(rows);
            fclose(fp);
            return -1;            
        }
        
        png_init_io( png_ptr, fp );

        png_set_compression_level( png_ptr, 3 );

        png_set_IHDR( png_ptr,
                      info_ptr,
                      width,
                      height,
                      8,
                      PNG_COLOR_TYPE_GRAY,
                      PNG_INTERLACE_NONE,
                      PNG_COMPRESSION_TYPE_DEFAULT, // The only option...
                      PNG_FILTER_TYPE_DEFAULT );

        png_write_info(png_ptr, info_ptr);

        if( setjmp( png_ptr->jmpbuf ) ) {
            fprintf(stderr, PROGNAME "Failed to setjmp\n" );
            png_destroy_write_struct( &png_ptr, 0 );
            free(rows);
            fclose(fp);
            return -1;            
        }
        
        pitch = png_get_rowbytes( png_ptr, info_ptr );
        
        // printf( "pitch is: %d\n",pitch );

        for( j = 0; j < height; ++j ) {
            unsigned char * row = malloc(pitch);
            if( ! row ) {
                int l;
                fprintf(stderr, PROGNAME "Failed to allocate a row\n" );
                png_destroy_write_struct( &png_ptr, 0 );
                for( l = 0; l < j; ++j ) {
                    free(rows[l]);
                }
                free(rows);
                fclose(fp);
                return -1;
            }
            for( k = 0; k < width; ++k ) {
                row[k] = image_data[(y + j)*image_rowbytes + (3 * (x + k))];
                rows[j] = row;
            }            
        }

        png_write_image( png_ptr, rows );

        png_write_end( png_ptr, 0 );

        png_destroy_write_struct( &png_ptr, &info_ptr );

        for( j = 0; j < height; ++j ) {
            free(rows[j]);
        }
        free( rows );
        
        free( targets[number_of_targets].output_filename );

    }

    return 0;
    
}

