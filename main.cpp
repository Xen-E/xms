/*******************************************************************************************
*    Copyright (c) 2023 Xen <xen-dev@pm.me>                                                *
*    This file is part of the Xen Music Sync project, AKA XMS.                             *
*    A tiny tool i made for myself to keep my Music library & removable disks synced.      *
*    Xen Music Sync is free software; you can redistribute it and/or modify it under       *
*    the terms of the GNU Lesser General Public License (LGPL) as published                *
*    by the Free Software Foundation; either version 3 of the License, or                  *
*    (at your option) any later version.                                                   *
*    Xen Music Sync is distributed in the hope that it will be useful, but WITHOUT         *
*    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or                 *
*    FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public                   *
*    License for more details.                                                             *
*    You should have received a copy of the GNU Lesser General Public License              *
*    along with this program. If not, see <https://www.gnu.org/licenses>.                  *
*******************************************************************************************/


#include <fstream>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <vector>
#include <string>
#include <ctime>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#endif

using namespace std;
namespace fs = filesystem;

const wstring MusicLibrary = L"C:/Users/Xen/Music/";     // Your main Music Library here
const wstring RemovableDir = L"F:/Music/";               // Location in Removable Disk
const vector<string> FILTER = {".mp3", ".wav", ".flac"}; // Filter, use comma (,)


struct UnsyncedFile {
    wstring name, humanFileSize;
    fs::path fullPath;
    double bytes;
};


//Creates a CLI progress bar based on given disk capacity & available space
//The ratio var controls the progress bar size.
const wstring PBFromFSSpace(const uintmax_t &capacity,
                            const uintmax_t &free,
                            const unsigned short ratio) {

    wstring progressBar = L"[";
    const unsigned short _ratio = (free * ratio) / capacity;
    const unsigned short left_percent = (free * 100) / capacity;

    for (short i=ratio; i>=0; i--) {
        if (i >= _ratio) progressBar += L"|";
        else progressBar += L"-";
    }
    progressBar += L"] " + to_wstring(left_percent) + L"% Free";
    return progressBar;
}

//Lists directory files recursively using filesystem::recursive_directory_iterator
//file paths need to match filter and also they will be trimmed, example:
//C:/Users/Xen/Music/song.mp3 -> song.mp3
const vector<wstring> listDir(const wstring &dir) {
    vector<wstring> files;

    for (const auto &fileName : fs::recursive_directory_iterator{fs::path(dir)})
    {
        for (const auto &ext : FILTER) {
            string fnE(fileName.path().extension().string());
            for (auto &c : fnE) { c = tolower(c); } //Some extensions have upper-case letters
            if (fnE == ext) {
                const wstring filePath(fileName.path().wstring());
                const wstring trimmedFN(filePath.substr(filePath.find(L"Music") + 6)); // Music/

                files.push_back(trimmedFN);
            }
        }
    }
    return files;
}

//Checks if directories ae synced by comparing size()
bool isSynced(const vector<wstring> &dir1, const vector<wstring> &dir2) {
    return (dir1.size() == dir2.size());
}

//Compares files between two dirs and list any missing paths
const vector<wstring> getUnsyncedFiles(const vector<wstring> &dir1, const vector<wstring> &dir2) {
    vector<wstring> unsyncedFiles;
    for (const auto &file : dir1) {
        if (!count(dir2.begin(), dir2.end(), file)) {
            unsyncedFiles.push_back(file);
        }
    }
    return unsyncedFiles;
}

//Takes bytes and return human readable file size
wchar_t *humanReadableFS(double size, wchar_t *buf) {
    int i = 0;
    const wchar_t *units[] = {L"B", L"KB", L"MB", L"GB", L"TB", L"PB", L"EB", L"ZB", L"YB"};
    while (size > 1024) {
        size /= 1024;
        i++;
    }

    #ifdef _WIN32
    swprintf(buf, L"%.*f %ls", i, size, units[i]);
    #else
    swprintf(buf, i, L"%.*f %ls", size, units[i]);
    #endif

    return buf;
}

