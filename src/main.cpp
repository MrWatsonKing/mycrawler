#include "surfer.h"
string g_downPath;

int main(int argc, char const *argv[])
{
    while(char ret = getopt(argc,(char*const*)argv,"h")){
        if(ret=='h'||ret=='?'){
            cout << "mycrawer-v1.0 @MrWatsonKing by:20180717." << endl;
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
    
    //检查下载路径 若不存在 则创建之
    char cwd[128] = {0};
    g_downPath = string(getcwd(cwd,128)) + "/download";

    if(access(g_downPath.c_str(),R_OK|W_OK|X_OK) == -1){
        if(mkdir(g_downPath.c_str(),0777) == -1){
            perror("mkdir error");
            exit(-1);
        }else
			printf("dir created OK:%s\n",g_downPath.c_str());
    }

    //连接主机
    int sfd = connectHost(url);
    if(sfd ==-1) exit(-1);
    //发送请求并获取应答  如果是image内容就写入本地 text/html就啥也不干
    vector<char> vcontent = getWebPage(sfd,url);
    //从html中抽取url_list，并循环请求获取url_list指向的资源
    //drawResources(sfd,vcontent);
    //断开连接
    close(sfd);
    
    return 0;
}
