#include "surfer.h"
extern string g_downPath;
extern string host_now;
const size_t np = string::npos;

//连接主机
int connectHost(const string &url){
    struct hostent *host;
    string hostUrl, pagePath;
    //将url解析为 主机名 和 网页路径
    parseHostAndPagePath(url, hostUrl, pagePath);    
    //通过主机名获取主机地址 失败则返回
    if(0==(host=gethostbyname(hostUrl.c_str()))){
        cout<<"gethostbyname error\n"<<endl;
        return -1;
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
string getWebPage(const string &url){
    //cout<<"\tthread_id: "<<this_thread::get_id()<<endl;
    string sbytes;    
    string hostUrl, pagePath;
    
    //将url解析为 主机名 和 网页路径
    //对"www.baidu.com"将不会进行处理
    //hostUrl = "www.baidu.com"   pagePath = "/"   
    parseHostAndPagePath(url,hostUrl,pagePath);

    //创建https协议请求头 pagePath以/开头 hostUrl==www.baidu.com或baidu.com
    string request = prepareHead(pagePath,hostUrl);
    sbytes = fetchData(request,url);

    //检查对应文件夹是否存在 若不存在 则创建之
    string downPath = g_downPath + "/" + host_now;
    checkDir(downPath);
    
    //获取有效的文件名 如果存在%或=或? 都去除之 保留特殊符号前面的字符
    string filename = pagePath.substr(pagePath.rfind("/")+1);
    if(filename.size()==0) filename = host_now; //这种情况只会是html主页 不会是图片
	//包含协议头的完整数据 写入本地文件
	//writeLocalFile(sbytes,filename+"_full.txt","",downPath); //可写入\0字符
    
	//将http协议头和网页内容分离
    int i=0;
    if(sbytes.size()>=4){
    	for(i=4;i<sbytes.size();i++)
	    	if(sbytes[i-4]=='\r' && sbytes[i-3]=='\n' && sbytes[i-2]=='\r' && sbytes[i-1]=='\n')
		    	break;
    }else{
        cout << "\tError: reply.size()<4! download canceled." << endl;
        cout << "\tbuffer size=" << sbytes.size() << endl << endl;
        return sbytes;
    }   
        
    //得到http协议头 并根据解析结果进行处理 
    string reply,scontent;
    reply.insert(reply.end(),sbytes.begin(),sbytes.begin()+i-1);
    //如果reply结果不是200 就返回。
    if(parseReply(reply) == -1) return sbytes;

    //如果是分块传输 则对十六进制分块长度标记数进行处理
    if(sbytes.find("chunked")!=np || sbytes.find("Chunked")!=np){
        //如果服务器按transfer-encoding:chunked 或 content-encoding:chunked 来分块发送数据包
        //则对数据包进行整理
        regex reg("\\r\\n[0-9a-fA-F]+\\r\\n");
        // sregex_iterator it(sbytes.begin(),sbytes.end(),reg);
        // sregex_iterator end;
        // for(;it!=end;it++){
        //     cout << it->str(1);
        // }
        //此函数第一个参数以const方式传入 不会改变自身的值
        sbytes = regex_replace(sbytes,reg,"");
        //为什么会把 200/r/n<! 替换掉？
        sbytes.insert(0,"<!");
    }

    //应答码==200 数据正常 则解析得到文件内容
	scontent.insert(scontent.end(),sbytes.begin()+i,sbytes.end());

#if 1 //生成本地文件与否
    //如果是网页内容 就在本地生成网页文件 包括html或shtml
    if(request.find("html") != np){        
        //将文件名中的/替换为.
        string pageName = hostUrl+pagePath;        
        size_t b=0, p=0;
        if(pageName.find("/") != np)            
            while(true){
                b = pageName.find("/",p);
                if(b == np) break;
                pageName.replace(b,1,".");
                p = b+1;
            }
        //将网页写入本地
        //请求.shtml时 pageName以.shtml结尾
        if(request.find("shtml") != np)
            writeLocalFile(scontent,pageName,"",downPath);
        //找不到shtml 就是普通网页 以.html结尾  或 没有结尾
        else{
            if(pageName.find("html") == np) //pagePath=/ 或xxx/ /都已经替换成.
                writeLocalFile(scontent,pageName+"html","",downPath);
            else //pagePath = xxx.html
                writeLocalFile(scontent,pageName,"",downPath);
        }        
    }
	//如果不是html 就生成本地文件 文件名为对应的资源文件名
    else{
        if(filename.find("%%") != np)
            filename = filename.substr(0,filename.rfind("%%"));
        if(filename.find("=") != np)
            filename = filename.substr(0,filename.rfind("="));
        if(filename.find("?") != np)
            filename = filename.substr(0,filename.rfind("?"));

        writeLocalFile(scontent,filename,"\t",downPath);
    }                
#endif

    return scontent;
}

//抽取网页资源
void drawResources(const string &scontent){
    //获取页面内容中的https     
    list<string> url_list = getHttps(scontent); 
    if(url_list.size() == 0){
        cout << "found no urls in \"" << host_now << "\"" << endl;
        return;
    }
    //将资源链接列表输出到本地txt文档`
    writeLocalFile(url_list,"url_list_"+host_now+".txt","\n",g_downPath+"/"+host_now);

    int cnt=0,nts = url_list.size();

#if 1 //多线程 同步执行 函数结束前统一等待回收
    //等待线程汇合 可以确保主线程和进程不会在其他线程没有完全结束之前就退出
    //因为主线程提前退出 导致进程退出 从而导致其他线程还没有执行完就被销毁了 所以分离线程不可行
    thread vts[nts];
    for(auto url:url_list){
        cnt++;
        cout << "\tdownloading file "<< cnt << ": "<< url << endl;
        //对线程数组的每一个元素进行赋值 thread创建的时候 线程函数会自动调用并运行
        vts[cnt-1] = thread(getWebPage,url);
    }
    //函数返回之前 同步等待所有的线程执行完毕
    for(int i=0;i<nts;i++)
        vts[i].join();
#else //单线程 顺序执行    
    for(auto url:url_list){
        cnt++;
        cout << "\tdownloading file "<< cnt << ": "<< url << endl;
        getWebPage(url);
    }
#endif
    //多线程 耗时 最长2.21秒 最短1.32秒
    //单线程 耗时 最长3.31秒 最短2.06秒
    //这表明多线程并发执行具有无可替代的速度优势
    cout << endl << "download over." << endl;
}

//网址解析
void parseHostAndPagePath(const string& url, string &hostUrl, string &pagePath){
    hostUrl=url; //url完整副本
    pagePath="/";
    //查找协议前缀 如果找到了 将协议前缀置换为空
    size_t pos=hostUrl.find("http://");
    if(pos != np)
        hostUrl=hostUrl.replace(pos,7,"");
    pos=hostUrl.find("https://");
    if(pos != np)
        hostUrl=hostUrl.replace(pos,8,"");
    //找到主机url中的第一个层次划分
    pos=hostUrl.find("/");
    //如果存在层次划分 则分别重置页面路径和主机地址
    if(pos != np){
        pagePath=hostUrl.substr(pos);
        hostUrl=hostUrl.substr(0,pos);
    }
    //对"www.baidu.com"将不会进行处理
    //hostUrl = "www.baidu.com"
    //pagePath = "/"
}

//准备好http协议请求头
string prepareHead(const string &pagePath,const string &hostUrl){
    string request;
    request = "GET "+pagePath+" HTTP/1.1\r\n";
    request += "Host: "+hostUrl+"\r\n";    
    size_t pos = 0;
    if((pos=pagePath.find(".png")) != np)
        request += "Accept: image/png\r\n";
    else if((pos=pagePath.find(".jpg")) != np)
        request += "Accept: image/jpg\r\n";
    else if((pos=pagePath.find(".gif")) != np)
        request += "Accept: image/gif\r\n";
    else if((pos=pagePath.find(".shtml")) != np)
        request += "Accept: text/shtml\r\n";
    else
        request += "Accept: text/html\r\n";
    request += "User-Agent: Mozilla/5.0\r\n";
    request += "connection:close\r\n";
    request += "\r\n";

    return request;
}

//发送http请求头 并获取数据
string fetchData(const string& request,const string& url){
    ssize_t len=0;
    int BUFLEN=2048;
    char buffer[BUFLEN];
    string sbytes;

    //连接主机
    int sfd = connectHost(url);
    if(sfd ==-1){
        close(sfd);
        return sbytes;
    }

    //发送协议头
    if(send(sfd, request.c_str(), request.size(), 0)==-1){
        cout<<"\tError: request send error!\n"<<endl;
        return sbytes;
    }
    //设置socket选项
    struct timeval timeout={1,0};
    setsockopt(sfd, SOL_SOCKET, SO_RCVTIMEO, (void *)&timeout, sizeof(struct timeval));
    //接收应答消息    
    while((len = recv(sfd, buffer, BUFLEN-10, 0))>0)
        for(int i=0;i<len;i++)
            sbytes.push_back(buffer[i]);    
    //string中间也可以包含\0字符 cout<<string会将\0字符输出为空格
    //所以在生成text文件时 可以使用cout 但生成图片文件时 则不能ofstream<<string 
    //而必须逐字节将所有字符（包含\0）都完整输出    
    //断开连接
    close(sfd);
    
    return sbytes;
}

int parseReply(const string& reply){
    string general = reply.substr(0,reply.find("\r\n"));
    string server,date,contentType,contentLength,connection,location;

    size_t start = reply.find("Server:"), end = 0;
    if(start != np){
        end = reply.find("\r\n",start);
        server = reply.substr(start,end-start);
    }    
    start = reply.find("Date:");
    if(start != np){
        end = reply.find("\r\n",start);
        date = reply.substr(start,end-start);
    }
    start = reply.find("Content-Type:");
    if(start != np){
        end = reply.find("\r\n",start);
        contentType = reply.substr(start,end-start);
    }
    start = reply.find("Content-Length:");
    if(start != np){
        end = reply.find("\r\n",start);
        contentLength = reply.substr(start,end-start);
    }
    start = reply.find("Connection:");
    if(start != np){
        end = reply.find("\r\n",start);
        connection = reply.substr(start,end-start);
    }
    start = reply.find("Location:");
    if(start != np){
        end = reply.find("\r\n",start);
        location = reply.substr(start,end-start);
    }
    //如果网页返回错误代码 输出查看当前httpHead 并返回
    if(general.find("HTTP/1.1 200") == np){
        cout << "\t" << "Error: resource not found! download canceled." << endl;
        cout << "\t" << general << endl; //general是肯定存在的 后面的项目不一定存在
        cout << "\t" << server << (server.size()>0 ? "\n\t":"");
        cout << date << (date.size()>0 ? "\n\t":"");
        cout << contentType << (contentType.size()>0 ? "\n\t":"");
        cout << contentLength << (contentLength.size()>0 ? "\n\t":"");
        cout << connection << (connection.size()>0 ? "\n\t":"");
        cout << location << endl;
        //注意：三目表达式 涉及到类型不同的 或者前扩 或者全扩 编译器才能正确解析 否则会报错
        return -1;
    }

    return 0;
}

void checkDir(const string& dir){
    if(access(dir.c_str(),R_OK|W_OK|X_OK) == -1){
        if(mkdir(dir.c_str(),0777) == -1)
            perror("mkdir error");
        else  
            printf("dir created OK:%s\n",dir.c_str());
    }
}

//后续可以追加资源类型标签 分类获取不同类型的资源地址
list<string> getHttps(const string &scontent,const char* type/*="images"*/){
    string url,pageContent;
    for(int i=0;i<scontent.size();i++)
        if(scontent[i] != '\0')
            pageContent.push_back(scontent[i]);

    list<string> url_list;
    size_t begin=0,end=0;
    int cnt = 0;
#if 0    
    while(true){
        begin = pageContent.find("url(",begin);
        if(begin == np) break;
        end = pageContent.find(")",begin);
        url = pageContent.substr(begin+4,end-begin-4);
        //去除url字符串中的'\'符号
        url.erase(remove(url.begin(),url.end(),'\\'),url.end());
        //去除url字符串中的‘
        url.erase(remove(url.begin(),url.end(),'\''),url.end());
        //去除不包含//的地址类型，如：#default#homepage
        if(url.find("//") == np){
            begin = end+1;
            continue;
        } 
        //保留//之后的地址
        url = url.substr(url.find("//")+2);        
        //排除重复元素
        for(auto elmt:url_list)
            if(elmt == url) cnt++;
        if(cnt==0)
            url_list.push_back(url);
        //重置重复计数cnt和查找位置begin
        cnt = 0;
        begin = end+1;
    }
#else
    //按第一种规则查找图片资源超链接    
    regex reg("url\\((.*?)\\)"); //解析url()资源地址
    sregex_iterator spos(pageContent.cbegin(),pageContent.cend(),reg);
    sregex_iterator send;
    for(;spos!=send;spos++){
        //cout << spos->str() << endl; //全匹配
        url = spos->str(1); //第一个分组
        //去除url字符串中的'\'符号
        url.erase(remove(url.begin(),url.end(),'\\'),url.end());
        //去除url字符串中的‘
        url.erase(remove(url.begin(),url.end(),'\''),url.end());
        //去除不包含//的地址类型，如：#default#homepage
        if(url.find("//") == np)
            continue;         
        //保留//之后的地址
        url = url.substr(url.find("//")+2);        
        //排除重复元素
        for(auto elmt:url_list)
            if(elmt == url) cnt++;
        if(cnt==0){
            url_list.push_back(url);
            // cout << url << endl;
        }            
        //重置重复计数cnt和查找位置begin
        cnt = 0;
    }
    //按第二种规则查找图片资源超链接
    regex reg1("<img.*?src=\\\"(.*?)\\\""); //解析<img src="">资源地址
    sregex_iterator spos1(pageContent.cbegin(),pageContent.cend(),reg1);
    for(;spos1!=send;spos1++){
        //cout << spos->str() << endl; //全匹配
        url = spos1->str(1); //第一个分组
        //去除url字符串中的'\'符号
        url.erase(remove(url.begin(),url.end(),'\\'),url.end());
        //去除url字符串中的‘
        url.erase(remove(url.begin(),url.end(),'\''),url.end());
        //去除不包含//的地址类型，如：#default#homepage
        if(url.find("//") == np)
            continue;
        //保留//之后的地址
        url = url.substr(url.find("//")+2);        
        //排除重复元素
        for(auto elmt:url_list)
            if(elmt == url) cnt++;
        if(cnt==0){
            url_list.push_back(url);
            // cout << url << endl;
        }            
        //重置重复计数cnt和查找位置begin
        cnt = 0;
    }
    //查找超文本引用链接
    list<string> href_list;
    reg1 = regex("href=\\\"(.*?)\\\""); //解析href超链接
    spos1 = sregex_iterator(pageContent.cbegin(),pageContent.cend(),reg1);
    for(;spos1!=send;spos1++){
        //cout << spos->str() << endl; //全匹配
        url = spos1->str(1); //第一个分组
        //去除url字符串中的'\'符号
        url.erase(remove(url.begin(),url.end(),'\\'),url.end());
        //去除url字符串中的‘
        url.erase(remove(url.begin(),url.end(),'\''),url.end());
        //去除不包含//的地址类型，如：#default#homepage
        if(url.find("//") == np)
            continue;
        //保留//之后的地址
        url = url.substr(url.find("//")+2);        
        //排除重复元素
        for(auto elmt:href_list)
            if(elmt == url) cnt++;
        if(cnt==0){
            href_list.push_back(url);
            // cout << url << endl;
        }            
        //重置重复计数cnt和查找位置begin
        cnt = 0;
    }
    writeLocalFile(href_list,"href_list_"+host_now+".txt","",g_downPath+"/"+host_now);
#endif

    return url_list;
}

void writeLocalFile(const list<string> &strlist,const string &filename,const char* suffix/*=""*/,const string &downpath/*=defDownPath*/){
    string filepath = downpath + '/' + filename;
    ofstream outfile(filepath,fstream::out);
    if(!outfile.is_open()){
        cout << "file open error\n";
        outfile.close();
        return;
    }
    for(auto str:strlist)
        outfile << str << endl;
    outfile.close();
    cout << "created file: " << filepath << suffix << endl;
}

void writeLocalFile(const string &scontent,const string &filename,const char* prefix/*=""*/,const string &downpath/*=defDownPath*/){
    string filepath = downpath + '/' + filename;
    ofstream outfile(filepath,fstream::out/*|fstream::binary*/);
    //如果文件名中出现异常字符 就会报错
    if(!outfile.is_open()){
        cout << "file open error\n";
        outfile.close();
        return;
    }
    //图片格式中包含的\0都是有意义的，一个都不能少
    for(int i=0;i<scontent.size();i++)
        outfile << scontent[i];
    
    outfile.close();
	cout << prefix << "created file: " << filepath << endl;
}
