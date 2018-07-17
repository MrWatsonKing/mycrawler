#ifndef SURFER_H
#define SURFER_H

#include <netdb.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <strings.h>
#include <iostream>
using namespace std;

//引用自网络 https://blog.csdn.net/orthocenterchocolate/article/details/38665937
void parseHostAndPagePath(const string url, string &hostUrl, string &pagePath);
bool getWebPage(const string &url);

//原创


#endif //SURFER_H