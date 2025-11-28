#include <iomanip>
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <crypto++/md5.h>
#include <crypto++/hex.h>
#include <assert.h>
#include <rapidjson/reader.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>
#include "rapidjson/filewritestream.h"
#include "rapidjson/filereadstream.h"
#include <rapidjson/writer.h>
#include "rapidjson/error/en.h"
#include <iostream>
#include <fstream>
#include <string>
#include <curl/curl.h>
#include <codecvt>
#include <locale>
#include <thread>
#include <chrono>
#include <filesystem>
#include <cstdio>
using namespace std;
//URLencode相关实现
unsigned char ToHex(unsigned char x) 
{ 
    return  x > 9 ? x + 55 : x + 48; 
}
 
unsigned char FromHex(unsigned char x) 
{ 
    unsigned char y;
    if (x >= 'A' && x <= 'Z') y = x - 'A' + 10;
    else if (x >= 'a' && x <= 'z') y = x - 'a' + 10;
    else if (x >= '0' && x <= '9') y = x - '0';
    else assert(0);
    return y;
}
 
std::string UrlEncode(const std::string& str)
{
    std::string strTemp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        if (isalnum((unsigned char)str[i]) || 
            (str[i] == '-') ||
            (str[i] == '_') || 
            (str[i] == '.') || 
            (str[i] == '~'))
            strTemp += str[i];
        else if (str[i] == ' ')
            strTemp += "+";
        else
        {
            strTemp += '%';
            strTemp += ToHex((unsigned char)str[i] >> 4);
            strTemp += ToHex((unsigned char)str[i] % 16);
        }
    }
    return strTemp;
}
 
std::string UrlDecode(const std::string& str)
{
    std::string strTemp = "";
    size_t length = str.length();
    for (size_t i = 0; i < length; i++)
    {
        if (str[i] == '+') strTemp += ' ';
        else if (str[i] == '%')
        {
            assert(i + 2 < length);
            unsigned char high = FromHex((unsigned char)str[++i]);
            unsigned char low = FromHex((unsigned char)str[++i]);
            strTemp += high*16 + low;
        }
        else strTemp += str[i];
    }
    return strTemp;
}


string domd5(const string &input)
{
    string md5;
    CryptoPP::Weak1::MD5 hash;
    CryptoPP::byte digest[CryptoPP::Weak1::MD5::DIGESTSIZE];

    hash.CalculateDigest(digest, reinterpret_cast<const CryptoPP::byte *>(input.c_str()), input.length());

    CryptoPP::HexEncoder encoder;
    encoder.Attach(new CryptoPP::StringSink(md5));
    encoder.Put(digest, sizeof(digest));
    encoder.MessageEnd();

    // 将结果转换为小写字母
    for (auto &c : md5)
    {
        c = tolower(c);
    }

    return md5;
}

size_t WriteCallback(char *contents, size_t size, size_t nmemb, void *userp)
{
    ((string *)userp)->append((char *)contents, size * nmemb);
    return size * nmemb;
}

void writeFileFromString(const string& filename,const string& body)
{
    ofstream ofile(filename);
    ofile<<body;
    ofile.close();
}
string geterr(ifstream& ifs,size_t st){
    st-=size_t(100);
    ifs.seekg(st);
    char ch;
    string str;
    while(ifs.get(ch)){
        str+=ch;
    }
    return str;
}
rapidjson::Document readJsonFile(const string &filename)
{
    using namespace rapidjson;
    FILE* fp = fopen(filename.c_str(), "r"); // 非 Windows 平台使用 "r"
 
    char readBuffer[65536];
    FileReadStream is(fp, readBuffer, sizeof(readBuffer));
 
    Document root;
    root.ParseStream(is);
 
    fclose(fp);
    return root;
}
    
void writeJsonFile(const string & filename,const rapidjson::Document & root){
    using namespace rapidjson;
    //1.写json的工厂对象
    FILE* file= fopen(filename.c_str(),"w");
    while(file==0){
        file= fopen(filename.c_str(),"w");
    }
    char writeBuffer[65536];
    FileWriteStream fws(file,writeBuffer,sizeof(writeBuffer));
    Writer<FileWriteStream> wt(fws);
    root.Accept(wt);
    fclose(file);
}
bool isjp(const string& str) {
    for (char c : str) {
        unsigned char uc = static_cast<unsigned char>(c);
        if(uc >= 0x80) {return true;}
    }
    return false;
}
//switch log
bool iflog=false;

void logln(string content){
    if(iflog==true){cout<<content<<endl;}
}
//打印进度
bool ifdoc=true;
void DoC(string content){
    if(ifdoc) cout<<content<<endl;
}
string readFile_str(string filename){
    ifstream ifs(filename);
    char ch;
    string buffer;
    bool ifinstr=false;
    while(ifs.get(ch)){
        buffer+=ch;
    }
    return buffer;
}
