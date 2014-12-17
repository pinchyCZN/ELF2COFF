#define LNKMAGIC "LNK"


#define TAG_HEADER 1000
#define TAG_EOF 0
#define TAG_PROC_TYPE 46

#define TAG_SECTION_SYM 16

#define TAG_FILE_INFO 28

#define TAG_SWITCH_SECTION 6
#define TAG_CODE_SECTION 2
#define TAG_PATCH 10
#define TAG_UNINIT_DATA 8

#define TAG_XDEF_SYM 12
#define TAG_XREF_SYM 14
#define TAG_LOCAL_SYM 18
#define TAG_XBSS_SYM 48


#pragma pack(1)

typedef struct{
	char MAGIC[3];
	char version;
}LNK_HEADER;

typedef struct{
	char type;
}PROC_TYPE;

typedef struct{
	unsigned short num;
	unsigned short group;
	unsigned char align;
	unsigned char str_len;
	char str[1];
}SECTION_SYM;

typedef struct{
	unsigned short num;
	unsigned char str_len;
	char str[1];
}FILE_INFO;

typedef struct{
	unsigned short num;
}SWITCH_SECTION;

typedef struct{
	unsigned short len;
	unsigned char data[1];
}CODE_SECTION;

typedef struct{
	unsigned char type;
	unsigned short offset;
	short _m;
	unsigned char sectbase;
	short _m2;
	unsigned int sectbase_offset;
}PATCH_SECTION;

/* 
patch 92:
type byte
offset short
? byte 02
index short [local]  //at offset 7fc with [1e]

patch 96:
type byte
offset short
?? short 2c 04
sectbase short
? byte 00
sectbae_offset int //at offset a68 with (sectbase(2)+$100)

type byte
offset short
? short 2c 00
offset int
? byte 00
sectbae_offset int //at offset a82 with ($f332211+[20])

*/

typedef struct{
	unsigned short num;
	unsigned short section;
	unsigned int offset;
	unsigned char str_len;
	char sym[1];
}XDEF_SYM;

typedef struct{
	unsigned short num;
	unsigned char str_len;
	char sym[1];
}XREF_SYM;

typedef struct{
	unsigned short num;
	unsigned short section;
	unsigned int size;
	unsigned char str_len;
	char sym[1];
}XBSS_SYM;

typedef struct{
	unsigned short section;
	unsigned int offset;
	unsigned char str_len;
	char str[1];
}LOCAL_SYM;
