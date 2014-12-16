#define LNKMAGIC "LNK"


#define CMD_HEADER 1000
#define CMD_EOF 0
#define CMD_PROC_TYPE 46

#define CMD_SECTION_SYM 16

#define CMD_FILE_INFO 28

#define CMD_SWITCH_SECTION 6
#define CMD_CODE_SECTION 2
#define CMD_PATCH 10
#define CMD_UNINIT_DATA 8

#define CMD_XDEF_SYM 12
#define CMD_XREF_SYM 14
#define CMD_LOCAL_SYM 18
#define CMD_XBSS_SYM 48


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
	short offset;
	short _m;
	unsigned char sectbase;
	short _m2;
	short sectbase_offset;
	short _m3;
}PATCH_SECTION;

typedef struct{
	short a;
	short section;
	unsigned int offset;
	unsigned char str_len;
	char sym[1];
}XDEF_SYM;


