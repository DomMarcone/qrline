#pragma once
#ifndef _QRLINE
#define _QRLINE
/***************************************************
**                                                **
**                     QRLINE                     **
** One header for generating text-based qr codes. **
**                                                **
** by Dominic Marcone                             **
****************************************************/

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>

#define QRLINE_MIN_SIZE 21
#define QRLINE_MAX_SIZE 177

#define QRLINE_DEFAULT_PAD 3

//mask define for overlaying timings and positional data
#define QRLINE_MASK 2

typedef int8_t qrline_bit;

//global variables
int qrline_pad = QRLINE_DEFAULT_PAD;
int qrline_left = 10;

//prototypes
char *       qrline_gen( char * );
void         qrline_free( char * );
int          qrline_calculate_size( char * );
char *       qrline_convert_bitp( qrline_bit *, int );
char *       qrline_block_to_char_ansi( qrline_bit * );
char *       qrline_block_to_char_unicode( qrline_bit * );
qrline_bit * qrline_generate_timing( int );
qrline_bit * qrline_overlay_format( qrline_bit * timing, int size, int pattern_format, int error_format );
void         qrline_debug_print( qrline_bit *, int );
void         qrline_xor_pattern( qrline_bit * data, qrline_bit * pattern, int size );
qrline_bit * qrline_generate_pattern( int type, int size );
qrline_bit * qrline_merge( qrline_bit * data, qrline_bit * timing, int size );
int          qrline_get_next_index( int current_index, int size );
int *        qrline_solve_block( qrline_bit * timing, int start_index, int size );
int *        qrline_generate_bit_index( qrline_bit * timing, int size );
qrline_bit * qrline_bch(int total_codewords, int data_codewords, qrline_bit *data);

int          qrline_char_to_int( char c );
qrline_bit * qrline_str_to_bits( int cci, int * size, char * s );
qrline_bit * qrline_arrange_bits( int bit_size, qrline_bit * bits );

qrline_bit * qrline_gen_bitp( int error_type, int pattern_type, int size, char * input );


char * qrline_gen( char * input )
{
	int size = qrline_calculate_size( input );
	
	qrline_bit * bits = qrline_gen_bitp( 0, 5, size, input );

	return qrline_convert_bitp( bits, size );
}


qrline_bit * qrline_gen_bitp( int error_type, int pattern_type, int size, char * input )
{
	int code_len;
	qrline_bit * code_bits;

	qrline_bit * timing = qrline_generate_timing( size );


	qrline_bit * pattern = qrline_generate_pattern( pattern_type, size );
	
	//memset(timing,0,size*size);
	//memset(pattern,0,size*size);
	
	//qrline_debug_print( qrline_generate_pattern( 6, 25), 25 );
	qrline_overlay_format( timing, size, pattern_type, error_type );

	int * bit_index = qrline_generate_bit_index( timing, size );
	//qrline_debug_print( timing, size );

	qrline_merge( pattern, timing, size );

	code_bits = qrline_str_to_bits( 9, &code_len, input );

	code_bits = qrline_arrange_bits( code_len, code_bits );
	
	for( int i = 0; i < code_len; ++i )
	{
		if( bit_index[ i ] != 0 )
		{
			pattern[ bit_index[i] ] = code_bits[ i ] == pattern[ bit_index[ i ] ] ? 0 : 1;
		}
	}

	free( bit_index );
	free( timing );

	//qrline_debug_print( pattern, size );

	return pattern;
}


/*
	Calculate the overall size needed for a code
*/

int qrline_calculate_size( char * input )
{
	//return dummy data for now
	return 25;
}

void qrline_helper_append(char **dest, int *dest_index, const char *src){
	int src_len = strlen(src);
	*dest = (char*)realloc(*dest, ((*dest_index) + src_len+1)*sizeof(char));
	strcpy((*dest+ (*dest_index)), src);
	*dest_index += src_len;
}

