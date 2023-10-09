#include "XingchenXcguizhenDesc.h"
#include "AttributeAttributeDesc.h"
#include "NFComm/NFPluginModule/NFCheck.h"

IMPLEMENT_IDCREATE_WITHTYPE(XingchenXcguizhenDesc, EOT_CONST_XINGCHEN_XCGUIZHEN_DESC_ID, NFShmObj)

XingchenXcguizhenDesc::XingchenXcguizhenDesc():NFIDescStore()
{
	if (EN_OBJ_MODE_INIT == NFShmMgr::Instance()->GetCreateMode()) {
		CreateInit();
	}
	else {
		ResumeInit();
	}
}

XingchenXcguizhenDesc::~XingchenXcguizhenDesc()
{
}

int XingchenXcguizhenDesc::CreateInit()
{
	return Initialize();
}

int XingchenXcguizhenDesc::ResumeInit()
{
	return 0;
}

int XingchenXcguizhenDesc::Load(NFResDB *pDB)
{
	NFLogTrace(NF_LOG_SYSTEMLOG, 0, "--begin--");
	CHECK_EXPR(pDB != NULL, -1, "pDB == NULL");

	NFLogTrace(NF_LOG_SYSTEMLOG, 0, "XingchenXcguizhenDesc::Load() strFileName = {}", GetFileName());

	proto_ff::Sheet_XingchenXcguizhen table;
	NFResTable* pResTable = pDB->GetTable(GetFileName());
	CHECK_EXPR(pResTable != NULL, -1, "pTable == NULL, GetTable:{} Error", GetFileName());

	int iRet = 0;
	iRet = pResTable->FindAllRecord(GetDBName(), &table);
	CHECK_EXPR(iRet == 0, -1, "FindAllRecord Error:{}", GetFileName());

	//NFLogTrace(NF_LOG_SYSTEMLOG, 0, "{}", table.Utf8DebugString());

	if ((table.e_xingchenxcguizhen_list_size() < 0) || (table.e_xingchenxcguizhen_list_size() > (int)(m_astDesc.max_size())))
	{
		NFLogError(NF_LOG_SYSTEMLOG, 0, "Invalid TotalNum:{}", table.e_xingchenxcguizhen_list_size());
		return -2;
	}

	for (int i = 0; i < (int)table.e_xingchenxcguizhen_list_size(); i++)
	{
		const proto_ff::E_XingchenXcguizhen& desc = table.e_xingchenxcguizhen_list(i);
		if (desc.has_m_id() == false && desc.ByteSize() == 0)
		{
			NFLogError(NF_LOG_SYSTEMLOG, 0, "the desc no value, {}", desc.Utf8DebugString());
			continue;
		}
		//NFLogTrace(NF_LOG_SYSTEMLOG, 0, "{}", desc.Utf8DebugString());
		if (m_astDescMap.find(desc.m_id()) != m_astDescMap.end())
		{
			if (IsReloading())
			{
				auto pDesc = GetDesc(desc.m_id());
				NF_ASSERT_MSG(pDesc, "the desc:{} Reload, GetDesc Failed!, id:{}", GetClassName(), desc.m_id());
				pDesc->read_from_pbmsg(desc);
			}
			else
			{
				NFLogError(NF_LOG_SYSTEMLOG, 0, "the desc:{} id:{} exist", GetClassName(), desc.m_id());
			}
			continue;
		}
		m_astDesc.push_back();
		auto pDesc = &m_astDesc.back();
		int curIndex = m_astDesc.size() - 1;
		CHECK_EXPR_ASSERT(pDesc, -1, "m_astDesc Index Failed desc.id:{}", desc.m_id());
		pDesc->read_from_pbmsg(desc);
		auto iter = m_astDescMap.emplace_hint(desc.m_id(), curIndex);
		CHECK_EXPR_ASSERT(iter != m_astDescMap.end(), -1, "m_astDescMap.Insert Failed desc.id:{}, key maybe exist", desc.m_id());
		CHECK_EXPR_ASSERT(GetDesc(desc.m_id()) == pDesc, -1, "GetDesc != pDesc, id:{}", desc.m_id());
	}

	NFLogTrace(NF_LOG_SYSTEMLOG, 0, "load {}, num={}", iRet, table.e_xingchenxcguizhen_list_size());
	NFLogTrace(NF_LOG_SYSTEMLOG, 0, "--end--");
	return 0;
}

int XingchenXcguizhenDesc::CheckWhenAllDataLoaded()
{
	int result = 0;
	for(int i = 0; i < (int)m_astDesc.size(); i++)
	{
		auto pDesc = &m_astDesc[i];
		for(int j = 0; j < (int)pDesc->m_gz.size(); j++)
		{
			CHECK_EXPR_MSG_RESULT((pDesc->m_gz[j].m_type <= 0 || AttributeAttributeDesc::Instance(m_pObjPluginManager)->GetDesc(pDesc->m_gz[j].m_type)), result, "can't find the gz:{} in the  excel:attribute sheet:attribute", pDesc->m_gz[j].m_type);
		}
		for(int j = 0; j < (int)pDesc->m_xx.size(); j++)
		{
			CHECK_EXPR_MSG_RESULT((pDesc->m_xx[j].m_type <= 0 || AttributeAttributeDesc::Instance(m_pObjPluginManager)->GetDesc(pDesc->m_xx[j].m_type)), result, "can't find the xx:{} in the  excel:attribute sheet:attribute", pDesc->m_xx[j].m_type);
		}
	}
	return result;
}

const proto_ff_s::E_XingchenXcguizhen_s * XingchenXcguizhenDesc::GetDesc(int64_t id) const
{
	auto iter = m_astDescMap.find(id);
	if (iter != m_astDescMap.end())
	{
		int index = iter->second;
		CHECK_EXPR_ASSERT(index >= 0 && index < (int)m_astDesc.size(), NULL, "the index:{} of the id:{} exist error, than the m_astDesc max index:{}", index, id, m_astDesc.size());
		return &m_astDesc[index];
	}

	return NULL;
}

proto_ff_s::E_XingchenXcguizhen_s * XingchenXcguizhenDesc::GetDesc(int64_t id)
{
	return const_cast<proto_ff_s::E_XingchenXcguizhen_s *>((static_cast<const XingchenXcguizhenDesc*>(this))->GetDesc(id));
}

int XingchenXcguizhenDesc::GetDescIndex(int id) const
{
	auto iter = m_astDescMap.find(id);
	if (iter != m_astDescMap.end())
	{
		return iter->second;
	}

	return -1;
}

const proto_ff_s::E_XingchenXcguizhen_s * XingchenXcguizhenDesc::GetDescByIndex(int index) const
{
	CHECK_EXPR_ASSERT(index < (int)m_astDesc.size(), NULL, "the index:{} exist error, than the m_astDesc max index:{}", index, m_astDesc.size());
	return &m_astDesc[index];
}

proto_ff_s::E_XingchenXcguizhen_s * XingchenXcguizhenDesc::GetDescByIndex(int index)
{
	CHECK_EXPR_ASSERT(index < (int)m_astDesc.size(), NULL, "the index:{} exist error, than the m_astDesc max index:{}", index, m_astDesc.size());
	return &m_astDesc[index];
}
