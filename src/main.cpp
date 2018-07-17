#include "surfer.h"

int main(int argc, char const *argv[])
{
    while(char ret = getopt(argc,(char*const*)argv,"h")){
        if(ret=='h'||ret=='?'){
            cout << "mycrawer-v1.0 @MrWatsonKing by:20180717." << endl;
            cout << "git: https://www.github.com/MrWatsonKing/mycrawler.git" << endl;
            return -1;
        }
        if(ret == -1) break;
    }
    
    getWebPage("www.baidu.com");
    
    return 0;
}
