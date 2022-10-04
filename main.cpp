#include <iostream>
#include <vector>
#include <unistd.h>
#include <string>
#include <dirent.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <algorithm>

std::string str_tolower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), 
                   [](unsigned char c){ return std::tolower(c); }
                  );
    // debug
    // printf("converted %s\n",s.c_str());
    return s;
}

void findFileInDir(std::string searchPath, std::string searchTerm, bool caseInsensitive, bool recursive)
{
    struct dirent* dirent_ptr;
    struct stat statbuf;
    DIR* dir_ptr;

    // open dir or return if unsuccessful
    if ((dir_ptr = opendir(searchPath.c_str())) == NULL){
        return;
    }

    // read dir and loop over all elements inside folder
    while ((dirent_ptr = readdir(dir_ptr)) != NULL) {
        std::string fileName = dirent_ptr->d_name;

        // these two elements can be skipped
        if(fileName.compare(".") == 0 || fileName.compare("..") == 0)
            continue;

        // check if searchterm matches current element in folder. 
        // also call function str_tolower if the option case inse
        if(searchTerm.compare(caseInsensitive ? str_tolower(fileName) : fileName) == 0){
            printf("[%d]: %s: %s\n",getpid(), searchTerm.c_str(),realpath(searchPath.c_str(),0));
            return;
        }

        // if recursion is turned on
        if(recursive){
            std::string fullSearchPath = searchPath + "/" + fileName;

            // create statbuf to check if element is a directory
            if(stat(fullSearchPath.c_str(),&statbuf) == -1){
                printf("ERROR!");
                return;
            }

            // check if file is directory
            if(S_ISDIR(statbuf.st_mode)){
                // search element if it is a directory
                findFileInDir(fullSearchPath, searchTerm,caseInsensitive,recursive);
                
                // debug
                // printf("%s is dir\n", fileName.c_str());
            }
        }

        // debug
        // printf("dir: %s - term: %s\n", fileName.c_str(), searchTerm.c_str());
        
    }

    closedir(dir_ptr);
}

// 
// A child proccess is forked for every search term a user has inputed. Then the function findFileInDir(), which opens the current folder and
// loops through every file inside this folder. If the current file is a folder the function findFileInDir() is called with this new search path
// If the search term of this process matches the current file which is being looped over, then the console outputs a success message
//

int main(int argc, char* argv[])
{    
    int c, optionCount = 1;
    bool optionIsRecursive = false, optionIsCaseInsensitive = false;

    // check if any options were given
    while((c = getopt(argc, argv, "Ri")) != EOF) {
        switch(c) {
            case 'i': 
                optionCount++;
                optionIsCaseInsensitive = true;
                std::cout << "i enabled" << std::endl;
            break;
            case 'R':
                optionCount++;
                optionIsRecursive = true;
                 std::cout << "R enabled" << std::endl;
            break;
            default: 
                std::cout << "Something is extremly wrong!" << std::endl;
            break;
        }
    }

    std::string searchPath = argv[optionCount];
    std::cout << "SearchPath: " << searchPath << std::endl;
    std::vector<std::string> searchTerms;

    for(int i = 0; i < argc-optionCount-1; i++) {
        searchTerms.push_back(argv[optionCount+i+1]);
        std::cout << "Term " << i << ": " << searchTerms[i] << std::endl;
    }
    
    pid_t pid;
    int i;

    // fork for every searchTerm
    for(i = 0; i < (int) searchTerms.size(); i++) {
        pid = fork();
        
        // get i search element
        std::string searchTerm = searchTerms[i];
        
        if(optionIsCaseInsensitive)
            searchTerm = str_tolower(searchTerm);

        switch(pid) {
            case -1:
            return EXIT_FAILURE;
            case 0:
                // CHILD PROCESS

                // start find for i search term
                findFileInDir(searchPath,searchTerm,optionIsCaseInsensitive,optionIsRecursive);
                
                sleep(3);

                // kill child process 
                return EXIT_SUCCESS;
            break;
            default:
                // PARENT PROCESS
                
            break;
        }
    }   
    // prevent zombie processes
    pid_t childpid;
    while((childpid = waitpid(-1,NULL,WNOHANG))){
        if((childpid == -1) && (errno != EINTR)){
            break;
        }
    }

    // wait for all processes
    int returnStatus;       
    waitpid(-1, &returnStatus, 0);

    return EXIT_SUCCESS;
}