char * qrline_convert_bitp( qrline_bit * ar, int size )
{
	char * output;
	char * convert_result;
	int convert_size;

	int out_size = size / 2;

	int line = out_size + 1;

	qrline_bit BLANK[] = {0,0};

	qrline_bit temp[4];//used for passing data to qrline_block_to_char()

	char* (*convert_function )( qrline_bit * );

	int index = 0;// output index;

	//pad the top...
	output = ( char * ) malloc( sizeof(char) );

	#ifdef _WIN32
		convert_function = qrline_block_to_char_ansi;
	#else
		convert_function = qrline_block_to_char_unicode;

		//set color in unix
		qrline_helper_append(&output,&index,"\033[1;37m");

	#endif

	//add spaces to account for left margin
	for( int i = 0; i < qrline_left; ++i )
	{
		qrline_helper_append(&output, &index, " ");
	}

	BLANK[0]=1;//Very top line...
	convert_result = convert_function(BLANK);
	for( int i = 0; i < size + (2*qrline_pad); ++i )
	{
		qrline_helper_append(&output, &index, convert_result);
	}
	qrline_helper_append(&output, &index, "\n");

	BLANK[0]=0;//switch back


	for( int j = 0; j < qrline_pad/2; ++j ){
		//add spaces to account for left margin
		convert_result = " ";
		for( int i = 0; i < qrline_left; ++i )
		{
			qrline_helper_append(&output, &index, convert_result);
		}

		//quiet space on the top
		convert_result = convert_function(BLANK);
		for( int i = 0; i < size + (2*qrline_pad); ++i )
		{
			qrline_helper_append(&output, &index, convert_result);
		}

		//end of line terminator
		qrline_helper_append(&output, &index, "\n");
	}

	for( int j = 0; j <= out_size; ++j )
	{
		//add spaces to account for left margin
		convert_result = " ";
		for( int i = 0; i < qrline_left; ++i )
		{
			qrline_helper_append(&output, &index, convert_result);
		}

		//pad line begining with blank characters
		convert_result = convert_function(BLANK);
		for( int i = 0; i < qrline_pad; ++i )
		{
			qrline_helper_append(&output, &index, convert_result);
		}

		for( int i = 0; i < size; ++i )
		{
			int y = 2 * j;
			temp[0] = ar[ ( i ) + ( y     ) * size ];

			if( j < out_size )
			{
				temp[1] = ar[ ( i ) + ( y + 1 ) * size ];
			} else
			{
				temp[1] = 0;
			}

			convert_result = convert_function(temp);
			qrline_helper_append(&output, &index, convert_result);
		}

		//quiet space on the right side
		convert_result = convert_function(BLANK);
		for( int i = 0; i < qrline_pad; ++i )
		{
			qrline_helper_append(&output, &index, convert_result);
		}

		qrline_helper_append(&output, &index, "\n");
	}

	//bottom line
	for(int j=0; j < qrline_pad/2; ++j){
		//add spaces to account for left margin
		for( int i = 0; i < qrline_left; ++i )
		{
			qrline_helper_append(&output, &index, " ");
		}

		convert_result = convert_function(BLANK);
		for( int i = 0; i < size + (2*qrline_pad); ++i )
		{
			qrline_helper_append(&output, &index, convert_result);
		}
		qrline_helper_append(&output, &index, "\n");
	}


	BLANK[1]=1;//switch back
	//add spaces to account for left margin
	for( int i = 0; i < qrline_left; ++i )
	{
		qrline_helper_append(&output, &index, " ");
	}

	convert_result = convert_function(BLANK);
	for( int i = 0; i < size + (2*qrline_pad); ++i )
	{
		qrline_helper_append(&output, &index, convert_result);
	}

	qrline_helper_append(&output, &index, "\n");

	#ifndef _WIN32

		//revert color in unix
		convert_result = "\033[0m";;
		for( int i = 0; i < qrline_left; ++i )
		{
			qrline_helper_append(&output, &index, convert_result);
		}

	#endif

	output[ index ] = 0;

	return output;
}



/*
	Convert a 2x2 block to a character
*/
char*  qrline_block_to_char_unicode( qrline_bit * in_ar )
{
	/*
		Blocks are read:
		12
		34
	*/
	qrline_bit ar[2];
	//invert, or not
	ar[0] = in_ar[0]==0 ? 1 : 0;
	ar[1] = in_ar[1]==0 ? 1 : 0;

	if( ar[0]==0 )
	{
		if( ar[1]==0 )
		{
			//00
			return " ";//a space will suffice
		}else
		{
			//01
			return "\u2584";
		}
	} else
	{
		if( ar[1]==0 )
		{
			//10
			return "\u2580";
		}else
		{
			//1111
			return "\u2588";//the second bit makes the background white
		}
	}

	return 0;
}

