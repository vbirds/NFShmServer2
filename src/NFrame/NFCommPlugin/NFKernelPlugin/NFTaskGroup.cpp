// -------------------------------------------------------------------------
//    @FileName         :    NFCTaskGroup.cpp
//    @Author           :    gaoyi
//    @Date             :    23-9-14
//    @Email			:    445267987@qq.com
//    @Module           :    NFCTaskGroup
//
// -------------------------------------------------------------------------

#include "NFTaskGroup.h"
#include "NFComm/NFPluginModule/NFLogMgr.h"
#include "NFTaskActor.h"
#include "NFComm/NFPluginModule/NFTask.h"
#include "NFComm/NFPluginModule/NFITaskComponent.h"
#include "NFComm/NFPluginModule/NFCheck.h"
#include "NFComm/NFCore/NFDateTime.hpp"
#include "NFComm/NFPluginModule/NFITaskModule.h"

NFTaskGroup::NFTaskGroup(NFIPluginManager *p): NFIModule(p)
{
    mnSuitIndex = 0;
    m_pFramework = NULL;
    m_pMainActor = NULL;
    m_taskGroupId = 0;
    srand(static_cast<unsigned>(time(nullptr)));
}

NFTaskGroup::~NFTaskGroup()
{
}

bool NFTaskGroup::Awake()
{
    return true;
}

bool NFTaskGroup::Init()
{
    return true;
}

bool NFTaskGroup::BeforeShut()
{
    return true;
}

bool NFTaskGroup::Shut()
{
    uint32_t startTime = NFGetSecondTime();
    //等待异步处理完毕，然后再退出系统
    while (GetNumQueuedMessages() > 0)
    {
        NFSLEEP(1000);
        if (NFGetSecondTime() - startTime >= 30)
        {
            NFLogError(NF_LOG_DEFAULT, 0, "task module shut, but has task not finish after wait 30 second!");
            break;
        }
    }

    return true;
}

bool NFTaskGroup::Finalize()
{
    CloseActorPool();
    return true;
}

bool NFTaskGroup::Execute()
{
    OnMainThreadTick();
    CheckTimeOutTask();
    return true;
}


/**
* @brief 初始化actor系统, 配置线程个数
*
* @param thread_num	线程数目，至少为1
* @return < 0 : Failed
*/
int NFTaskGroup::InitActorThread(int taskGroup, int thread_num, int yieldstrategy)
{
    m_taskGroupId = taskGroup;
    //根据物理硬件， 确定需要的线程数目
    if (thread_num <= 0) thread_num = 1;

    Theron::Framework::Parameters params;
    params.mThreadCount = thread_num;
    params.mYieldStrategy = (Theron::YieldStrategy)yieldstrategy;

    m_pFramework = new Theron::Framework(params);
    m_pMainActor = new NFTaskActor(*m_pFramework, this);

    return 0;
}

/**
* @brief 向系统请求请求一个actor
*
* @return 返回actor的唯一索引
*/
int NFTaskGroup::RequireActor()
{
    NFTaskActor* pActor(new NFTaskActor(*m_pFramework, this));
    if (pActor)
    {
        if (m_vecActorPool.empty())
        {
            m_vecActorPool.resize(pActor->GetAddress().AsInteger() + 1);
            for (size_t i = 0; i < m_vecActorPool.size(); i++)
            {
                m_vecActorPool[i] = nullptr;
            }
        }
        else
        {
            m_vecActorPool.push_back(nullptr);
        }

        NF_ASSERT((size_t)pActor->GetAddress().AsInteger() < m_vecActorPool.size());
        m_vecActorPool[pActor->GetAddress().AsInteger()] = pActor;

        return pActor->GetAddress().AsInteger();
    }
    return -1;
}

/**
* @brief 通过自己保存的actorIndex将发送数据给actor线程
*
* @param nActorIndex	actor唯一索引
* @param pData			要发送的数据
* @return 是否成功
*/
int NFTaskGroup::SendMsgToActor(const int nActorIndex, NFTask* pData)
{
    NFTaskActor* pActor = GetActor(nActorIndex);
    return SendMsgToActor(pActor, pData);
}

