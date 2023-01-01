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

using namespace std;
namespace fs = filesystem;

#define MusicLibrary "C:/Users/Xen/Music/"      // Your main Music Library here
#define RemovableDir "F:/Music/"                   // Location in Removable Disk
vector<string> FILTER = {".mp3", ".wav", ".flac"}; // Filter, use comma (,)



//Lists directory files recursively using filesystem::recursive_directory_iterator
//file paths need to match filter and also they will be trimmed, example:
//C:/Users/Xen/Music/song.mp3 -> song.mp3
vector<string> listDir(const string &dir) {
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
vector<string> getUnsyncedFiles(const vector<string> &localDir, const vector<string> &removableDir) {
    vector<string> unsyncedFiles;
    for (const auto &musicFile : localDir) {
        if (!count(removableDir.begin(), removableDir.end(), musicFile)) {
            unsyncedFiles.push_back(musicFile);
        }
    }
    return unsyncedFiles;
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

recheck:
    cout << "Calculating local music library: \"" << MusicLibrary << "\"..." << endl;
    vector<string> localMusicFiles = listDir(MusicLibrary);
    cout << "Done. " << localMusicFiles.size() << " In local music library. " << endl;

    cout << "Calculating removable music directory: \"" << RemovableDir << "\"..." << endl;
    vector<string> removableMusicFiles = listDir(RemovableDir);
    cout << "Done. " << removableMusicFiles.size() << " In removable music directory." << endl << endl;

    bool evenStevens = isSynced(localMusicFiles, removableMusicFiles);

    cout << "Are both synced? " << (evenStevens ? "YES" : "NO") << endl;

    if (!evenStevens) {
        vector<string> unsyncedFiles(getUnsyncedFiles(localMusicFiles, removableMusicFiles));
        char showUnsyncedFiles;
        cout << "Do you want to show the unsynced files? [y/n]: ";
        cin >> showUnsyncedFiles;
        if (showUnsyncedFiles == 'y') {
            cout << endl;
            int num = 1;
            for (auto &unsyncedFile : unsyncedFiles) {
                cout << num << ". " << unsyncedFile << endl;
                num++;
            }
            cout << endl;
        }

        char copyUnsyncedFiles;
        cout << "Do you want to copy the unsynced files into the removable directory? [y/n]: ";
        cin >> copyUnsyncedFiles;
        if (copyUnsyncedFiles == 'y') {
            int num = 1;
            for (const auto & unsyncedFile : unsyncedFiles) {
            fs::path fromPath{MusicLibrary+unsyncedFile};
            fs::path toPath{RemovableDir+unsyncedFile};

                cout << endl << endl << "Working on \"" << unsyncedFile << "\"..." << endl;

                try {
                    //filesystem::copy_file is so dumb, we most create the dirs first before we copy
                    //or it is going to fail, no need to check for existing ones...
                    fs::create_directories(toPath.string().substr(0, toPath.string().find_last_of("/\\")));

                    //Now we can copy...
                    fs::copy_file(fromPath.make_preferred(), toPath.make_preferred());
                    cout << "[" << num << "/"<< unsyncedFiles.size() << "] File \""
                    << unsyncedFile << "\" copied to \"" << toPath.make_preferred().string() << "\".";

                }
                catch(fs::filesystem_error const &ex) {
                    cerr << "[" << num << "/"<< unsyncedFiles.size() <<
                    "] Failed to copy file \"" << unsyncedFile << "\"!!!" << endl;
                    cerr
                        << "Why: " << ex.code().message() << endl
                        << "path1: " << ex.path1() << endl
                        << "path2: " << ex.path2() << endl;
                }
                num++;
            }
            cout << endl << "All files has been processed." << endl;
            cout << "Do you want to rescan both folders to make sure they're synced? [y/n]: ";
            char rescanDirs;
            cin >> rescanDirs;

            if (rescanDirs == 'y') goto recheck;
            cout << "Alright, take care!";

            return 0;
        }
    }
    else {
        cout << "Looks like there's nothing to do here. CYA!!!" << endl;
        return 0;
    }

    return 0;
}
