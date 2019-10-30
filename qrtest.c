#include<stdio.h>
#include "qrline.h"

int main( int argc, char * argv[] )
{
	if( argc > 0 )
	{
		char * temp; 
		puts( temp = qrline_gen( argv[1] ) );
		qrline_free(temp);
	}
	
	return 0;
}
