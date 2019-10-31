#include<stdio.h>
#include "qrline.h"

int main( int argc, char * argv[] )
{
	/*
	if( argc > 0 )
	{
		char * temp; 
		puts( temp = qrline_gen( argv[1] ) );
		qrline_free(temp);
	}*/
	
	if( argc > 0 )
	{
		char * temp; 
		for(int i=21; i<41; i+= 2){
			for(int j=0; j<8; ++j){
				
				printf("\nCode params %d %d\n",j,i);
				qrline_bit * bits = qrline_gen_bitp( 0, j, i, argv[1] );

				temp = qrline_convert_bitp( bits, i );
				puts( temp );
				
				qrline_free(temp);
			}
		}
	}
	
	return 0;
}
