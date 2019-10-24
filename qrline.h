#pragma once
#ifndef _QRLINE
#define _QRLINE
/***************************************************
**                                                **
**                     QRLINE                     **
** One header for generating text-based qr codes. **
**                                                **
** initial commit by Dominic Marcone              **
****************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<string.h>

#define QRLINE_MIN_SIZE 21
#define QRLINE_MAX_SIZE 177

//mask define for overlaying timings and positional data
#define QRLINE_MASK 2


typedef int8_t qrline_bit;


//prototypes
char *       qrline_gen( char * );
int          qrline_calculate_size( char * );
char *       qrline_convert_bitp( qrline_bit *, int );
char         qrline_block_to_char_ansi( qrline_bit * );
char         qrline_block_to_char_unicode( qrline_bit * );
qrline_bit * qrline_generate_timing( int );
qrline_bit * qrline_overlay_format( qrline_bit * timing, int size, int pattern_format, int error_format );
void         qrline_debug_print( qrline_bit *, int );
void         qrline_xor_pattern( qrline_bit * data, qrline_bit * pattern, int size );
qrline_bit * qrline_generate_pattern( int type, int size );
qrline_bit * qrline_merge( qrline_bit * data, qrline_bit * timing, int size );
int          qrline_get_next_index( int current_index, int size );
int *        qrline_solve_block( qrline_bit * timing, int start_index, int size );
int *        qrline_generate_bit_index( qrline_bit * timing, int size );
qrline_bit * qrline_bch( int result_size, int frame_size, int gen_size, qrline_bit * frame, qrline_bit * gen );

int          qrline_char_to_int( char c );
qrline_bit * qrline_str_to_bits( int cci, int * size, char * s );
qrline_bit * qrline_arrange_bits( int bit_size, qrline_bit * bits );

qrline_bit * qrline_gen_bitp( int error_type, int pattern_type, int * size, char * input );


char * qrline_gen( char * input )
{
	int size;
	
	qrline_bit * bits = qrline_gen_bitp( 0, 5, &size, input );
	
	return qrline_convert_bitp( bits, size );
}


qrline_bit * qrline_gen_bitp( int error_type, int pattern_type, int * size, char * input )
{
	*size = qrline_calculate_size( input );
	
	int code_len;
	qrline_bit * code_bits;
	
	qrline_bit * timing = qrline_generate_timing( *size );
	

	qrline_bit * pattern = qrline_generate_pattern( pattern_type, *size );
	
	//qrline_debug_print( qrline_generate_pattern( 6, 25), 25 );
	qrline_overlay_format( timing, *size, pattern_type, error_type );
	
	int * bit_index = qrline_generate_bit_index( timing, *size );
	//qrline_debug_print( timing, *size );
	
	qrline_merge( pattern, timing, *size );
	
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
	
	qrline_debug_print( pattern, *size );
	
	return pattern;	
}


/*
	Calculate the overall size needed for a code
*/

int qrline_calculate_size( char * input )
{
	//return dummy data for now
	return 21;
}


