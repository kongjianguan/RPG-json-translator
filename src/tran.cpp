#include "header/allinclude.h"
using namespace std;
// 配置选项数
//const int config_count=4;
// 线程锁
mutex mtx;
mutex outmtx;
mutex queuemtx;
mutex tnmtx;
// 目标文件名
string file_name;
string file_path;
string oripath;
// bool ifend = false;
//  字符缓存
char temp;
// 用于读取的对象
ifstream file;
string myname;
string myfullpath;
char **thisargv;
uint thisargc;
uint thread_num;
//
uintptr_t queue[65536];
// 用于输出的对象
ofstream saveres;

// Json data path
string datapath = "0";
// 日志对象
ofstream loger("./log/run.log", ios_base::app);
// 行数
unsigned int line = 1;
// 列数
double col = 1;
// 读取字符数
double fp = 0;

void translatorThread(rapidjson::Value *root);

// 打印日志
//  日志文件函数
void log(string logstr)
{
    loger << logstr << "    " << col << " " << line << "  " << fp << endl;
}
void fileio()
{
    // 打开目标文件
    if (file.is_open())
    {
        file.seekg(0);
        log("成功打开 " + file_name);
    }
    else
    {
        log("无法打开 " + file_name);
        exit(-1);
    }
    // 打开输出文件
    if (saveres.is_open())
    {
        log("成功打开 result/" + file_name);
    }
    else
    {
        log("无法打开 result/" + file_name);
        exit(-1);
    }
    cout << "成功打开所需文件\n";
}
void getname(const string &sss)
{
    file_name = sss;
}
void action(rapidjson::Value &root);
// void amountOfaction(rapidjson::Value &root);
// void arrayIterator(rapidjson::Value &root)
// {
//     size_t array_size = root.size();
//     for (unsigned int i = 0; i <= array_size; ++i)
//     {
//         if (root[i].isObject() or root[i].isArray())
//         {
//             action(root[i]);
//         }
//     }
// }
void threadManager();
rapidjson::Document sfile;
void startaction()
{
    logln("打开"+file_name);
    sfile = readJsonFile(file_path + file_name);
    string command="cp ./tranhistory.json "+result_path;
    system(command.c_str());
    col = 0;
    fp = 0;
    ifend = false;
    numConcurrentThreads = 0;
    thread threadmanager(threadManager);
    threadmanager.detach();
    thread timerThread(timer);
    timerThread.detach();
    action(sfile);
    this_thread::sleep_for(chrono::seconds(1));
    while (true)
    {
        if (fp == col)
        {
            //this_thread::sleep_for(chrono::seconds(10));
            // double mark=fp/col;
            // double mark2=mark;
            // for(int i=0;mark==mark2;++i){
                // mark=fp/col;
                // this_thread::sleep_for(chrono::seconds(1));
                // if(i>=5){break;}
            // }
            ifend = true;
            while (numConcurrentThreads != 0)
            {
                printf("wait for subThread(%d)",numConcurrentThreads);
                this_thread::sleep_for(chrono::seconds(2));
            }
            break;
        }
        this_thread::sleep_for(chrono::seconds(1));
        //logln("8282");
    }
    //logln("9990");
    if(col==0 || fp==0){
        string command="cp "+file_path + file_name+" "+result_path;
        system(command.c_str());
        cout<<file_path + file_name<<"无处理"<<endl;
        return ;
    }
    writeJsonFile(result_path + file_name, sfile);
    printf("共处理%f次\n",fp);
    printf("Completed!\n");
    command="cp "+result_path+"tranhistory.json .";
    system(command.c_str());
    //this_thread::sleep_for(chrono::seconds(5));
    //cin.get();
}
mutex queueWorkermtx;
void threadAction()
{
    {
        lock_guard<mutex> lock(nctmtx);
        ++numConcurrentThreads;
    }
    this_thread::sleep_for(chrono::seconds(2));
    while (ifend == false)
    {
        // logln("TaskThread构造");
        //  tnmtx.lock();
        //  ++thread_num;
        //  tnmtx.unlock();
        //logln("TaskThread loop");
        //queueWorkermtx.lock();
        for (int i = 0; i < 65535 && ifend==false; i += 2)
        {
            // logln("Task Thread is travesaling");
            if (queue[i] == 1)
            {
                queueWorkermtx.lock();
                if(queue[i]==2){
                    queueWorkermtx.unlock();
                    continue;
                }
                queue[i] = 2;
                queueWorkermtx.unlock();
                thread transt(translatorThread, reinterpret_cast<rapidjson::Value *>(queue[i + 1]));
                transt.join();
                queue[i] = 0;
                logln("thread running");
                break;
            }
            if(ifend==true){
                lock_guard<mutex> lock(nctmtx);
                --numConcurrentThreads;
                return;
            }
        }
        this_thread::sleep_for(chrono::milliseconds(1));
        //if(ifend==true){break;}
        //this_thread::sleep_for(chrono::milliseconds(10));
    }
    {
        lock_guard<mutex> lock(nctmtx);
        --numConcurrentThreads;
    }
}
void threadManager()
{
    for(int i=0;i!=config["threads"].GetInt();++i){
        thread ta(threadAction);
        ta.detach();
    }
    // while (true)
    // {
    // this_thread::sleep_for(chrono::seconds(2));
    // if (ifend == true)
    // {
    // return;
    // }
    // }
    return;
}
void queueAdd(uintptr_t addres)
{
    lock_guard<mutex> lock(queuemtx);
    ++col;
    while (true)
    {
        for (uint i = 0; i < 65535; i += 2)
        {
            if (queue[i] == 0)
            {
                //logln("New Queue task");
                queue[i] = 1;
                queue[i + 1] = addres;
                return;
            }
        }
    }
}
// mutex colmtx;
void translatorThread(rapidjson::Value *rootp)
{
    //{
    // lock_guard<mutex> lock(outmtx);
    // cout << "This root=" << root.asString() << endl;
    // printJsonType(root);
    // writeJsonFile("./666.json", root);
    //}
    {
        lock_guard<mutex> lock(nctmtx);
        ++numConcurrentThreads;
    }
    rapidjson::Value &root = *rootp;
    if (root.IsString())
    {
        //logln("The rootp is String");
        string strtemp = translator(root.GetString());
        // cin.get();
        if (strtemp == "-ERROR-54003")
        {
            while (strtemp == "-ERROR-54003")
            {
                strtemp = translator(root.GetString());
            }
        }
        else if (strtemp == "-ERROR-")
        {
            strtemp = translator(root.GetString());
            if (strtemp == "-ERROR-")
            {
                logln("request retry failed");
                exit(-1);
            }
        }
        lock_guard<mutex> lock(mtx);
        root.SetString(strtemp.c_str(),sfile.GetAllocator());
        ++fp;
    }
    {
        lock_guard<mutex> lock(nctmtx);
        --numConcurrentThreads;
    }
    DoC("已完成" + to_string((int)fp) + "/" + to_string((int)col) + " | " + to_string(fp / col * 100) + "%");
}
void action(rapidjson::Value &root)
{
    if (root.IsObject())
    {
        //cout << "是对象" << endl;
        // cin.get();
        if (root.HasMember("displayName"))
        {
            thread t(queueAdd, reinterpret_cast<uintptr_t>(&root["displayName"]));
            t.detach();
            // {
            // this_thread::sleep_for(chrono::seconds(1));
            // cout<<"wait for queue"<<endl;
            // }
        }
        if (root.HasMember("id") && root.HasMember("name"))
        {
            // cout << 10023 << endl;
            thread t(queueAdd, reinterpret_cast<uintptr_t>(&root["name"]));
            t.detach();
            // {
            // this_thread::sleep_for(chrono::seconds(1));
            // cout<<"wait for queue"<<endl;
            // }
            if (root.HasMember("description"))
            {
                thread t(queueAdd, reinterpret_cast<uintptr_t>(&root["description"]));
                t.detach();
                // {
                // this_thread::sleep_for(chrono::seconds(1));
                // cout<<"wait for queue"<<endl;
                // }
            }
        }
        if (root.HasMember("code") && root["code"].GetInt() == 401)
        {
            if (root.HasMember("parameters"))
            {
                thread t(queueAdd, reinterpret_cast<uintptr_t>(&root["parameters"][0]));
                t.detach();
                // {
                // this_thread::sleep_for(chrono::seconds(1));
                // cout<<"wait for queue"<<endl;
                // //调试语句，观察队列情况
                // //for(int i=1;i<=255;i+=2){cout<<"queue["<<i<<"]="<<(reinterpret_cast<Json::Value*>(queue[i]))->asString()<<endl;}cin.get();
                // }
            }
        }
        for (auto& i : root.GetObject())
        {
            if (i.value.IsObject() or i.value.IsArray())
            {
                action(i.value);
            }
        }
    }
    else if (root.IsArray())
    {
        for (auto& i : root.GetArray())
        {
            if (i.IsObject() or i.IsArray())
            {
                action(i);
            }
        }
    }
}
void mainProcess(const string &file_name)
{
    getname(file_name);
    startaction();
    //cout<<2<<endl;

}
void getconf();
void opt(string inter)
{
    if (inter=="logoff")
    {
        iflog = false;
        cout << "日志关闭" << endl;
    }
    else if (inter == "logon")
    {
        iflog = true;
        cout << "日志打开" << endl;
    }
    else if (inter == "quit")
    {
        exit(0);
    }
    else if (inter == "-help")
    {
        cout << "|  quit                        退出" << endl
             << "|  logon/logoff   打开/关闭日志输出" << endl
             << "|  lq        列出队列中使用中的任务" << endl
             << "|  laq           列出队列中全部任务" << endl
             << "|  switchsapi          切换源API"<<endl
             << "|  getconf         重新获取配置"<<endl;
    }
    else if (inter == "lq")
    {
        for (int i = 0; i != 65536; i += 2)
        {
            if (queue[i] == 1 or queue[i] == 2)
                cout << "queue[" << i << "]" << (*reinterpret_cast<rapidjson::Value *>(queue[i + 1])).GetString() << endl;
        }
    }
    else if (inter == "laq")
    {
        for (int i = 0; i != 65536; ++i)
        {
            cout << "queue[" << i << "]" << queue[i] << endl;
        }
    }
    //更换源
    else if(inter== "switchsapi"){
        sourceapi=!(bool)sourceapi;
        cout<<"已更换源"<<endl;
    }
    else if(inter == "getconf"){
        getconf();
        cout<<"尝试写入配置"<<endl;
    }
    else if(inter=="switchdoc"){
        cout<<"请输入要切换的api"<<endl;
        cin>>sourceapi;
        switch(sourceapi){
            case 0:break;
            case 1:break;
            //case 2:break;
            default :
                cout<<"输入错误"<<endl;
                exit(-1);
        }
        cout<<"切换进度输出"<<endl;
    }
}
bool ifuirun=true;
void userInteractThread(int argc, char *argv[])
{
    string inter;
    for (int i = 0; i != argc; ++i)
    {
        opt(argv[i]);
    }
    cout << "输入-help获取帮助" << endl;
    while (getline(cin, inter) && ifuirun) opt(inter);
}
auto find(string& str,size_t po,char flag)->size_t{
    for(;po!=str.length();++po){
        if(str[po]==flag){
            return po;
        }
    }
    return string::npos;
}
//获取配置数据
void getconf(){
    using namespace rapidjson;
    //string note=";";
    ifstream ifs("./tranconf.json");
    if(!ifs.is_open()){
        cerr<<"无法打开配置文件，请检查是否存在!\n";
        exit(-1);
    }
    config=readJsonFile("./tranconf.json");
    string configopt[]={"threads","qps","appid","key"};
    for (string& i :configopt){
        if(!config.HasMember(i.c_str())){
            cerr<<"配置缺失"<<endl;
            exit(-1);
        }
    }
    //cout<<config.GetString();
    // char ch;
    // string config;
    // //string* allconfig[conf_count]
    // while(ifs.get(ch)){
        // if(ch==' ' || ch == '\n' || ch == '\r'){
            // continue;
        // }
        // config+=ch;
    // }
    // cout<<"config="<<config<<endl;
    // size_t po;
    // po=config.find("threads=");
    // if(po!=string::npos){
        // cout<<config.substr(po+8,config.find(';',po)-po-8)<<endl;
        // ThreadCount=stoi(config.substr(po+8,config.find(';',po)-po-8));
    // }
    // else {
        // cerr<<"配置缺失";
        // exit(-1);
    // }
    // po=config.find("qps=");
    // if(po!=string::npos){
        // cout<<config.substr(po+7,config.find(";",po)-po-7);
        // maxlimit=stoi(config.substr(po+7,config.find(";",po)-po-7));
    // }
    // else {
        // cerr<<"配置缺失";
        // exit(-1);
    // }
    // po=config.find("appid=");
    // if(po!=string::npos){
        // cout<<config.substr(po+6,config.find(";",po)-po-6);
        // appid=config.substr(po+6,config.find(";",po)-po-6);
    // }
    // else {
        // cerr<<"配置缺失";
        // exit(-1);
    // }
    // po=config.find("key=");
    // if(po!=string::npos){
        // cout<<config.substr(po+4,config.find(";",po)-po-4);
        // key=config.substr(po+4,config.find(";",po)-po-4);
    // }
    // else {
        // cerr<<"配置缺失";
        // exit(-1);
    // }
}


