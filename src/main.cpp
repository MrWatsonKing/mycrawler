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
    
    //struct timeval{tv.sec,tv.usec}; 秒数和微秒数
    timeval tv1;
    gettimeofday(&tv1,NULL); //sys/time.h
    //发送请求并获取应答  不论是网页还是资源 都会在本地生成
    vector<char> vcontent = getWebPage(url);
    //从html中抽取url_list，并循环请求获取url_list指向的资源
    drawResources(vcontent);
    timeval tv2;
    gettimeofday(&tv2,NULL);

    cout.setf(ios::fixed); //自动补零
    cout << "surfing finished. time used: "
        << fixed << setprecision(2) //iomanip.h 2位小数精度
        << double((tv2.tv_sec-tv1.tv_sec)*1000000 + tv2.tv_usec-tv1.tv_usec)/1000000 
        << " seconds." << endl;
    return 0;
}
