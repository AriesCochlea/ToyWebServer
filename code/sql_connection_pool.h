#ifndef _CONNECTION_POOL_
#define _CONNECTION_POOL_

#include <list>
#include <mysql/mysql.h> //MYSQL，mysql_init()， mysql_real_connect()， mysql_close()
#include <string>
#include <semaphore.h>   //#include <semaphore>需要C++20支持
#include <mutex>   
using namespace std;


/*
127.0.0.1相当于使用网络去访问本机
localhost不用联网即可访问本地服务权限
*/



class connection_pool
{
public:
	//单例模式
	static connection_pool *GetInstance(){
		static connection_pool connPool;           //局部静态变量单例模式之懒汉式：C++11起能保证线程安全，无需加锁
		return &connPool;
}

	MYSQL *GetConnection();				 //从数据库连接池中返回一个可用连接
	bool ReleaseConnection(MYSQL *conn); //释放当前使用的数据库连接

	void init(string url, string User, string PassWord, string DataBaseName, int Port, int MaxConn, int close_log); 

private:
	connection_pool();
	~connection_pool();
	list<MYSQL *> connList;             //连接池
	mutex locker;                       //用互斥锁锁住connList
    sem_t sem;  
	int m_close_log; //是否关闭日志
};



class connectionRAII{

public:
	connectionRAII(MYSQL **con, connection_pool *connPool);
	~connectionRAII();
	
private:
	MYSQL *conRAII;
	connection_pool *poolRAII;
};

#endif
