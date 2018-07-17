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
    //将url解析为 主机地址 和 网页路径
    parseHostAndPagePath(url, hostUrl, pagePath);
    //通过主机地址获取主机信息 失败则返回
    if(0==(host=gethostbyname(hostUrl.c_str()))){
        cout<<"gethostbyname error\n"<<endl;
        exit(-1);
    }
 
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

    //创建https协议请求头
    string requestHeader;
    requestHeader="GET "+pagePath+" HTTP/1.1\r\n";
    requestHeader+="Host: "+hostUrl+"\r\n";
    requestHeader+="Accept: */*\r\n";
    requestHeader+="User-Agent: Mozilla/5.0\r\n";
    requestHeader+="connection:Keep-Alive\r\n";
    requestHeader+="\r\n";
 
    if(connect(isock, (const sockaddr*)&pin, sizeof(pin))==-1){
        cout<<"connect error\n"<<endl;
        exit(1);
    }
    if(send(isock, requestHeader.c_str(), requestHeader.size(), 0)==-1){
        cout<<"send error\n"<<endl;
        exit(1);
    }
 
    struct timeval timeout={1,0};
    setsockopt(isock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));
    char c;
    bool flag=true;
    while(recv(isock, &c, 1, 0)>0){
        if('\r'==c){
            continue;
        }else if('\n'==c){
            if(false==flag)
                break;
            flag=false;
        }else{
            flag=true;
        }
    }
 
    int len, BUFFER_SIZE=512;
    char buffer[BUFFER_SIZE];
    string pageContent="";
    while((len = recv(isock, buffer, BUFFER_SIZE-1, 0))>0){
        buffer[len]='\0';
        pageContent+=buffer;
    }
    cout << pageContent << endl;
    return true;
}