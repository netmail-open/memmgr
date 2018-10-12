#ifndef XPLOLD_H
#define XPLOLD_H

#ifdef __cplusplus
extern "C"{
#endif

/**********************
  Crypto Locks
 **********************/
void XPLCryptoLockInit(void);
void XPLCryptoLockDestroy(void);

#define NWDOS_NAME_SPACE     1
#define NWOS2_NAME_SPACE     6

#define XPLLongToDos(In, Out, OutSize)            strlen(strcpy(Out, In))
#define XPLDosToLong(In, PrefixLen, Out, OutSize) strlen(strcpy(Out, In + PrefixLen + 1))
#define NWSetNameSpaceEntryName(Path, Type, Newname)
#define NWGetNameSpaceEntryName(Path, Type, Len, Newname)       0
#define SetCurrentNameSpace(Type)
#define SetTargetNameSpace(Type)

#define d_nameDOS d_name

#ifdef __cplusplus
}
#endif

#endif
