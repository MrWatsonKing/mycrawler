#include "surfer.h"
extern string g_downPath;

//连接主机
int connectHost(const string &url){
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
    bzero(&pin,sizeof(pin));
    pin.sin_family = AF_INET;
    pin.sin_port = htons(80);
    pin.sin_addr.s_addr = ((struct in_addr*)(host->h_addr))->s_addr;
    //创建套接字
    int sfd;
    if((sfd = socket(AF_INET, SOCK_STREAM, 0))==-1){
        cout<<"socket open error\n"<<endl;
        return -1;
    }
    //连接主机
    if(connect(sfd, (const sockaddr*)&pin, sizeof(pin))==-1){
        cout<<"connect error\n"<<endl;
        return -1;
    }
    //返回套接字
    return sfd;
}

//获取网页
string getWebPage(int sfd,const string &url){
    struct hostent *host;
    string hostUrl, pagePath;
    //将url解析为 主机名 和 网页路径
    parseHostAndPagePath(url, hostUrl, pagePath);    
    
    //创建https协议请求头 pagePath以/开头 hostUrl==www.baidu.com或baidu.com
    string requestHeader;
    requestHeader = "GET "+pagePath+" HTTP/1.1\r\n";
    requestHeader += "Host: "+hostUrl+"\r\n";    
    int pos = 0;
    if((pos=pagePath.find(".png"))>0)
        requestHeader += "Accept: image/png\r\n";
    else if((pos=pagePath.find(".jpg"))>0)
        requestHeader += "Accept: image/jpg\r\n";
    else if((pos=pagePath.find(".gif"))>0)
        requestHeader += "Accept: image/gif\r\n";
    else
        requestHeader += "Accept: text/html\r\n";
    requestHeader += "User-Agent: Mozilla/5.0\r\n";
    requestHeader += "connection:Keep-Alive\r\n";
    requestHeader += "\r\n";
     
    //发送协议头
    if(send(sfd, requestHeader.c_str(), requestHeader.size(), 0)==-1){
        cout<<"send error\n"<<endl;
        exit(1);
    }
    //设置socket选项
    struct timeval timeout={1,0};
    setsockopt(sfd, SOL_SOCKET, SO_RCVTIMEO, (void *)&timeout, sizeof(struct timeval));
    //接收应答消息
    int len, BUFFER_SIZE=1024;
    char buffer[BUFFER_SIZE];
    string pageContent="";
    while((len = recv(sfd, buffer, BUFFER_SIZE-1, 0))>0){
        //recv或read函数并不会追加\0,所以需要我们手动添加\0，以便生成完整有效的字符串
        buffer[len] = '\0';
        pageContent += buffer;
    }
    //去除非网页内容的http协议头
    int posC = pageContent.find("\r\n\r\n")+4;    
    pageContent= pageContent.substr(posC);
    //如果不是html 就生成本地文件 文件名为
    if(requestHeader.find("html")==-1)
        writeLocalFile(pageContent,pagePath.substr(pagePath.rfind("/")+1),g_downPath);

    return pageContent;
}

//抽取网页资源
void drawResources(int sfd,const string &pageContent){
    //将网页内容写入本地文本
    // writeLocalFile(pageContent,"www.baidu.com_index.html",g_downPath);
    //获取页面内容中的https     
    list<string> url_list = getHttps(pageContent); 
    //将资源链接列表输出到本地txt文档
    writeLocalFile(url_list,"url_list.txt",g_downPath);
    for(auto url:url_list)
        getWebPage(sfd,url);
}

//网址解析
void parseHostAndPagePath(const string& url, string &hostUrl, string &pagePath){
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

//后续可以追加资源类型标签 分类获取不同类型的资源地址
list<string> getHttps(const string &pageContent,const char* type/*="images"*/){
    list<string> url_list;
    int begin=0,end=0,b1=0,e1=0;
    string url;
    int cnt = 0;
    while(true){
        begin = pageContent.find("url(http",begin);
        if(begin == -1) break;
        end = pageContent.find(")",begin);
        url = pageContent.substr(begin+4,end-begin-4);
        url.erase(remove(url.begin(),url.end(),'\\'),url.end());
        //排除重复元素
        for(auto elmt:url_list)
            if(elmt == url) cnt++;
        if(cnt==0) 
            url_list.push_back(url);
        cnt = 0;
        begin = end+1;
    }
    return url_list;
}

void writeLocalFile(const string &content,const string &filename,const string &downpath/*=defDownPath*/){
    string filepath = downpath + '/' + filename;
    ofstream outfile(filepath,fstream::out);
    if(!outfile.is_open()){
        cout << "file open error\n";
        outfile.close();
        exit(-1);
    }
    outfile << content;
    outfile.close();
}

void writeLocalFile(const list<string> &strlist,const string &filename,const string &downpath/*=defDownPath*/){
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
