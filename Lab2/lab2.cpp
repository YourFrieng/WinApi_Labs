#include <windows.h>
#include <tchar.h> 
#include <stdio.h>
#include <strsafe.h>
#include <wchar.h>
#include <iostream>
#include <string>
#include <atlconv.h>
#include <sstream>
#include <vector>
#include <fileapi.h>
#include <ctime>
#pragma comment(lib, "User32.lib")

#define BUFSIZE MAX_PATH

void DisplayErrorBox(LPTSTR lpszFunction);


VOID CALLBACK FileIOCompletionRoutine(
    DWORD dwErrorCode,
    DWORD dwNumberOfBytesTransfered,
    LPOVERLAPPED lpOverlapped
    ) {
    std::wcout << "IO completed with error code" << dwErrorCode << std::endl;

}// ôóíêö³ÿ çàâåðøåííÿ


int ReadDir(int argc, TCHAR* argv[])
{
    WIN32_FIND_DATA ffd;
    LARGE_INTEGER filesize;
    TCHAR szDir[MAX_PATH];
    size_t length_of_arg;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    DWORD dwError = 0;

    // If the directory is not specified as a command-line argument,
    // print usage.

    if (argc != 2)
    {
        _tprintf(TEXT("\nUsage: %s <directory name>\n"), argv[0]);
        return (-1);
    }

    // Check that the input path plus 3 is not longer than MAX_PATH.
    // Three characters are for the "\*" plus NULL appended below.

    StringCchLength(argv[1], MAX_PATH, &length_of_arg);

    if (length_of_arg > (MAX_PATH - 3))
    {
        _tprintf(TEXT("\nDirectory path is too long.\n"));
        return (-1);
    }

    _tprintf(TEXT("\nTarget directory is %s\n\n"), argv[1]);

    // Prepare string for use with FindFile functions.  First, copy the
    // string to a buffer, then append '\*' to the directory name.

    StringCchCopy(szDir, MAX_PATH, argv[1]);
    StringCchCat(szDir, MAX_PATH, TEXT("\\*"));

    // Find the first file in the directory.

    hFind = FindFirstFile(szDir, &ffd);

    if (INVALID_HANDLE_VALUE == hFind)
    {
        DisplayErrorBox(LPTSTR(L"FindFirstFile"));
        return dwError;
    }

    // List all the files in the directory with some info about them.

    do
    {
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            _tprintf(TEXT("  %s   <DIR>\n"), ffd.cFileName);
        }
        else
        {
            filesize.LowPart = ffd.nFileSizeLow;
            filesize.HighPart = ffd.nFileSizeHigh;
            _tprintf(TEXT("  %s   %ld bytes\n"), ffd.cFileName, filesize.QuadPart);
        }
    } while (FindNextFile(hFind, &ffd) != 0);

    dwError = GetLastError();
    if (dwError != ERROR_NO_MORE_FILES)
    {
        DisplayErrorBox(LPTSTR(L"FindFirstFile"));
    }

    FindClose(hFind);
    return dwError;
}


void DisplayErrorBox(LPTSTR lpszFunction)
{
    // Retrieve the system error message for the last-error code

    LPVOID lpMsgBuf;
    LPVOID lpDisplayBuf;
    DWORD dw = GetLastError();

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR)&lpMsgBuf,
        0, NULL);

    // Display the error message and clean up

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
        (lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
    StringCchPrintf((LPTSTR)lpDisplayBuf,
        LocalSize(lpDisplayBuf) / sizeof(TCHAR),
        TEXT("%s failed with error %d: %s"),
        lpszFunction, dw, lpMsgBuf);
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

    LocalFree(lpMsgBuf);
    LocalFree(lpDisplayBuf);
}

const std::wstring& Wrong_Command_Format(const std::wstring& command) {
    return L"Command " + command + L" must be like this: " + command + L" <Folder_Name>\n";
}