char * qrline_convert_bitp( qrline_bit * ar, int size )
{
	char * output;
	
	int out_size = size / 2;
	
	int line = out_size + 1;
	
	qrline_bit temp[4];//used for passing data to qrline_block_to_char()
	
	char (*convert_function )( qrline_bit * );
	
	int index = 0;// output index;


	output = ( char * ) malloc( ( out_size + 6 ) * ( size + 6 ) * sizeof( char ) );
	
	/*
	//select convert function based on the sizeof(char);
	if( sizeof(char) == 1 )
	{
		convert_function = qrline_block_to_char_ansi;
	} else if ( sizeof(char) == 2 )
	{
		convert_function = qrline_block_to_char_unicode;
	}
	*/
	
	convert_function = qrline_block_to_char_ansi;
	
	
	//quiet space on the top line
	for( int i = 0; i < size + 6; ++i )
	{
		output[ index++ ] = '\xDB';// more quiet zone
	}
	output[ index++ ] = '\n';// quiet zone
	
	for( int i = 0; i < size + 6; ++i )
	{
		output[ index++ ] = '\xDB';// more quiet zone
	}
	output[ index++ ] = '\n';// quiet zone
	
	
	//output[ index++ ] = '\xDB';// more quiet zone
	
	//loop through each block
	for( int j = 0; j <= out_size; ++j )
	{
		output[ index++ ] = '\xDB';//quiet zone
		output[ index++ ] = '\xDB';//quiet zone
		output[ index++ ] = '\xDB';//quiet zone
		
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
			
			
			output[ index++ ] = convert_function( temp );
			
			/*
			if( i == size - 1 )
			{
				output[ index++ ] = '\xDB';//quiet zone
				output[ index++ ] = '\n';
			}
			*/
		}
		
		output[ index++ ] = '\xDB';//quiet zone
		output[ index++ ] = '\xDB';//quiet zone
		output[ index++ ] = '\xDB';//quiet zone
		output[ index++ ] = '\n';
		
	}
	
	for( int i = 0; i < size + 6; ++i )
	{
		output[ index++ ] = '\xDB';// more quiet zone
	}
	output[ index++ ] = '\n';
	
	for( int i = 0; i < size + 6; ++i )
	{
		output[ index++ ] = '\xDF';// more quiet zone
	}
	output[ index++ ] = '\n';
	
	output[ index++ ] = 0;
	
	return output;
}



/*
	Convert a 2x2 block to a character
*/
char qrline_block_to_char_unicode( qrline_bit * ar )
{
	/*
		Blocks are read:
		12
		34
	*/
	if( ar[0]==0 )
	{
		if( ar[1]==0 )
		{
			//00
			return ' ';
		}else 
		{
			//01
			return (char) 0x2584;
		}
	} else 
	{
		if( ar[1]==0 )
		{
			//10
			return (char) 0x2598;
		}else 
		{
			//1111
			return (char) 0x2588;
		}
	}
	
	return 0;
}

