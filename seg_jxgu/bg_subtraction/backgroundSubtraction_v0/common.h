#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#ifndef COMMON_H
#define COMMON_H

#define		MAX_FEATURE		1000
#define		pi				3.14159265358979323846

/* This is just an inline that allocates images. I did this to reduce clutter in the
* actual computer vision algorithmic code. Basically it allocates the requested image
* unless that image is already non-NULL. It always leaves a non-NULL image as-is even
* if that image's size, depth, and/or channels are different than the request.
*/

inline static void allocateOnDemand( IplImage **img, CvSize size, int depth, int channels)
{
	if ( *img != NULL ) return;
	*img = cvCreateImage( size, depth, channels );
	if ( *img == NULL )
	{
		fprintf(stderr, "Error: Couldn't allocate image. Out of memory?\n");
		exit(-1);
	}
}
void strreverse(char* begin, char* end) {
	
	char aux;
	
	while(end>begin)
	
		aux=*end, *end--=*begin, *begin++=aux;
	
}

// Implementation of itoa()
void my_itoa(int value, char* str, int base) {
	
	static char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	
	char* wstr=str;
	
	int sign;
	
	div_t res;
		
	// Validate base
	if (base<2 || base>35){ *wstr='\0'; return; }
		
	// Take care of sign
	if ((sign=value) < 0) value = -value;
		
	// Conversion. Number is reversed.
	do {
		res = div(value,base);
		*wstr++ = num[res.rem];
	}while(value=res.quot);
	
	if(sign<0) *wstr++='-';
	*wstr='\0';
		
	// Reverse string
	strreverse(str,wstr-1);
}

char *generateFileName(int imageNum, char* ext, char* fileName, int digit, char *fileAbsoluteName, char *dir)
{
	char *num = (char*)malloc(sizeof(char)*digit);
	char *formattedNum = (char*)malloc(sizeof(char)*digit);
	for(int i=0;i<digit;i++){
		num[i]=0;
		formattedNum[i]=0;
	}

	my_itoa(imageNum, num, 10);
	int numZero = digit - strlen(num);
	for(int i=0;i<numZero;i++){
		formattedNum[i] = '0';
	}

	sprintf(fileAbsoluteName, "%s%s%s%s%s", dir, fileName, formattedNum, num, ext);
	if(digit>1){
		free(num);
		free(formattedNum);
	}
	return fileAbsoluteName;
}

#endif