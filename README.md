# qrline
A single, C header file, cabable of generating text based QR-Codes in terminal/ssh/cmd.exe

### A Simple Example
```
#include<stdio.h>
#include "qrline.h"

void main( int argc, char * argv[] )
{
	if( argc > 0 )
	{
		printf( qrline_gen( argv[1] ) );
	}
}
```

- Please note that this is still in beta. Currently, it can produce codes which a reader can identify as a QR Code, but it lacks the error correction bits needed for proper decoding.
