#ifndef SURFER_H
#define SURFER_H

#include <netdb.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string>
#include <regex>
#include <thread>
#include <strings.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <list>
#include <algorithm>
using namespace std;
#define defDownPath "/home/king/vscode/mysurfer/download"

int connectHost(const string &url);
string getWebPage(const string &url); //局部变量只能返回值 不能返回引用
void drawResources(const string &scontent);
void parseHostAndPagePath(const string &url, string &hostUrl, string &pagePath);
list<string> getHttps(const string &vcontent,const char* type="images");
void writeLocalFile(const list<string> &strlist,const string &filename,const char* suffix="",const string &downpath=defDownPath);
void writeLocalFile(const string &scontent,const string &filename,const char* prefix="",const string &downpath=defDownPath);

#endif //SURFER_H