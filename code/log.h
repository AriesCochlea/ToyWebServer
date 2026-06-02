#ifndef LOG_H
#define LOG_H
#include <mutex>
#include <condition_variable>
#include <queue>
#include <string>
#include <stdarg.h>
#include <thread>
#include <exception>

using namespace std;

class Log
{
public:

    static Log *get_instance()
    {
        static Log instance;            //单例模式的“懒汉”式
        return &instance;               //从C++11开始静态局部对象不用加锁也能保证多线程下安全
    }

    
    bool init(const char *file_name, int close_log, int log_buf_size = 8192, int split_lines = 5000000, int max_queue_size = 0);

    void write_log(int level, const char *format, ...);

    void flush(void);

private:
    Log();
    virtual ~Log();
    void async_write_log()
    {
        string single_log;
        while (true)
        {
            unique_lock<mutex> lock(locker);
            while (m_log_queue.empty())
            {
                m_cv.wait(lock);
            }
            single_log = m_log_queue.front();
            m_log_queue.pop();
            lock.unlock();
            
            locker.lock();
            fputs(single_log.c_str(), m_fp);
            locker.unlock();
        }
    }

private:
    char dir_name[128]; //路径名
    char log_name[128]; //log文件名
    int m_split_lines;  //日志最大行数
    int m_log_buf_size; //日志缓冲区大小
    long long m_count;  //日志行数记录
    int m_today;        //因为按天分类,记录当前时间是那一天
    FILE *m_fp;         //打开log的文件指针
    char *m_buf;
    bool m_is_async;    //是否异步
    int m_close_log;    //关闭日志

    mutex locker;
    condition_variable m_cv;
    queue<string> m_log_queue; //阻塞队列
    thread *m_async_thread;
};


//__VA_ARGS__是一个可变参数的宏，定义时宏定义中参数列表的最后一个参数为省略号，在实际使用时会发现有时会加##，有时又不加。
#define LOG_DEBUG(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(0, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_INFO(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(1, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_WARN(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(2, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_ERROR(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(3, format, ##__VA_ARGS__); Log::get_instance()->flush();}
//Debug，调试代码时的输出，在系统实际运行时，一般不使用。
//Warn，这种警告与调试时终端的warning类似，同样是调试代码时使用。
//Info，报告系统当前的状态，当前执行的流程或接收的信息等。
//Error，输出系统的错误信息。



#endif
