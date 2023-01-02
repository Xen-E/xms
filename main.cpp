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

using namespace std;
namespace fs = filesystem;

#define MusicLibrary "C:/Users/Xen/Music/"               // Your main Music Library here
#define RemovableDir "F:/Music/"                         // Location in Removable Disk
const vector<string> FILTER = {".mp3", ".wav", ".flac"}; // Filter, use comma (,)


struct UnsyncedFile {
    string name, humanFileSize;
    fs::path fullPath;
    double bytes;
};


//Creates a CLI progress bar based on given disk capacity & available space
//The ratio var controls the progress bar size.
const string PBFromFSSpace( const uintmax_t &capacity,
                            const uintmax_t &free,
                            const unsigned short ratio) {

    string progressBar = "[";
    const unsigned short _ratio = (free * ratio) / capacity;
    const unsigned short left_percent = (free * 100) / capacity;

    for (short i=ratio; i>=0; i--) {
        if (i >= _ratio) progressBar += "|";
        else progressBar += "-";
    }
    progressBar += "] " + to_string(left_percent) + "% Free";
    return progressBar;
}

//Lists directory files recursively using filesystem::recursive_directory_iterator
//file paths need to match filter and also they will be trimmed, example:
//C:/Users/Xen/Music/song.mp3 -> song.mp3
const vector<string> listDir(const string &dir) {
    vector<string> files;

    for (const auto &fileName : fs::recursive_directory_iterator{fs::path(dir)})
    {
        for (const auto &ext : FILTER) {
            string fnE(fileName.path().extension().string());
            for (auto &c : fnE) { c = tolower(c); } //Some extensions have upper-case letters
            if (fnE == ext) {
                const string filePath(fileName.path().string());
                const string trimmedFN(filePath.substr(filePath.find("Music") + 6)); // Music/

                files.push_back(trimmedFN);
            }
        }
    }
    return files;
}

//Checks if directories ae synced by comparing size()
bool isSynced(const vector<string> &dir1, const vector<string> &dir2) {
    return (dir1.size() == dir2.size());
}

//Compares files between two dirs and list any missing paths
const vector<string> getUnsyncedFiles(const vector<string> &dir1, const vector<string> &dir2) {
    vector<string> unsyncedFiles;
    for (const auto &file : dir1) {
        if (!count(dir2.begin(), dir2.end(), file)) {
            unsyncedFiles.push_back(file);
        }
    }
    return unsyncedFiles;
}

//Takes bytes and return human readable file size
char *humanReadableFS(double size, char *buf) {
    int i = 0;
    const char *units[] = {"B", "kB", "MB", "GB", "TB", "PB", "EB", "ZB", "YB"};
    while (size > 1024) {
        size /= 1024;
        i++;
    }
    sprintf(buf, "%.*f %s", i, size, units[i]);
    return buf;
}

