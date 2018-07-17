#include "surfer.h"

//网址解析
void parseHostAndPagePath(const string url, string &hostUrl, string &pagePath){
    hostUrl=url; //url完整副本
    pagePath="/";
    //查找协议前缀 如果找到了 将协议前缀置换为空
    int pos=hostUrl.find("http://");
    if(-1!=pos)
        hostUrl=hostUrl.replace(pos,7,"");
    pos=hostUrl.find("https://");
    if(-1!=pos)
        hostUrl=hostUrl.replace(pos,8,"");
    //找到主机url中的第一个层次划分
    pos=hostUrl.find("/");
    //如果存在层次划分 则分别重置页面路径和主机地址
    if(-1!=pos){
        pagePath=hostUrl.substr(pos);
        hostUrl=hostUrl.substr(0,pos);
    }
    //对"www.baidu.com"将不会进行处理
    //hostUrl = "www.baidu.com"
    //pagePath = "/"
}

//获取网页
bool getWebPage(const string &url){
    struct hostent *host;
    string hostUrl, pagePath;
    //将url解析为 主机名 和 网页路径
    parseHostAndPagePath(url, hostUrl, pagePath);
    //通过主机名获取主机地址 失败则返回
    if(0==(host=gethostbyname(hostUrl.c_str()))){
        cout<<"gethostbyname error\n"<<endl;
        exit(-1);
    }
 
    //创建ip地址结构体 并将主机ip地址拷贝过去
    struct sockaddr_in pin;
    int port=80;
    bzero(&pin,sizeof(pin));
    pin.sin_family=AF_INET;
    pin.sin_port=htons(port);
    pin.sin_addr.s_addr=((struct in_addr*)(host->h_addr))->s_addr;

    //创建套接字
    int isock;
    if((isock = socket(AF_INET, SOCK_STREAM, 0))==-1){
        cout<<"open socket error\n"<<endl;
        exit(-1);
    }

    //创建https协议请求头 pagePath以/开头 hostUrl==www.baidu.com或baidu.com
    string requestHeader;
    requestHeader="GET "+pagePath+" HTTP/1.1\r\n";
    requestHeader+="Host: "+hostUrl+"\r\n";
    requestHeader+="Accept: */*\r\n";
    requestHeader+="User-Agent: Mozilla/5.0\r\n";
    requestHeader+="connection:Keep-Alive\r\n";
    requestHeader+="\r\n";
 
    //连接主机
    if(connect(isock, (const sockaddr*)&pin, sizeof(pin))==-1){
        cout<<"connect error\n"<<endl;
        exit(-1);
    }
    //发送协议头
    if(send(isock, requestHeader.c_str(), requestHeader.size(), 0)==-1){
        cout<<"send error\n"<<endl;
        exit(1);
    }
    
    //设置socket选项
    struct timeval timeout={1,0};
    setsockopt(isock, SOL_SOCKET, SO_RCVTIMEO, (void *)&timeout, sizeof(struct timeval));

    // char c;
    // bool flag=true;
    // //这里是干嘛？
    // while(recv(isock, &c, 1, 0)>0){
    //     if('\r'==c){
    //         continue;
    //     }else if('\n'==c){
    //         if(false==flag)
    //             break;
    //         flag=false;
    //     }else{
    //         flag=true;
    //     }
    // }
 
    int len, BUFFER_SIZE=1024;
    char buffer[BUFFER_SIZE];
    string pageContent="";
    while((len = recv(isock, buffer, BUFFER_SIZE-1, 0))>0){
        //recv或read函数并不会追加\0,所以需要我们手动添加\0，以便生成完整有效的字符串
        buffer[len]='\0';
        pageContent+=buffer;
    }
    cout << pageContent << endl;
    close(isock);

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

    //hostUrl不带/ pagePath以/开头
    downPath += (pagePath.size()==1 ? "/"+hostUrl:pagePath)+".html";
    //cout << downPath << endl;
    ofstream outfile(downPath,fstream::out);
    if(!outfile.is_open()){
        cout << "file open error\n";
        outfile.close();
        exit(-1);
    }
    outfile << pageContent;
    outfile.close();



    return true;
}