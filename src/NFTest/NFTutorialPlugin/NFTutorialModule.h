// -------------------------------------------------------------------------
//    @FileName         :    NFTutorialModule.h
//    @Author           :    gaoyi
//    @Date             :    24-1-13
//    @Email			:    445267987@qq.com
//    @Module           :    NFTutorialModule
//
// -------------------------------------------------------------------------

#pragma once

#include <NFComm/NFPluginModule/NFIDynamicModule.h>

class NFTutorialModule : public NFIDynamicModule
{
public:
    NFTutorialModule(NFIPluginManager* p);
    
    virtual ~NFTutorialModule();

    virtual int Awake();

    virtual int Init();

    virtual int Tick();

    virtual int Shut();

    virtual int Finalize();

    /*
     * ��̬�ȸ�dll/so֮��ģ����õĺ���
     * */
    virtual int OnDynamicPlugin();

	/**
	 * \brief 
	 * \param nTimerID ��ʱ������
	 * \return 
	 */
	virtual int OnTimer(uint32_t nTimerID) override;
private:
	uint32_t m_idCount;
};