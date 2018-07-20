#ifndef SURFER_H
#define SURFER_H

#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <strings.h>
#include <iostream>
#include <fstream>
#include <list>
#include <algorithm>
using namespace std;

//引用自网络 https://blog.csdn.net/orthocenterchocolate/article/details/38665937
void parseHostAndPagePath(const string url, string &hostUrl, string &pagePath);
string getWebPage(const string &url); //局部变量只能返回值 不能返回引用

//原创
void parseWebPage(const string &pageContent);
list<string> getHttps(const string &pageContent,const char* type="images");
void writeLocalFile(const string &content,const string &filename,const string &downpath=string("/home/king/vscode/mysurfer/download"));
void writeLocalFile(const list<string> &strlist,const string &filename,const string &downpath=string("/home/king/vscode/mysurfer/download"));


#endif //SURFER_H