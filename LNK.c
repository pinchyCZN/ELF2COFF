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
			val=TAG_HEADER;
			increment=0;
		}
		else{
			val=get_data(data,0,1,LENDIAN);
			data++;
			increment=1;
			printf("%08X %i : ",offset,val);
		}
		switch(val){
		case TAG_HEADER:
			{
				LNK_HEADER *lh=data;
				printf("Header : %c%c%c version %i\n",lh->MAGIC[0],lh->MAGIC[1],lh->MAGIC[2],lh->version);
				increment+=sizeof(LNK_HEADER);
			}
			break;
		case TAG_PROC_TYPE:
			{
				PROC_TYPE *p=data;
				printf("Processor type %i\n",p->type);
				increment+=sizeof(PROC_TYPE);
			}
			break;
		case TAG_SECTION_SYM:
			{
				SECTION_SYM *ssym=data;
				char str[256]={0};
				memcpy(str,&ssym->str,ssym->str_len);
				str[ssym->str_len]=0;
				printf("Section symbol number %i '%s' in group %i alignment %i\n",ssym->num,str,ssym->group,ssym->align);
				increment+=ssym->str_len+offsetof(SECTION_SYM,str);
			}
			break;
		case TAG_FILE_INFO:
			{
				FILE_INFO *fi=data;
				char str[256]={0};
				memcpy(str,&fi->str,fi->str_len);
				str[fi->str_len]=0;
				printf("Define file number %i as \"%s\"\n",fi->num,str);
				increment+=fi->str_len+offsetof(FILE_INFO,str);
			}
			break;
		case TAG_SWITCH_SECTION:
			{
				SWITCH_SECTION *ss=data;
				printf("Switch to section %i\n",ss->num);
				increment+=sizeof(SWITCH_SECTION);
			}
			break;
		case TAG_CODE_SECTION:
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
		case TAG_PATCH:
			{
				PATCH_SECTION *ps=data;
				printf("Patch type %i at offset 0x%X with ",ps->type,ps->offset);
				switch(ps->a){
				case 2:
					printf("[0x%X]\n",ps->local);
					increment+=6;
					break;
				case 0x2c:
					switch(ps->b){
					case 4:
						{
							short *sectbase=data+5;
							printf("(sectbase (%i)",*sectbase);
							increment+=7;
						}
						break;
					case 0:
						{
							int *offset=data+5;
							printf("(0x%X",*offset);
							increment+=9;
						}
						break;
					default:
						printf("2c UKNOWN CASE!!");
						increment+=1;
						break;
					}
					increment--;
					{
						char *a=data+increment;
						if(a[0]==0){
LAST:
							{
							int *offset=data+increment+1;
							printf("+ 0x%X)",*offset);
							increment+=5;
							}
						}
						else if(0[a]==0x2C && 1[a]==4){
							short *sectbase=data+increment+2;
							printf("+sectbase (%i)",*sectbase);
							increment+=4;
							if(a[4]==0)
								goto LAST;
							printf(")");
						}else if(0[a]==2){
							short *b=data+increment+1;
							printf("+[%X])",*b);
							increment+=3;
						}else{
							printf("unknown last byte");
							increment+=1;
						}

					}
					printf("\n");
					increment++;
					break;
				default:
					printf("UKNOWN CASE!!\n");
					increment+=1;
					break;
				}
			}
			break;
		case TAG_EOF:
			offset=buf_len;
			printf("End of file\n\n");
			break;
		case TAG_UNINIT_DATA:
			{
				int *size=data;
				printf("Uninitialised data, %i bytes\n",*size);
				increment+=4;
			}
			break;
		case TAG_XREF_SYM:
			{
				XREF_SYM *xr=data;
				increment+=xr->str_len+offsetof(XREF_SYM,sym);
				printf("xref\n");
			}
			break;
		case TAG_XDEF_SYM:
			{
				XDEF_SYM *xs=data;
				increment+=xs->str_len+offsetof(XDEF_SYM,sym);
				printf("xdef\n");
			}
			break;
		case TAG_XBSS_SYM:
			{
				XBSS_SYM *xb=data;
				increment+=xb->str_len+offsetof(XBSS_SYM,sym);
				printf("xbss\n");
			}
			break;
		case TAG_LOCAL_SYM:
			{
				LOCAL_SYM *ls=data;
				increment+=ls->str_len+offsetof(LOCAL_SYM,sym);
				printf("xbss\n");
			}
			break;
		default:
			printf("unsupported cmd\n");
			break;
		}

		offset+=increment;
	}

}