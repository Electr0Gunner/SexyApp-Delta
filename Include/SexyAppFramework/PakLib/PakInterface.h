#pragma once

#include <list>
#include <map>
#include <string>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define PAK_MAGIC_WORD 0xBAC04AC0

class PakCollection;

class PakRecord
{
public:
	PakCollection* mCollection;
	std::string mFileName;
	FILETIME mFileTime;
	int mStartPos;
	int mSize;
};

typedef std::map<std::string, PakRecord> PakRecordMap;

class PakCollection
{
public:
	HANDLE mFileHandle;
	HANDLE mMappingHandle;
	void* mDataPtr;
};

typedef std::list<PakCollection> PakCollectionList;

struct PFILE
{
	PakRecord* mRecord;
	int mPos;
	FILE* mFP;
};

struct PFindData
{
	HANDLE mWHandle;
	std::string mLastFind;
	std::string mFindCriteria;
};

class PakInterfaceBase
{
public:
	virtual PFILE* FOpen(const char* theFileName, const char* theAccess) = 0;
	virtual PFILE* FOpen(const wchar_t* theFileName, const wchar_t* theAccess)
	{
		return nullptr;
	}
	virtual int FClose(PFILE* theFile) = 0;
	virtual int FSeek(PFILE* theFile, long theOffset, int theOrigin) = 0;
	virtual int FTell(PFILE* theFile) = 0;
	virtual size_t FRead(void* thePtr, int theElemSize, int theCount, PFILE* theFile) = 0;
	virtual int FGetC(PFILE* theFile) = 0;
	virtual int UnGetC(int theChar, PFILE* theFile) = 0;
	virtual char* FGetS(char* thePtr, int theSize, PFILE* theFile) = 0;
	virtual wchar_t* FGetS(wchar_t* thePtr, int theSize, PFILE* theFile)
	{
		return thePtr;
	}
	virtual int FEof(PFILE* theFile) = 0;

	virtual HANDLE FindFirstFile(LPCTSTR lpFileName, LPWIN32_FIND_DATA lpFindFileData) = 0;
	virtual BOOL FindNextFile(HANDLE hFindFile, LPWIN32_FIND_DATA lpFindFileData) = 0;
	virtual BOOL FindClose(HANDLE hFindFile) = 0;
};

class PakInterface : public PakInterfaceBase
{
public:
	PakCollectionList mPakCollectionList;
	PakRecordMap mPakRecordMap;

public:
	bool PFindNext(PFindData* theFindData, LPWIN32_FIND_DATA lpFindFileData);

public:
	PakInterface();
	~PakInterface();

	bool AddPakFile(const std::string& theFileName);
	PFILE* FOpen(const char* theFileName, const char* theAccess);
	int FClose(PFILE* theFile);
	int FSeek(PFILE* theFile, long theOffset, int theOrigin);
	int FTell(PFILE* theFile);
	size_t FRead(void* thePtr, int theElemSize, int theCount, PFILE* theFile);
	int FGetC(PFILE* theFile);
	int UnGetC(int theChar, PFILE* theFile);
	char* FGetS(char* thePtr, int theSize, PFILE* theFile);
	int FEof(PFILE* theFile);

	HANDLE FindFirstFile(LPCTSTR lpFileName, LPWIN32_FIND_DATA lpFindFileData);
	BOOL FindNextFile(HANDLE hFindFile, LPWIN32_FIND_DATA lpFindFileData);
	BOOL FindClose(HANDLE hFindFile);
};

extern PakInterface* gPakInterface;

static HANDLE gPakFileMapping = nullptr;
static PakInterfaceBase** gPakInterfaceP = nullptr;

static PakInterfaceBase* GetPakPtr()
{
	if (gPakFileMapping == nullptr)
	{
		char aName[256];
		sprintf(aName, "gPakInterfaceP_%d", GetCurrentProcessId());
		gPakFileMapping = ::CreateFileMappingA((HANDLE)INVALID_HANDLE_VALUE, nullptr, PAGE_READWRITE, 0, sizeof(PakInterface*), aName);
		gPakInterfaceP = (PakInterfaceBase**)MapViewOfFile(gPakFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(PakInterface*));
	}
	return *gPakInterfaceP;
}

