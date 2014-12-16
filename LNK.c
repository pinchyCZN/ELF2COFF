#include <stddef.h>
#include <stdio.h>

//only works on little endian platform
#include "LNK.h"

#define LENDIAN 1
#define BENDIAN 2


int dump_lnk(unsigned char *buf,int buf_len)
{
	int offset=0;
	while(buf_len>offset){
		int val;
		int increment=1;
		unsigned char *data;
		data=buf+offset;
		if(offset==0){
			val=CMD_HEADER;
			increment=0;
		}
		else{
			val=get_data(data,0,1,LENDIAN);
			data++;
			increment=1;
			printf("%08X %i : ",offset,val);
		}
		switch(val){
		case CMD_HEADER:
			{
				LNK_HEADER *lh=data;
				printf("Header : %c%c%c version %i\n",lh->MAGIC[0],lh->MAGIC[1],lh->MAGIC[2],lh->version);
				increment+=sizeof(LNK_HEADER);
			}
			break;
		case CMD_PROC_TYPE:
			{
				PROC_TYPE *p=data;
				printf("Processor type %i\n",p->type);
				increment+=sizeof(PROC_TYPE);
			}
			break;
		case CMD_SECTION_SYM:
			{
				SECTION_SYM *ssym=data;
				char str[256]={0};
				memcpy(str,&ssym->str,ssym->str_len);
				str[ssym->str_len]=0;
				printf("Section symbol number %i '%s' in group %i alignment %i\n",ssym->num,str,ssym->group,ssym->align);
				increment+=ssym->str_len+offsetof(SECTION_SYM,str);
			}
			break;
		case CMD_FILE_INFO:
			{
				FILE_INFO *fi=data;
				char str[256]={0};
				memcpy(str,&fi->str,fi->str_len);
				str[fi->str_len]=0;
				printf("Define file number %i as \"%s\"\n",fi->num,str);
				increment+=fi->str_len+offsetof(FILE_INFO,str);
			}
			break;
		case CMD_SWITCH_SECTION:
			{
				SWITCH_SECTION *ss=data;
				printf("Switch to section %i\n",ss->num);
				increment+=sizeof(SWITCH_SECTION);
			}
			break;
		case CMD_CODE_SECTION:
			{
				int i,tmp;
				CODE_SECTION *cs=data;
				tmp=offset+sizeof(CODE_SECTION);
				printf("Code %i bytes\n",cs->len);
				for(i=0;i<cs->len;i++){
					if((i%16)==0)
						printf("\n%06X:",tmp);
					tmp++;
					printf("%02X ",cs->data[i]);
				}
				printf("\n\n");
				increment+=cs->len+offsetof(CODE_SECTION,data);
			}
			break;
		case CMD_PATCH:
			{
				PATCH_SECTION *ps=data;

				printf("Patch type %i at offset %X with (sectbase(%i)+0x%04X)\n",
					ps->type,ps->offset,ps->sectbase,ps->sectbase_offset);
				increment+=sizeof(PATCH_SECTION);
			}
			break;
		case CMD_EOF:
			offset=buf_len;
			printf("End of file\n\n");
			break;
		case CMD_XDEF_SYM:
			{
				XDEF_SYM *xs=data;
				increment+=xs->str_len+offsetof(XDEF_SYM,sym);
				printf("xdef\n");
			}
			break;
		default:
			printf("unsupported cmd\n");
			break;
		}

		offset+=increment;
	}

}