//Takes milliseconds and return human readable time
const string formatTimer(const double &time)
{
    double calcET; stringstream calcSS;

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
    cout << "Hey buddy! Give me a moment..." << endl << endl;


    if (!fs::exists(MusicLibrary)) {
        cerr << "Can't find Music Library: \"" << MusicLibrary << "\". Exiting..." << endl;
        return 1;
    }
    if (!fs::exists(RemovableDir)) {
        cerr << "Can't find Removable Directory: \"" << RemovableDir << "\". Exiting..." << endl;
        return 1;
    }

    uintmax_t   mlCapacity(fs::space(MusicLibrary).capacity),
                mlFree(fs::space(MusicLibrary).free),
                rdCapacity(fs::space(RemovableDir).capacity),
                rdFree(fs::space(RemovableDir).free);

    char    mlCapacityChar[10], mlFreeChar[10],
            rdCapacityChar[10], rdFreeChar[10];

    cout << "Music Library Path: " << MusicLibrary << endl;
    cout << "Capacity\tFree" << endl;
    cout
    << humanReadableFS(mlCapacity, mlCapacityChar) << "\t"
    << humanReadableFS(mlFree, mlFreeChar) << "\t"
    << PBFromFSSpace(mlCapacity, mlFree, 15) << endl << endl;

    cout << "Removable Directory Path: " << RemovableDir << endl;
    cout << "Capacity\tFree" << endl;
    cout
    << humanReadableFS(rdCapacity, rdCapacityChar) << "\t"
    << humanReadableFS(rdFree, rdFreeChar) << "\t"
    << PBFromFSSpace(rdCapacity, rdFree, 15) << endl << endl;

recheck:
    cout << "Calculating local Music Library..." << endl;
    vector<string> localMusicFiles = listDir(MusicLibrary);
    cout << "Done. " << localMusicFiles.size() << " Files in local Music Library." << endl << endl;

    cout << "Calculating Removable Directory..." << endl;
    vector<string> removableMusicFiles = listDir(RemovableDir);
    cout << "Done. " << removableMusicFiles.size() << " Files in Removable Directory." << endl << endl;

    bool evenStevens = isSynced(localMusicFiles, removableMusicFiles);

    cout << endl << "Are both locations synced? -> " << (evenStevens ? "YES" : "NO!") << endl << endl;

    if (!evenStevens) {

        bool musicLibraryToRemovable = localMusicFiles.size() > removableMusicFiles.size();

        vector<string> unsyncedFiles(   musicLibraryToRemovable ?
                                        getUnsyncedFiles(localMusicFiles, removableMusicFiles) :
                                        getUnsyncedFiles(removableMusicFiles, localMusicFiles)  );


        vector<UnsyncedFile> _UnsyncedFiles;

        double totalFileSize = 0;
        for (auto &unsyncedFile : unsyncedFiles) {
            UnsyncedFile _UnsyncedFile;

            const string strPath =  musicLibraryToRemovable ?
                                    MusicLibrary + unsyncedFile :
                                    RemovableDir + unsyncedFile;

            fs::path truePath{strPath};

            _UnsyncedFile.name     = unsyncedFile;
            _UnsyncedFile.fullPath = truePath.make_preferred();
            _UnsyncedFile.bytes    = fs::file_size(truePath.make_preferred());

            char humanBytes[10];
            _UnsyncedFile.humanFileSize = humanReadableFS(_UnsyncedFile.bytes, humanBytes);

            totalFileSize += _UnsyncedFile.bytes;

            _UnsyncedFiles.push_back(_UnsyncedFile);
        }

        char humanTotalFS[10], showUnsyncedFiles;

        cout << "Do you want to show the unsynced files? [y/n]: ";
        cin >> showUnsyncedFiles;
        if (showUnsyncedFiles == 'y') {
            cout << endl;
            int num = 1;
            for (const auto &_UnsyncedFile : _UnsyncedFiles) {
                cout << num << ". " << _UnsyncedFile.name <<
                " | " << _UnsyncedFile.humanFileSize << endl;
                num++;
            }
            cout << endl;
        }

        char copyUnsyncedFiles;
        cout << "Do you want to copy the unsynced files (" <<
        humanReadableFS(totalFileSize, humanTotalFS) <<
        ") into the " << (musicLibraryToRemovable ?
        "Removable Directory" : "Music Library") << "? [y/n]: ";
        cin >> copyUnsyncedFiles;
        if (copyUnsyncedFiles == 'y') {
            int num = 1;
            double totalTime = 0;

            for (const auto & _UnsyncedFile : _UnsyncedFiles) {
                const string strPath =  musicLibraryToRemovable ?
                                    RemovableDir + _UnsyncedFile.name :
                                    MusicLibrary + _UnsyncedFile.name;

                fs::path destinationPath{strPath};

                cout << endl << endl << "Working on \"" << _UnsyncedFile.name << "\"..." << endl;

                try {
                    clock_t timer; timer = clock();

                    //filesystem::copy_file is so dumb, Won't work even with fs::copy_options::recursive
                    //we most create the dirs first before we copy
                    //or it is going to fail, no need to check for existing ones...
                    fs::create_directories(destinationPath.string().substr(0, destinationPath.string().find_last_of("/\\")));

                    //Now we can copy...
                    fs::copy_file(_UnsyncedFile.fullPath, destinationPath.make_preferred());

                    double timeElapsed = (clock() - timer) / (double)(CLOCKS_PER_SEC / 1000);
                    totalTime += timeElapsed;

                    cout << "[" << num << "/"<< _UnsyncedFiles.size() << "] File \""
                    << _UnsyncedFile.name << "\" copied to \"" << destinationPath.make_preferred().string() << "\"." << endl;
                    cout << "File size: "<< _UnsyncedFile.humanFileSize << ". It took " << formatTimer(timeElapsed) << endl;

                }
                catch(fs::filesystem_error const &ex) {
                    cerr << "[" << num << "/"<< _UnsyncedFiles.size() <<
                    "] Failed to copy file \"" << _UnsyncedFile.name << "\"!!!" << endl;
                    cerr
                        << "Why: " << ex.code().message() << endl
                        << "path1: " << ex.path1() << endl
                        << "path2: " << ex.path2() << endl;
                }
                num++;
            }
            cout << endl << endl << "All files has been processed. Time elapsed: " << formatTimer(totalTime) << endl;
            cout << endl << "Do you want to rescan both folders to make sure they're synced? [y/n]: ";
            char rescanDirs;
            cin >> rescanDirs;

            if (rescanDirs == 'y') goto recheck;
            cout << "Alright, take care!" << endl << endl;

            return 0;
        }
    }
    else {
        cout << "Well...Looks like there's nothing to do here. CYA!!!" << endl << endl;
        return 0;
    }

    return 0;
}
