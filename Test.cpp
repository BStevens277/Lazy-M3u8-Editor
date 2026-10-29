
/*
MUSIC APP TO MANAGE PLAYLISTS WITH A CLI

NOTE - Once program name is decided, add logic to folder population to exclude the application





Use cases:
    Main Menu:
        (always display current playlist w/ name & song indices)
        change current playlist name-
        export current playlist-
        import playlist to current-
            search available playlists iteratively
        clear current playlist
        change directory
        import all available
        import song (loop searh and loop additions until done)



Tutorials:
    fstream: 
    https://www.w3schools.com/cpp/ref_fstream_fstream.asp

To Do:
    for windows:
        force slashes to be forward
        to compile, explicit conversions from path to string

    housekeeping:
        ability to cancel actions, back out of menus
        input validation so no crashes when oob


    GUI (NOPE)
        wxWidgets (obnoxious library issues on flatpak vscode)

*/

#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <vector>
#include <math.h>



using namespace std;

namespace fs = std::filesystem;

// use fstream for playlist writing & reading
// https://www.geeksforgeeks.org/cpp/file-handling-c-classes/ 

// use filesystem for directory listing (switch to recursive_directory_iterator to hopefully see everything)

// use __ for ui



// format song entries to m3u8 entries
string parseSongOut(string song)
{
    // removing ./ from path
    string path = song;
    path = path.substr(2);

    // parsing out title
    string title = path;
    title = fs::path(path).stem().string();

    // replacing spaces with %20
    string retPath; // the path with %20
    retPath.reserve(path.size()*1.2); // reserve memory for %20 expansion
    for (char c : path){
        if (c == ' '){
            retPath += "%20";
        }
        else {
            retPath += c;
        }
    }
    string retStr = ("#EXTINF:0," + title + "\n" + retPath);
    return retStr;
}


// export current playlist to m3u8
void exportCurrentPlaylist(string name, vector<string> cPlaylist){
    fstream MyFile((name + ".m3u8"), std::ios::out | std::ios::trunc);
    MyFile << "#EXTM3U\n";

    for (string s : cPlaylist){
        MyFile << (parseSongOut(s) + "\n");
    }

}

// find
int find(string target, vector<string> pool){
    for (int i = 0; i < pool.size(); i++){
        if (pool[i] == target){
            return i;// if found
        }
    }
    return -1;// if not found
}

// erase is just vctr.erase(vctr.begin() + indexNumber);

// insert 
// (replace start: works | replace end: works | replace random: works | change order: works | first to last: works | last to first: works | oob+: works | oob-: works)
int insert(int i, string s, vector<string>& playlist){
    // validation
    if ((playlist.size() - 1) < i || i < 0){
        return -1;
    } else {
        // check if song already in | if it is, delete it
        int delind = find(s, playlist);
        if (delind != -1){
            playlist.erase(playlist.begin() + delind);
        }
        // add one and shift items over
        playlist.push_back("empty");
        for(int j = (playlist.size() - 1); j > (i); j--){
            playlist[j] = playlist[j - 1];
        }
        // insert song
        playlist[i] = s;
        return 1;// means success
    }
}

// add song to playlist
int add(string s, vector<string>& playlist){
    if (find(s, playlist) == -1){
        playlist.push_back(s);
        return 1;
    } else {
        return -1;
    }
}


// populate all vectors at startup
void startup(fs::path& path, vector<string>& folders, vector<string>& playlists, vector<string>& songs){
// iterating through the current directory and adding playlist files
    for (const auto& entry : fs::recursive_directory_iterator(path)){
        if(entry.path().filename().string().find(".m3u8") != std::string::npos){
            playlists.push_back(entry.path().filename());
        }
        // populate folders
        if(entry.path().filename().string().find(".") == std::string::npos){
            folders.push_back(entry.path());
        }
        // populate songs
        string tempSong;
        if(entry.path().filename().string().find(".mp3") != std::string::npos){
            tempSong = entry.path();
            songs.push_back(tempSong);
        }
    }
}

// takes ascii code and outputs actual character
char codeToChar(string str){
    std::string byteStr = str;
    // Convert hex substring to integer, then cast to char
    char ch = static_cast<char>(std::stoi(byteStr, nullptr, 16));
    return ch;

}

// import playlist from file
void importPlaylist(string importName, vector<string>& playlist, string& currName){
    fstream MyFile(importName, std::ios::in);

    currName = importName.substr(0,(importName.size()-5));

    string tempStr;
    string retStr;
    while (getline(MyFile, tempStr)){
        if (tempStr[0] != '#'){ 
            // FORMAT
            // adding "./" to the front
            tempStr += "@@";
            for (int i = (tempStr.size() - 1); i >= 2; i--){
                tempStr[i] = tempStr[i-2];
            }
            tempStr[0] = '.';
            tempStr[1] = '/';

            // swapping ascii %dd codes for actual characters
            int i = 0;
            while (i < (tempStr.size() - 2)){
                if(tempStr[i] == '%'){
                    retStr += codeToChar(tempStr.substr(i+1, 2));
                    i += 3;
                } else {
                    retStr += tempStr[i];
                    i++;
                }
            }
            retStr += tempStr[tempStr.size()-2];
            retStr += tempStr[tempStr.size()-1];
            playlist.push_back(retStr);
            retStr.clear();
        }
    }
}

