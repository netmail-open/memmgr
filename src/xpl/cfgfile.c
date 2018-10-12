//#define STAND_ALONE
#ifdef STAND_ALONE
#define EXPORT
#else
#include<memmgr-config.h>
#define CFG_INTERNAL 1
#include<xpl.h>
#endif

#include<stdio.h>
#include<string.h>

#if !defined(MACOSX)
#include<malloc.h>
#endif

#define COMMENT_BYTE ';'
#define CFGFILE_SIGNATURE 0x46474643    // 'CFGF'

typedef struct CFGItem
{
        struct CFGItem *next;
        char *name;
        char *value;
}CFGItem;

typedef struct CFGSection
{
        struct CFGSection*next;
        char *name;
        CFGItem *itemList;
        CFGItem **itemEnd;
}CFGSection;

typedef struct
{
        unsigned long signature;
        char *name;
        CFGSection *sectionList;
        CFGSection **sectionEnd;
        int changed;
}CFGFile;

// interface
EXPORT CFGFile *XplCFGOpen( char *name );
EXPORT CFGFile *XplCFGCreate( char *name );
EXPORT int XplCFGFlush( CFGFile *config );
EXPORT int XplCFGClose( CFGFile *config );
EXPORT char *XplCFGGetValue( CFGFile *config, char *sectionName, char *name );
EXPORT int XplCFGSetValue( CFGFile *config, char *sectionName, char *name, char *value );

// internal
CFGFile *_InternalOpen( char *name, int create );
char *Clean( char *string );
CFGSection *FindSection( CFGFile *config, char *name );
CFGSection *AddSection( CFGFile *config, char *name );
CFGItem *FindItem( CFGSection *section, char *name );
CFGItem *AddItem( CFGSection *section, char *name, char *value );

#ifdef STAND_ALONE
int main( int argc, char **argv )
{
        CFGFile *cfgFile;

        cfgFile = XplCFGCreate( "test.cfg" );
        if( cfgFile )
        {
                XplCFGSetValue( cfgFile, NULL, NULL, " semicolon comments" );
                XplCFGSetValue( cfgFile, NULL, NULL, " NULL section" );
                XplCFGSetValue( cfgFile, NULL, "test", "chicken" );
                XplCFGSetValue( cfgFile, NULL, NULL, NULL );
                XplCFGSetValue( cfgFile, NULL, NULL, " junk section" );
                XplCFGSetValue( cfgFile, "junk", "test", "chicken" );


                XplCFGClose( cfgFile );
        }

        cfgFile = XplCFGOpen( "test.cfg" );

        printf( "test in NULL section:'%s'\n", XplCFGGetValue( cfgFile, NULL, "test" ) );
        printf( "test in junk section:'%s'\n", XplCFGGetValue( cfgFile, "junk", "test" ) );
        XplCFGSetValue( cfgFile, "junk", "test", "junker" );
        printf( "test in junk section:'%s'\n", XplCFGGetValue( cfgFile, "junk", "test" ) );
        XplCFGSetValue( cfgFile, "junk", NULL, NULL );
        XplCFGSetValue( cfgFile, "junk", NULL, ";chicken section" );
        XplCFGSetValue( cfgFile, "chicken", "test", "junk" );
        printf( "test in chicken section:'%s'\n", XplCFGGetValue( cfgFile, "chicken", "test" ) );

        XplCFGClose( cfgFile );

        return 0;
}
#endif

char *Clean( char *string )
{
        char *start, *end;

        start = string;
        while( *start == ' ' || *start == '\t' )
                start++;
        end = start + strlen( start ) - 1;
        while( (end >= start) && (*end == '\r' || *end == '\n' ||
                        *end == ' ' || *end == '\t' ) )
        {
                *end = '\0';
                end--;
        }
        return start;
}

CFGSection *FindSection( CFGFile *cfgFile, char *name )
{
        CFGSection *section;

        for( section = cfgFile->sectionList; section; section = section->next )
        {
                if( name )
                {
                        if( section->name )
                        {
                                if( !stricmp( name, section->name ) )
                                        return section;
                        }
                }
                else
                {
                        if( !section->name )
                                return section;
                }
        }
        return NULL;
}

