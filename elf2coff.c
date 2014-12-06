#include <stdio.h>
#include <stdlib.h>

typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;
typedef unsigned __int64 uint64;

typedef char int8;
typedef short int16;
typedef long int32;
typedef __int64 int64;

#include "elf.h"
#include "coff.h"

int dump_obj_file(char *fname)
{
	if(fname){
		FILE *f;
		f=fopen(fname,"rb");
		if(f){
			char buf[8]={0};
			fread(buf,1,4,f);

			fclose(f);
		}

	}
}
int main(int argc,char **argv)
{
	enum {DUMP_FILE,CONVERT_FILE};
	int i,op;
	char *fname=0;

	for(i=1;i<argc;i++){
		if(strcmp(argv[i],"-d")==0){
			if(argc>=i+1){
				fname=argv[i+1];
				op=DUMP_FILE;
			}
		}
	}
	switch(op){
	case DUMP_FILE:
		dump_obj_file(fname);
		break;
	default:
		printf("[-d] <objfile>    dump object file");
		exit(0);
		break;
	}
	return 0;
}