void CommandWorker(const std::wstring& action) {
    std::wstringstream ss;
    ss << action;
    std::wstring command, event;
    ss >> command;

    if (command == L"cd") {
        if (!ss.eof()) {
            ss.ignore(1);
            ss >> event;
            SetCurrentDirectory(event.c_str());
        }
        else
            std::wcout << Wrong_Command_Format(command);
    }
    else if (command == L"open") {
        if (!ss.eof()) {
            ss.ignore(1);
            ss >> event;
            
            HANDLE _File = CreateFile(
                event.c_str(),
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL);

            if (_File) {
                std::string Variant = "FI-94 V2";
                unsigned long FileSize = 0;
                FileSize = GetFileSize(_File, NULL);
                std::wcout << "Filesize " << event << " : " << FileSize << "bytes" << std::endl;
                SetFilePointer(_File, FileSize, NULL, FILE_BEGIN);
                LPDWORD bytes_was_writen = 0;
                if (WriteFile(_File, L"FI-94 V2", 20, bytes_was_writen, NULL))
                    std::wcout << "Write into file completed" << std::endl;
            }

            CloseHandle(_File);
        }
    }
    else if (command == L"copy") {
        if (!ss.eof()) {
            std::wstring from, to, type;
            ss.ignore(1);
            ss >> from;
            ss.ignore(1);
            if (*(from.end() - 1) != '\\')
                from += '\\';
            ss >> to;
            if (to.empty()) {
                TCHAR CurDir[BUFSIZE];
                GetCurrentDirectory(BUFSIZE, CurDir);
                to = CurDir;
                to += '\\';
            }
            HANDLE hFind = INVALID_HANDLE_VALUE;
            WIN32_FIND_DATA ffd;
            hFind = FindFirstFile((from + L"\\*.*").c_str(), &ffd);
            size_t count_of_files = 0;
            do
            {
                if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    if (CopyFile((from + ffd.cFileName).c_str(), (to + ffd.cFileName).c_str(), 0)) {
                        std::wcout << "Copied " << ffd.cFileName << std::endl;
                        ++count_of_files;
                    }
                    else
                        std::wcout << "Error: " << GetLastError() << " in file: " << ffd.cFileName << std::endl;
                }
            } while (FindNextFile(hFind, &ffd) != 0);

            std::wcout << count_of_files << " files were copied" << std::endl;
        }
        else
            std::wcout << Wrong_Command_Format(command);
    }
    else if (command == L"find") {
        if (!ss.eof()) {
            ss.ignore(1);
            ss >> event;
            size_t count_of_files = 0;
            TCHAR CurDir[BUFSIZE];
            GetCurrentDirectory(BUFSIZE, CurDir);
            HANDLE hFind = INVALID_HANDLE_VALUE;
            std::wstring file = CurDir;
            file += '\\';
            file += event;
            WIN32_FIND_DATA ffd;
            hFind = FindFirstFile(file.c_str(), &ffd);
            do
            {
                if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    std::wcout << "File: " << ffd.cFileName << std::endl;
                    ++count_of_files;
                }
            } while (FindNextFile(hFind, &ffd) != 0);
            std::wcout << count_of_files << " files were find with: " << event << std::endl;
        }
    }
    else if (command == L"mkfile") {
        if (!ss.eof()) {
            ss.ignore(1);
            ss >> event;
            CreateFile(
                event.c_str(), 
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL);
        }
        else
            std::wcout << Wrong_Command_Format(command);

    }
    else if (command == L"flags") {
        if (!ss.eof()) {
            ss.ignore(1);
            ss >> event;
            WIN32_FIND_DATA FindFileData;
            HANDLE File = FindFirstFile((LPCWSTR)event.c_str(), &FindFileData);
            if (File == NULL) {
                File = CreateFile(
                event.c_str(),
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL,
                    CREATE_ALWAYS,
                    FILE_ATTRIBUTE_NORMAL,
                    NULL);
            }
            SetFileAttributes(event.c_str(), FILE_ATTRIBUTE_HIDDEN);
            SYSTEMTIME st;
            FILETIME ft;
            GetSystemTime(&st);
            SystemTimeToFileTime(&st, &ft);
            SetFileTime(File, &ft, (LPFILETIME)NULL, (LPFILETIME)NULL);
        }
    }
    else if (command == L"read") {
        if (!ss.eof()) {
            ss.ignore(1);
            ss >> event;
            HANDLE _File = CreateFile(
            event.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_OVERLAPPED,
            NULL); 
            BYTE Buffer[MAX_PATH];
            OVERLAPPED overlapped;
            ZeroMemory(&overlapped, sizeof(overlapped));
            overlapped.Offset = 0/*GetFileSize(_File, &overlapped.OffsetHigh)*/;

            if (_File != INVALID_HANDLE_VALUE) {
                DWORD buf;
                if (!ReadFileEx(_File, Buffer, 1, &overlapped, &FileIOCompletionRoutine))
                    std::wcout << "Error: " << GetLastError() << std::endl;
                else {
                    std::wcout << Buffer << std::endl;
                    DWORD Res = SleepEx(15000, TRUE); // ôóíêö³ÿ ÷åêàííÿ

                    std::wcout << "WriteFileEx: success" << std::endl;
                    switch (Res) {
                    case 0:
                        std::wcout << "SleepEx: Timeout" << std::endl;
                        break;
                    case WAIT_IO_COMPLETION:
                        std::wcout << "SleepEx: IO completion" << std::endl;
                        break;
                    default:
                        std::wcout << false << std::endl;

                    }
                }
            
        }

            CloseHandle(overlapped.hEvent);
            CloseHandle(_File);

        }
    }
    else if (command == L"lock") {
        if (!ss.eof()) {
            ss.ignore(1);
            ss >> event;
            HANDLE _File = CreateFile(
                event.c_str(),
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL);
            OVERLAPPED overlapped;
            overlapped.hEvent = NULL;
            if (_File != INVALID_HANDLE_VALUE) {
                SetFilePointer(_File, 0, NULL, FILE_BEGIN);
                if (LockFileEx(_File, LOCKFILE_EXCLUSIVE_LOCK, NULL, 1024, 1024, &overlapped))
                    std::wcout << "Lock completed" << std::endl;
            }
        }
    }
    else if (command == L"mkdir") {
        if (ss.eof()) {
               ss.ignore(1);
               ss >> event;
               if (CreateDirectory(L".\\FILE11", NULL))
                   std::cout << "Dir FILE11 created\n";
               if (CreateDirectory(L".\\FILE11\\FILE12", NULL))
                   std::cout << "Dir FILE12 created\n";
               if (CreateDirectory(L".\\FILE11\\FILE12\\FILE13", NULL))
                   std::cout << "Dir FILE13 created\n";
               if (CreateDirectory(L".\\FILE21", NULL))
                   std::cout << "Dir FILE21 created\n";
        }
        else
            std::wcout << Wrong_Command_Format(command);
    }
    else if (command == L"drives") {
        wchar_t arr[1024], * b;
        GetLogicalDriveStringsW(sizeof(arr), arr);
        b = arr;
        for (size_t i = 0; arr[i] != '\0'; i += 4)
            std::cout << (char)arr[i] << ":\\" << std::endl;
            
   
    }
    else if (command == L"ls") {
            if (!ss.eof()) {
                ss.ignore(1);
                std::vector<std::wstring> substrs;
                TCHAR CurDir[BUFSIZE];
                while (!ss.eof())
                {
                    std::getline(ss, event, L'\\');
                    substrs.push_back(event);
                }
                TCHAR** buf = new TCHAR * [substrs.size() + 1];
                //buf[0] = W2T((LPWSTR)drive.c_str());
                buf[0] = new TCHAR[BUFSIZE];
                GetCurrentDirectory(BUFSIZE, buf[0]);
                GetCurrentDirectory(BUFSIZE, CurDir);
                
                for (size_t i = 0; i < substrs.size(); ++i)
                {
                    std::wstring temp = CurDir;
                    for (size_t j = 0; j < i; ++j)
                    {
                        temp += L'\\' + substrs[j];
                    }
                    buf[1] = W2T((LPWSTR)temp.c_str());

                    ReadDir(2, buf);
                    
                }
            }
            else {
                TCHAR** buf = new TCHAR * [2];
                buf[0] = new TCHAR[BUFSIZE];
                //buf[0] = W2T((LPWSTR)drive.c_str());
                GetCurrentDirectory(BUFSIZE, buf[0]);
                //buf[1] = W2T((LPWSTR)L"");
                buf[1] = new TCHAR[BUFSIZE];
                GetCurrentDirectory(BUFSIZE, buf[1]);
                ReadDir(2, buf);
            }
    }
    else if (command == L"exit") {
    throw(std::string("Exit command, bye!\n"));
    }
    else {
        if (!command.empty()) {
            std::wcout << L"Unknown command: " + command;
        }
    }
}



int main() {
    TCHAR Buffer[BUFSIZE];
    DWORD dwRet;
    std::wstring command;
    dwRet = GetCurrentDirectory(BUFSIZE, Buffer);
    std::wcout << Buffer << ": ";
    while (std::getline(std::wcin, command))
    {
        try
        {

            CommandWorker(command);
            dwRet = GetCurrentDirectory(BUFSIZE, Buffer);
            std::wcout << Buffer << ": ";
        }
        catch (const std::string& e)
        {
             std::cout << e << std::endl;
            break;
        }
        catch (...) {
            std::cout << "Unknown signal 6" << std::endl;
            break;
        }
    }

    return 0;
}