CFGSection *AddSection( CFGFile *cfgFile, char *name )
{
        CFGSection *section;

        section = (CFGSection *)malloc( sizeof( CFGSection ) );
        if( !section )
                return NULL;

        section->next = NULL;
        if( name )
        {
                section->name = (char *)malloc( strlen( name ) + 1 );
                if( !section->name )
                {
                        free( section );
                        return NULL;
                }
                strcpy( section->name, name );
        }
        else
                section->name = NULL;
        section->itemList = NULL;
        section->itemEnd = &section->itemList;

        *cfgFile->sectionEnd = section;
        cfgFile->sectionEnd = &section->next;

        return section;
}

CFGItem *FindItem( CFGSection *section, char *name )
{
        CFGItem *item;

        if( !name )
                return NULL;

        for( item=section->itemList; item; item = item->next )
        {
                if( !item->name )
                        continue;
                if( !stricmp( name, item->name ) )
                        return item;
        }
        return NULL;
}

CFGItem *AddItem( CFGSection *section, char *name, char *value )
{
        CFGItem *item;

        item = (CFGItem *)malloc( sizeof( CFGItem ) );
        if( !item )
                return NULL;

        item->next = NULL;
        if( name )
        {
                item->name = (char *)malloc( strlen( name ) + 1 );
                if( !item->name )
                        goto Error;
                strcpy( item->name, name );
        }
        else
                item->name = NULL;

        if( value )
        {
                item->value = (char *)malloc( strlen( value ) + 1 );
                if( !item->value )
                        goto Error;
                strcpy( item->value, value );
        }
        else
                item->value = NULL;

        *section->itemEnd = item;
        section->itemEnd = &item->next;

        return item;

Error:
        if( item->name )
                free( item->name );
        free( item );
        return NULL;
}

EXPORT CFGFile *XplCFGOpen( char *name )
{
        return _InternalOpen( name, 0 );
}

EXPORT CFGFile *XplCFGCreate( char *name )
{
        return _InternalOpen( name, 1 );
}


CFGFile *_InternalOpen( char *name, int create )
{
        FILE *fp;
        CFGFile *cfgFile;
        CFGSection *section;
        char *p, *start, *value;
        int line;
        char lineBuff[256];

        cfgFile = (CFGFile *)malloc( sizeof( CFGFile ) );
        if( !cfgFile )
                return NULL;
        cfgFile->signature = CFGFILE_SIGNATURE;
        cfgFile->name = (char *)malloc( strlen( name ) + 1 );
        if( !cfgFile->name )
        {
                free( cfgFile );
                return NULL;
        }
        strcpy( cfgFile->name, name );
        cfgFile->sectionList = NULL;
        cfgFile->sectionEnd = &cfgFile->sectionList;
        cfgFile->changed = 0;

        if( create )
        {
                fp = fopen( name, "wt+" );
        }
        else
        {
                fp = fopen( name, "rt+" );
        }
        if( !fp )
                goto Error;

        line = 0;
        section = NULL;
        while( !feof( fp ) && !ferror( fp ) )
        {
                line++;
                if( !fgets( lineBuff, sizeof( lineBuff ), fp ) )
                        continue;
                start = Clean( lineBuff );

                if( *start == '[' )
                {
                        p = strchr( start, ']' );
                        if( !p )
                        {
                                printf( "Error missing ']' on line %d\n", line );
                                continue;
                        }
                        *p = '\0';
                        start++;
                        if( !strlen( start ) )
                        {
                                printf( "Error empty section name on line %d\n", line );
                                continue;
                        }
                        if( FindSection( cfgFile, start ) )
                        {
                                printf( "Error duplicate section on line %d\n", line );
                                continue;
                        }
                        section = AddSection( cfgFile, start );
                }
                else
                {
                        if( !section )
                                section = AddSection( cfgFile, NULL );
                        if( !section )
                        {
                                printf( "Error allocating NULL section on line %d\n", line );
                        }

                        if( !*start )
                        {
                                AddItem( section, NULL, NULL );
                                continue;
                        }
                        if( *start == COMMENT_BYTE )
                        {
                                AddItem( section, NULL, start+1 );
                                continue;
                        }

                        p = strchr( start, '=' );
                        if( !p )
                        {
                                printf( "Error missing '=' on line %d\n", line );
                                continue;
                        }
                        *p = '\0';
                        p++;
                        start = Clean( start );
                        if( !strlen( start ) )
                        {
                                printf( "Error empty item name on line %d\n", line );
                                continue;
                        }
                        if( FindItem( section, start ) )
                        {
                                printf( "Error duplicate item on line %d\n", line );
                                continue;
                        }
                        value = Clean( p );
                        AddItem( section, start, value );
                }
        }
        fclose( fp );

        return cfgFile;

Error:
        XplCFGClose( cfgFile );
        return NULL;
}

