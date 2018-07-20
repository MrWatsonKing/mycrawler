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
#include <vector>
#include <algorithm>
using namespace std;
#define defDownPath "/home/king/vscode/mysurfer/download"

int connectHost(const string &url);
string getWebPage(int sfd,const string &url); //局部变量只能返回值 不能返回引用
void drawResources(int sfd,const string &pageContent);
void parseHostAndPagePath(const string &url, string &hostUrl, string &pagePath);
list<string> getHttps(const string &pageContent,const char* type="images");
void writeLocalFile(const string &content,const string &filename,const string &downpath=defDownPath);
void writeLocalFile(const list<string> &strlist,const string &filename,const string &downpath=defDownPath);
void writeLocalFile(const vector<char> &vbytes,const string &filename,const string &downpath=defDownPath);

#endif //SURFER_H