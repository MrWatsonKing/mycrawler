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
    //检查下载路径 若不存在 则创建之
    char cwd[128] = {0};
    string downPath = getcwd(cwd,128);
    downPath += "/download";
    if(access(downPath.c_str(),R_OK|W_OK|X_OK) == -1){
        if(mkdir(downPath.c_str(),0777) == -1){
            perror("mkdir error");
            exit(-1);
        }else
			printf("dir created OK:%s\n",downPath.c_str());
    }

    //发送请求并获取页面数据
    string pageContent = getWebPage("www.baidu.com");
    //提取页面数据中的有效信息 写入本地文件，并提取其中的资源地址 写入本地文件
    parseWebPage(pageContent); 
    
    return 0;
}