EXPORT int XplCFGFlush( CFGFile *cfgFile )
{
        FILE *fp;
        CFGSection *section;
        CFGItem *item;

        if( cfgFile->changed )
        {
                fp = fopen( cfgFile->name, "wt" );
                if( fp )
                {
                        for(section=cfgFile->sectionList; section; section=section->next)
                        {
                                if( section->name )
                                {
                                        fprintf( fp, "[%s]\n", section->name );
                                }
                                for(item=section->itemList; item; item=item->next)
                                {
                                        if( item->name )
                                                fprintf( fp, "%s=%s\n", item->name, item->value );
                                        else if( item->value )
                                                fprintf( fp, "%c%s\n", COMMENT_BYTE, item->value );
                                        else
                                                fprintf( fp, "\n" );
                                }
                        }
                        fclose( fp );
                }
                cfgFile->changed = 0;
        }
        return 0;
}

EXPORT int XplCFGClose( CFGFile *cfgFile )
{
        CFGSection *section;
        CFGItem *item;

        if( !cfgFile || cfgFile->signature != CFGFILE_SIGNATURE )
                return -1;

        XplCFGFlush( cfgFile );

        cfgFile->signature = 0;

        while( cfgFile->sectionList )
        {
                section = cfgFile->sectionList;
                while( section->itemList )
                {
                        item = section->itemList;
                        if( item->name )
                                free( item->name );
                        if( item->value )
                                free( item->value );
                        section->itemList = item->next;
                        free( item );
                }
                if( section->name )
                        free( section->name );
                cfgFile->sectionList = section->next;
                free( section );
        }
        if( cfgFile->name )
                free( cfgFile->name );
        free( cfgFile );

        return 0;
}

// cfgFile              must be valid
// sectionName  may be NULL or a valid section name
// name must be valid
// returns NULL on error or a pointer to the value associated with name

EXPORT char *XplCFGGetValue( CFGFile *cfgFile, char *sectionName, char *name )
{
        CFGSection *section;
        CFGItem *item;

        if( !cfgFile || cfgFile->signature != CFGFILE_SIGNATURE )
                return NULL;

        section = FindSection( cfgFile, sectionName );
        if( !section )
                return NULL;
        item = FindItem( section, name );
        if( !item )
                return NULL;
        return item->value;
}

// cfgFile              must be valid
// sectionName  may be NULL or a section name, sections that don't exist will
// be added to the end of the file.
//
// name and value defined as follows
//
// name                 value           file contents
// ----------------------------------------
// valid                        NULL            INVALID
// valid                        valid           name = value (replaces if name exists or adds to end)
// NULL                 valid           value = comment (adds to end of section)
// NULL                 NULL            blank line (adds to end of section)

EXPORT int XplCFGSetValue( CFGFile *cfgFile, char *sectionName, char *name, char *value )
{
        CFGSection *section;
        CFGItem *item;
        char *temp;

        if( !cfgFile || cfgFile->signature != CFGFILE_SIGNATURE )
                return -1;

        section = FindSection( cfgFile, sectionName );
        if( !section )
        {
                section = AddSection( cfgFile, sectionName );
                if( !section )
                        return -1;
                cfgFile->changed = 1;
        }

        if( name )
        {
                if( !value )
                        return -1;

                item = FindItem( section, name );
                if( !item )
                {
                        item = AddItem( section, name, value );
                        if( !item )
                                return -1;
                        cfgFile->changed = 1;
                        return 0;
                }
                if( !(temp = (char *)malloc( strlen( value ) + 1 ) ) )
                        return -1;

                strcpy( temp, value );
                free( item->value );
                item->value = temp;

				cfgFile->changed = 1;
                return 0;
        }

        AddItem( section, NULL, value );
        return 0;
}
