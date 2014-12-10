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
int read_header_val(int member,int val)
{
	switch(member){
	case e_ident:
		printf("magic header");
		break;
	case e_bitness:
		switch(val){
		case 1:printf("32 bit");break;
		case 2:printf("64 bit");break;
		default:printf("invalid");break;
		}
		break;
	case e_endian:
		switch(val){
		case 1:printf("little endian");break;
		case 2:printf("big endian");break;
		default:printf("invalid");break;
		}
		break;
	case e_elfver:
		switch(val){
		case 1:printf("elf version");break;
		default:printf("invalid");break;
		}
		break;
	case e_reserved0:printf("reserved0");
		break;
	case e_reserved1:printf("reserved1");
		break;
	case e_reserved2:printf("reserved2");
		break;
	case e_type:
		printf("file type:");
		switch(val){
		case ET_NONE:printf("No file type");break;
		case ET_REL:printf("Relocatable");break;
		case ET_EXEC:printf("Executable");break;
		default:printf("other");break;
		}
		break;
	case e_machine:
		printf("architecture:");
		switch(val){
		case EM_MIPS:printf("MIPS");break;
		default:printf("other");break;
		}
		break;
	case e_version:
		printf("version:");
		switch(val){
		case EV_NONE:printf("Invalid ELF version");break;
		case EV_CURRENT:printf("Current");break;
		case EV_NUM:printf("num");break;
		}
		break;
	case e_entry:
		break;
	case e_phoff:
		break;
	case e_shoff:
		break;
	case e_flags:
		{
			int i;
			printf("\n\t(flags valid only for MIPS):\n");
			for(i=0;i<32;i++){
				int printed=TRUE;
				int bit=val&(1<<i);
				if(bit!=0)
					printf("\t");
				switch(bit){
				case EF_MIPS_NOREORDER:printf("At least one .noreorder assembly");break;
				case EF_MIPS_PIC:printf("This file contains position-independent code");break;
				case E_MIPS_ABI_O32:printf("This file follows the first MIPS 32 bit ABI");break;
				case E_MIPS_ABI_O64:printf("This file follows the UCODE MIPS 64 bit ABI");break;
				case E_MIPS_ABI_EABI32:printf("Embedded Application Binary Interface for 32-bit");break;
				case E_MIPS_ABI_EABI64:printf("Embedded Application Binary Interface for 64-bit");break;
				default:
					if(bit!=0){
						printf("other flag %08X",bit);
						printed=TRUE;
					}
					else
						printed=FALSE;
					break;
				}
				if(printed){
					printf("\n");
				}
			}
		}
		break;
	case e_ehsize:
		break;
	case e_phentsize:
		break;
	case e_phnum:
		break;
	case e_shentsize:
		break;
	case e_shnum:
		break;
	case e_shstrndx:
		break;
	}
}
int read_shead_val(int member,int val)
{
	switch(member){
	case sh_type:
		switch(val){
		case SHT_NULL:printf("Section header table entry unused");break;
		case SHT_PROGBITS:printf("Program data");break;
		case SHT_SYMTAB:printf("Symbol table");break;
		case SHT_STRTAB:printf("String table");break;
		case SHT_RELA:printf("Relocation entries with addends. Warning: Works only in 64 bit mode in my tests!");break;
		case SHT_HASH:printf("Symbol hash table");break;
		case SHT_DYNAMIC:printf("Dynamic linking information");break;
		case SHT_NOTE:printf("Notes");break;
		case SHT_NOBITS:printf("Program space with no data (bss)");break;
		case SHT_REL:printf("Relocation entries, no addends");break;
		case SHT_SHLIB:printf("Reserved");break;
		case SHT_DYNSYM:printf("Dynamic linker symbol table");break;
		case SHT_INIT_ARRAY:printf("Array of constructors");break;
		case SHT_FINI_ARRAY:printf("Array of destructors");break;
		case SHT_PREINIT_ARRAY:printf("Array of pre-constructors");break;
		case SHT_GROUP:printf("Section group");break;
		case SHT_SYMTAB_SHNDX:printf("Extended section indeces");break;
		case SHT_NUM:printf("Number of defined types. ");break;
		case SHT_LOOS:printf("Start OS-specific");break;
		case SHT_CHECKSUM:printf("Checksum for DSO content. ");break;
		case SHT_LOSUNW:printf("Sun-specific low bound. ");break;
		case SHT_GNU_verdef:printf("Version definition section. ");break;
		case SHT_GNU_verneed:printf("Version needs section. ");break;
		case SHT_GNU_versym:printf("Version symbol table. ");break;
		case SHT_LOPROC:printf("Start of processor-specific");break;
		case SHT_HIPROC:printf("End of processor-specific");break;
		case SHT_LOUSER:printf("Start of application-specific");break;
		case SHT_HIUSER:printf("End of application-specific");break;
		case SHT_REMOVE_ME:printf("Specific to objconv program: Removed debug or exception handler section");break;
		}
		break;
	case sh_flags:
		switch(val){
		case SHF_WRITE:printf("Writable");break;
		case SHF_ALLOC:printf("Occupies memory during execution");break;
		case SHF_EXECINSTR:printf("Executable");break;
		case SHF_MERGE:printf("Might be merged");break;
		case SHF_STRINGS:printf("Contains nul-terminated strings");break;
		case SHF_INFO_LINK:printf("`sh_info' contains SHT index");break;
		case SHF_LINK_ORDER:printf("Preserve order after combining");break;
		case SHF_OS_NONCONFORMING:printf("Non-standard OS specific handling required");break;
		case SHF_MASKOS:printf("OS-specific. ");break;
		case SHF_MASKPROC:printf("Processor-specific");break;
		}
		break;
	case sh_link:
		break;
	case sh_info:
		break;
	}
}
int dump_elf(unsigned char *buf,int len)
{
	if(buf && len>0){
		int i,sect_count;
		for(i=0;i<sizeof(ELF_HEADER_NAMES)/sizeof(char *);i++){
			int val=0;
			if(get_header_val(i,buf,len,&val)){
				printf("%-11s=0x%08X ",ELF_HEADER_NAMES[i],val);
				read_header_val(i,val);
			}
			printf("\n");
		}
		if(get_header_val(e_shnum,buf,len,&sect_count)){
			int j;
			for(j=0;j<sect_count;j++){
				printf("---section %i-----\n",j);
				for(i=0;i<sizeof(ELF_SHEADER_NAMES)/sizeof(char *);i++){
					int val=0;
					if(get_sheader_val(i,j,buf,len,&val)){
						printf("%-12s=0x%08X ",ELF_SHEADER_NAMES[i],val);
						read_shead_val(i,val);
						if(i==sh_name){
							int name_index;
							if(get_header_val(e_shstrndx,buf,len,&name_index)){
								int name_offset;
								if(get_sheader_val(sh_offset,name_index,buf,len,&name_offset)){
									printf("-> %s",buf+name_offset+val);
								}
							}
						}
						printf("\n");
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