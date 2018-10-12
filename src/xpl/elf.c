#include<memmgr-config.h>
#include<xpl.h>
#include<memmgr.h>

#include<stdio.h>
#include<fcntl.h>
#ifdef WIN32
#include<io.h>
#endif

#if defined(LINUX) || defined(S390RH) || defined(MACOSX)
#ifndef __USE_GNU
#define __USE_GNU
#endif
#include <pthread.h>
#undef __USE_GNU
#endif

#include"elf.h"

void *ZMemMalloc( int size )
{
	void *mem;

	if( ( mem = (void *)MemMalloc( size ) ) )
	{
		memset( mem, 0, size );
	}
	return mem;
}

#if defined(LINUX) || defined(S390RH) || defined(MACOSX)
#define O_BINARY	0
#endif

ProgramData *LoadProgramData( char *binary )
{
	int fd, l;
	ProgramData *programData;
	Elf32_Ehdr *header;
	char *stringTable = NULL;
	Section *sec, *secNext;
	Symbol	*sym, *symNext;
	External *ext, *extNext;

	if( !(programData = (ProgramData *)ZMemMalloc( sizeof( ProgramData ) ) ) )
	{
		return NULL;
	}
	programData->sectionHead = NULL;
	programData->sectionEnd = &programData->sectionHead;
	programData->symbolHead = NULL;
	programData->symbolEnd = &programData->symbolHead;
	programData->externalHead = NULL;
	programData->externalEnd = &programData->externalHead;

	if( -1 == (fd = open( binary, O_BINARY|O_RDONLY ) ) )
	{
		MemFree( programData );
		return NULL;
	}
	if( !(header = (Elf32_Ehdr *)ZMemMalloc( sizeof( Elf32_Ehdr ) ) ) )
	{
		goto Failure;
	}

	if( Read( fd, 0, header, sizeof( Elf32_Ehdr ) ) )
	{
		goto Failure;
	}
	for(l=0;l<header->e_shnum;l++)
	{
		if( !(sec = (Section *)ZMemMalloc( sizeof( Section ) ) ) )
		{
			goto Failure;
		}
		sec->num = l;
		if( Read( fd, header->e_shoff + l * sizeof( Elf32_Shdr ), &sec->header, sizeof( Elf32_Shdr ) ) )
		{
			goto Failure;
		}
		if( sec->header.sh_addralign )
		{
			if( !(sec->actual = (char *)ZMemMalloc( sec->header.sh_size + sec->header.sh_addralign - 1 ) ) )
			{
				goto Failure;
			}
			sec->data = sec->actual + ((unsigned long)sec->actual % sec->header.sh_addralign);
		}
		else
		{
			if( !(sec->actual = (char *)ZMemMalloc( sec->header.sh_size ) ) )
			{
				goto Failure;
			}
			sec->data = sec->actual;
		}
		if( Read( fd, sec->header.sh_offset, sec->data, sec->header.sh_size ) )
		{
			goto Failure;
		}
		if( l == header->e_shstrndx )
		{
			stringTable = sec->data;
		}
		sec->next = NULL;
		*programData->sectionEnd = sec;
		programData->sectionEnd = &sec->next;
	}
	for(sec=programData->sectionHead;sec;sec=sec->next)
	{
		sec->name = stringTable + sec->header.sh_name;
	}
	for(sec=programData->sectionHead;sec;sec=sec->next)
	{
		if( sec->header.sh_type == SHT_SYMTAB )
		{
			if( LoadSymbols( programData, sec ) )
			{
				goto Failure;
			}
		}
	}
#if 0
	for(s=sectionHead;s;s=s->next)
	{
		if( s->header.sh_type == SHT_RELA )
			Relocate( s );
	}
	for(s=sectionHead;s;s=s->next)
		printf( "(%02x:%08x:%08x):%s\n", s->header.sh_type, s->header.sh_flags,
			s->header.sh_type, s->name );

	// program entry point
	ConvertValue( programData, header->e_entry, &start );
	start();
#endif

	MemFree( header );
	close( fd );
	return programData;

Failure:
	if( header )
	{
		MemFree( header );
	}
	for(sec=programData->sectionHead;sec;sec=secNext)
	{
		secNext = sec->next;
		if( sec->actual )
		{
			MemFree( sec->actual );
		}

		MemFree( sec );
	}
	for(sym=programData->symbolHead;sym;sym=symNext)
	{
		symNext = sym->next;
		MemFree( sym );
	}
	for(ext=programData->externalHead;ext;ext=extNext)
	{
		extNext = ext->next;
		MemFree( ext );
	}

	MemFree( programData );
	close( fd );
	return NULL;
}

