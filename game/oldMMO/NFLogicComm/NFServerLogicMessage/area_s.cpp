#include "area_s.h"

namespace proto_ff_s {

E_AreaArea_s::E_AreaArea_s() {
	if (EN_OBJ_MODE_INIT == NFShmMgr::Instance()->GetCreateMode()) {
		CreateInit();
	} else {
		ResumeInit();
	}
}

int E_AreaArea_s::CreateInit() {
	m_id = (int64_t)0;
	m_subtype = (int32_t)0;
	m_belongtosceneid = (int64_t)0;
	m_shapetype = (int32_t)0;
	m_isnotice = (int32_t)0;
	return 0;
}

int E_AreaArea_s::ResumeInit() {
	return 0;
}

void E_AreaArea_s::write_to_pbmsg(::proto_ff::E_AreaArea & msg) const {
	msg.set_m_id((int64_t)m_id);
	msg.set_m_subtype((int32_t)m_subtype);
	msg.set_m_belongtosceneid((int64_t)m_belongtosceneid);
	msg.set_m_shapetype((int32_t)m_shapetype);
	msg.set_m_isnotice((int32_t)m_isnotice);
}

void E_AreaArea_s::read_from_pbmsg(const ::proto_ff::E_AreaArea & msg) {
	//dont't use memset, the class maybe has virtual //memset(this, 0, sizeof(struct E_AreaArea_s));
	m_id = msg.m_id();
	m_subtype = msg.m_subtype();
	m_belongtosceneid = msg.m_belongtosceneid();
	m_shapetype = msg.m_shapetype();
	m_isnotice = msg.m_isnotice();
}

Sheet_AreaArea_s::Sheet_AreaArea_s() {
	if (EN_OBJ_MODE_INIT == NFShmMgr::Instance()->GetCreateMode()) {
		CreateInit();
	} else {
		ResumeInit();
	}
}

int Sheet_AreaArea_s::CreateInit() {
	return 0;
}

int Sheet_AreaArea_s::ResumeInit() {
	return 0;
}

void Sheet_AreaArea_s::write_to_pbmsg(::proto_ff::Sheet_AreaArea & msg) const {
	for(int32_t i = 0; i < (int32_t)E_AreaArea_List.size(); ++i) {
		::proto_ff::E_AreaArea* temp_e_areaarea_list = msg.add_e_areaarea_list();
		E_AreaArea_List[i].write_to_pbmsg(*temp_e_areaarea_list);
	}
}

void Sheet_AreaArea_s::read_from_pbmsg(const ::proto_ff::Sheet_AreaArea & msg) {
	//dont't use memset, the class maybe has virtual //memset(this, 0, sizeof(struct Sheet_AreaArea_s));
	E_AreaArea_List.resize((int)msg.e_areaarea_list_size() > (int)E_AreaArea_List.max_size() ? E_AreaArea_List.max_size() : msg.e_areaarea_list_size());
	for(int32_t i = 0; i < (int32_t)E_AreaArea_List.size(); ++i) {
		const ::proto_ff::E_AreaArea & temp_e_areaarea_list = msg.e_areaarea_list(i);
		E_AreaArea_List[i].read_from_pbmsg(temp_e_areaarea_list);
	}
}

}