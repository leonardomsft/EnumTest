#include <windows.h>
#include <stdio.h>
#include <strsafe.h>
#include <chrono>

//Globals
BOOL bReadTarget = FALSE;
BOOL bRecurse = FALSE;
int intTestType = 1;

long int intObjectsTotal = 0;
long int intObjectsCurrent = 0;
long int intObjectsError = 0;
long int intLastObjectsTotal = 0;
long int intObjectsPerPeriod = 0;

//Prototypes
void DisplayError(LPWSTR lpszFunction);
int EnumDir(LPWSTR szDir);
int GetObjectsTotal(LPWSTR szDir);
void ReadTargetFile(LPWSTR szFile, DWORD dwSizeinBytes);
void GetTargetAttributes(LPWSTR szFile);
int ReadOwner(LPWSTR szFile);
BOOL ReporterThread(int RefRate);

int wmain(int argc, WCHAR* argv[])
{
    HANDLE hReporterThread;
    WCHAR szTarget[MAX_PATH];
    size_t length_of_arg;
    int intRefRate = 0;

    //Start the timer
    auto start = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed;

    wprintf(L"\nEnumTest.exe - Directory Enumeration Test Utility \n");
    wprintf(L"Created by Leonardo Fagundes. No rights reserved.\n\n");


    //Expect 3 arguments
    
    if (argc != 4)
    {
        wprintf(L"\nUsage: EnumTest.exe <directory name> <refresh rate in seconds>  <test type>\n");
        wprintf(L"\nTest Types:\n  1 - Enumeration only\n  2 - Enumeration + Atributes\n");
        wprintf(L"\nExample: EnumTest.exe ""\\SERVER01\Docs"" 1 2\n\n");

        return (-1);
    }

    //Directory name + 3 must not be longer than MAX_PATH (trailing "\*" + NULL needs to be appended)

    StringCchLengthW(argv[1], MAX_PATH, &length_of_arg);

    if (length_of_arg > (MAX_PATH - 3))
    {
        wprintf(L"\nDirectory path is too long.\n");

        return (-1);
    }

    intRefRate = _wtoi(argv[2]);

    intTestType = _wtoi(argv[3]);


    if (intTestType < 1 || intTestType > 2)
    {
        wprintf(L"\nInvalid Test Type.\n");

        return (-1);

    }
         

    StringCchCopyW(szTarget, MAX_PATH, argv[1]);


    //Obtain objects count
    GetObjectsTotal(szTarget);


    //Launch reporter thread
    hReporterThread = CreateThread(
        NULL,
        NULL,
        (LPTHREAD_START_ROUTINE)ReporterThread,
        (LPVOID)intRefRate,
        NULL,
        NULL);

    if (NULL == hReporterThread) {

        wprintf(L"CreateThread failed: 0x%X\n", GetLastError());

        return -1;
    }


    //Run the enumeration
    EnumDir(szTarget);

    CloseHandle(hReporterThread);

    //Stop the timer
    auto finish = std::chrono::high_resolution_clock::now();
    elapsed = finish - start;


    wprintf(L"\n--------------- Test Results ---------------\n");
    wprintf(L"Total Objects found: %d\n", intObjectsCurrent);
    wprintf(L"Total Errors: %d\n", intObjectsError);
    wprintf(L"Total elapsed time: %0.1f seconds\n\n", elapsed);
}