Section *GetSection( ProgramData *programData, int num )
{
	Section *s;

	for(s=programData->sectionHead;s;s=s->next)
	{
		if( s->num == num )
		{
			return s;
		}
	}

	return NULL;
}

int Read( int fd, unsigned long offset, void *buffer, int length )
{
	if( offset == (unsigned long)lseek( fd, offset, SEEK_SET ) )
	{
		if( length == read( fd, buffer, length ) )
		{
			return 0;
		}
	}
	return -1;
}

// link : string table
// info : one greater than the symbol table index of the last local symbol
int LoadSymbols( ProgramData *programData, Section *s )
{
	Symbol *symbol;
	Elf32_Sym *sym;
	Section *strSec;
	int num;

	if( (strSec = GetSection( programData, s->header.sh_link ) ) )
	{
		for(sym=s->sym,num=0;(char *)sym < s->data + s->header.sh_size;sym++,num++)
		{
			if( (symbol = (Symbol *)ZMemMalloc( sizeof( Symbol ) ) ) )
			{
				symbol->section = s;
				symbol->num = num;
				symbol->name = strSec->data + sym->st_name;
				symbol->sym = sym;

				symbol->next = NULL;
				*programData->symbolEnd = symbol;
				programData->symbolEnd = &symbol->next;
			}
			else
			{
				return -1;
			}
		}
	}
	else
	{
		return -1;
	}
	return 0;
}

#if 0
int ConvertSymbols( ProgramData *programData )
{
	Symbol *symbol;

	for(symbol=programData->symbolHead;symbol;symbol=symbol->next)
	{
		if( symbol->sym->st_value )
		{
			if( ConvertValue( programData, symbol->sym->st_value, &symbol->sym->st_value ) )
				printf( "Error converting value.\n" );
		}

		switch( ELF32_ST_BIND( symbol->sym->st_info ) )
		{
			case STB_GLOBAL:
				if( symbol->sym->st_value )
				{
					if( Export( programData, symbol->name, symbol->sym->st_value ) )
						printf( "Multiple export %s\n", symbol->name );
				}
				else
				{
					if( Import( programData, symbol->name, &symbol->sym->st_value ) )
						printf( "Unresolved external symbol %s\n", symbol->name );
				}
				break;

			case STB_WEAK:
				break;
		}
	#if 0
		printf( "Symbol : %s\n", symbol->name );
		printf( "Value  : %08x\n", sym->st_value );
		printf( "Size   : %04x\n", sym->st_size );
		printf( "Binding: %02x\n", ELF32_ST_BIND( sym->st_info ) );
		printf( "Type   : %02x\n", ELF32_ST_TYPE( sym->st_info ) );
		printf( "Other  : %02x\n", sym->st_other );
	#endif
	}
	return 0;
}
#endif

Symbol *GetSymbol( ProgramData *programData, Section *s, int num )
{
	Symbol *symbol;

	for(symbol=programData->symbolHead;symbol;symbol=symbol->next)
	{
		if( symbol->section == s && symbol->num == num )
			return symbol;
	}
	return NULL;
}

// link : symbol table
// info : relocation section
void Relocate( ProgramData *programData, Section *s )
{
	Elf32_Rela *rela;
	Section *symSec, *relSec;
	Symbol *symbol;
	Elf32_Word *rel;

	symSec = GetSection( programData, s->header.sh_link );
	relSec = GetSection( programData, s->header.sh_info );

//	printf( "Relocating section %s\n", relSec->name );
	for(rela = s->rela;(char *)rela < s->data + s->header.sh_size;rela++)
	{
		symbol = GetSymbol( programData, symSec, ELF32_R_SYM( rela->r_info ) );
		rel = (Elf32_Word *)(relSec->data + rela->r_offset);
		switch( ELF32_R_TYPE( rela->r_info ) )
		{
			case R_386_32:
				*rel = symbol->sym->st_value + rela->r_addend;
				break;

			case R_386_PC32:
//				*rel = symbol->sym->st_value + rela->r_addend - (Elf32_Word)rel;
				*rel = symbol->sym->st_value + rela->r_addend - (Elf32_Word)rel - 4;
				break;
		}
#if 0
		printf( "Relocating symbol %s\n", symbol->name );
		printf( "Offset: %08x\n", rela->r_offset );
		printf( "Symbol: %02x\n", ELF32_R_SYM( rela->r_info ) );
		printf( "Type  : %02x\n", ELF32_R_TYPE( rela->r_info ) );
		printf( "Addend: %08x\n", rela->r_addend );
#endif
	}
}

