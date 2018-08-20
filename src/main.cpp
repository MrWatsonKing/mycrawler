#include "surfer.h"
string g_downPath;
string host_now;


int main(int argc, char const *argv[])
{
    while(char ret = getopt(argc,(char*const*)argv,"h")){
        if(ret=='h'||ret=='?'){
            cout << "mysurfer-v1.0 @MrWatsonKing by:20180717." << endl;
            cout << "git: https://www.github.com/MrWatsonKing/mysurfer.git" << endl;
            return -1;
        }
        if(ret == -1) break;
    }

    if(argc != 2){
        cout << "usage: surf <url>" << endl;
        return -1;
    }
    string url(argv[1]);
    host_now = url.substr(0,url.find("/"));
    
    //检查下载路径 若不存在 则创建之
    char cwd[128] = {0};
    g_downPath = string(getcwd(cwd,128)) + "/download";
    if(access(g_downPath.c_str(),R_OK|W_OK|X_OK) == -1){
        if(mkdir(g_downPath.c_str(),0777) == -1){
            perror("mkdir error");
            return -1;
        }else
			printf("dir created OK:%s\n",g_downPath.c_str());
    }
    
    time_t t1 = time(0);
    //发送请求并获取应答  不论是网页还是资源 都会在本地生成
    vector<char> vcontent = getWebPage(url);
    //从html中抽取url_list，并循环请求获取url_list指向的资源
    drawResources(vcontent);
    time_t t2 = time(0);

    cout << "surfing finished. time used: " << t2-t1 << " seconds." << endl;
    return 0;
}
