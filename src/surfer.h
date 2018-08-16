#ifndef SURFER_H
#define SURFER_H

#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <regex>
#include <thread>
#include <strings.h>
#include <iostream>
#include <fstream>
#include <list>
#include <vector>
#include <algorithm>
using namespace std;
#define defDownPath "/home/king/vscode/mysurfer/download"

int connectHost(const string &url);
vector<char> getWebPage(const string &url); //局部变量只能返回值 不能返回引用
void drawResources(const vector<char> &vcontent);
void parseHostAndPagePath(const string &url, string &hostUrl, string &pagePath);
list<string> getHttps(const vector<char> &vcontent,const char* type="images");
void writeLocalFile(const string &content,const string &filename,const string &downpath=defDownPath);
void writeLocalFile(const list<string> &strlist,const string &filename,const string &downpath=defDownPath);
void writeLocalFile(const vector<char> &vcontent,const string &filename,const string &downpath=defDownPath,const char* prefix="");

#endif //SURFER_H