External *FindExternal( ProgramData *programData, char *name )
{
	External *ex;

	for(ex=programData->externalHead;ex;ex=ex->next)
	{
		if( !strcmp( ex->name, name ) )
			return ex;
	}
	return NULL;
}

int Export( ProgramData *programData, char *name, Elf32_Word value )
{
	External *ex;

	if( FindExternal( programData, name ) )
		return -1;

	if( (ex = (External *)ZMemMalloc( sizeof( External ) + strlen( name + 1 ) ) ) )
	{
		ex->name = (char *)(ex + 1);
		strcpy( ex->name, name );
		ex->value = value;
		ex->next = NULL;
		*programData->externalEnd = ex;
		programData->externalEnd = &ex->next;

		return 0;
	}
	return -1;
}

int Import( ProgramData *programData, char *name, Elf32_Word *value )
{
	External *ex;

	if( (ex = FindExternal( programData, name ) ) )
	{
		*value = ex->value;
		return 0;
	}
	return -1;
}

int ConvertValue( ProgramData *programData, Elf32_Word value, Elf32_Word *newValue )
{
	Section *s;

	for(s=programData->sectionHead;s;s=s->next)
	{
		if( value >= s->header.sh_addr && value < s->header.sh_addr + s->header.sh_size )
		{
			*newValue = (Elf32_Word )(s->data + value - s->header.sh_addr);
			return 0;
		}
	}

	return -1;
}

#if 0
static int GetStack( void **stackAddr, size_t *stackSize )
{
#ifdef LINUX
	pthread_attr_t attr;

	pthread_getattr_np( pthread_self(), &attr );
	pthread_attr_getstack( &attr, stackAddr, stackSize);
#endif
	return 0;
}

static Symbol *FindSymbol( ProgramData *programData, Elf32_Addr address )
{
	Symbol *symbol, *sym;
	Section *sec;

	symbol = NULL;

	for(sec=programData->sectionHead;sec;sec=sec->next)
	{
		if( address >= sec->header.sh_addr && address < sec->header.sh_addr + sec->header.sh_size )
		{
			break;
		}
	}

	if( !sec )
	{
		return NULL;
	}

	for(sym=programData->symbolHead;sym;sym=sym->next)
	{
		if( sym->sym->st_value < sec->header.sh_addr || sym->sym->st_value >= sec->header.sh_addr + sec->header.sh_size )
		{
			continue;
		}
		if( sym->sym->st_value == address )
		{
			return sym;
		}
		if( sym->sym->st_value < address && ( !symbol || sym->sym->st_value > symbol->sym->st_value ) )
		{
			symbol = sym;
		}
		#if 0
			printf( "Symbol : %s\n", symbol->name );
			printf( "Value  : %08x\n", sym->st_value );
			printf( "Size   : %04x\n", sym->st_size );
			printf( "Binding: %02x\n", ELF32_ST_BIND( sym->st_info ) );
			printf( "Type   : %02x\n", ELF32_ST_TYPE( sym->st_info ) );
			printf( "Other  : %02x\n", sym->st_other );
		#endif
	}
	return symbol;
}

int PrintStack( ProgramData *programData )
{
	Symbol *symbol;
	unsigned long *walk = (unsigned long *)&programData;
	char *stackAddr;
	size_t stackSize;

	GetStack( (void **)&stackAddr, &stackSize );

	while( walk >= (unsigned long *)stackAddr && walk < (unsigned long *)(stackAddr + stackSize) )
	{
		if( (symbol = FindSymbol( programData, (Elf32_Addr)*walk ) ) )
		{
			if( *symbol->name )
			{
				if( symbol->sym->st_value == (Elf32_Addr)*walk )
				{
					printf( "%s\n", symbol->name );
				}
				else
				{
					printf( "%s + %lx\n", symbol->name, (Elf32_Addr)*walk - symbol->sym->st_value );
				}
			}
		}
		walk++;
	}
	return 0;
}
#endif