//  search partial string (returns list of all songs containing substring OR even better method of obsidian search style "show if the target letter sequence appears in the comparee string")
vector<string> search(string srchstr, vector<string> songs){
    vector<string> searchResults;
    for (string song : songs){
        if ( searchResults.size() > 9){
            return searchResults;
        }
        int matchCount = 0;
        int srchIndx = 0;
        // iterate through each character of searchee and score it on what proportion of the target it contains
        for (int i = 0; i < song.size(); i++){
            // validation & if song char matches search char
            if (srchIndx < (srchstr.size()) & ((song[i] == srchstr[srchIndx]) || song[i] == static_cast<char>(srchstr[srchIndx] - 32) || song[i] == static_cast<char>(srchstr[srchIndx] + 32))){
                matchCount++;
                srchIndx++;
            }
        }
        // if song contains whole search string, add to results
        if (matchCount >= srchstr.size()) {
            searchResults.push_back(song);
        }
    }
    return searchResults;
}

// change path
void setPath (fs::path& path, string newpath){
    path = newpath;
}

// reset path
void resetPath (fs::path& path){
    path = ".";
}

// song refresh
void refreshSongs(vector<string>& songs, fs::path path){
    songs.clear();
    for (const auto& entry : fs::recursive_directory_iterator(path)){
        string tempSong;
        if(entry.path().filename().string().find(".mp3") != std::string::npos){
            tempSong = entry.path();
            songs.push_back(tempSong);
        }
    }

}

// get number from string (fix for only single digit user input)
int parseInputInt(string str){
    vector<int> digs;
    int i = 0;
    int total = 0; 
    while(isdigit(str[i])){
        digs.push_back(str[i] - 48);
        i++;
    }
    
    for(int j = 0; j < digs.size(); j++){
        total += (digs[j] * pow(10, (digs.size()-1-j))); 
    }

    return total;
}

// RENDERING

// Header render
void HRender(string headertext){
    cout << "\n/--------------------------------------------------\\  " << headertext;
}

// Footer render
void FRender(){
    cout << "\n\\__________________________________________________/\n";
}

// Vector render with indices
void VRender(vector<string> list){
    for (int i = 0; i < list.size(); i++){
        cout << "\n" << i << "  " << list[i]; 
    } 
}

// List Section render
void SecRender(string header, vector<string> list){
    HRender(header);
    VRender(list);
    FRender();
}

