#include <Gosu/Platform.hpp>
#if defined(GOSU_IS_WIN)

#include <Gosu/IO.hpp>
#include <Gosu/Utility.hpp>
#include <cassert>
#include <cstdio>
#include <windows.h>

// Adapted from http://www.codeproject.com/KB/GDI/xfont.aspx.
// Kudos to Philip Patrick and Hans Dietrich!

struct FONT_PROPERTIES_ANSI
{
    char csName[1024];
    char csCopyright[1024];
    char csTrademark[1024];
    char csFamily[1024];
};
struct TT_OFFSET_TABLE
{
    USHORT    uMajorVersion;
    USHORT    uMinorVersion;
    USHORT    uNumOfTables;
    USHORT    uSearchRange;
    USHORT    uEntrySelector;
    USHORT    uRangeShift;
};
struct TT_TABLE_DIRECTORY
{
    char    szTag[4];            //table name
    ULONG    uCheckSum;            //Check sum
    ULONG    uOffset;            //Offset from beginning of file
    ULONG    uLength;            //length of the table in bytes
};
struct TT_NAME_TABLE_HEADER
{
    USHORT    uFSelector;            //format selector. Always 0
    USHORT    uNRCount;            //Name Records count
    USHORT    uStorageOffset;        //Offset for strings storage, from start of the table
};
struct TT_NAME_RECORD
{
    USHORT    uPlatformID;
    USHORT    uEncodingID;
    USHORT    uLanguageID;
    USHORT    uNameID;
    USHORT    uStringLength;
    USHORT    uStringOffset;    //from start of storage area
};

#define SWAPWORD(x)        MAKEWORD(HIBYTE(x), LOBYTE(x))
#define SWAPLONG(x)        MAKELONG(SWAPWORD(HIWORD(x)), SWAPWORD(LOWORD(x)))

#define _T(x) x
#define TRACE printf

namespace Gosu
{
std::string get_name_from_ttf_file(const std::string& filename)
{
    FONT_PROPERTIES_ANSI fp;
    FONT_PROPERTIES_ANSI * lpFontProps = &fp;
    memset(lpFontProps, 0, sizeof(FONT_PROPERTIES_ANSI));

    Buffer buffer;
    load_file(buffer, filename);

    // get the file size
    DWORD dwFileSize = buffer.size();
    LPBYTE lpMapAddress = (LPBYTE) buffer.data();

    BOOL bRetVal = FALSE;
    int index = 0;

    TT_OFFSET_TABLE ttOffsetTable;
    memcpy(&ttOffsetTable, &lpMapAddress[index], sizeof(TT_OFFSET_TABLE));
    index += sizeof(TT_OFFSET_TABLE);

    ttOffsetTable.uNumOfTables = SWAPWORD(ttOffsetTable.uNumOfTables);
    ttOffsetTable.uMajorVersion = SWAPWORD(ttOffsetTable.uMajorVersion);
    ttOffsetTable.uMinorVersion = SWAPWORD(ttOffsetTable.uMinorVersion);

    //check is this is a true type font and the version is 1.0
    if (ttOffsetTable.uMajorVersion != 1 || ttOffsetTable.uMinorVersion != 0) {
        throw std::runtime_error("Only version 1.0 of the TTF file format is supported");
    }

    TT_TABLE_DIRECTORY tblDir;
    memset(&tblDir, 0, sizeof(TT_TABLE_DIRECTORY));
    BOOL bFound = FALSE;
    char szTemp[4096];
    memset(szTemp, 0, sizeof(szTemp));

    for (int i = 0; i< ttOffsetTable.uNumOfTables; i++)
    {
        memcpy(&tblDir, &lpMapAddress[index], sizeof(TT_TABLE_DIRECTORY));
        index += sizeof(TT_TABLE_DIRECTORY);

        strncpy(szTemp, tblDir.szTag, 4);
        if (stricmp(szTemp, "name") == 0)
        {
            bFound = TRUE;
            tblDir.uLength = SWAPLONG(tblDir.uLength);
            tblDir.uOffset = SWAPLONG(tblDir.uOffset);
            break;
        }
        else if (szTemp[0] == 0)
        {
            break;
        }
    }

    if (bFound)
    {
        index = tblDir.uOffset;

        TT_NAME_TABLE_HEADER ttNTHeader;
        memcpy(&ttNTHeader, &lpMapAddress[index], sizeof(TT_NAME_TABLE_HEADER));
        index += sizeof(TT_NAME_TABLE_HEADER);

        ttNTHeader.uNRCount = SWAPWORD(ttNTHeader.uNRCount);
        ttNTHeader.uStorageOffset = SWAPWORD(ttNTHeader.uStorageOffset);
        TT_NAME_RECORD ttRecord;
        bFound = FALSE;

        for (int i = 0;
             i < ttNTHeader.uNRCount &&
             (lpFontProps->csCopyright[0] == 0 ||
              lpFontProps->csName[0] == 0      ||
              lpFontProps->csTrademark[0] == 0 ||
              lpFontProps->csFamily[0] == 0);
             i++)
        {
            memcpy(&ttRecord, &lpMapAddress[index], sizeof(TT_NAME_RECORD));
            index += sizeof(TT_NAME_RECORD);

            ttRecord.uNameID = SWAPWORD(ttRecord.uNameID);
            ttRecord.uStringLength = SWAPWORD(ttRecord.uStringLength);
            ttRecord.uStringOffset = SWAPWORD(ttRecord.uStringOffset);

            if (ttRecord.uNameID == 1 || ttRecord.uNameID == 0 || ttRecord.uNameID == 7)
            {
                int nPos = index;

                index = tblDir.uOffset + ttRecord.uStringOffset + ttNTHeader.uStorageOffset;

                memset(szTemp, 0, sizeof(szTemp));

                memcpy(szTemp, &lpMapAddress[index], ttRecord.uStringLength);
                index += ttRecord.uStringLength;

                if (szTemp[0] != 0)
                {
                    assert (strlen(szTemp) < sizeof(lpFontProps->csName));

                    switch (ttRecord.uNameID)
                    {
                        case 0:
                            if (lpFontProps->csCopyright[0] == 0)
                                strncpy(lpFontProps->csCopyright, szTemp,
                                    sizeof(lpFontProps->csCopyright)-1);
                            break;

                        case 1:
                            if (lpFontProps->csFamily[0] == 0)
                                strncpy(lpFontProps->csFamily, szTemp,
                                    sizeof(lpFontProps->csFamily)-1);
                            bRetVal = TRUE;
                            break;

                        case 4:
                            if (lpFontProps->csName[0] == 0)
                                strncpy(lpFontProps->csName, szTemp,
                                    sizeof(lpFontProps->csName)-1);
                            break;

                        case 7:
                            if (lpFontProps->csTrademark[0] == 0)
                                strncpy(lpFontProps->csTrademark, szTemp,
                                    sizeof(lpFontProps->csTrademark)-1);
                            break;

                        default:
                            break;
                    }
                }
                index = nPos;
            }
        }
    }

    return lpFontProps->csName[0] ? lpFontProps->csName : lpFontProps->csFamily;
}
}

#endif