int EnumDir(LPWSTR szTarget)
{
    WCHAR szDir[MAX_PATH];
    WCHAR szNewTarget[MAX_PATH];
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW ffd;
    LARGE_INTEGER filesize;


    //wprintf(L"\nDirectory is %s\n\n", szTarget);

    //Prepare the string
    StringCchCopyW(szDir, MAX_PATH, szTarget);
    StringCchCatW(szDir, MAX_PATH, L"\\*");


    // Find the first file in the directory

    hFind = FindFirstFileW(szDir, &ffd);

    if (INVALID_HANDLE_VALUE == hFind)
    {
        //wprintf(L"ERROR: 0x%X\n", GetLastError());
        //DisplayError((LPWSTR)L"FindFirstFile");

        intObjectsError++;

        return -1;
    }

    // List all the files in the directory with some info about them.

    do
    {
        StringCchCopyW(szNewTarget, MAX_PATH, szTarget);
        StringCchCatW(szNewTarget, MAX_PATH, L"\\");
        StringCchCatW(szNewTarget, MAX_PATH, ffd.cFileName);


        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            //wprintf(L"  %s   <DIR>\n", ffd.cFileName);

            if (wcscmp(ffd.cFileName, L".") && wcscmp(ffd.cFileName, L".."))
            {
                //Recurse here
                if (bRecurse)
                    EnumDir(szNewTarget);

            }

        }
        else
        {
            filesize.LowPart = ffd.nFileSizeLow;
            filesize.HighPart = ffd.nFileSizeHigh;
            //wprintf(L"  %s   %ld bytes\n", ffd.cFileName, filesize.QuadPart);


            if (intTestType == 2)
                GetTargetAttributes(szNewTarget);


            if (bReadTarget)
                ReadTargetFile(szNewTarget, filesize.QuadPart);


        }



        //increment
        intObjectsCurrent++;

    } while (FindNextFileW(hFind, &ffd) != 0);


    FindClose(hFind);

}

int GetObjectsTotal(LPWSTR szTarget)
{
    WCHAR szDir[MAX_PATH];
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW ffd;
    long int intCount = 0;
    WCHAR szNewTarget[MAX_PATH];


    //Prepare the string
    StringCchCopyW(szDir, MAX_PATH, szTarget);
    StringCchCatW(szDir, MAX_PATH, L"\\*");


    // Find the first file in the directory

    hFind = FindFirstFileW(szDir, &ffd);

    if (INVALID_HANDLE_VALUE == hFind)
    {
        return -1;
    }

    // Count all the files in the directory 

    do
    {
        StringCchCopyW(szNewTarget, MAX_PATH, szTarget);
        StringCchCatW(szNewTarget, MAX_PATH, L"\\");
        StringCchCatW(szNewTarget, MAX_PATH, ffd.cFileName);


        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            //wprintf(L"  %s   <DIR>\n", ffd.cFileName);

            if (wcscmp(ffd.cFileName, L".") && wcscmp(ffd.cFileName, L".."))
            {
                //Recurse here
                if (bRecurse)
                    GetObjectsTotal(szNewTarget);

            }
        }
        //increment
        intObjectsTotal++;

    } while (FindNextFileW(hFind, &ffd) != 0);


    FindClose(hFind);

}



