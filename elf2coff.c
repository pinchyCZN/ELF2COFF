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

enum {r_offset,r_type,r_sym};

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
int get_reloc_entry(int member,int index,int offset,unsigned char *buf,int buf_size,int *result)
{
	int data,endian;
	data=buf+offset+index*sizeof(struct Elf32_Rel);
	if((data+8)>(buf+buf_size)){
		printf("relocation entry out of range");
		return FALSE;
	}
	if(get_header_val(e_endian,buf,buf_size,&endian)){
		int val=0;
		switch(member){
		default:printf("invalid relocation member");return FALSE;break;
		case r_offset:
			val=get_data(data,0,4,endian);
			break;
		case r_type:
			val=get_data(data,4,4,endian);
			val=ELF32_R_TYPE(val);
			break;
		case r_sym:
			val=get_data(data,4,4,endian);
			val=ELF32_R_SYM(val);
			break;
		}
		if(result)
			*result=val;
		return TRUE;
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
			printf("\n\t(flags valid only for MIPS):\n\t");
			switch(val&EF_MIPS_ARCH){
			case EF_MIPS_ARCH_1:printf("-mips1 code");break;
			case EF_MIPS_ARCH_2:printf("-mips2 code");break;
			case EF_MIPS_ARCH_3:printf("-mips3 code");break;
			case EF_MIPS_ARCH_4:printf("-mips4 code");break;
			case EF_MIPS_ARCH_5:printf("-mips5 code");break;
			case EF_MIPS_ARCH_32:printf("MIPS32 code");break;
			case EF_MIPS_ARCH_64:printf("MIPS64 code");break;
			case EF_MIPS_ARCH_32R2:printf("MIPS32r2 code");break;
			case EF_MIPS_ARCH_64R2:printf("MIPS64r2 code");break;
			}
			printf("\n");
			for(i=0;i<32;i++){
				int printed=TRUE;
				int bit=val&(1<<i);
				if(bit!=0)
					printf("\t");
				switch(bit){
				case EF_MIPS_NOREORDER:printf("At least one .noreorder assembly directive appeared in a source contributing to the object");break;
				case EF_MIPS_PIC:printf("This file contains position-independent code");break;
				case EF_MIPS_CPIC:printf("This file's code follows standard conventions for calling position-independent code");break;
				case EF_MIPS_XGOT:printf("This file contains large (32-bit) GOT");break;
				case EF_MIPS_64BIT_WHIRL:printf("This file contains WHIRL intermediate relocation language code (SGI/Open64)");break;
				case EF_MIPS_ABI2:printf("This file follows the n32 abi");break;
				case EF_MIPS_ABI_ON32:printf("(obsolete)");break;
				case EF_MIPS_OPTIONS_FIRST:printf("The .MIPS.options section in this file contains one or more descriptors, currently types ODK_GP_GROUP and/or ODK_IDENT, which should be processed first by ld");break;
				case EF_MIPS_32BITMODE:printf("binaries compiled for a 32bit ABI, but a 64bit ISA, have this flag set, as the kernel will refuse to execute 64bt code (i.e. not o32 or n32 ABI)");break;
				case E_MIPS_FP64:printf("32-bit machine but FP registers are 64 bit (-mfp64)");break;
				case E_MIPS_NAN2008:printf("Code in file uses the IEEE 754-2008 NaN encoding convention");break;
				case E_MIPS_ABI_O32:
					if((val&E_MIPS_ABI_EABI32)!=E_MIPS_ABI_EABI32){
						printf("This file follows the first MIPS 32 bit ABI (UCODE)");break;
					}
					printed=FALSE;
					break;
				case E_MIPS_ABI_O64:
					if((val&E_MIPS_ABI_EABI32)!=E_MIPS_ABI_EABI32){
						printf("This file follows the UCODE MIPS 64 bit ABI (obsolete)");break;
						break;
					}
					//fall thru E_MIPS_ABI_EABI32	0x00003000
				case E_MIPS_ABI_EABI32:printf("Embedded Application Binary Interface for 32-bit");break;
				case E_MIPS_ABI_EABI64:printf("Embedded Application Binary Interface for 64-bit");break;
				case EF_MIPS_ARCH_ASE_MDMX:printf("Uses MDMX multimedia extensions");break;
				case EF_MIPS_ARCH_ASE_M16:printf("Uses MIPS-16 ISA extensions");break;
				case EF_MIPS_ARCH_ASE_MICROMIPS:printf("Uses MicroMips. Actually not an extension, but a full architecture");break;
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
int read_shead_val(int member,unsigned int val)
{
	switch(member){
	case sh_type:
		switch(val){
		case SHT_NULL:printf("Section header table entry unused");break;
		case SHT_PROGBITS:printf("Program data");break;
		case SHT_SYMTAB:printf("Symbol table");break;
		case SHT_STRTAB:printf("String table");break;
		case SHT_RELA:printf("Relocation entries with addends");break;
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
		case SHT_LOUSER:printf("Start of application-specific");break;
		case SHT_HIUSER:printf("End of application-specific");break;
		default:
			if(val>=SHT_LOPROC && val<=SHT_HIPROC){
				switch(val){
				case SHT_MIPS_LIBLIST:printf("DSO library information used to link");break;
				case SHT_MIPS_MSYM:printf("MIPS symbol table extension");break;
				case SHT_MIPS_CONFLICT:printf("Symbol conflicting with DSO defined symbols");break;
				case SHT_MIPS_GPTAB:printf("Global pointer table");break;
				case SHT_MIPS_UCODE:printf("Reserved");break;
				case SHT_MIPS_DEBUG:printf("Reserved (obsolete debug information)");break;
				case SHT_MIPS_REGINFO:printf("Register usage information");break;
				case SHT_MIPS_PACKAGE:printf("OSF reserved");break;
				case SHT_MIPS_PACKSYM:printf("OSF reserved");break;
				case SHT_MIPS_RELD:printf("Dynamic relocations (obsolete)");break;
				case SHT_MIPS_IFACE:printf("Subprogram interface information");break;
				case SHT_MIPS_CONTENT:printf("Section content information");break;
				case SHT_MIPS_OPTIONS:printf("General options");break;
				case SHT_MIPS_DELTASYM:printf("Delta C++ symbol table (obsolete)");break;
				case SHT_MIPS_DELTAINST:printf("Delta C++ instance table (obsolete)");break;
				case SHT_MIPS_DELTACLASS:printf("Delta C++ class table (obsolete)");break;
				case SHT_MIPS_DWARF:printf("Dwarf debug information");break;
				case SHT_MIPS_DELTADECL:printf("Delta C++ declarations (obsolete)");break;
				case SHT_MIPS_SYMBOL_LIB:printf("Symbol to library mapping");break;
				case SHT_MIPS_EVENTS:printf("Section event mapping");break;
				case SHT_MIPS_TRANSLATE:printf("Old pixie translation table (obsolete)");break;
				case SHT_MIPS_PIXIE:printf("Pixie specific sections (SGI)");break;
				case SHT_MIPS_XLATE:printf("Address translation table");break;
				case SHT_MIPS_XLATE_DEBUG:printf("SGI internal address translation table");break;
				case SHT_MIPS_WHIRL:printf("Intermediate code (MipsPro compiler)");break;
				case SHT_MIPS_EH_REGION:printf("C++ exception handling region information");break;
				case SHT_MIPS_XLATE_OLD:printf("obsolete");break;
				case SHT_MIPS_PDR_EXCEPTION:printf("Runtime procedure descriptor table exception information (ucode");break;
				}
			}else if(val>=SHT_LOUSER && val<=SHT_HIUSER){
				printf("application specific");
			}
			break;
		}
		break;
	case sh_flags:
		{
			int i;
			for(i=0;i<32;i++){
				int bit;
				bit=val&(1<<i);
				switch(bit){
				case SHF_WRITE:printf("Writable");break;
				case SHF_ALLOC:printf("Occupies memory during execution");break;
				case SHF_EXECINSTR:printf("Executable");break;
				case SHF_MERGE:printf("Might be merged");break;
				case SHF_STRINGS:printf("Contains nul-terminated strings");break;
				case SHF_INFO_LINK:printf("`sh_info' contains SHT index");break;
				case SHF_LINK_ORDER:printf("Preserve order after combining");break;
				case SHF_OS_NONCONFORMING:printf("Non-standard OS specific handling required");break;
				case SHF_MIPS_GPREL:printf("SHF_MIPS_GPREL");break;
				case SHF_MIPS_MERGE:printf("SHF_MIPS_MERGE");break;
				case SHF_MIPS_ADDR:printf("SHF_MIPS_ADDR");break;
				case SHF_MIPS_STRINGS:printf("SHF_MIPS_STRINGS");break;
				case SHF_MIPS_NOSTRIP:printf("SHF_MIPS_NOSTRIP");break;
				case SHF_MIPS_LOCAL:printf("SHF_MIPS_LOCAL");break;
				case SHF_MIPS_NAMES:printf("SHF_MIPS_NAMES");break;
				case SHF_MIPS_NODUPE:printf("SHF_MIPS_NODUPE");break;
				}
				if(bit)
					printf(",");
			}
		}
		break;
	case sh_link:
		break;
	case sh_info:
		break;
	}
}
int read_reloc_val(int member,int val)
{
	switch(member){
	case r_type:
		switch(val){
		case R_MIPS_NONE:printf("R_MIPS_NONE: none");break;
		case R_MIPS_16:printf("R_MIPS_16: S + sign_extend(A)");break;
		case R_MIPS_32:printf("R_MIPS_32: S+A");break;
		case R_MIPS_REL32:printf("R_MIPS_REL_32: S + A - EA");break;
		case R_MIPS_26:printf("R_MIPS_26: (((A << 2) | ((P + 4) & 0xf0000000)) + S) >> 2");break;
		case R_MIPS_HI16:printf("R_MIPS_HI16: high(AHL + S) The high(x) function is ( x - (short)x ) >> 16");break;
		case R_MIPS_LO16:printf("R_MIPS_LO16: AHL + S");break;
		case R_MIPS_GPREL16:printf("R_MIPS_GPREL16: sign_extend(A) + S + GP0 - GP");break;
		case R_MIPS_LITERAL:printf("R_MIPS_LITERAL: sign_extend(A) + L");break;
		case R_MIPS_GOT16:printf("R_MIPS_GOT16: See  description");break;
		case R_MIPS_PC16:printf("R_MIPS_PC16: sign_extend(A) + S - P");break;
		case R_MIPS_CALL16:printf("R_MIPS_CALL16: G");break;
		case R_MIPS_GPREL32:printf("R_MIPS_GPREL32: A + S + GP0 - GP");break;
		default:printf("unknown:%i (0x%X)",val,val);break;
		}
		break;
	}
}
int get_symbol(int index,int section,char *out,int out_len,unsigned char *buf,int buf_len)
{
	int offset,size;
	if(get_sheader_val(sh_offset,section,buf,buf_len,&offset)
		&& get_sheader_val(sh_size,section,buf,buf_len,&size)){
		unsigned char *data;
		if( (index+1)*sizeof(struct Elf32_Sym)>size ){
			printf("symbol offset out of range\n");
			return FALSE;
		}
		data=buf+offset+(index*sizeof(struct Elf32_Sym));
		if((data+sizeof(struct Elf32_Sym)) > (buf+buf_len)){
			printf("symbol out of range\n");
			return FALSE;
		}

		_snprintf(out,out_len,"%s",data);
		return TRUE;
	}
	return FALSE;
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
				printf("---section %i (0x%X)-----\n",j,j);
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
			//dump relocations
			for(j=0;j<sect_count;j++){
				int val=0;
				if(get_sheader_val(sh_type,j,buf,len,&val)){
					if(val==SHT_REL){
						int offset,size;
						if(get_sheader_val(sh_offset,j,buf,len,&offset)
							&& get_sheader_val(sh_size,j,buf,len,&size)){
							int k;
							printf("---reloc---\n");
							for(k=0;k<size/sizeof(struct Elf32_Rel);k++){
								int roffset,rtype,rsym;
								get_reloc_entry(r_offset,k,offset,buf,len,&roffset);
								get_reloc_entry(r_type,k,offset,buf,len,&rtype);
								get_reloc_entry(r_sym,k,offset,buf,len,&rsym);
								printf("\treloc # %i (0x%X)\n",k,k);
								printf("\toffset=%08X\n",roffset);
								printf("\ttype=%08X ",rtype);
								read_reloc_val(r_type,rtype);
								printf("\n");
								printf("\tsym=%08X ",rsym);
								{
									int link=0;
									char tmp[256]={0};
									get_sheader_val(sh_link,j,buf,len,&link);
									get_symbol(rsym,link,tmp,sizeof(tmp),buf,len);
									printf("%s\n",tmp);
								}
							}
						}
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