/**
* @brief 主线程通过自己保存的actorIndex将发送数据给actor线程
*
* @param nActorIndex	actor唯一索引
* @param pData			要发送的数据
* @return 是否成功
*/
int NFTaskGroup::SendMsgToActor(NFTaskActor* pActor, NFTask* pData)
{
    if (pActor != nullptr && m_pMainActor != nullptr && m_pFramework != nullptr)
    {
        NFTaskActorMessage xMessage;

        xMessage.nMsgType = NFTaskActorMessage::ACTOR_MSG_TYPE_COMPONENT;
        xMessage.pData = pData;
        xMessage.nFromActor = m_pMainActor->GetAddress().AsInteger();

        bool iRet = m_pFramework->Send(xMessage, m_pMainActor->GetAddress(), pActor->GetAddress());
        CHECK_EXPR(iRet, -1, "m_pFramework->Send Failed");
    }

    return 0;
}

/**
* @brief 释放actor资源
*
* @return
*/
void NFTaskGroup::ReleaseActor()
{
    for (size_t i = 0; i < m_vecActorPool.size(); i++)
    {
        if (m_vecActorPool[i])
        {
            NF_SAFE_DELETE(m_vecActorPool[i]);
        }
    }

    if (m_pMainActor)
    {
        NF_SAFE_DELETE(m_pMainActor);
    }
    m_pMainActor = nullptr;

    if (m_pFramework)
    {
        NF_SAFE_DELETE(m_pFramework);
    }
    m_pFramework = nullptr;
}

/**
* @brief 通过actorIndex获得一个actor
*
* @param nActorIndex	actor索引地址
* @return 返回获得的actor, 若没有，为NULL
*/
NFTaskActor* NFTaskGroup::GetActor(const int nActorIndex)
{
    if (nActorIndex >= 0 && nActorIndex < (int)m_vecActorPool.size())
    {
        return m_vecActorPool[nActorIndex];
    }

    return nullptr;
}

/**
* @brief 添加一个Actor组件
*
* @return
*/
int NFTaskGroup::AddActorComponent(const int nActorIndex, NFITaskComponent* pComonnet)
{
    NFTaskActor* pActor = GetActor(nActorIndex);
    CHECK_EXPR(pActor, -1, "GetActor Failed:{}", nActorIndex);

    pComonnet->SetActorId(pActor->GetActorId());
    pActor->AddComponnet(pComonnet);
    return 0;
}

/**
* @brief 获得所有component
*
* @param
* @return
*/
NFITaskComponent* NFTaskGroup::GetTaskComponent(int nActorIndex)
{
    NFTaskActor* pActor = GetActor(nActorIndex);
    if (pActor == nullptr)
    {
        return nullptr;
    }

    return pActor->GetTaskComponent();
}

int NFTaskGroup::GetNumQueuedMessages()
{
    int count = 0;
    if (m_pMainActor)
    {
        count += m_pMainActor->GetNumQueuedMessages();
    }

    for (size_t i = 0; i < m_vecActorPool.size(); i++)
    {
        if (m_vecActorPool[i])
        {
            count += m_vecActorPool[i]->GetNumQueuedMessages();
        }
    }

    return count;
}

int NFTaskGroup::CloseActorPool()
{
    ReleaseActor();
    m_vecActorPool.clear();
    return 0;
}

bool NFTaskGroup::HandlerEx(const NFTaskActorMessage& message, const int from)
{
    if (message.nMsgType != NFTaskActorMessage::ACTOR_MSG_TYPE_COMPONENT)
    {
        return m_mQueue.Push(message);
    }

    return false;
}

int NFTaskGroup::AddTask(NFTask* pTask)
{
    const int nActorId = GetBalanceActor(pTask->GetBalanceId());
    CHECK_EXPR(nActorId > 0, -1, "GetBalanceActor Failed! nActorId:{}", nActorId);

    int iRet = SendMsgToActor(nActorId, pTask);
    CHECK_EXPR(iRet == 0, -1, "SendMsgToActor Failed! nActorId:{}", nActorId);

    return 0;
}

int NFTaskGroup::AddTask(int actorId, NFTask* pTask)
{
    CHECK_EXPR(actorId > 0, -1, "actorId <= 0:{}", actorId);

    int iRet = SendMsgToActor(actorId, pTask);
    CHECK_EXPR(iRet == 0, -1, "SendMsgToActor error");

    return 0;
}

/**
* @brief 获得最大Actor Thread Num
*
* @return
*/
int NFTaskGroup::GetMaxThreads()
{
    return m_pFramework->GetMaxThreads();
}

int NFTaskGroup::GetBalanceActor(uint64_t balanceId)
{
    if (balanceId == 0)
    {
        return GetRandActor();
    }

    if (m_vecActorPool.empty())
    {
        NFLogError(NF_LOG_DEFAULT, 0, "error");
        return -1;
    }

    //数组的0和1 是空的
    uint32_t index = balanceId % (m_vecActorPool.size()-2) + 2;
    if (m_vecActorPool[index])
    {
        return m_vecActorPool[index]->GetAddress().AsInteger();
    }
    else
    {
        NFLogError(NF_LOG_DEFAULT, 0, "error");
        return -1;
    }
}