/*
	Convert a 2x2 block to a character
*/
char qrline_block_to_char_ansi( qrline_bit * ar )
{
	/*
		Blocks are read:
		0
		1
	*/
	
	//invert
	ar[0] = ar[0]==0 ? 1 : 0;
	ar[1] = ar[1]==0 ? 1 : 0;
	
	if( ar[0]==0 )
	{
		if( ar[1]==0 )
		{
			//00
			return ' ';
		}else 
		{
			//01
			return (char) 0xDC;
		}
	} else 
	{
		if( ar[1]==0 )
		{
			//10
			return (char) 0xDF;
		}else 
		{
			//11
			return (char) 0xDB;
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
	
	qrline_bit generator[] = { 1, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1 };
	
	//pattern_format = pattern_format&7;
	//error_format = error_format&7;
	
	//generate format
	format[0] = error_format>>1&1;
	format[1] = error_format>>0&1;
	format[2] = pattern_format>>2&1;
	format[3] = pattern_format>>1&1;
	format[4] = pattern_format>>0&1;
	
	qrline_bit mask_pattern[] = { 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0 };
	
	for( int i = 0; i < 5; ++i )printf( "%d", (int) format[ i ] );
	//dummy values while i try to get this to work;
	
	qrline_bit * bch = qrline_bch( 10, 5, 11, format, generator );
	
	for( int i = 0; i < 10; ++i ) format[ i + 4 ] = bch[ i ];
	
	for( int i = 0; i < 15; ++i ) format[ i ] = format[ i ] == 0 ? 1 : 0;
	
	free( bch );
	
	for( int i = 0; i < 15; ++i )
	{
		format[ i ] = format[ i ] == mask_pattern[ i ] ? 0 : 1;
	}
	printf( "\n\n" );
	
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
qrline_bit * qrline_generate_pattern( int type, int size )
{
	qrline_bit * pattern;
	
	if( type > 7 || type < 0 )type = 0;
	
	pattern = ( qrline_bit * ) malloc( size * ( size + 1 ) * sizeof( qrline_bit ) );
	switch( type )
	{
		case 0 :
			// ( i + j )mod 2 = 0
			for( int i = 0; i < size; ++i )
			{
				for( int j = 0; j < size; ++j )
				{	
					pattern[ i + j * size ] = ( i + j )%2 == 0 ? 1 : 0;	
				}
			}
			break;
			
		case 1 :
			// ( i + j )mod 2 = 0 - edit please
			for( int i = 0; i < size; ++i )
			{
				for( int j = 0; j < size; ++j )
				{	
					pattern[ i + j * size ] = i%2 == 0 ? 1 : 0;	
				}
			}
			break;
			
		case 2 :
			// ( i + j )mod 2 = 0 - edit please
			for( int i = 0; i < size; ++i )
			{
				for( int j = 0; j < size; ++j )
				{	
					pattern[ i + j * size ] = j%3 == 0 ? 1 : 0;	
				}
			}
			break;
			
		case 3 :
			// ( i + j )mod 2 = 0 - edit please
			for( int i = 0; i < size; ++i )
			{
				for( int j = 0; j < size; ++j )
				{	
					pattern[ i + j * size ] = ( i + j )%3 == 0 ? 1 : 0;	
				}
			}
			break;
			
		case 4 :
			// ( i/2 + j/3 )mod 2 = 0 - edit please
			for( int i = 0; i < size; ++i )
			{
				for( int j = 0; j < size; ++j )
				{	
					pattern[ i + j * size ] = ( (i/2) + (j/3) )%2 == 0 ? 0 : 1;	
				}
			}
			break;

		case 5 :
			// ( i/2 + j/3 )mod 2 = 0 - edit please
			for( int i = 0; i < size; ++i )
			{
				for( int j = 0; j < size; ++j )
				{	
					pattern[ i + j * size ] = ( i * j )%2 + ( i * j )%3 == 0 ? 1 : 0;	
				}
			}
			break;
			
		case 6 :
			// ( i/2 + j/3 )mod 2 = 0 - edit please
			for( int i = 0; i < size; ++i )
			{
				for( int j = 0; j < size; ++j )
				{	
					pattern[ j + i * size ] = ( ( i * j )%2 + ( i * j )%2 )%3 == 0 ? 1 : 0;	
				}
			}
			break;
			
		case 7 :
			// ( i/2 + j/3 )mod 2 = 0 - edit please
			for( int i = 0; i < size; ++i )
			{
				for( int j = 0; j < size; ++j )
				{	
					pattern[ i + j * size ] = ( ( i * j )%3 + ( i + j )%2 )%2 == 0 ? 1 : 0;	
				}
			}
			break;
	
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

qrline_bit * qrline_bch( int result_size, int frame_size, int gen_size, qrline_bit * frame, qrline_bit * gen )
{
	qrline_bit * result;
	
	result = ( qrline_bit * ) malloc( result_size * sizeof( qrline_bit ) );
	
	qrline_bit * intermediate_results = ( qrline_bit * ) malloc( ( result_size + frame_size ) * sizeof( qrline_bit ) ); 
	
	//clear the working memory
	for( int i = 0; i < result_size + frame_size; ++i )
	{
		int temp = 0;
		if( i < frame_size ) temp = frame[ i ];// copy working memory into the first frame_size bits.
		intermediate_results[ i ] = temp;
	}
	
	for( int i = 0; i < ( result_size + frame_size ) - gen_size; ++i )
	{
		if( intermediate_results[i] != 0 )
		{
			
			for( int j = 0; j < gen_size; ++j )
			{
				
				intermediate_results[ i + j ] = intermediate_results[ i + j ] == gen[ j ] ? 0 : 1;
				
			}
			
		}
		
	}
	
	for( int i = 0; i < result_size; ++i )
	{
		result[ i ] = intermediate_results[ ( ( result_size + frame_size ) - gen_size ) + i + 1 ];
	}
	
	free( intermediate_results );
	
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

//END HEADER
#endif