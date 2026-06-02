#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <list>
#include <cstdio>
#include <exception>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <vector>
#include <atomic>
#include "sql_connection_pool.h"

template <typename T>
class threadpool
{
public:
    /*thread_number是线程池中线程的数量，max_requests是请求队列中最多允许的、等待处理的请求的数量*/
    threadpool(int actor_model, connection_pool *connPool, int thread_number = 8, int max_request = 10000);
    ~threadpool();
    bool append(T *request, int state);
    bool append_p(T *request);

private:
    /*工作线程运行的函数，它不断从工作队列中取出任务并执行之*/
    void run();

private:
    int m_thread_number;        //线程池中的线程数
    int m_max_requests;         //请求队列中允许的最大请求数
    std::vector<std::thread> m_threads; //线程池
    std::list<T *> m_workqueue; //请求队列
    std::mutex m_queuelocker;   //保护请求队列的互斥锁
    std::condition_variable m_queuestat; //是否有任务需要处理
    connection_pool *m_connPool;  //数据库
    int m_actor_model;          //模型切换
    std::atomic<bool> m_stop;   //线程池停止标志
};
template <typename T>
threadpool<T>::threadpool( int actor_model, connection_pool *connPool, int thread_number, int max_requests) : m_actor_model(actor_model), m_thread_number(thread_number), m_max_requests(max_requests), m_connPool(connPool)
{
    m_stop.store(false);
    if (thread_number <= 0 || max_requests <= 0)
        throw std::exception();
    for (int i = 0; i < thread_number; ++i)
    {
        m_threads.emplace_back(&threadpool::run, this);
    }
}
template <typename T>
threadpool<T>::~threadpool()
{
    m_stop.store(true);
    m_queuestat.notify_all();
    for (auto &th : m_threads)
    {
        if (th.joinable())
            th.join();
    }
}
template <typename T>
bool threadpool<T>::append(T *request, int state)
{
    std::unique_lock<std::mutex> lock(m_queuelocker);
    if (m_workqueue.size() >= m_max_requests)
    {
        return false;
    }
    request->m_state = state;
    m_workqueue.push_back(request);
    lock.unlock();
    m_queuestat.notify_one();
    return true;
}
template <typename T>
bool threadpool<T>::append_p(T *request)
{
    std::unique_lock<std::mutex> lock(m_queuelocker);
    if (m_workqueue.size() >= m_max_requests)
    {
        return false;
    }
    m_workqueue.push_back(request);
    lock.unlock();
    m_queuestat.notify_one();
    return true;
}
template <typename T>
void threadpool<T>::run()
{
    while (true)
    {
        std::unique_lock<std::mutex> lock(m_queuelocker);
        while ( (!m_stop.load()) && m_workqueue.empty())
        {
            m_queuestat.wait(lock);
        }
        if (m_stop.load() && m_workqueue.empty())
        {
            return;
        }
        T *request = m_workqueue.front();
        m_workqueue.pop_front();
        lock.unlock();
        
        if (!request)
            continue;
        if (1 == m_actor_model)
        {
            if (0 == request->m_state)
            {
                if (request->read_once())
                {
                    request->improv = 1;
                    connectionRAII mysqlcon(&request->mysql, m_connPool);
                    request->process();
                }
                else
                {
                    request->improv = 1;
                    request->timer_flag = 1;
                }
            }
            else
            {
                if (request->write())
                {
                    request->improv = 1;
                }
                else
                {
                    request->improv = 1;
                    request->timer_flag = 1;
                }
            }
        }
        else
        {
            connectionRAII mysqlcon(&request->mysql, m_connPool);
            request->process();
        }
    }
}
#endif