int NFTaskGroup::GetRandActor()
{
    if (m_vecActorPool.empty())
    {
        NFLogError(NF_LOG_DEFAULT, 0, "error");
        return -1;
    }

    uint32_t index = mnSuitIndex++;
    index = (index % (m_vecActorPool.size()-2)) + 2;
    if (m_vecActorPool[index])
    {
        return m_vecActorPool[index]->GetAddress().AsInteger();
    }
    else
    {
        NFLogError(NF_LOG_DEFAULT, 0, "error");
        return -1;
    }
}

/**
* @brief 获取所有ActorId
*
* @return
*/
std::vector<int> NFTaskGroup::GetAllActorId() const
{
    std::vector<int> vecActorId;
    for (size_t index = 0; index < m_vecActorPool.size(); index++)
    {
        if (m_vecActorPool[index])
        {
            vecActorId.push_back(m_vecActorPool[index]->GetAddress().AsInteger());
        }
    }
    return vecActorId;
}

/**
* @brief 记录监控Task
*
* @return
*/
void NFTaskGroup::MonitorTask(NFTask* pTask)
{

}

/**
* @brief 检查超时
*
* @return
*/
void NFTaskGroup::CheckTimeOutTask()
{
    for (size_t i = 0; i < m_vecActorPool.size(); i++)
    {
        if (m_vecActorPool[i])
        {
            m_vecActorPool[i]->CheckTimeoutTask();
            int num = m_vecActorPool[i]->GetNumQueuedMessages();
            if (num >= 100)
            {
                NFLogError(NF_LOG_DEFAULT, 0, "the actor has too many task, the not handle task num:{} actorId:{}", num, m_vecActorPool[i]->GetActorId());
            }
        }
    }
}

void NFTaskGroup::OnMainThreadTick()
{
    std::vector<NFTaskActorMessage> listTask;
    const bool ret = m_mQueue.Pop(listTask);
    if (ret)
    {
        //const uint64_t start = NFTime::Tick();
        for (auto it = listTask.begin(); it != listTask.end(); ++it)
        {
            NFTaskActorMessage& xMsg = *it;
            if (xMsg.nMsgType != NFTaskActorMessage::ACTOR_MSG_TYPE_COMPONENT)
            {
                auto pTask = static_cast<NFTask*>(xMsg.pData);
                if (pTask)
                {
                    if (pTask->m_useTime >= 100)
                    {
                        NFLogError(NF_LOG_DEFAULT, 0, "the taskGroup:{} task:{} use time out:{} ms, handle time:{} actorId:{}", m_taskGroupId, pTask->m_taskName, pTask->m_useTime/(double)1000,
                                   NFDateTime(pTask->m_handleStartTime/1000000, pTask->m_handleStartTime%1000000).GetLongTimeString(), pTask->m_handleActorId);
                    }
                    const NFTask::TPTaskState state = pTask->MainThreadProcess();
                    switch (state)
                    {
                        case NFTask::TPTASK_STATE_COMPLETED:
                        {
                            NF_SAFE_DELETE(pTask);
                        }
                            break;
                        case NFTask::TPTASK_STATE_CONTINUE_CHILDTHREAD:
                        {
                            NFLogInfo(NF_LOG_DEFAULT, 0, "the taskGroup:{} task:{} will trans to the taskGroup:{} run", m_taskGroupId, pTask->m_taskName, pTask->m_nextActorGroup);
                            FindModule<NFITaskModule>()->AddTask(pTask->m_nextActorGroup, pTask);
                        }
                        break;
                        case NFTask::TPTASK_STATE_CONTINUE_MAINTHREAD:
                        {
                            m_mQueue.Push(xMsg);
                        }
                            break;
                        default:
                        {
                            //error
                            NF_SAFE_DELETE(pTask);
                        }
                    }
                }
            }
            else
            {
                //error
                auto pTask = static_cast<NFTask*>(xMsg.pData);
                if (pTask)
                {
                    NF_SAFE_DELETE(pTask);
                }
                else
                {
                    //error
                }
                NFLogError(NF_LOG_DEFAULT, 0, "task actor module error................");
            }
        }

        if (!listTask.empty())
        {
            //NFLogDebug(NF_LOG_DEFAULT, 0, "handle main thread tick task num:{}, use time:{}", listTask.size(),
            //           NFTime::Tick() - start);
        }
    }

    listTask.clear();
}


