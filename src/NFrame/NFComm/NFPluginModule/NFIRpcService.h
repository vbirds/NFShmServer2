// -------------------------------------------------------------------------
//    @FileName         :    NFIRpcService.h
//    @Author           :    gaoyi
//    @Date             :    23-3-24
//    @Email			:    445267987@qq.com
//    @Module           :    NFIRpcService
//
// -------------------------------------------------------------------------

#pragma once

#include "NFComm/NFCore/NFPlatform.h"
#include "NFComm/NFPluginModule/NFBaseObj.h"

#ifdef NF_DEBUG_MODE
#define DEFINE_RPC_SERVICE_TIME_OUT_MS (2000000) //200s
#else
#define DEFINE_RPC_SERVICE_TIME_OUT_MS (3000) //3s
#endif

class NFObject;
class NFIRpcService : public NFBaseObj
{
public:
    NFIRpcService(NFIPluginManager* p):NFBaseObj(p)
    {

    }

    virtual int run(uint64_t unLinkId, const NFrame::Proto_FramePkg& reqSvrPkg, uint64_t param1, uint64_t param2) = 0;
};

class NFIDynamicRpcService : public NFBaseObj
{
public:
    NFIDynamicRpcService(NFIPluginManager* p):NFBaseObj(p)
    {

    }

    virtual int run(NFBaseObj *pObject, google::protobuf::Message& request, google::protobuf::Message& respone) = 0;

    virtual int run(NFObject* pObject, google::protobuf::Message& request, google::protobuf::Message& respone) = 0;
};

template<typename BaseType, typename RequestType, typename ResponeType>
class NFCDynamicRpcService : public NFIDynamicRpcService
{
    static_assert((TIsDerived<RequestType, google::protobuf::Message>::Result), "the class RequestType must is google::protobuf::Message");
    static_assert((TIsDerived<ResponeType, google::protobuf::Message>::Result), "the class ResponeType must is google::protobuf::Message");
public:
    NFCDynamicRpcService(NFIPluginManager* p, BaseType *pBase, int (BaseType::*handleRecieve)(RequestType& request, ResponeType& respone)): NFIDynamicRpcService(p)
    {
        m_function = handleRecieve;
    }

    virtual int run(NFBaseObj *pObject, google::protobuf::Message& request, google::protobuf::Message& respone) override
    {
        RequestType* pRequest = dynamic_cast<RequestType*>(&request);
        ResponeType* pRespone = dynamic_cast<ResponeType*>(&respone);
        BaseType* pBase = dynamic_cast<BaseType*>(pObject);
        if (pBase && pRequest && pRespone && m_function)
        {
            return (pBase->*m_function)(dynamic_cast<RequestType&>(request), dynamic_cast<ResponeType&>(respone));
        }

        return NFrame::ERR_CODE_RPC_MSG_FUNCTION_UNEXISTED;
    }

    virtual int run(NFObject *pObject, google::protobuf::Message& request, google::protobuf::Message& respone) override
    {
        RequestType* pRequest = dynamic_cast<RequestType*>(&request);
        ResponeType* pRespone = dynamic_cast<ResponeType*>(&respone);
        BaseType* pBase = dynamic_cast<BaseType*>(pObject);
        if (pBase && pRequest && pRespone && m_function)
        {
            return (pBase->*m_function)(dynamic_cast<RequestType&>(request), dynamic_cast<ResponeType&>(respone));
        }

        return NFrame::ERR_CODE_RPC_MSG_FUNCTION_UNEXISTED;
    }

    int (BaseType::*m_function)(RequestType& request, ResponeType& respone);
};

/**
 * @brief 定义RPC绑定宏 将RPC服务协议号，请求类型，返回类型在编译期间就固定好，防止程序运行的时候才发现RPC服务区有问题
 *        服务器架构层定义在NFServerComm/NFServerCommon/NFServerBindRpcService.h
 *        游戏逻辑层定义在游戏的NFLogicBindRpcService.h
 */
#define DEFINE_BIND_RPC_SERVICE(msgId, RequestType, ResponeType) \
template<>                                                      \
struct NFBindRpcService<msgId, RequestType, ResponeType>\
{                                                               \
    enum { value = 1 };\
    typedef std::true_type type;\
};

/**
 * @brief 如果你没有定义RPC绑定宏，那么就会导致这一个宏在编译期间，编译失败，确保rpc服务在注册rpc以及调用rpc服务的编译期间就报错，方便告诉程序员rpc服务有问题
 */
#define STATIC_ASSERT_BIND_RPC_SERVICE(msgId, RequestType, ResponeType) static_assert(NFBindRpcService<msgId, RequestType, ResponeType>::value, \
    "You Must First Define DEFINE_BIND_RPC_SERVICE("#msgId", "#RequestType", "#ResponeType")")\

template<size_t msgId, class RequestType, class ResponeType>
struct NFBindRpcService
{
    enum { value = 0 };
    typedef std::false_type type;
};
