#include <mysql/mysql.h>
#include <string>
#include <list>
#include "sql_connection_pool.h"
#include "log.h"

using namespace std;

connection_pool::connection_pool()
{
}


//构造初始化
void connection_pool::init(string url, string User, string PassWord, string DBName, int Port, int MaxConn, int close_log)
{
	m_close_log = close_log;
	for (int i = 0; i < MaxConn; i++)
	{
		MYSQL *con = NULL;
		con = mysql_init(con);        //#include <mysql/mysql.h> 

		if (con == NULL)
		{
			LOG_ERROR("MySql init error");
			exit(1);
		}
		con = mysql_real_connect(con, url.c_str(), User.c_str(), PassWord.c_str(), DBName.c_str(), Port, NULL, 0);  //#include <mysql/mysql.h> 

		if (con == NULL)
		{
			LOG_ERROR("MySql connect error");
			exit(1);
		}
		connList.push_back(con);
	}

	sem_init(&sem, 0, MaxConn);
    //LOG_INFO("MysqlConnectionPool created");
}


//当有请求时，从数据库连接池中返回一个可用连接
MYSQL *connection_pool::GetConnection()
{
	MYSQL *con = NULL;
	if (0 == connList.size())
		return NULL;

	sem_wait(&sem); //信号量原子地减1，为0则阻塞等待
    locker.lock();
	con = connList.front();
	connList.pop_front();
	locker.unlock();
	return con;
}

//释放当前使用的连接
bool connection_pool::ReleaseConnection(MYSQL *con)
{
	if (NULL == con)
		return false;

	locker.lock();
	connList.push_back(con);
	locker.unlock();
	sem_post(&sem); //信号量原子地加1
	return true;
}



connection_pool::~connection_pool()
{
	locker.lock();
	if (connList.size() > 0)
	{
		list<MYSQL *>::iterator it;
		for (it = connList.begin(); it != connList.end(); ++it)
		{
			MYSQL *con = *it;
			mysql_close(con); //#include <mysql/mysql.h> 
		}
		connList.clear();
	}
	mysql_library_end();      //#include <mysql/mysql.h>
	locker.unlock();
}





connectionRAII::connectionRAII(MYSQL **SQL, connection_pool *connPool){
	*SQL = connPool->GetConnection();
	
	conRAII = *SQL;
	poolRAII = connPool;
}

connectionRAII::~connectionRAII(){
	poolRAII->ReleaseConnection(conRAII);
}