#include "tran.h"
#include <unistd.h>
// #include<cstdlib>
mutex curlmtx;
bool ifend = false;
string result_path;
int ThreadCount;
rapidjson::Document config;
rapidjson::Document TranHistory;
//Thread exit strategy
int numConcurrentThreads=0;
//翻译源 
//0:Baidu 
//1:Translator-shell
int sourceapi=0;
mutex nctmtx;
mutex storemtx;
//缓存翻译结果
bool StorageAdd(string& ori,string& q){
    using namespace rapidjson;
    if(!TranHistory.HasMember(ori.c_str())){
        //logln("This qreny is never recorded");
        //cin.get();
        lock_guard<mutex> lock(storemtx);
        Value key(ori.c_str(),TranHistory.GetAllocator());
        Value value(q.c_str(),TranHistory.GetAllocator());
        TranHistory.AddMember(key,value,TranHistory.GetAllocator());
        writeJsonFile(result_path+"tranhistory.json",TranHistory);
        return true;
    }
    return false;
}




bool StorageCheck(string& ori){
    if(TranHistory.HasMember(ori.c_str())){
        return true;
    }
    return false;
}
string StorageGet(string &ori){
    if (TranHistory.HasMember(ori.c_str())){
        return TranHistory[ori.c_str()].GetString();
    }
    return "";
}





std::string unicodeToString(const std::string &str)
{
    std::string u8str;
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> converter;
    for (size_t i = 0; i < str.length();)
    {
        char32_t uhan = strtol(str.substr(i, 4).c_str(), nullptr, 16);
        u8str += converter.to_bytes(uhan);

        i += 4;
    }

    return u8str;
}

