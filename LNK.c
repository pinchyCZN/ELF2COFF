#include "LNK.h"

#define LENDIAN 1
#define BENDIAN 2


int dump_lnk(unsigned char *buf,int buf_len)
{
	int offset;
	offset=4;
	while(buf_len>offset){
		int val;
		int increment=1;
		unsigned char *data;
		data=buf+offset;
		val=get_data(data,0,1,LENDIAN);
		printf("%08X %i : ",offset,val);
		switch(val){
		case PROC_TYPE:
			printf("Processor type %i\n",val);
			increment=_PROC_TYPE_SIZE+1;
			break;
		case SECTION_SYM:
			{
				int num,group,align,slen;
				char str[256]={0};
				num=get_data(data,_SECTION_SYM_NUM,2,LENDIAN);
				group=get_data(data,_SECTION_SYM_GRP,2,LENDIAN);
				align=get_data(data,_SECTION_SYM_ALIGN,1,LENDIAN);
				slen=0xFF&get_data(data,_SECTION_STR_SIZE,1,LENDIAN);
				memcpy(str,data+_SECTION_STR_START,slen);
				str[slen]=0;
				printf("Section symbol number %i '%s' in group %i alignment %i\n",num,str,group,align);
				increment=slen+_SECTION_SYM_LEN_MIN;
			}
			break;
		}
		offset+=increment;
	}

}