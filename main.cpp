#include "code/webserver.h"
#include <string>
using namespace std;



int main()
{   //数据库连接信息
    string user = "root";
    string passwd = "";
    string databasename = "yourdb";

    //端口号
    int PORT = 8888;

    //触发组合模式,默认listenfd ET + connfd ET
    int TRIGMode = 3;               //取值范围0～3
    //优雅关闭链接，默认不使用
    int OPT_LINGER = 0;
    //数据库连接池数量，默认8
    int sql_num = 8;
    //线程池内的线程数量，默认8
    int thread_num = 8;
    //日志,默认关闭
    int close_log = 1;
    //并发模型，0是Proactor，1是Reactor
    int actor_model = 0; 


    WebServer server;

    //初始化
    server.init(PORT, user, passwd, databasename, 
                OPT_LINGER, TRIGMode,  sql_num,  thread_num, 
                close_log, actor_model);
    

    //日志
    server.log_write();

    //数据库
    server.sql_pool();

    //线程池
    server.thread_pool();

    //触发模式
    server.trig_mode();

    //监听
    server.eventListen();

    //运行
    server.eventLoop();

    return 0;
}