/*
	Convert a 2x2 block to a character
*/
char * qrline_block_to_char_ansi( qrline_bit * ar_in )
{
	/*
		Blocks are read:
		0
		1
	*/
	qrline_bit ar[2];

	//invert, or not
	ar[0] = ar_in[0]==0 ? 1 : 0;
	ar[1] = ar_in[1]==0 ? 1 : 0;

	if( ar[0]==0 )
	{
		if( ar[1]==0 )
		{
			//00
			return " ";
		}else
		{
			//01
			return "\xDC";
		}
	} else
	{
		if( ar[1]==0 )
		{
			//10
			return "\xDF";
		}else
		{
			//11
			return "\xDB";
		}
	}

	return 0;
}


/*
	generate a positional template
*/
qrline_bit * qrline_generate_timing( int size )
{
	qrline_bit * output;

	output = ( qrline_bit * ) malloc( size * size * sizeof( qrline_bit ) );

	for( int i = 0; i < size * size; ++i )output[i] = QRLINE_MASK;

	//timing pattern
	for( int i = 0; i < size; ++i )
	{
		int k = i + 1;

		//horizontal timing pattern
		output[ i + 6 * size ] = k%2;

		//vertical timing pattern
		output[ 6 + i * size ] = k%2;
	}

	//upper left box
	{
		for( int j = 0; j <= 7; ++j )
		{
			for( int i = 0; i <= 7; ++i )
			{
				output[ i + j * size ] = 0;
			}
		}
		for( int j = 0; j <= 6; ++j )
		{
			for( int i = 0; i <= 6; ++i )
			{
				output[ i + j * size ] = 1;
			}
		}
		for( int j = 1; j <= 5; ++j )
		{
			for( int i = 1; i <= 5; ++i )
			{
				output[ i + j * size ] = 0;
			}
		}
		for( int j = 2; j <= 4; ++j )
		{
			for( int i = 2; i <= 4; ++i )
			{
				output[ i + j * size ] = 1;
			}
		}
	}

	//upper right box
	{
		int off = size - 8;
		for( int j = 0; j <= 7; ++j )
		{
			for( int i = 0 + off; i <= 7 + off; ++i )
			{
				output[ i + j * size ] = 0;
			}
		}
		off = size - 7;
		for( int j = 0; j <= 6; ++j )
		{
			for( int i = 0 + off; i <= 6 + off; ++i )
			{
				output[ i + j * size ] = 1;
			}
		}
		for( int j = 1; j <= 5; ++j )
		{
			for( int i = 1 + off; i <= 5 + off; ++i )
			{
				output[ i + j * size ] = 0;
			}
		}
		for( int j = 2; j <= 4; ++j )
		{
			for( int i = 2 + off; i <= 4 + off; ++i )
			{
				output[ i + j * size ] = 1;
			}
		}
	}

	//bottom left box
	{
		int off = size - 8;
		for( int j = 0 + off; j <= 7 + off; ++j )
		{
			for( int i = 0; i <= 7; ++i )
			{
				output[ i + j * size ] = 0;
			}
		}
		off = size - 7;
		for( int j = 0 + off; j <= 6 + off; ++j )
		{
			for( int i = 0; i <= 6; ++i )
			{
				output[ i + j * size ] = 1;
			}
		}
		for( int j = 1 + off; j <= 5 + off; ++j )
		{
			for( int i = 1; i <= 5; ++i )
			{
				output[ i + j * size ] = 0;
			}
		}
		for( int j = 2 + off; j <= 4 + off; ++j )
		{
			for( int i = 2; i <= 4; ++i )
			{
				output[ i + j * size ] = 1;
			}
		}
	}

	//bottom right
	if( size >= 25 ){
		int off = size - 9;
		for( int j = 0 + off; j <= 4 + off; ++j )
		{
			for( int i = 0 + off; i <= 4 + off; ++i )
			{
				output[ i + j * size ] = 1;
			}
		}
		for( int j = 1 + off; j <= 3 + off; ++j )
		{
			for( int i = 1 + off; i <= 3 + off; ++i )
			{
				output[ i + j * size ] = 0;
			}
		}
		output[ ( off + 2 ) + ( ( off + 2 ) * size ) ] = 1;
	}

	return output;
}

