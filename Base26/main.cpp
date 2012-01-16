#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc,char **argv)
{
	// test, if there are too few cmd parameters:
	if(argc < 4)
	{
		printf("Too few arguments\nExample: base26.exe -enc|-dec sourcefile destinationfile");
		exit(1);
	}

	// declare variables:
	FILE * pFile;
	long lSize;
	unsigned char * buffer;
	size_t result;
	int nullCounter = 0;

	// opening the file:
	pFile = fopen ( argv[2] , "rb");
	if (pFile==NULL)
	{
		fputs ("File error",stderr);
		exit (2);
	}

	// obtain file size:
	fseek (pFile , 0 , SEEK_END);
	lSize = ftell (pFile);
	rewind (pFile);

	// allocate memory to contain the whole file:
	buffer = (unsigned char*) malloc (sizeof(unsigned char)*lSize);
	if (buffer == NULL)
	{
		fputs ("Memory error",stderr);
		exit (3);
	}

	// copy the file into the buffer:
	result = fread (buffer,1,lSize,pFile);
	if (result != lSize)
	{
		fputs ("Reading error",stderr);
		exit (4);
	}

	// close sourcefile:
	fclose (pFile);
	pFile = NULL;

	/* the whole file is now loaded in the memory buffer. */

	/* now parsing parameters and choose the path to go. */

	// path for encoding:
	if(!strcmp("-enc",argv[1]))
	{

		// opening the new file:
		pFile = fopen(argv[3], "wb");
		if (pFile==NULL)
		{
			fputs ("File error",stderr);
			exit (2);
		}

		// big encoding algorithm follows:
		for(long i = 0; i < lSize; i++)
		{
			 nullCounter = buffer[i] / 26;
			 if(nullCounter > 0)
			 {
				 for(;nullCounter > 0; nullCounter--)
				 {
					 fputc('0',pFile);
				 }
			 }
			 fputc((buffer[i] % 26)+65,pFile);
		}

	//closing encoding:
	}

	// path for decoding:
	else if(!strcmp("-dec",argv[1]))
	{

		// opening the new file:
		pFile = fopen(argv[3], "wb");
		if (pFile==NULL)
		{
			fputs ("File error",stderr);
			exit (2);
		}

		// big decoding algorithm follows:
		for(long i = 0; i < lSize; i++)
		{
			 if(buffer[i] == '0')
			 {
				 nullCounter+=26;
			 }
			 else
			 {
				 nullCounter+=buffer[i]-65;
				 fputc(nullCounter,pFile);
				 nullCounter = 0;
			 }
		}

	//closing decoding:
	}

	// closing the new file:
	fclose(pFile);
		
	// free memory buffer:
	free (buffer);

	return 0;
}