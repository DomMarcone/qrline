#include<stdio.h>
#include "qrline.h"

int main( int argc, char * argv[] )
{
	if( argc > 0 )
	{
		puts( qrline_gen( argv[1] ) );
	}
	
	return 0;
}