qrline_bit * qrline_overlay_format( qrline_bit * timing, int size, int pattern_format, int error_format )
{
	qrline_bit format[15];
	
	memset(format,0,sizeof(format));
	
	//generate format
	format[0] = (error_format>>1)%2;
	format[1] = (error_format>>0)%2;
	format[2] = (pattern_format>>2)%2;
	format[3] = (pattern_format>>1)%2;
	format[4] = (pattern_format)%2;

	qrline_bit mask_pattern[] = { 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0 };
	
	printf("Current format data (before bch) : ");
	for( int i=0; i<15; ++i )printf(" %d", format[i]);
	printf("\n");
	
	qrline_bit * bch = qrline_bch( 15, 5, format );
	
	for( int i = 0; i < 10; ++i ) format[ i + 5 ] = bch[i]%2;
	
	printf("Current format data : ");
	for( int i=0; i<15; ++i )printf(" %d", format[i]);
	printf("\n");

	//mask pattern
	for( int i = 0; i < 15; ++i )
	{
		format[ i ] = mask_pattern[ i ] ? !format[ i ] : format[ i ];
	}
	
	printf("Current format data after masking : ");
	for( int i=0; i<15; ++i )printf(" %d", format[i]);
	printf("\n");
	
	free(bch);


	//do a hard-coded fill of the upper right corner
	timing[ 8            ] = format[ 0 ];
	timing[ 8 +     size ] = format[ 1 ];
	timing[ 8 + 2 * size ] = format[ 2 ];
	timing[ 8 + 3 * size ] = format[ 3 ];
	timing[ 8 + 4 * size ] = format[ 4 ];
	timing[ 8 + 5 * size ] = format[ 5 ];
	timing[ 8 + 7 * size ] = format[ 6 ];
	timing[ 8 + 8 * size ] = format[ 7 ];
	timing[ 7 + 8 * size ] = format[ 8 ];
	timing[ 5 + 8 * size ] = format[ 9 ];
	timing[ 4 + 8 * size ] = format[ 10 ];
	timing[ 3 + 8 * size ] = format[ 11 ];
	timing[ 2 + 8 * size ] = format[ 12 ];
	timing[ 1 + 8 * size ] = format[ 13 ];
	timing[     8 * size ] = format[ 14 ];

	timing[ ( size - 1 ) + 8 * size ] = format[ 0 ];
	timing[ ( size - 2 ) + 8 * size ] = format[ 1 ];
	timing[ ( size - 3 ) + 8 * size ] = format[ 2 ];
	timing[ ( size - 4 ) + 8 * size ] = format[ 3 ];
	timing[ ( size - 5 ) + 8 * size ] = format[ 4 ];
	timing[ ( size - 6 ) + 8 * size ] = format[ 5 ];
	timing[ ( size - 7 ) + 8 * size ] = format[ 6 ];
	timing[ ( size - 8 ) + 8 * size ] = format[ 7 ];

	timing[ 8 + ( size - 8 ) * size ] = 1;//blank
	timing[ 8 + ( size - 7 ) * size ] = format[ 8 ];
	timing[ 8 + ( size - 6 ) * size ] = format[ 9 ];
	timing[ 8 + ( size - 5 ) * size ] = format[ 10 ];
	timing[ 8 + ( size - 4 ) * size ] = format[ 11 ];
	timing[ 8 + ( size - 3 ) * size ] = format[ 12 ];
	timing[ 8 + ( size - 2 ) * size ] = format[ 13 ];
	timing[ 8 + ( size - 1 ) * size ] = format[ 14 ];


	int off = size - 8;

	return timing;
}

//generate patterns for xoring.
qrline_bit qrline_noise_0(int i, int j){ return ( i + j )%2 == 0 ? 1 : 0; }
qrline_bit qrline_noise_1(int i, int j){ return i%2 == 0 ? 1 : 0; }
qrline_bit qrline_noise_2(int i, int j){ return j%3 == 0 ? 1 : 0; }
qrline_bit qrline_noise_3(int i, int j){ return ( i + j )%3 == 0 ? 1 : 0; }
qrline_bit qrline_noise_4(int i, int j){ return ( (i/2) + (j/3) )%2 == 0 ? 0 : 1; }
qrline_bit qrline_noise_5(int i, int j){ return ( i * j )%2 + ( i * j )%3 == 0 ? 1 : 0; }
qrline_bit qrline_noise_6(int i, int j){ return ( ( i * j )%2 + ( i * j )%2 )%3 == 0 ? 1 : 0; }
qrline_bit qrline_noise_7(int i, int j){ return ( ( i * j )%3 + ( i + j )%2 )%2 == 0 ? 1 : 0; }

