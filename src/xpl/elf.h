#ifndef __ELF_H__
#define __ELF_H__

typedef unsigned long Elf32_Addr;
typedef unsigned short Elf32_Half;
typedef unsigned long Elf32_Off;
typedef signed long Elf32_Sword;
typedef unsigned long Elf32_Word;

#define EI_MAG0		0	// 0x7f
#define EI_MAG1		1	// 'E'
#define EI_MAG2		2	// 'L'
#define EI_MAG3		3	// 'F'
#define EI_CLASS		4
#define EI_DATA		5
#define EI_VERSION	6	// EV_CURRENT
#define EI_PAD			7
#define EI_NIDENT		16

// e_ident[EI_CLASS]
#define ELFCLASSNONE	0
#define ELFCLASS32	1
#define ELFCLASS64	2

// e_ident[EI_DATA]
#define ELFDATANONE	0
#define ELFDATA2LSB	1
#define ELFDATA2MSB	2

// e_type
#define ET_NONE	0
#define ET_REL		1
#define ET_EXEC	2
#define ET_DYN		3
#define ET_CORE	4
#define ET_LOPROC	0xff00
#define ET_HIPROC	0xffff

// e_machine
#define EM_NONE	0
#define EM_M32		1
#define EM_SPARC	2
#define EM_386		3
#define EM_68K		4
#define EM_88K		5
#define EM_860		7
#define EM_MIPS	8

// e_ident[EI_VERSION] and e_version
#define EV_NONE		0
#define EV_CURRENT	1

typedef struct
{
	unsigned char	e_ident[EI_NIDENT];
	Elf32_Half		e_type;
	Elf32_Half		e_machine;
	Elf32_Word		e_version;
	Elf32_Addr		e_entry;
	Elf32_Off		e_phoff;
	Elf32_Off		e_shoff;
	Elf32_Word		e_flags;
	Elf32_Half		e_ehsize;
	Elf32_Half		e_phentsize;
	Elf32_Half		e_phnum;
	Elf32_Half		e_shentsize;
	Elf32_Half		e_shnum;
	Elf32_Half		e_shstrndx;
}Elf32_Ehdr;

#define SHN_UNDEF			0
#define SHN_LORESERVE	0xff00
#define SHN_LOPROC		0xff00
#define SHN_HIPROC		0xff1f
#define SHN_ABS			0xfff1
#define SHN_COMMON		0xfff2
#define SHN_HIRESERVE	0xffff

// sh_type
#define SHT_NULL		0
#define SHT_PROGBITS	1
#define SHT_SYMTAB	2
#define SHT_STRTAB	3
#define SHT_RELA		4
#define SHT_HASH		5
#define SHT_DYNAMIC	6
#define SHT_NOTE		7
#define SHT_NOBITS	8
#define SHT_REL		9
#define SHT_SHLIB		10
#define SHT_DYNSYM	11
#define SHT_LOPROC	0x70000000
#define SHT_HIPROC	0x7fffffff
#define SHT_LOUSER	0x80000000
#define SHT_HIUSER	0xffffffff

// sh_flags
#define SHF_WRITE			0x01
#define SHF_ALLOC			0x02
#define SHF_EXECINSTR	0x04
#define SHF_MASKPROC		0xf0000000


typedef struct
{
	Elf32_Word	sh_name;
	Elf32_Word	sh_type;
	Elf32_Word	sh_flags;
	Elf32_Addr	sh_addr;
	Elf32_Off	sh_offset;
	Elf32_Word	sh_size;
	Elf32_Word	sh_link;
	Elf32_Word	sh_info;
	Elf32_Word	sh_addralign;
	Elf32_Word	sh_entsize;
}Elf32_Shdr;

//st_info
#define ELF32_ST_BIND(i)	((i) >> 4)
#define ELF32_ST_TYPE(i)	((i) & 0x0f)
#define ELF32_ST_INFO(b,t)	(((b) << 4) + ((t) & 0x0f))
//ELF32_ST_BIND
#define STB_LOCAL		0
#define STB_GLOBAL	1
#define STB_WEAK		2
#define STB_LOPROC	13
#define STB_HIPROC	15
//ELF32_ST_TYPE
#define STT_NOTYPE	0
#define STT_OBJECT	1
#define STT_FUNC		2
#define STT_SECTION	3
#define STT_FILE		4
#define STT_LOPROC	13
#define STT_HIPROC	15

typedef struct
{
	Elf32_Word		st_name;	//in sh_link string table
	Elf32_Addr		st_value;
	Elf32_Word		st_size;
	unsigned char	st_info;
	unsigned char	st_other;
	Elf32_Half		st_shndx;
}Elf32_Sym;

//r_info
#define ELF32_R_SYM(i)		((i) >> 8)
#define ELF32_R_TYPE(i)		((unsigned char)(i))
#define ELF32_R_INFO(s,t)	(((s) << 8) + (unsigned char)(t))
//ELF32_R_TYPE
#define R_386_NONE		0
#define R_386_32			1
#define R_386_PC32		2
#define R_386_GOT32		3
#define R_386_PLT32		4
#define R_386_COPY		5
#define R_386_GLOB_DAT	6
#define R_386_JMP_SLOT	7
#define R_386_RELATIVE	8
#define R_386_GOTOFF		9
#define R_386_GOTOPC		10


typedef struct
{
	Elf32_Addr	r_offset;
	Elf32_Word	r_info;
}Elf32_Rel;

typedef struct
{
	Elf32_Addr	r_offset;
	Elf32_Word	r_info;
	Elf32_Sword	r_addend;
}Elf32_Rela;

typedef struct Section
{
	struct Section *next;
	char *name;
	int num;
	char *actual;
	union
	{
		char *data;
		Elf32_Sym *sym;
		Elf32_Rel *rel;
		Elf32_Rela *rela;
	};
	Elf32_Shdr header;
}Section;

typedef struct Symbol
{
	struct Symbol *next;
	char *name;
	int num;
	Section *section;
	Elf32_Sym *sym;
}Symbol;

typedef struct External
{
	struct External *next;
	char *name;
	Elf32_Word value;
}External;

typedef struct ProgramData
{
	Section		*sectionHead;
	Section		**sectionEnd;
	Symbol		*symbolHead;
	Symbol		**symbolEnd;
	External	*externalHead;
	External	**externalEnd;
}ProgramData;

void *ZMemMalloc( int size );
ProgramData *LoadProgramData( char *binary );
Section *GetSection( ProgramData *programData, int num );
int Read( int fd, unsigned long offset, void *buffer, int length );
int LoadSymbols( ProgramData *programData, Section *s );
Symbol *GetSymbol( ProgramData *programData, Section *s, int num );
void Relocate( ProgramData *programData, Section *s );
External *FindExternal( ProgramData *programData, char *name );
int Export( ProgramData *programData, char *name, Elf32_Word value );
int Import( ProgramData *programData, char *name, Elf32_Word *value );
int ConvertValue( ProgramData *programData, Elf32_Word value, Elf32_Word *newValue );

#endif