// main function
int main()
{




    cout << "\n\nLazy .m3u8 Playlist Manager:\n\n";

    // initializations
    fs::path path = ".";
    fs::path workPath = ".";
    vector<string> folders;
    vector<string> playlists;
    vector<string> songs; // Holds ALL songs available: each song string contains local path and is stored like that the whole time internally
    vector<string> current_Playlist;
    vector<string> second_Playlist;
    string currentPlaylistName = "Current Playlist"; // for swapping playlist names, doesn't need extension
    // string tempSong;
    bool running = true;
    string input;

    startup(path, folders, playlists, songs);


    while (running){
        SecRender(currentPlaylistName, current_Playlist);
        cout 
        << "\n|====================|Main Menu|===================|"
        << "\n|             D - Change directory                 |"
        << "\n|             S - Import songs                     |"
        << "\n|             I - Import playlist                  |"
        << "\n|             X - Edit current playlist            |"
        << "\n|             N - Change current playlist name     |"
        << "\n|             E - Export playlist                  |"
        << "\n|             C - Clear playlist                   |"
        << "\n|             Q - Quit                             |"
        << "\n|==================================================|\n";

        cin >> input;

        // changing playlist name only does single word names for now
        if (input == "n" || input == "N") { 
            cout << "\nOld Name: " << currentPlaylistName << "\nInput new name for playlist: ";
            cin >> currentPlaylistName;
        }

        if (input == "e" || input == "E") {
            exportCurrentPlaylist(currentPlaylistName, current_Playlist);
        }

        if (input == "i" || input == "I"){
            vector<string> searchedList;
            bool cont = true;
            while (cont){
                cout << "\nInput search string (or number to make selection): ";
                input = "";
                cin >> input;
                if (isdigit(input[0])){
                    importPlaylist(searchedList[static_cast<int>(input[0]) - 48], current_Playlist, currentPlaylistName);
                    cont = false;
                } else{
                    searchedList = search(input, playlists);
                    SecRender(input, searchedList);
                }
            }
        }

        if (input == "c" || input == "C") {
            current_Playlist.clear();
        }

        if (input == "d" || input == "D") {
            vector<string> searchedList;
            bool cont = true;
            while (cont){
                cout << "\nInput search string (number to make selection): ";
                input = "";
                cin >> input;
                if (input == "reset"){
                    workPath = ".";
                    refreshSongs(songs, workPath);
                    cont = false;
                }
                if (isdigit(input[0])){
                    setPath(workPath, searchedList[static_cast<int>(input[0]) - 48]);
                    refreshSongs(songs, workPath);
                    cont = false;
                } else{
                    searchedList = search(input, folders);
                    SecRender(input, searchedList);
                }
            }
        }

        if (input == "s" || input == "S") {
            vector<string> searchedList;
            bool cont = true;
            while (cont){
                cout << "\nInput search string (\"all\" for all songs in directory, or number to make selection): ";
                input = "";
                cin >> input;
                if (input == "all"){
                    for (int i = 0; i < songs.size(); i++){
                        add(songs[i], current_Playlist);
                    }
                    cont = false;
                } else if (isdigit(input[0])){
                    add(searchedList[static_cast<int>(input[0]) - 48], current_Playlist);
                    cont = false;
                } else{
                    searchedList = search(input, songs);
                    SecRender(input, searchedList);
                }
            }
        }

        if (input == "x" || input == "X"){
            // INCOMPLETE
            /*
            other options:
                shuffle
                reverse
                alphabetical



            */
           bool cont = true;
            while (cont) {
                SecRender(currentPlaylistName, current_Playlist);
                cout 
                << "\n[Edit playlist]"
                << "\nM - Move/Remove songs"
                << "\nQ - Quit editing"
                << "\n----------\n";
                cin >> input;

                if (input == "m" || input == "M") {
                    string selectedSong;
                    cout << "\nSelect a song: (use index number)\n";
                    cin >> input;
                    
                    if (isdigit(input[0])){
                        selectedSong = current_Playlist[parseInputInt(input)];
                        cout << "Selected: " << selectedSong;
                        cout << "\n Choose the new index: (\"r\" to remove)\n";
                        cin >> input;
                        
                        if (input == "r" || input == "r"){
                            int ind = find(selectedSong, current_Playlist);
                            current_Playlist.erase(current_Playlist.begin() + ind);
                        } else if (isdigit(input[0])) {
                            insert(parseInputInt(input), selectedSong, current_Playlist);
                            cout << "";
                        }
                    }
                }

                if (input == "q" || input == "Q"){
                    cont = false;
                    input = ""; // so it doesn't quit the whole application
                }
            }

        }


        if (input == "q" || input == "Q"){
            running = false;
        }

    }















    return 0;
}
/*
        TESTING:    
    */
    // cout << "\n\n=======search=======\n(f)\n";
    
    // for (string s : search("f", songs)){
    //     cout << s << endl;
    // }

    // cout << "-----usual format test-----\n\n";
    // for (int i = 0; i < 5; i++){
    //     cout << songs[i+53] << endl;
    // }
    // importPlaylist("y_rl montage.m3u8", current_Playlist);
    // cout << "-----Playlist Import Test-----\n\n";
    // for (string song : current_Playlist){
    //     cout << song << endl;
    // }

    // cout << "\n\n-----Find Test-----\n(Should be all found)\n";
    // for (string song : current_Playlist){
    //     cout << song << endl << find(song, songs) << endl;
    // }



    // DISPLAYING VECTOR CONTENTS
    // int count = 0;
    // cout << "\n\n=======folders=======\n\n";
    // for (string str : folders){
    //     cout << str << count << " \n";
    //     count++;
    // }
    // cout << "\n\n=======playlists=======\n\n";
    // for (string str : playlists){
    //     cout << str << endl;
    // }
    // cout << "\n\n=======songs=======\n\n";
    // for (string str : songs){
    //     cout << str << endl;
    // }

    // TESTING
    // currentPlaylistName = "0 test";
    // for (int i = 0; i < 10; i++){
    //     add(songs[i], current_Playlist);
    // }
    // cout << add(songs[3], current_Playlist);
    // // cout << "Insert test---\nFunction value: " << insert(10, current_Playlist[9], current_Playlist) << "\nSong to add: " << current_Playlist[9] << "\nSong at position in resulting playlist: " << current_Playlist[0];
    // exportCurrentPlaylist(currentPlaylistName, current_Playlist);
    
    // string temp = songs[150];
    // cout << "find test: " << find(temp, songs) << endl;
    // songs.erase(songs.begin() + 150);
    // cout << "erasing..." << endl;
    // cout << "find test: " << find(temp, songs) << endl;

        // cout << "test: 774328 = " << parseInputInt("774328hijab342nn234jj234nn3244");
