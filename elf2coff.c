#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define TRUE 1
#define FALSE 0
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

const int ELF_HEADER_OFFSETS[20][2]={ //offset,size
{0,4},{4,1},{5,1},{6,1},{7,1},{8,4},{12,4},{16,2},{18,2},{20,4},{24,4},{28,4},{32,4},{36,4},{40,2},{42,2},{44,2},{46,2},{48,2},{50,2}
};
const char *ELF_HEADER_NAMES[20]={
"e_ident","e_bitness","e_endian","e_elfver","e_reserved0","e_reserved1","e_reserved2","e_type","e_machine","e_version","e_entry","e_phoff","e_shoff","e_flags","e_ehsize","e_phentsize","e_phnum","e_shentsize","e_shnum","e_shstrndx"
};
enum {e_ident=0,e_bitness,e_endian,e_elfver,e_reserved0,e_reserved1,e_reserved2,e_type,e_machine,e_version,e_entry,e_phoff,e_shoff,e_flags,e_ehsize,e_phentsize,e_phnum,e_shentsize,e_shnum,e_shstrndx};

const int ELF_SHEADER_OFFSETS[10][2]={ //offset,size
{0,4},{4,4},{8,4},{12,4},{16,4},{20,4},{24,4},{28,4},{32,4},{36,4}
};
const char *ELF_SHEADER_NAMES[10]={
"sh_name","sh_type","sh_flags","sh_addr","sh_offset","sh_size","sh_link","sh_info","sh_addralign","sh_entsize"
};
enum {sh_name=0,sh_type,sh_flags,sh_addr,sh_offset,sh_size,sh_link,sh_info,sh_addralign,sh_entsize};



#define MEMBER_OFFSET 0
#define MEMBER_SIZE 1
#define B_ENDIAN ELFDATA2MSB
#define L_ENDIAN ELFDATA2LSB

int get_32bit(unsigned char *buf,int endian)
{
	int result=0;
	switch(endian){
	default:
	case B_ENDIAN:
		result=(buf[0]<<24)|(buf[1]<<16)|(buf[2]<<8)|buf[3];
		break;
	case L_ENDIAN:
		result=(buf[3]<<24)|(buf[2]<<16)|(buf[1]<<8)|buf[0];
		break;
	}
	return result;
}
int get_16bit(unsigned char *buf,int endian)
{
	int result=0;
	switch(endian){
	default:
	case B_ENDIAN:
		result=(buf[0]<<8)|buf[1];
		break;
	case L_ENDIAN:
		result=(buf[1]<<8)|buf[0];
		break;
	}
	return result;
}
int get_data(unsigned char *buf,int offset,int size,int endian)
{
	int result=0;
	switch(size){
	default:
	case 1:
		result=buf[offset];
		break;
	case 2:
		result=get_16bit(buf+offset,endian);
		break;
	case 4:
		result=get_32bit(buf+offset,endian);
		break;
	}
	return result;
}
int get_header_val(int member,unsigned char *buf,int buf_size,int *result)
{
	int endian,offset,size;
	if(buf_size<sizeof(struct ELF32_HEADER)){
		printf("buffer too small, error getting %s\n",ELF_HEADER_NAMES[member]);
		return FALSE;
	}
	if(ELFCLASS32!=buf[ELF_HEADER_OFFSETS[e_bitness][MEMBER_OFFSET]]){
		printf("only 32bit elf supported\n");
		return FALSE;
	}
	
	offset=ELF_HEADER_OFFSETS[member][MEMBER_OFFSET];
	size=ELF_HEADER_OFFSETS[member][MEMBER_SIZE];
	endian=buf[ELF_HEADER_OFFSETS[e_endian][MEMBER_OFFSET]];
	if(result){
		*result=get_data(buf,offset,size,endian);
		return TRUE;
	}
	return FALSE;
}

int get_sheader_val(int member,int index,unsigned char *buf,int buf_size,int *result)
{
	int sect_offset,sect_count,sect_size,endian;
	if(get_header_val(e_endian,buf,buf_size,&endian)
		&& get_header_val(e_shoff,buf,buf_size,&sect_offset)
		&& get_header_val(e_shnum,buf,buf_size,&sect_count)
		&& get_header_val(e_shentsize,buf,buf_size,&sect_size)){
		if(index>=sect_count){
			printf("section count out of range\n");
		}else{
			int offset,size;
			offset=ELF_SHEADER_OFFSETS[member][MEMBER_OFFSET];
			size=ELF_SHEADER_OFFSETS[member][MEMBER_SIZE];
			if(result){
				char *data=buf+sect_offset+index*sect_size;
				if((data+size+offset)>(buf+buf_size)){
					printf("section header offset out of range\n");
				}else{
					*result=get_data(data,offset,size,endian);
					return TRUE;
				}
			}
		}
	}
	return FALSE;
}
int dump_elf(unsigned char *buf,int len)
{
	if(buf && len>0){
		int i,sect_count;
		for(i=0;i<sizeof(ELF_HEADER_NAMES)/sizeof(char *);i++){
			int val=0;
			if(get_header_val(i,buf,len,&val))
				printf("%-11s=0x%08X\n",ELF_HEADER_NAMES[i],val);
		}
		if(get_header_val(e_shnum,buf,len,&sect_count)){
			int j;
			for(j=0;j<sect_count;j++){
				printf("---section %i-----\n",j);
				for(i=0;i<sizeof(ELF_SHEADER_NAMES)/sizeof(char *);i++){
					int val=0;
					if(get_sheader_val(i,j,buf,len,&val)){
						printf("%-12s=0x%08X\n",ELF_SHEADER_NAMES[i],val);
					}
				}
			}
		}
	}
}

int dump_obj_file(char *fname)
{
	if(fname){
		FILE *f;
		f=fopen(fname,"rb");
		if(f){
			char buf[8]={0};
			char *elfmagic=ELFMAG;
			int len=sizeof(ELFMAG)-1;
			fread(buf,1,len,f);
			if(memcmp(buf,elfmagic,len)==0){
				int size;
				char *buf;
				fseek(f,0,SEEK_END);
				size=ftell(f);
				buf=malloc(size);
				if(buf){
					fseek(f,0,SEEK_SET);
					fread(buf,1,size,f);
					dump_elf(buf,size);
					free(buf);
				}
			}
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
				i++;
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
	if(getenv("PROMPT")==0){
		printf("press any key\n");
		getch();
	}
	return 0;
}