int main(int argc, char *argv[])
{
    // myname=argv[0];
    // myfullpath="./"+myname;
    // thisargv=new char*[argc-1];
    //  for(int i=0;i!=argc-1;++i){
    //      thisargv[i]=new char[strlen(argv[i+1])+1];
    //      strcpy(thisargv[i],argv[i+1]);
    //  }
    // thisargc=argc-1;
    using namespace std::filesystem;
    getconf();
    // check if path exists in command-line arguments
    for (int i = 0; i != argc; ++i)
    {
        if (i == (argc - 1))
        {
            cout << "Please enter the path of \"www/data\"." << endl;
            cin >> file_path;
            break;
        }
        else if (!strcmp(argv[i], "-p"))
        {
            file_path = argv[i+1];
            break;
        }
    }
    for (int i = 0; i != argc; ++i)
    {
        if (i == (argc - 1))
        {
            cout << "Please enter the source api"<< endl;
            cin >> sourceapi;
            break;
        }
        else if (!strcmp(argv[i], "-sourceapi"))
        {
            sourceapi = *(argv[i+1])^48;
            break;
        }
    }
    if (sourceapi==2){
        puts("sourceapi=2   正在遍历当前目录下/翻译文件.json");
    }
    thread ui(userInteractThread, argc, argv);
    if (file_path[file_path.length() - 1] != '/')
    {
        file_path += "/";
    }
    path spath(file_path);
    if (!exists(spath))
    {
        cout << "The path "<<file_path<<" is invalid.Please enter valid path.\n";
        exit(1);
    }
    result_path = file_path + "../result/";
    // Check if history of translation exists
    ifstream history_file(result_path + "tranhistory.json");
    if (history_file.is_open())
    {
        TranHistory = readJsonFile(result_path + "tranhistory.json");
    }
    else TranHistory.SetObject();
    path rpath(result_path);
    if (!exists(rpath))
    {
        create_directories(rpath);
    }
    for (auto &it : directory_iterator(spath))
    {
        string source_file = it.path().filename().string();
        ifstream rf(result_path + source_file);
        if (!rf.is_open())
        {
            if (source_file.find(".json") != string::npos)
            {
                cout << "processing " << source_file << endl;
                // cin.get();
                //     ./res/data
                mainProcess(source_file);
                //cout<<1<<endl;
                // if(ifend==)
            }
        }
        else rf.close();
        //logln("filesystem loop");
    }
    string js_path=file_path+"../js/plugins/";
    path jspath(js_path);
    //未完成。这里的目的是使某些标识符保持一致
    if(0&&exists(jspath)){
        cout<<"发现插件目录，正在尝试同步某些对 www/data/*.json 的修改到 plugins 中"<<endl;
        for (auto &it : directory_iterator(jspath)){
            string plugin_file_path = it.path().filename().string();
            string plugin_file_content=readFile_str(js_path+plugin_file_path);
            logln("processing "+it.path().filename().string());
            for(auto& i : TranHistory.GetObject()){
                string fstr=i.name.GetString();
                Fix(fstr);
                logln("正在查找 : "+fstr);
                size_t po1=plugin_file_content.find('"');
                size_t po2=plugin_file_content.find('"',po1);
                while(po1!=string::npos){
                    string range=plugin_file_content.substr(po1,po2-po1);
                    size_t po=range.find(fstr);
                    while(po!=string::npos){
                        plugin_file_content.replace(po+po1,fstr.length(),i.value.GetString());
                        po=range.find(fstr,po);
                    }
                    po1=plugin_file_content.find('"',po2);
                    po2=plugin_file_content.find('"',po1);
                }
            }
            writeFileFromString(result_path+plugin_file_path,plugin_file_content);
        }
        cout<<"完成同步修改"<<endl;
    }
    ifuirun=!ifuirun;
    cout<<"请输入任意内容并回车以结束本进程"<<endl;
    ui.join();
    return 0;
}