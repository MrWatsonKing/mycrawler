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
string getWebPage(const string &url){
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
    requestHeader+="Accept: html/text\r\n";
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

    int len, BUFFER_SIZE=1024;
    char buffer[BUFFER_SIZE];
    string pageContent="";
    while((len = recv(isock, buffer, BUFFER_SIZE-1, 0))>0){
        //recv或read函数并不会追加\0,所以需要我们手动添加\0，以便生成完整有效的字符串
        buffer[len]='\0';
        pageContent+=buffer;
    }

    close(isock);
    return pageContent;
}

void parseWebPage(const string &pageContent){
    //去除非网页内容的http协议头
    string content = pageContent.substr(pageContent.find("<html>"));
    //将网页内容写入本地文本
    writeLocalFile(content,"www.baidu.com_index.html");
    //获取页面内容中的https     
    list<string> url_list = getHttps(content); 
    writeLocalFile(url_list,"www.baidu.com_imageURLs.txt");    
}

//后续可以追加分类处理
list<string> getHttps(const string &pageContent,const char* type/*="images"*/){
    list<string> url_list;
    int begin=0,end=0,b1=0,e1=0;
    string url;
    while(true){
        begin = pageContent.find("url(http",begin);
        if(begin == -1) break;
        end = pageContent.find(")",begin);
        url = pageContent.substr(begin+4,end-begin-4);
        url.erase(remove(url.begin(),url.end(),'\\'),url.end());
        url_list.push_back(url);
        begin = end+1;
    }
    return url_list;
}

void writeLocalFile(const string &content,const string &filename,const string &downpath/*=string("/home/king/vscode/mysurfer/download")*/){
    string filepath = downpath + '/' + filename;
    ofstream outfile(filepath,fstream::out);
    if(!outfile.is_open()){
        cout << "file open error\n";
        outfile.close();
        exit(-1);
    }
    outfile << content << endl;
    outfile.close();
}

void writeLocalFile(const list<string> &strlist,const string &filename,const string &downpath/*=string("/home/king/vscode/mysurfer/download")*/){
    string filepath = downpath + '/' + filename;
    ofstream outfile(filepath,fstream::out);
    if(!outfile.is_open()){
        cout << "file open error\n";
        outfile.close();
        exit(-1);
    }
    for(auto str:strlist)
        outfile << str << endl;
    outfile.close();
}