static PFILE* p_fopen(const char* theFileName, const char* theAccess)
{
	if (GetPakPtr() != nullptr)
		return (*gPakInterfaceP)->FOpen(theFileName, theAccess);
	FILE* aFP = fopen(theFileName, theAccess);
	if (aFP == nullptr)
		return nullptr;
	PFILE* aPFile = new PFILE();
	aPFile->mRecord = nullptr;
	aPFile->mPos = 0;
	aPFile->mFP = aFP;
	return aPFile;
}

static PFILE* p_fopen(const wchar_t* theFileName, const wchar_t* theAccess)
{
	if (GetPakPtr() != nullptr)
		return (*gPakInterfaceP)->FOpen(theFileName, theAccess);
	FILE* aFP = _wfopen(theFileName, theAccess);
	if (aFP == nullptr)
		return nullptr;
	PFILE* aPFile = new PFILE();
	aPFile->mRecord = nullptr;
	aPFile->mPos = 0;
	aPFile->mFP = aFP;
	return aPFile;
}

static int p_fclose(PFILE* theFile)
{
	if (GetPakPtr() != nullptr)
		return (*gPakInterfaceP)->FClose(theFile);
	int aResult = fclose(theFile->mFP);
	delete theFile;
	return aResult;
}

static int p_fseek(PFILE* theFile, long theOffset, int theOrigin)
{
	if (GetPakPtr() != nullptr)
		return (*gPakInterfaceP)->FSeek(theFile, theOffset, theOrigin);
	return fseek(theFile->mFP, theOffset, theOrigin);
}

static int p_ftell(PFILE* theFile)
{
	if (GetPakPtr() != nullptr)
		return (*gPakInterfaceP)->FTell(theFile);
	return ftell(theFile->mFP);
}

static size_t p_fread(void* thePtr, int theSize, int theCount, PFILE* theFile)
{
	if (GetPakPtr() != nullptr)
		return (*gPakInterfaceP)->FRead(thePtr, theSize, theCount, theFile);
	return fread(thePtr, theSize, theCount, theFile->mFP);
}

static size_t p_fwrite(const void* thePtr, int theSize, int theCount, PFILE* theFile)
{
	if (theFile->mFP == nullptr)
		return 0;
	return fwrite(thePtr, theSize, theCount, theFile->mFP);
}

static int p_fgetc(PFILE* theFile)
{
	if (GetPakPtr() != nullptr)
		return (*gPakInterfaceP)->FGetC(theFile);
	return fgetc(theFile->mFP);
}

static int p_ungetc(int theChar, PFILE* theFile)
{
	if (GetPakPtr() != nullptr)
		return (*gPakInterfaceP)->UnGetC(theChar, theFile);
	return ungetc(theChar, theFile->mFP);
}

static char* p_fgets(char* thePtr, int theSize, PFILE* theFile)
{
	if (GetPakPtr() != nullptr)
		return (*gPakInterfaceP)->FGetS(thePtr, theSize, theFile);
	return fgets(thePtr, theSize, theFile->mFP);
}

static wchar_t* p_fgets(wchar_t* thePtr, int theSize, PFILE* theFile)
{
	if (GetPakPtr() != nullptr)
		return (*gPakInterfaceP)->FGetS(thePtr, theSize, theFile);
	return fgetws(thePtr, theSize, theFile->mFP);
}

static int p_feof(PFILE* theFile)
{
	if (GetPakPtr() != nullptr)
		return (*gPakInterfaceP)->FEof(theFile);
	return feof(theFile->mFP);
}

static HANDLE p_FindFirstFile(LPCTSTR lpFileName, LPWIN32_FIND_DATA lpFindFileData)
{
	if (GetPakPtr() != nullptr)
		return (*gPakInterfaceP)->FindFirstFile(lpFileName, lpFindFileData);
	return FindFirstFile(lpFileName, lpFindFileData);
}

static BOOL p_FindNextFile(HANDLE hFindFile, LPWIN32_FIND_DATA lpFindFileData)
{
	if (GetPakPtr() != nullptr)
		return (*gPakInterfaceP)->FindNextFile(hFindFile, lpFindFileData);
	return FindNextFile(hFindFile, lpFindFileData);
}

static BOOL p_FindClose(HANDLE hFindFile)
{
	if (GetPakPtr() != nullptr)
		return (*gPakInterfaceP)->FindClose(hFindFile);
	return FindClose(hFindFile);
}