qrline_bit * qrline_generate_pattern( int type, int size )
{
	qrline_bit * pattern;

	qrline_bit (*pattern_func)(int i, int j) = qrline_noise_0;

	if( type > 7 || type < 0 )type = 0;

	pattern = ( qrline_bit * ) malloc( size * ( size + 1 ) * sizeof( qrline_bit ) );

	switch( type )
	{
	case 0 :
		pattern_func = qrline_noise_0;
		break;
	case 1 :
		pattern_func = qrline_noise_1;
		break;
	case 2 :
		pattern_func = qrline_noise_2;
		break;
	case 3 :
		pattern_func = qrline_noise_3;
		break;
	case 4 :
		pattern_func = qrline_noise_4;
		break;
	case 5 :
		pattern_func = qrline_noise_5;
		break;
	case 6 :
		pattern_func = qrline_noise_6;
		break;
	case 7 :
		pattern_func = qrline_noise_7;
		break;
	}

	for( int i = 0; i < size; ++i )
	{
		for( int j = 0; j < size; ++j )
		{
			pattern[ i + j * size ] = pattern_func(i,j);
		}
	}

	return pattern;
}


//merge
qrline_bit * qrline_merge( qrline_bit * data, qrline_bit * timing, int size )
{
	for( int i = 0; i < size * size; ++i )
	{
		if( timing[i] != QRLINE_MASK )data[i] = timing[i];
	}

	return data;
}


//overlay pattern
void qrline_xor_pattern( qrline_bit * data, qrline_bit * pattern, int size )
{
	for( int i = 0; i < size * size; ++i )
	{
		if( data[i] == pattern[i] ) data[i] = 0;
		else data[i] = 1;
	}
}


//return 0 if there's not a next open spot
int qrline_get_next_index( int current_index, int size )
{
	int row = current_index%size;//( (size - 1) - ( current_index%size ) )%4;

	row += 1;
	row = size - row;
	row %= 4;

	/*
		Returns index regardless of whats in the cell
	*/
	//printf( "row: %d\n", row );
	//move exceptions
	if( current_index < 0 || current_index >= size * size )
	{
		//printf("This shouldn't happen... %d\n", current_index );
		//index is either below 0, or beyond the array.
		return 0;
	}

	if( row == 1 && current_index < size )
	{
		//at the top, so go left
		if( current_index%size <= 1 )
		{
			//printf( "can't go left, returning 0 from index %d\n", current_index );
			return 0;
		}
		//printf("at the top, going left. %d\n", current_index );
		return current_index - 1;
	}

	if( row == 3 && current_index/size == ( size - 1 ) )
	{
		//at the bottom, so go left
		if( current_index%size <= 1 )
		{
			//printf( "can't go left, returning 0 from index %d\n", current_index );
			return 0;
		}
		//printf("at the bottom, going left. %d\n", current_index );
		return current_index - 1;
	}


	//normal moves
	if( row == 0 ) return current_index - 1;//go left

	if( row == 1 )
	{
		int temp = ( current_index - size ) + 1;
		if( temp < 0 ) return 0;
		return temp;// go up &right
	}

	if( row == 2 ) return current_index - 1;//go left

	if( row == 3 ) return ( current_index + size ) + 1;//go down and right

	return 0;
}


//return 0, for no more blocks.
//put the begining of the next block at index 8
int * qrline_solve_block( qrline_bit * timing, int start_index, int size )
{

	static int ind[9];

	ind[0] = start_index;

	for( int i = 1; i <= 8; ++i )
	{
		ind[ i ] = qrline_get_next_index( ind[ i - 1 ], size );

		while( timing[ ind[ i ] ] != QRLINE_MASK && ind[ i ] != 0 )
		{
			ind[ i ] = qrline_get_next_index( ind[ i ], size );
		}

		if( i != 8 && ind[ i ] <= 0 )
		{
			return 0;
		}

	}

	return ind;
}


//index list for bit assignment
int * qrline_generate_bit_index( qrline_bit * timing, int size )
{
	int * index;

	int count = 0;

	index = ( int * ) malloc( size * size * sizeof( int ) );

	//Initialize array to 0, an impossible index
	for( int j = 0; j < size * size; ++j )
	{
		index[j] = 0;
	}

	int * block;
	int j = ( size * size ) - 1;// start index

	while( count < size * size - 9 )
	{
		block = qrline_solve_block( timing, j, size );

		if( block == 0 )break;

		for( int i = 0; i < 8; ++i )
		{
			index[ count++ ] = block[i];
		}
		j = block[ 8 ];

	}

	return index;
}

