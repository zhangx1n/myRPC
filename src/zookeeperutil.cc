#include "zookeeperutil.h"
#include "myrpcapplication.h"
#include <semaphore.h>
#include <iostream>

// 全局的watcher观察器   zkserver给zkclient的通知
void global_watcher(zhandle_t *zh, int type,
                   int state, const char *path, void *watcherCtx)
{
    if (type == ZOO_SESSION_EVENT)  // 回调的消息类型是和会话相关的消息类型
	{
		if (state == ZOO_CONNECTED_STATE)  // zkclient和zkserver连接成功
		{
			sem_t *sem = (sem_t*)zoo_get_context(zh);
            sem_post(sem);
		}
	}
}

ZkClient::ZkClient() : m_zhandle(nullptr)
{
}

ZkClient::~ZkClient()
{
    if (m_zhandle != nullptr)
    {
        zookeeper_close(m_zhandle); // 关闭句柄，释放资源  MySQL_Conn
    }
}

// 连接zkserver
void ZkClient::Start()
{
    std::string host = myrpcApplication::GetInstance().GetConfig().Load("zookeeperip");
    std::string port = myrpcApplication::GetInstance().GetConfig().Load("zookeeperport");
    std::string connstr = host + ":" + port;
    
	/*
	zookeeper_mt：多线程版本
	zookeeper的API客户端程序提供了三个线程
	API调用线程 
	网络I/O线程  pthread_create  poll
	watcher回调线程 pthread_create
	ZOOAPI:
	zhandle_t *zookeeper_init(const char *host, watcher_fn fn, int recv_timeout, 
		const clientid_t *clientid, void *context, int flags);
		host: 服务集群主机地址的字符串，地址格式为host:port，每组地址以逗号分隔。
		fn：回调函数地址，如果设置了监视点，znode节点发生相关变化后，该回调函数会被调用。
		recv_timeout：会话过期时间，单位毫秒。
		clientid:之前已建立的一个会话的客户端ID，用于客户端重新连接。在建立会话时，通过调用zoo_client_id来获取客户端ID。指定参数0开始新的会话。
		context:传入一个上下文对象，在回调函数fn中使用，也可以通过`zoo。
		flags:暂时未使用，设置为0.
	回调函数声明如下：
		typedef void (*watcher_fn)(zhandle_t *zh, int type, 
        	int state, const char *path,void *watcherCtx);
	注意连接的创建是异步的，也就是当zookeeper_init返回时连接并不一定建立，只有在fn回调函数调用时返回state = ZOO_CONNECTED_STATE时才连接成功。
	*/
    m_zhandle = zookeeper_init(connstr.c_str(), global_watcher, 30000, nullptr, nullptr, 0);
    if (nullptr == m_zhandle) 
    {
        std::cout << "zookeeper_init error!" << std::endl;
        exit(EXIT_FAILURE);
    }

    sem_t sem;
    sem_init(&sem, 0, 0);
    zoo_set_context(m_zhandle, &sem);

    sem_wait(&sem);
    std::cout << "zookeeper_init success!" << std::endl;
}

void ZkClient::Create(const char *path, const char *data, int datalen, int state)
{
    char path_buffer[128];
    int bufferlen = sizeof(path_buffer);
    int flag;
	// 先判断path表示的znode节点是否存在，如果存在，就不再重复创建了
	flag = zoo_exists(m_zhandle, path, 0, nullptr);
	if (ZNONODE == flag) // 表示path的znode节点不存在
	{
		// 创建指定path的znode节点了
		flag = zoo_create(m_zhandle, path, data, datalen,
			&ZOO_OPEN_ACL_UNSAFE, state, path_buffer, bufferlen);
		if (flag == ZOK)
		{
			std::cout << "znode create success... path:" << path << std::endl;
		}
		else
		{
			std::cout << "flag:" << flag << std::endl;
			std::cout << "znode create error... path:" << path << std::endl;
			exit(EXIT_FAILURE);
		}
	}
}

// 根据指定的path，获取znode节点的值
std::string ZkClient::GetData(const char *path)
{
    char buffer[64];
	int bufferlen = sizeof(buffer);
	int flag = zoo_get(m_zhandle, path, 0, buffer, &bufferlen, nullptr);
	if (flag != ZOK)
	{
		std::cout << "get znode error... path:" << path << std::endl;
		return "";
	}
	else
	{
		return buffer;
	}
}