mutex timemtx;
int limit=0;
int maxlimit;
void timer()
{    
    {
        lock_guard<mutex> lock(nctmtx);
        ++numConcurrentThreads;
    }

    while (true)
    {
        limit = config["qps"].GetInt();
        this_thread::sleep_for(chrono::seconds(1));
        if(ifend==true) break;
    }
    
    {
        lock_guard<mutex> lock(nctmtx);
        --numConcurrentThreads;
    }

}
void qpsWait()
{
    while (limit <= 0)
    {
        this_thread::sleep_for(chrono::milliseconds(10));
        //cout<<"wait for curl queue"<<endl;
    }
    {
        lock_guard<mutex> lock(timemtx);
        --limit;
    }
}
void repl(string& letter,string from,string to,bool direct){
    if(direct==true)
    while(letter.find(from)!=string::npos){
        letter.replace(letter.find(from),from.length(),to);
    }
    else
    while(letter.find(to)!=string::npos){
        letter.replace(letter.find(to),to.length(),from);
    }
}
//修正一些文本错误并补全一些用于处理的内容
void Fix(string& ori,string& q){
    //添加规则需要三个步骤
    uint strnum=2;
    string strmap[strnum]={"\\\\/","\\\\\\\\N"};
    string strrep[strnum]={"/","\\\\N"};
    for(int i=0;i<strnum;i++){
        if(q.find(strmap[i])!=string::npos && ori.find(strmap[i])==string::npos){
            q.replace(q.find(strmap[i]),strmap[i].length(),strrep[i]);
        }
    }
    repl(q," ","-ASPACE-",0);
    repl(q,"\\n","-AWRAP-",0);
    repl(q,"「","-AJYHFI-",0);
    repl(q,"」","-AJYHSE-",0);
    repl(q,"<","-ALANGEL-",0);
    repl(q,">","-ARANGEL-",0);
    repl(q,"/","-AXIE-",0);
    repl(q,"\\{","-AQWDC-",0);
    repl(q,"\\","-AKSDP-",0);
    repl(q,"\\N","-ANKKS",0);
    repl(q,":","-AMAOH-",0);

}
void Fix(string& q){
    repl(q," ","-ASPACE-",0);
    repl(q,"\\n","-AWRAP-",0);
    repl(q,"「","-AJYHFI-",0);
    repl(q,"」","-AJYHSE-",0);
    repl(q,"<","-ALANGEL-",0);
    repl(q,">","-ARANGEL-",0);
    repl(q,"/","-AXIE-",0);
    repl(q,"\\{","-AQWDC-",0);
    repl(q,"\\","-AKSDP-",0);
    repl(q,"\\N","-ANKKS",0);
    repl(q,":","-AMAOH-",0);
}
//CURL *curl[]=new CURL[ThreadCount];
//uint *curlQueue[]=new uint[ThreadCount];
using namespace std;
void fileio();
void log(string logstr);
const string weburl = "https://fanyi-api.baidu.com/api/trans/vip/translate?";
const string salt = "28473837363627",from = "jp", to = "zh";
string translator(string q)
{
    string appid = config["appid"].GetString();
    string key = config["key"].GetString();
    if(sourceapi==0){
    logln("cURL be called");
    if (q == "")
    {
        logln("query is NULL");
        return "";
    }
    logln("query is not NULL");
    
    //进行过滤，手动排除一些莫名其妙的错误
    {
    
    //if(!isjp(q)){return q;}
    repl(q," ","-ASPACE-",1);
    repl(q,"\\n","-AWRAP-",1);
    repl(q,"「","-AJYHFI-",1);
    repl(q,"」","-AJYHSE-",1);
    repl(q,"<","-ALANGEL-",1);
    repl(q,">","-ARANGEL-",1);
    repl(q,"/","-AXIE-",1);
    repl(q,R"(\{)","-AQWDC-",1);
    repl(q,R"(\)","-AKSDP-",1);
    repl(q,R"(\N)","-ANKKS",1);
    repl(q,":","-AMAOH-",1);
    // repl(q,"——")

    }
    string ori=q;
    if(StorageCheck(ori)){
        return StorageGet(ori);
    }
    qpsWait();
    int i = 0;
    // {
        // lock_guard<mutex> lock(curlmtx);
        // while (true)
        // {
            // if (*curlQueue[i] == 0)
            // {
                // *curlQueue[i] = 1;
                // break;
            // }
            // if (i == (ThreadCount-1))
            // {
                // i = 0;
                // this_thread::sleep_for(chrono::seconds(1));
                // continue;
            // }
            // ++i;
        // }
    // }
    // this_thread::sleep_for(chrono::seconds(1));
    string url, sign, strres, buffer;
    sign = domd5(appid + q + salt + key);
    url = weburl + "q=" + UrlEncode(q) + "&from=" + from + "&to=" + to + "&appid=" + appid + "&salt=" + salt + "&sign=" + sign;
    // curlmtx.lock();
    // CURL *curl;
    //cout<<"url is "<<url<<endl;
    CURL* curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &strres);
        CURLcode res = curl_easy_perform(curl);
        if (res == CURLE_OK)
        {
            curl_easy_cleanup(curl);
            //curlQueue[i] = 0;
            // curlmtx.unlock();
            //处理API报错
            size_t po1 = strres.find("error");
            if (po1 != string::npos)
            {
                //针对54003的OPS过大报错，重试请求
                if(strres.find("54003")!=string::npos){
                    cout << strres << endl;
                    return "-ERROR-54003"; 
                }
                cout << strres << endl;
                exit(-1);
            }
            // cout<<"now the raw raw strres="<<strres<<endl;
            // cin.get();
            po1 = strres.find("\"dst\":\"");
            size_t po2 = strres.find("\"", po1 + 7);
            strres = strres.substr(po1 + 7, po2 - po1 - 7);
            // cout<<"the url="<<url<<endl;
            string &temp = url = "";
            po1 = strres.find("\\u");
            // q="";
            // cout<<"strres="<<strres<<endl;
            while (po1 != string::npos)
            {
                // cout<<strres.substr(po1+2,4)<<endl;
                // buffer=stoi(strres.substr(po1+2,4),nullptr,16);
                // cout<<buffer<<endl;
                // cin.get();
                strres.replace(po1, 6, unicodeToString(strres.substr(po1 + 2, 4)));
                // cout<<strres<<endl;
                // cin.get();
                po1 = strres.find("\\u");
            }
            // cout << "the translated strres=" << strres << endl;
            // cout << "the translated strres's HEX=" << std::hex<<*(unsigned int*)(strres.c_str()) <<std::dec<< endl;
            // cin.get();
            
            //Fix()对目前已发现的莫名其妙的错误进行修正
            Fix(ori,strres);
            StorageAdd(ori,strres);
            logln(ori+"->"+strres);
            return strres;
        }
        //curl request失败
        else
        {
            curl_easy_cleanup(curl);
            // curlmtx.unlock();
            cout << "ERROR!" << curl_easy_strerror(res) << endl;
            cout << "now translateint q=" << q << endl;
            return "-ERROR-";
        }
    }
    //初始化失败
    else
    {
        curl_easy_cleanup(curl);
        //curlmtx.unlock();
        cout << "初始化CURL失败\n";
        cout << "now translateint q=" << q << endl;
        return "-ERROR-";
    }
    return "";
    }
    else if(sourceapi==1){
        if (q == "")
        {
            logln("query is NULL");
            return "";
        }
        logln("query is not NULL");
        
        //进行过滤，手动排除一些莫名其妙的错误
        {
        
        //if(!isjp(q)){return q;}
        repl(q," ","-ASPACE-",1);
        repl(q,"\\n","-AWRAP-",1);
        repl(q,"「","-AJYHFI-",1);
        repl(q,"」","-AJYHSE-",1);
        repl(q,"<","-ALANGEL-",1);
        repl(q,">","-ARANGEL-",1);
        repl(q,"/","-AXIE-",1);
        repl(q,R"(\{)","-AQWDC-",1);
        repl(q,R"(\)","-AKSDP-",1);
        repl(q,R"(\N)","-ANKKS-",1);
        repl(q,":","-AMAOH-",1);
        // repl(q,"——")
    
        }
        string ori=q;
        if(StorageCheck(ori)){
            return StorageGet(ori);
        }
        qpsWait();
        char buffer[65536];
        string command="trans -brief \""+q+"\" --source=ja --target=zh -no-auto";
        FILE* s=popen(command.c_str(),"r");
        fgets(buffer,65536,s);
        string strres=buffer;
        strres[strres.length()-1]=NULL;
        if(strres.find("[ERROR]")!=string::npos){
            cerr<<"Translator-shell ERROR"<<endl
                <<strres<<endl;
            return "-ERROR-54003";
        }
        //Fix()对目前已发现的莫名其妙的错误进行修正
        Fix(ori,strres);
        StorageAdd(ori,strres);
        logln(ori+"->"+strres);
        return strres;
    }
    else if(sourceapi==2){
        
    }
    else {
        cerr<<"sourceapi is NULL"<<endl;
        exit(-1);
    }
}