//Takes milliseconds and return human readable time
const wstring formatTimer(const double &time)
{
    double calcET; wstringstream calcSS;

    if (time < 1000) {
        calcSS << fixed << setprecision(2) << time << " ms";
    }
    else if (time > 999 && time < 60000) {
        calcET = time / 1000.0;
        calcSS << fixed << setprecision(2) << calcET << " sec";
    }
    else if (time > 59999) {
        calcET = (time / 1000.0) / 60.0;
        calcSS << fixed << setprecision(2) << calcET << " min";
    }

    return calcSS.str();
}

int main()
{
    #ifdef _WIN32
        _setmode(_fileno(stdout), _O_U16TEXT);
    #endif

    wcout << endl << "Hey buddy! Give me a moment..." << endl << endl;


    if (!fs::exists(MusicLibrary)) {
        wcerr << "Can't find Music Library: \"" << MusicLibrary << "\". Exiting..." << endl;
        return 1;
    }
    if (!fs::exists(RemovableDir)) {
        wcerr << "Can't find Removable Directory: \"" << RemovableDir << "\". Exiting..." << endl;
        return 1;
    }

    uintmax_t   mlCapacity(fs::space(MusicLibrary).capacity),
                mlFree(fs::space(MusicLibrary).free),
                rdCapacity(fs::space(RemovableDir).capacity),
                rdFree(fs::space(RemovableDir).free);

    wchar_t mlCapacityChar[10], mlFreeChar[10],
            rdCapacityChar[10], rdFreeChar[10];

    wcout << "Music Library Path: " << MusicLibrary << endl;
    wcout << "Capacity\tFree" << endl;
    wcout
    << humanReadableFS(mlCapacity, mlCapacityChar) << "\t"
    << humanReadableFS(mlFree, mlFreeChar) << "\t"
    << PBFromFSSpace(mlCapacity, mlFree, 15) << endl << endl;

    wcout << "Removable Directory Path: " << RemovableDir << endl;
    wcout << "Capacity\tFree" << endl;
    wcout
    << humanReadableFS(rdCapacity, rdCapacityChar) << "\t"
    << humanReadableFS(rdFree, rdFreeChar) << "\t"
    << PBFromFSSpace(rdCapacity, rdFree, 15) << endl << endl;

recheck:
    wcout << "Calculating local Music Library..." << endl;
    vector<wstring> localMusicFiles = listDir(MusicLibrary);
    wcout << "Done. " << localMusicFiles.size() << " Files in local Music Library." << endl << endl;

    wcout << "Calculating Removable Directory..." << endl;
    vector<wstring> removableMusicFiles = listDir(RemovableDir);
    wcout << "Done. " << removableMusicFiles.size() << " Files in Removable Directory." << endl << endl;

    bool evenStevens = isSynced(localMusicFiles, removableMusicFiles);

    wcout << endl << "Are both locations synced? -> " << (evenStevens ? "YES" : "NO!") << endl << endl;

    if (!evenStevens) {

        bool musicLibraryToRemovable = localMusicFiles.size() > removableMusicFiles.size();

        vector<wstring> unsyncedFiles(  musicLibraryToRemovable ?
                                        getUnsyncedFiles(localMusicFiles, removableMusicFiles) :
                                        getUnsyncedFiles(removableMusicFiles, localMusicFiles)  );


        vector<UnsyncedFile> _UnsyncedFiles;

        double totalFileSize = 0;
        for (const wstring  &unsyncedFile : unsyncedFiles) {
            UnsyncedFile _UnsyncedFile;

            const wstring strPath =  musicLibraryToRemovable ?
                                    MusicLibrary + unsyncedFile :
                                    RemovableDir + unsyncedFile;

            fs::path truePath{strPath};

            _UnsyncedFile.name     = unsyncedFile;
            _UnsyncedFile.fullPath = truePath.make_preferred();
            _UnsyncedFile.bytes    = fs::file_size(truePath.make_preferred());

            wchar_t humanBytes[10];
            _UnsyncedFile.humanFileSize = humanReadableFS(_UnsyncedFile.bytes, humanBytes);

            totalFileSize += _UnsyncedFile.bytes;

            _UnsyncedFiles.push_back(_UnsyncedFile);
        }

        wchar_t humanTotalFS[10], showUnsyncedFiles;

        wcout << "Do you want to show the unsynced files? [y/n]: ";
        wcin >> showUnsyncedFiles;
        if (showUnsyncedFiles == 'y') {
            wcout << endl;
            int num = 1;
            for (const auto &_UnsyncedFile : _UnsyncedFiles) {
                wcout << num << ". " << _UnsyncedFile.name << " | "
                << _UnsyncedFile.humanFileSize << endl;
                num++;
            }
            wcout << endl;
        }

        wchar_t copyUnsyncedFiles;
        wcout << "Do you want to copy the unsynced files (" <<
        humanReadableFS(totalFileSize, humanTotalFS) <<
        ") into the " << (musicLibraryToRemovable ?
        "Removable Directory" : "Music Library") << "? [y/n]: ";
        wcin >> copyUnsyncedFiles;
        if (copyUnsyncedFiles == 'y') {
            int num = 1;
            double totalTime = 0;

            for (const auto & _UnsyncedFile : _UnsyncedFiles) {
                const wstring strPath =  musicLibraryToRemovable ?
                                    RemovableDir + _UnsyncedFile.name :
                                    MusicLibrary + _UnsyncedFile.name;

                fs::path destinationPath{strPath};

                wcout << endl << endl << "Working on \"" << _UnsyncedFile.name << "\"..." << endl;

                try {
                    clock_t timer; timer = clock();

                    //filesystem::copy_file is so dumb, Won't work even with fs::copy_options::recursive
                    //we most create the dirs first before we copy
                    //or it is going to fail, no need to check for existing ones...
                    fs::create_directories(destinationPath.wstring().substr(0, destinationPath.wstring().find_last_of(L"/\\")));

                    //Now we can copy...
                    fs::copy_file(_UnsyncedFile.fullPath, destinationPath.make_preferred());

                    double timeElapsed = (clock() - timer) / (double)(CLOCKS_PER_SEC / 1000);
                    totalTime += timeElapsed;

                    wcout << "[" << num << "/"<< _UnsyncedFiles.size() << "] File \""
                    << _UnsyncedFile.name << "\" copied to \"" << destinationPath.make_preferred().wstring() << "\"." << endl;
                    wcout << "File size: "<< _UnsyncedFile.humanFileSize << ". It took " << formatTimer(timeElapsed) << endl;

                }
                catch(fs::filesystem_error const &ex) {
                    wcerr << "[" << num << "/"<< _UnsyncedFiles.size() <<
                    "] Failed to copy file \"" << _UnsyncedFile.name << "\"!!!" << endl;

                    const int err_msg_len = sizeof(ex.code().message());
                    wchar_t err_msg[err_msg_len];
                    mbstowcs(err_msg, ex.code().message().c_str(), err_msg_len);

                    wcerr
                        << "Why: " << err_msg << endl
                        << "path1: " << ex.path1() << endl
                        << "path2: " << ex.path2() << endl;
                }
                num++;
            }
            wcout << endl << endl << "All files has been processed. Time elapsed: " << formatTimer(totalTime) << endl;
            wcout << endl << "Do you want to rescan both folders to make sure they're synced? [y/n]: ";
            wchar_t rescanDirs;
            wcin >> rescanDirs;

            if (rescanDirs == 'y') goto recheck;
            wcout << "Alright, take care!" << endl << endl;

            return 0;
        }
    }
    else {
        wcout << "Well...Looks like there's nothing to do here. CYA!!!" << endl << endl;
        return 0;
    }

    return 0;
}