qrline_bit * qrline_bch(int total_codewords, int data_codewords, qrline_bit *data)
{
	int result_size = total_codewords - data_codewords;

  qrline_bit *result = (qrline_bit*)malloc(result_size * sizeof(qrline_bit));
  memset(result, 0, sizeof(qrline_bit)*result_size);

  //temp generator polynomial
  qrline_bit gen_poly[11] = {1,0,1,0,0,1,1,0,1,1,1};


  for(int i = data_codewords-1; i>=0; i--){
    int feedback = data[i] ^ result[result_size-1];
    if( feedback != 0 ){

      for(int j = result_size-1; j >= 0; j-- ){
        if( gen_poly[j] != 0) {
          result[j] = result[j - 1] ^ feedback;
        } else {
          result[j] = result[j - 1];
        }
      }

    } else {

      for(int j = result_size; j > 0; j-- ){
        result[j] = result[j - 1];
      }

      result[0]=0;
    }
  }

  return result;
}


int qrline_char_to_int( char c )
{
	//numeric
	if( c >= '0' && c <= '9' )return c - '0';

	//alpha
	if( c >= 'a' && c <= 'z' )return ( c - 'a' ) + 10;
	if( c >= 'A' && c <= 'Z' )return ( c - 'A' ) + 10;

	switch( c )
	{
	case ' ' :
		return 36;
		break; //redundant break for readability...

	case '$' :
		return 37;
		break;

	case '%' :
		return 38;
		break;

	case '*' :
		return 39;
		break;

	case '+' :
		return 40;
		break;

	case '-' :
		return 41;
		break;

	case '.' :
		return 42;
		break;

	case '/' :
		return 43;
		break;

	case ':' :
		return 44;
		break;
	}

	return 42; //'.' character...
}


//cci = Character Count Indentifier Length
qrline_bit * qrline_str_to_bits( int cci, int * size, char * s )
{
	int index = 0;

	qrline_bit * bits;

	int string_size = strlen( s );

	*size = string_size;

	if( *size%2 == 0 )
	{
		//even number of characters
		*size = ( *size/2 ) * 11;
	}

	if( *size%2 == 1 )
	{
		//odd number of characters
		*size = ( ( *size - 1 )/2 ) * 11;
		*size += 6;
	}

	*size += cci; //character count identifier
	*size += 4; //mode indicator

	//allocate bits
	bits = ( qrline_bit * ) malloc( *size * sizeof( qrline_bit ) );

	//fill bits
	{
		//ALPHA NUMERIC MODE
		bits[ index++ ] = 0;
		bits[ index++ ] = 0;
		bits[ index++ ] = 1;
		bits[ index++ ] = 0;

		//Character Count
		for( int i = cci - 1; i >= 0; --i )
		{
			bits[ index++ ] = (string_size>>i)&1;
		}

		//Fill bits
		for( int i = 0; i < string_size; i += 2 )
		{
			if( s[ i + 1 ] != 0 )
			{
				//two characters
				int temp = 45 * qrline_char_to_int( s[ i ] ) + qrline_char_to_int( s[ i + 1] );

				for( int j = 10; j >= 0; --j )
				{
					bits[ index++ ] = (temp>>j)&1;
				}

			}
			else
			{
				// one character
				int temp = qrline_char_to_int( s[ i ] );

				for( int j = 5; j >= 0; --j )
				{
					bits[ index++ ] = (temp>>j)&1;
				}
			}


		}

	}


	return bits;
}


qrline_bit * qrline_arrange_bits( int bit_size, qrline_bit * bits )
{
	qrline_bit * out = ( qrline_bit * ) malloc( bit_size * sizeof( qrline_bit ) );

	int k = 0;

	for( int i = 0; i < bit_size / 8; ++i )
	{

		for( int j = 7; j >= 0; --j )
		{

			if( j + ( i * 8 ) <= bit_size )
			{
				out[ k++ ] = bits[ j + ( i * 8 ) ];
			}

		}

	}

	free( bits );

	return out;
}


//print the values in the initial array.
void qrline_debug_print( qrline_bit * data, int size )
{

	printf( "RAW ARRAY:\n" );
	for( int j = 0; j < size; ++j )
	{
		for( int i = 0; i < size; ++i )
		{
			printf( "%c", ( int ) data[ i + j * size ] + '0' );
		}
		printf( "\n" );
	}
	printf( "\n" );
}


//free generated block characters
void qrline_free(char *a){
	free(a);
}

#ifdef __cplusplus
}
#endif //__cplusplus

//END HEADER
#endif