void GetTargetAttributes(LPWSTR szFile)
{

    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD dwBytesRead = 0;
    LPVOID lpBuff = NULL;
    DWORD dwError = NULL;


    //1. GetFileAttributesW, QueryBasicInformationFile
    dwError = GetFileAttributesW(szFile);

    if (INVALID_FILE_ATTRIBUTES == dwError)
    {
        intObjectsError++;

    }


    //Open a Handle to the file
    hFile = CreateFileW(szFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (INVALID_HANDLE_VALUE == hFile)
    {
        //DisplayError((LPWSTR)L"CreateFileW");

        intObjectsError++;

    }
    else
    {
        //2. GetFileType, QueryDeviceInformationVolume
        dwError = GetFileType(hFile);

        if (FILE_TYPE_UNKNOWN == dwError)
        {
            //There's no way to distinguish between success and failure without calling GetLastError. Sigh...
            if (GetLastError())
            {
                intObjectsError++;

            }

        }

        //3. GetFileSize, QueryStandardInformationFile
        dwError = GetFileSize(hFile, NULL);
        
        if (INVALID_FILE_SIZE == dwError)
        {
            //DisplayError((LPWSTR)L"CreateFileW");

            intObjectsError++;

        }

    }

    CloseHandle(hFile);

    //4. GetFileSecurityW, Information: Owner
    dwError = ReadOwner(szFile);

}


void ReadTargetFile(LPWSTR szFile, DWORD dwSizeinBytes)
{

    HANDLE hFile = INVALID_HANDLE_VALUE;
    DWORD dwBytesRead = 0;
    LPVOID lpBuff = NULL;


    //Open a Handle to the file
    hFile = CreateFileW(szFile, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (INVALID_HANDLE_VALUE == hFile)
    {
        //DisplayError((LPWSTR)L"CreateFileW");

        intObjectsError++;

    }

    //Allocate a buffer
    lpBuff = (LPVOID)LocalAlloc(LMEM_ZEROINIT, dwSizeinBytes);

    if (lpBuff == NULL)
    {
        DisplayError((LPWSTR)L"LocalAlloc");

        CloseHandle(hFile);

    }


    //Read the file
    while (TRUE)
    {
        if (!ReadFile(hFile, lpBuff, dwSizeinBytes, &dwBytesRead, NULL))
        {
            //DisplayError((LPWSTR)L"ReadFile");

            intObjectsError++;

            break;
        }

        // Check for EOF reached
        if (dwBytesRead == 0)
            break;



    }

    LocalFree(lpBuff);
    CloseHandle(hFile);

}


int ReadOwner(LPWSTR szFile)
{
    PSECURITY_DESCRIPTOR pSD = NULL;
    DWORD sdLen = 0;
    LPVOID lpBuff = NULL;


    GetFileSecurity(szFile, OWNER_SECURITY_INFORMATION, pSD, 0, &sdLen);



    //Allocate a buffer
    lpBuff = (LPVOID)LocalAlloc(LMEM_ZEROINIT, sdLen);

    if (lpBuff == NULL)
    {
        DisplayError((LPWSTR)L"LocalAlloc");

        return -1;
    }

    GetFileSecurity(szFile, OWNER_SECURITY_INFORMATION, pSD, sdLen, &sdLen);


    LocalFree(lpBuff);

}


void DisplayError(LPWSTR lpszFunction)
{
    // translate error codes

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();

    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&lpMsgBuf,
        0, NULL);

    // Display the error message and clean up

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, (lstrlenW((LPCWSTR)lpMsgBuf) + lstrlenW((LPCWSTR)lpszFunction) + 40) * sizeof(WCHAR));
    
    StringCchPrintfW((LPWSTR)lpDisplayBuf, 
        LocalSize(lpDisplayBuf) / sizeof(WCHAR), 
        L"ERROR: Function %s failed with error 0x%X: %s",
        lpszFunction, dw, lpMsgBuf);

    wprintf(L"%s\n", lpDisplayBuf);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
}


BOOL ReporterThread(int RefRate)
{
    if (RefRate == 0) RefRate = 1;
    int intRefreshRate = RefRate * 1000;
    float PercentCompletion = 0;
    float EstimatedCompletion = 0;
    int intLoopCount = 1;
    int intAverageCount = 0;

    //Start the timer
    auto start = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed;
    

    wprintf(L"Total objects in the directory: %d\n\n", intObjectsTotal);



    while (TRUE)
    {
        intLastObjectsTotal = intObjectsCurrent + 1;

        Sleep(intRefreshRate);

        intObjectsPerPeriod = intObjectsCurrent - intLastObjectsTotal;

        intAverageCount = intObjectsCurrent / intLoopCount;

        PercentCompletion = ((float)intObjectsCurrent / (float)intObjectsTotal) * 100;

        EstimatedCompletion = (((float)intObjectsTotal / (float)intAverageCount) * intRefreshRate) / 1000 ;


        //Stop the timer
        auto finish = std::chrono::high_resolution_clock::now();
        elapsed = finish - start;

        //estimated = elapsed

        wprintf(L"  # of objects enumerated: %d (%0.0f%%)\n", intObjectsCurrent, PercentCompletion);
        wprintf(L"  # of objects every %d seconds : %d (%d on average)\n", RefRate, intObjectsPerPeriod, intAverageCount);
        wprintf(L"  Partial elapsed time: %0.0f seconds (estimated completion time: %0.0f seconds)\n\n", elapsed, EstimatedCompletion);

        intLoopCount++;
    }
}