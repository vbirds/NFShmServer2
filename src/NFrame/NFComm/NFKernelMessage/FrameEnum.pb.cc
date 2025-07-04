// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: FrameEnum.proto

#include "FrameEnum.pb.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/port.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// This is a temporary google only hack
#ifdef GOOGLE_PROTOBUF_ENFORCE_UNIQUENESS
#include "third_party/protobuf/version.h"
#endif
// @@protoc_insertion_point(includes)

namespace NFrame {
}  // namespace NFrame
namespace protobuf_FrameEnum_2eproto {
void InitDefaults() {
}

const ::google::protobuf::EnumDescriptor* file_level_enum_descriptors[6];
const ::google::protobuf::uint32 TableStruct::offsets[1] = {};
static const ::google::protobuf::internal::MigrationSchema* schemas = NULL;
static const ::google::protobuf::Message* const* file_default_instances = NULL;

void protobuf_AssignDescriptors() {
  AddDescriptors();
  AssignDescriptors(
      "FrameEnum.proto", schemas, file_default_instances, TableStruct::offsets,
      NULL, file_level_enum_descriptors, NULL);
}

void protobuf_AssignDescriptorsOnce() {
  static ::google::protobuf::internal::once_flag once;
  ::google::protobuf::internal::call_once(once, protobuf_AssignDescriptors);
}

void protobuf_RegisterTypes(const ::std::string&) GOOGLE_PROTOBUF_ATTRIBUTE_COLD;
void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
}

void AddDescriptorsImpl() {
  InitDefaults();
  static const char descriptor[] GOOGLE_PROTOBUF_ATTRIBUTE_SECTION_VARIABLE(protodesc_cold) = {
      "\n\017FrameEnum.proto\022\006NFrame\032\014nanopb.proto*"
      "\252\004\n\016NF_SERVER_TYPE\022 \n\nNF_ST_NONE\020\000\032\020\222\?\r\272"
      "\001\nNoneServer\022+\n\023NF_ST_MASTER_SERVER\020\001\032\022\222"
      "\?\017\272\001\014MasterServer\022)\n\022NF_ST_ROUTE_SERVER\020"
      "\002\032\021\222\?\016\272\001\013RouteServer\022\034\n\030NF_ST_ROUTE_AGEN"
      "T_SERVER\020\003\022\026\n\022NF_ST_PROXY_SERVER\020\004\022\034\n\030NF"
      "_ST_PROXY_AGENT_SERVER\020\005\022\026\n\022NF_ST_STORE_"
      "SERVER\020\006\022\026\n\022NF_ST_LOGIN_SERVER\020\007\022\026\n\022NF_S"
      "T_WORLD_SERVER\020\010\022\026\n\022NF_ST_LOGIC_SERVER\020\t"
      "\022\025\n\021NF_ST_GAME_SERVER\020\n\022\024\n\020NF_ST_SNS_SER"
      "VER\020\013\022\024\n\020NF_ST_WEB_SERVER\020\014\022\027\n\023NF_ST_ONL"
      "INE_SERVER\020\r\022\027\n\023NF_ST_CENTER_SERVER\020\016\022\026\n"
      "\022NF_ST_MATCH_SERVER\020\017\022\025\n\021NF_ST_CITY_SERV"
      "ER\020\020\022\026\n\022NF_ST_CHECK_SERVER\020\021\022\030\n\024NF_ST_NA"
      "VMESH_SERVER\020\022\022\r\n\tNF_ST_MAX\020\023\032\005\222\?\002h\001*\241\001\n"
      "\022NF_SNS_SERVER_TYPE\022\024\n\020NF_SNS_TYPE_NONE\020"
      "\000\022\023\n\017NF_SNS_TYPE_SNS\020\001\022\026\n\022NF_SNS_TYPE_FR"
      "IEND\020\001\022\025\n\021NF_SNS_TYPE_GUILD\020\001\022\026\n\022NF_SNS_"
      "TYPE_FAMILY\020\001\022\025\n\021NF_SNS_TYPE_GROUP\020\001\032\002\020\001"
      "*Z\n\017FrameGlobalEnum\022\024\n\020SERVER_ENUM_NONE\020"
      "\000\022\025\n\021MAX_SQL_TABLE_NUM\020d\022\032\n\026MAX_SERVER_P"
      "LUGINS_NUM\020\024*\251\001\n\014NF_LOG_LEVEL\022\024\n\020NLL_TRA"
      "CE_NORMAL\020\000\022\024\n\020NLL_DEBUG_NORMAL\020\001\022\023\n\017NLL"
      "_INFO_NORMAL\020\002\022\025\n\021NLL_WARING_NORMAL\020\003\022\024\n"
      "\020NLL_ERROR_NORMAL\020\004\022\027\n\023NLL_CRITICAL_NORM"
      "AL\020\005\022\022\n\016NLL_OFF_NORMAL\020\006*K\n\tNF_LOG_ID\022\022\n"
      "\016NF_LOG_DEFAULT\020\000\022\024\n\020NF_LOG_STATISTIC\020\001\022"
      "\024\n\020NF_LOG_BEHAVIOUR\020\002*\3065\n\016FrameErrorCode"
      "\022\023\n\017ERR_CODE_SVR_OK\020\000\022&\n\031ERR_CODE_SVR_SY"
      "STEM_ERROR\020\377\377\377\377\377\377\377\377\377\001\022(\n\033ERR_CODE_SVR_SY"
      "STEM_TIMEOUT\020\376\377\377\377\377\377\377\377\377\001\022/\n\"ERR_CODE_SVR_"
      "SYSTEM_DATABASE_ERROR\020\375\377\377\377\377\377\377\377\377\001\022$\n\027ERR_"
      "VALUE_CHECK_INVALID\020\374\377\377\377\377\377\377\377\377\001\022+\n\036ERR_LO"
      "G_SYSTEM_SERVER_OBJ_NULL\020\373\377\377\377\377\377\377\377\377\001\022\"\n\025E"
      "RR_CODE_MSG_RPC_BASE\020\234\377\377\377\377\377\377\377\377\001\022\'\n\032ERR_C"
      "ODE_RPC_INVALID_PARAM\020\233\377\377\377\377\377\377\377\377\001\022\'\n\032ERR_"
      "CODE_RPC_ENCODE_FAILED\020\232\377\377\377\377\377\377\377\377\001\022\'\n\032ERR"
      "_CODE_RPC_DECODE_FAILED\020\231\377\377\377\377\377\377\377\377\001\022,\n\037ER"
      "R_CODE_RPC_RECV_EXCEPTION_MSG\020\230\377\377\377\377\377\377\377\377\001"
      "\022&\n\031ERR_CODE_RPC_UNKNOWN_TYPE\020\227\377\377\377\377\377\377\377\377\001"
      "\0221\n$ERR_CODE_RPC_UNSUPPORT_FUNCTION_NAME"
      "\020\226\377\377\377\377\377\377\377\377\001\022+\n\036ERR_CODE_RPC_SESSION_NOT_"
      "FOUND\020\225\377\377\377\377\377\377\377\377\001\022%\n\030ERR_CODE_RPC_SEND_FA"
      "ILED\020\224\377\377\377\377\377\377\377\377\001\022)\n\034ERR_CODE_RPC_REQUEST_"
      "TIMEOUT\020\223\377\377\377\377\377\377\377\377\001\022/\n\"ERR_CODE_RPC_FUNCT"
      "ION_NAME_EXISTED\020\222\377\377\377\377\377\377\377\377\001\022&\n\031ERR_CODE_"
      "RPC_SYSTEM_ERROR\020\221\377\377\377\377\377\377\377\377\001\022)\n\034ERR_CODE_"
      "RPC_PROCESS_TIMEOUT\020\220\377\377\377\377\377\377\377\377\001\0220\n#ERR_CO"
      "DE_RPC_MSG_FUNCTION_UNEXISTED\020\217\377\377\377\377\377\377\377\377\001"
      "\022.\n!ERR_CODE_RPC_SYSTEM_OVERLOAD_BASE\020\216\377"
      "\377\377\377\377\377\377\377\001\022)\n\034ERR_CODE_RPC_MESSAGE_EXPIRED"
      "\020\215\377\377\377\377\377\377\377\377\001\022\'\n\032ERR_CODE_RPC_TASK_OVERLOA"
      "D\020\214\377\377\377\377\377\377\377\377\001\022!\n\024ERR_CODE_RPC_CO_USED\020\213\377\377"
      "\377\377\377\377\377\377\001\022$\n\027ERR_CODE_ZK_SYSTEMERROR\020\270\376\377\377\377"
      "\377\377\377\377\001\022-\n ERR_CODE_ZK_RUNTIMEINCONSISTENC"
      "Y\020\267\376\377\377\377\377\377\377\377\001\022*\n\035ERR_CODE_ZK_DATAINCONSIS"
      "TENCY\020\265\376\377\377\377\377\377\377\377\001\022\'\n\032ERR_CODE_ZK_CONNECTI"
      "ONLOSS\020\264\376\377\377\377\377\377\377\377\001\022)\n\034ERR_CODE_ZK_MARSHAL"
      "LINGERROR\020\263\376\377\377\377\377\377\377\377\001\022&\n\031ERR_CODE_ZK_UNIM"
      "PLEMENTED\020\262\376\377\377\377\377\377\377\377\001\022)\n\034ERR_CODE_ZK_OPER"
      "ATIONTIMEOUT\020\261\376\377\377\377\377\377\377\377\001\022%\n\030ERR_CODE_ZK_B"
      "ADARGUMENTS\020\260\376\377\377\377\377\377\377\377\001\022%\n\030ERR_CODE_ZK_IN"
      "VALIDSTATE\020\257\376\377\377\377\377\377\377\377\001\022$\n\027ERR_CODE_kSM_DN"
      "SFAILURE\020\256\376\377\377\377\377\377\377\377\001\022!\n\024ERR_CODE_ZK_APIER"
      "ROR\020\255\376\377\377\377\377\377\377\377\001\022\037\n\022ERR_CODE_ZK_NONODE\020\254\376\377"
      "\377\377\377\377\377\377\001\022\037\n\022ERR_CODE_ZK_NOAUTH\020\253\376\377\377\377\377\377\377\377\001"
      "\022#\n\026ERR_CODE_ZK_BADVERSION\020\252\376\377\377\377\377\377\377\377\001\0220\n"
      "#ERR_CODE_ZK_NOCHILDRENFOREPHEMERALS\020\251\376\377"
      "\377\377\377\377\377\377\001\022#\n\026ERR_CODE_ZK_NODEEXISTS\020\250\376\377\377\377\377"
      "\377\377\377\001\022!\n\024ERR_CODE_ZK_NOTEMPTY\020\247\376\377\377\377\377\377\377\377\001\022"
      "\'\n\032ERR_CODE_ZK_SESSIONEXPIRED\020\246\376\377\377\377\377\377\377\377\001"
      "\022(\n\033ERR_CODE_ZK_INVALIDCALLBACK\020\245\376\377\377\377\377\377\377"
      "\377\001\022#\n\026ERR_CODE_ZK_INVALIDACL\020\244\376\377\377\377\377\377\377\377\001\022"
      "#\n\026ERR_CODE_ZK_AUTHFAILED\020\243\376\377\377\377\377\377\377\377\001\022 \n\023"
      "ERR_CODE_ZK_CLOSING\020\242\376\377\377\377\377\377\377\377\001\022 \n\023ERR_CO"
      "DE_ZK_NOTHING\020\241\376\377\377\377\377\377\377\377\001\022%\n\030ERR_CODE_ZK_"
      "SESSIONMOVED\020\240\376\377\377\377\377\377\377\377\001\022 \n\023ERR_CODE_ZK_N"
      "OQUOTA\020\237\376\377\377\377\377\377\377\377\001\022\'\n\032ERR_CODE_ZK_SERVERO"
      "VERLOAD\020\236\376\377\377\377\377\377\377\377\001\022\'\n\032ERR_CODE_ZK_NOT_SE"
      "T_APPKEY\020\235\376\377\377\377\377\377\377\377\001\022\"\n\025ERR_CODE_MESSAGE_"
      "BASE\020\324\375\377\377\377\377\377\377\377\001\022!\n\024ERR_CODE_NAMING_BASE\020"
      "\360\374\377\377\377\377\377\377\377\001\022+\n\036ERR_CODE_NAMING_NOT_SUPPOR"
      "TTED\020\357\374\377\377\377\377\377\377\377\001\022*\n\035ERR_CODE_NAMING_INVAI"
      "LD_PARAM\020\356\374\377\377\377\377\377\377\377\001\022+\n\036ERR_CODE_NAMING_U"
      "RL_REGISTERED\020\355\374\377\377\377\377\377\377\377\001\022+\n\036ERR_CODE_NAM"
      "ING_URL_NOT_BINDED\020\354\374\377\377\377\377\377\377\377\001\022,\n\037ERR_COD"
      "E_NAMING_REGISTER_FAILED\020\353\374\377\377\377\377\377\377\377\001\022-\n E"
      "RR_CODE_NAMING_FACTORY_MAP_NULL\020\352\374\377\377\377\377\377\377"
      "\377\001\022,\n\037ERR_CODE_NAMING_FACTORY_EXISTED\020\351\374"
      "\377\377\377\377\377\377\377\001\022!\n\024ERR_CODE_ROUTER_BASE\020\214\374\377\377\377\377\377"
      "\377\377\001\022+\n\036ERR_CODE_ROUTER_NOT_SUPPORTTED\020\213\374"
      "\377\377\377\377\377\377\377\001\022*\n\035ERR_CODE_ROUTER_INVAILD_PARA"
      "M\020\212\374\377\377\377\377\377\377\377\001\022.\n!ERR_CODE_ROUTER_NONE_VAL"
      "ID_HANDLE\020\211\374\377\377\377\377\377\377\377\001\022-\n ERR_CODE_ROUTER_"
      "FACTORY_MAP_NULL\020\210\374\377\377\377\377\377\377\377\001\022,\n\037ERR_CODE_"
      "ROUTER_FACTORY_EXISTED\020\207\374\377\377\377\377\377\377\377\001\022;\n.ERR"
      "_CODE_ROUTER_DISPATCHFAILD_DESTSVR_NOTEX"
      "IST\020\206\374\377\377\377\377\377\377\377\001\022 \n\023ERR_CODE_TIMER_BASE\020\250\373"
      "\377\377\377\377\377\377\377\001\022+\n\036ERR_CODE_STORESVR_ERRCODE_BU"
      "SY\020\303\372\377\377\377\377\377\377\377\001\022-\n ERR_CODE_STORESVR_ERRCO"
      "DE_EINVAL\020\302\372\377\377\377\377\377\377\377\001\022<\n/ERR_CODE_STORESV"
      "R_ERRCODE_PARSEFROMSTRINGFAILED\020\301\372\377\377\377\377\377\377"
      "\377\001\022\?\n2ERR_CODE_STORESVR_ERRCODE_DBNAME_O"
      "R_MODKEY_INVALID\020\300\372\377\377\377\377\377\377\377\001\0225\n(ERR_CODE_"
      "STORESVR_ERRCODE_TBNAME_INVALID\020\277\372\377\377\377\377\377\377"
      "\377\001\0223\n&ERR_CODE_STORESVR_ERRCODE_SELECT_E"
      "MPTY\020\276\372\377\377\377\377\377\377\377\001\022G\n:ERR_CODE_STORESVR_ERR"
      "CODE_SELECT_FILLMESSAGEBYRESULTFAILED\020\275\372"
      "\377\377\377\377\377\377\377\001\0223\n&ERR_CODE_STORESVR_ERRCODE_SE"
      "LECTFAILED\020\274\372\377\377\377\377\377\377\377\001\0223\n&ERR_CODE_STORES"
      "VR_ERRCODE_INSERTFAILED\020\271\372\377\377\377\377\377\377\377\001\0223\n&ER"
      "R_CODE_STORESVR_ERRCODE_DELETEFAILED\020\270\372\377"
      "\377\377\377\377\377\377\001\022=\n0ERR_CODE_STORESVR_ERRCODE_DEL"
      "ETERECORDISNOTEXIST\020\267\372\377\377\377\377\377\377\377\001\022=\n0ERR_CO"
      "DE_STORESVR_ERRCODE_UPDATERECORDISNOTEXI"
      "ST\020\266\372\377\377\377\377\377\377\377\001\022:\n-ERR_CODE_STORESVR_ERRCO"
      "DE_UPDATENOROWAFFECTED\020\265\372\377\377\377\377\377\377\377\001\0223\n&ERR"
      "_CODE_STORESVR_ERRCODE_UPDATEFAILED\020\264\372\377\377"
      "\377\377\377\377\377\001\0229\n,ERR_CODE_STORESVR_ERRCODE_UPDA"
      "TEINSERTFAILED\020\263\372\377\377\377\377\377\377\377\001\022.\n!ERR_CODE_ST"
      "ORESVR_ERRCODE_UNKNOWN\020\262\372\377\377\377\377\377\377\377\001\022&\n\031ERR"
      "_CODE_CO_INVALID_PARAM\020\337\371\377\377\377\377\377\377\377\001\022)\n\034ERR"
      "_CODE_CO_NOT_IN_COROUTINE\020\336\371\377\377\377\377\377\377\377\001\022$\n\027"
      "ERR_CODE_CO_NOT_RUNNING\020\335\371\377\377\377\377\377\377\377\001\022+\n\036ER"
      "R_CODE_CO_START_TIMER_FAILED\020\334\371\377\377\377\377\377\377\377\001\022"
      " \n\023ERR_CODE_CO_TIMEOUT\020\333\371\377\377\377\377\377\377\377\001\0223\n&ERR"
      "_CODE_CO_CANNOT_RESUME_IN_COROUTINE\020\332\371\377\377"
      "\377\377\377\377\377\001\022*\n\035ERR_CODE_CO_COROUTINE_UNEXIST\020"
      "\331\371\377\377\377\377\377\377\377\001\022/\n\"ERR_CODE_CO_COROUTINE_STAT"
      "US_ERROR\020\327\371\377\377\377\377\377\377\377\001\022&\n\031ERR_CODE_NFBUS_ER"
      "R_PARAMS\020\373\370\377\377\377\377\377\377\377\001\022%\n\030ERR_CODE_NFBUS_ER"
      "R_INNER\020\372\370\377\377\377\377\377\377\377\001\022\'\n\032ERR_CODE_NFBUS_ERR"
      "_NO_DATA\020\371\370\377\377\377\377\377\377\377\001\022*\n\035ERR_CODE_NFBUS_ER"
      "R_BUFF_LIMIT\020\370\370\377\377\377\377\377\377\377\001\022&\n\031ERR_CODE_NFBU"
      "S_ERR_MALLOC\020\367\370\377\377\377\377\377\377\377\001\022&\n\031ERR_CODE_NFBU"
      "S_ERR_SCHEME\020\366\370\377\377\377\377\377\377\377\001\022(\n\033ERR_CODE_NFBU"
      "S_ERR_BAD_DATA\020\365\370\377\377\377\377\377\377\377\001\022,\n\037ERR_CODE_NF"
      "BUS_ERR_INVALID_SIZE\020\364\370\377\377\377\377\377\377\377\001\022*\n\035ERR_C"
      "ODE_NFBUS_ERR_NOT_INITED\020\363\370\377\377\377\377\377\377\377\001\022.\n!E"
      "RR_CODE_NFBUS_ERR_ALREADY_INITED\020\362\370\377\377\377\377\377"
      "\377\377\001\022+\n\036ERR_CODE_NFBUS_ERR_ACCESS_DENY\020\361\370"
      "\377\377\377\377\377\377\377\001\022&\n\031ERR_CODE_NFBUS_ERR_UNPACK\020\360\370"
      "\377\377\377\377\377\377\377\001\022$\n\027ERR_CODE_NFBUS_ERR_PACK\020\357\370\377\377"
      "\377\377\377\377\377\001\0220\n#ERR_CODE_NFBUS_ERR_NFNODE_NOT_"
      "FOUND\020\350\370\377\377\377\377\377\377\377\001\0221\n$ERR_CODE_NFBUS_ERR_N"
      "FNODE_INVALID_ID\020\373\370\377\377\377\377\377\377\377\001\0224\n\'ERR_CODE_"
      "NFBUS_ERR_NFNODE_NO_CONNECTION\020\371\370\377\377\377\377\377\377\377"
      "\001\0225\n(ERR_CODE_NFBUS_ERR_NFNODE_FAULT_TOL"
      "ERANT\020\370\370\377\377\377\377\377\377\377\001\0222\n%ERR_CODE_NFBUS_ERR_N"
      "FNODE_INVALID_MSG\020\367\370\377\377\377\377\377\377\377\001\0227\n*ERR_CODE"
      "_NFBUS_ERR_NFNODE_BUS_ID_NOT_MNFCH\020\366\370\377\377\377"
      "\377\377\377\377\001\022*\n\035ERR_CODE_NFBUS_ERR_NFNODE_TTL\020\365"
      "\370\377\377\377\377\377\377\377\001\0224\n\'ERR_CODE_NFBUS_ERR_NFNODE_M"
      "ASK_CONFLICT\020\364\370\377\377\377\377\377\377\377\001\0222\n%ERR_CODE_NFBU"
      "S_ERR_NFNODE_ID_CONFLICT\020\363\370\377\377\377\377\377\377\377\001\0226\n)E"
      "RR_CODE_NFBUS_ERR_NFNODE_SRC_DST_IS_SAME"
      "\020\362\370\377\377\377\377\377\377\377\001\0226\n)ERR_CODE_NFBUS_ERR_CHANNE"
      "L_SIZE_TOO_SMALL\020\361\370\377\377\377\377\377\377\377\001\0226\n)ERR_CODE_"
      "NFBUS_ERR_CHANNEL_BUFFER_INVALID\020\360\370\377\377\377\377\377"
      "\377\377\001\0224\n\'ERR_CODE_NFBUS_ERR_CHANNEL_ADDR_I"
      "NVALID\020\357\370\377\377\377\377\377\377\377\001\022/\n\"ERR_CODE_NFBUS_ERR_"
      "CHANNEL_CLOSING\020\356\370\377\377\377\377\377\377\377\001\0223\n&ERR_CODE_N"
      "FBUS_ERR_CHANNEL_NOT_SUPPORT\020\355\370\377\377\377\377\377\377\377\001\022"
      ";\n.ERR_CODE_NFBUS_ERR_CHANNEL_UNSUPPORTE"
      "D_VERSION\020\354\370\377\377\377\377\377\377\377\001\022;\n.ERR_CODE_NFBUS_E"
      "RR_CHANNEL_ALIGN_SIZE_MISMATCH\020\353\370\377\377\377\377\377\377\377"
      "\001\022<\n/ERR_CODE_NFBUS_ERR_CHANNEL_ARCH_SIZ"
      "E_T_MISMATCH\020\352\370\377\377\377\377\377\377\377\001\0227\n*ERR_CODE_NFBU"
      "S_ERR_NODE_BAD_BLOCK_NODE_NUM\020\336\370\377\377\377\377\377\377\377\001"
      "\0228\n+ERR_CODE_NFBUS_ERR_NODE_BAD_BLOCK_BU"
      "FF_SIZE\020\324\370\377\377\377\377\377\377\377\001\0226\n)ERR_CODE_NFBUS_ERR"
      "_NODE_BAD_BLOCK_WSEQ_ID\020\312\370\377\377\377\377\377\377\377\001\0226\n)ER"
      "R_CODE_NFBUS_ERR_NODE_BAD_BLOCK_CSEQ_ID\020"
      "\300\370\377\377\377\377\377\377\377\001\022,\n\037ERR_CODE_NFBUS_ERR_NODE_TI"
      "MEOUT\020\266\370\377\377\377\377\377\377\377\001\022.\n!ERR_CODE_NFBUS_ERR_S"
      "HM_GET_FAILED\020\254\370\377\377\377\377\377\377\377\001\022-\n ERR_CODE_NFB"
      "US_ERR_SHM_NOT_FOUND\020\253\370\377\377\377\377\377\377\377\001\0220\n#ERR_C"
      "ODE_NFBUS_ERR_SHM_CLOSE_FAILED\020\252\370\377\377\377\377\377\377\377"
      "\001\0220\n#ERR_CODE_NFBUS_ERR_SHM_PATH_INVALID"
      "\020\251\370\377\377\377\377\377\377\377\001\022.\n!ERR_CODE_NFBUS_ERR_SHM_MA"
      "P_FAILED\020\250\370\377\377\377\377\377\377\377\001\0220\n#ERR_CODE_NFBUS_ER"
      "R_SOCK_BIND_FAILED\020\242\370\377\377\377\377\377\377\377\001\0222\n%ERR_COD"
      "E_NFBUS_ERR_SOCK_LISTEN_FAILED\020\241\370\377\377\377\377\377\377\377"
      "\001\0223\n&ERR_CODE_NFBUS_ERR_SOCK_CONNECT_FAI"
      "LED\020\240\370\377\377\377\377\377\377\377\001\0220\n#ERR_CODE_NFBUS_ERR_PIP"
      "E_BIND_FAILED\020\230\370\377\377\377\377\377\377\377\001\0222\n%ERR_CODE_NFB"
      "US_ERR_PIPE_LISTEN_FAILED\020\227\370\377\377\377\377\377\377\377\001\0223\n&"
      "ERR_CODE_NFBUS_ERR_PIPE_CONNECT_FAILED\020\226"
      "\370\377\377\377\377\377\377\377\001\0222\n%ERR_CODE_NFBUS_ERR_PIPE_ADD"
      "R_TOO_LONG\020\225\370\377\377\377\377\377\377\377\001\0222\n%ERR_CODE_NFBUS_"
      "ERR_PIPE_REMOVE_FAILED\020\224\370\377\377\377\377\377\377\377\001\0222\n%ERR"
      "_CODE_NFBUS_ERR_DNS_GETADDR_FAILED\020\216\370\377\377\377"
      "\377\377\377\377\001\0224\n\'ERR_CODE_NFBUS_ERR_CONNECTION_N"
      "OT_FOUND\020\215\370\377\377\377\377\377\377\377\001\022,\n\037ERR_CODE_NFBUS_ER"
      "R_WRITE_FAILED\020\214\370\377\377\377\377\377\377\377\001\022+\n\036ERR_CODE_NF"
      "BUS_ERR_READ_FAILED\020\213\370\377\377\377\377\377\377\377\001\022&\n\031ERR_CO"
      "DE_NFBUS_ERR_EV_RUN\020\211\370\377\377\377\377\377\377\377\001\022)\n\034ERR_CO"
      "DE_NFBUS_ERR_NO_LISTEN\020\211\370\377\377\377\377\377\377\377\001\022\'\n\032ERR"
      "_CODE_NFBUS_ERR_CLOSING\020\210\370\377\377\377\377\377\377\377\001\032\002\020\001b\006"
      "proto3"
  };
  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
      descriptor, 7966);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "FrameEnum.proto", &protobuf_RegisterTypes);
  ::protobuf_nanopb_2eproto::AddDescriptors();
}

void AddDescriptors() {
  static ::google::protobuf::internal::once_flag once;
  ::google::protobuf::internal::call_once(once, AddDescriptorsImpl);
}
// Force AddDescriptors() to be called at dynamic initialization time.
struct StaticDescriptorInitializer {
  StaticDescriptorInitializer() {
    AddDescriptors();
  }
} static_descriptor_initializer;
}  // namespace protobuf_FrameEnum_2eproto
namespace NFrame {
const ::google::protobuf::EnumDescriptor* NF_SERVER_TYPE_descriptor() {
  protobuf_FrameEnum_2eproto::protobuf_AssignDescriptorsOnce();
  return protobuf_FrameEnum_2eproto::file_level_enum_descriptors[0];
}
bool NF_SERVER_TYPE_IsValid(int value) {
  switch (value) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
      return true;
    default:
      return false;
  }
}

const ::google::protobuf::EnumDescriptor* NF_SNS_SERVER_TYPE_descriptor() {
  protobuf_FrameEnum_2eproto::protobuf_AssignDescriptorsOnce();
  return protobuf_FrameEnum_2eproto::file_level_enum_descriptors[1];
}
bool NF_SNS_SERVER_TYPE_IsValid(int value) {
  switch (value) {
    case 0:
    case 1:
      return true;
    default:
      return false;
  }
}

const ::google::protobuf::EnumDescriptor* FrameGlobalEnum_descriptor() {
  protobuf_FrameEnum_2eproto::protobuf_AssignDescriptorsOnce();
  return protobuf_FrameEnum_2eproto::file_level_enum_descriptors[2];
}
bool FrameGlobalEnum_IsValid(int value) {
  switch (value) {
    case 0:
    case 20:
    case 100:
      return true;
    default:
      return false;
  }
}

const ::google::protobuf::EnumDescriptor* NF_LOG_LEVEL_descriptor() {
  protobuf_FrameEnum_2eproto::protobuf_AssignDescriptorsOnce();
  return protobuf_FrameEnum_2eproto::file_level_enum_descriptors[3];
}
bool NF_LOG_LEVEL_IsValid(int value) {
  switch (value) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
      return true;
    default:
      return false;
  }
}

const ::google::protobuf::EnumDescriptor* NF_LOG_ID_descriptor() {
  protobuf_FrameEnum_2eproto::protobuf_AssignDescriptorsOnce();
  return protobuf_FrameEnum_2eproto::file_level_enum_descriptors[4];
}
bool NF_LOG_ID_IsValid(int value) {
  switch (value) {
    case 0:
    case 1:
    case 2:
      return true;
    default:
      return false;
  }
}

const ::google::protobuf::EnumDescriptor* FrameErrorCode_descriptor() {
  protobuf_FrameEnum_2eproto::protobuf_AssignDescriptorsOnce();
  return protobuf_FrameEnum_2eproto::file_level_enum_descriptors[5];
}
bool FrameErrorCode_IsValid(int value) {
  switch (value) {
    case -1016:
    case -1015:
    case -1013:
    case -1012:
    case -1011:
    case -1010:
    case -1004:
    case -1003:
    case -1002:
    case -1001:
    case -1000:
    case -992:
    case -991:
    case -990:
    case -984:
    case -983:
    case -982:
    case -981:
    case -980:
    case -970:
    case -960:
    case -950:
    case -940:
    case -930:
    case -920:
    case -918:
    case -917:
    case -916:
    case -915:
    case -914:
    case -913:
    case -912:
    case -911:
    case -910:
    case -909:
    case -908:
    case -907:
    case -906:
    case -905:
    case -904:
    case -903:
    case -902:
    case -901:
    case -809:
    case -807:
    case -806:
    case -805:
    case -804:
    case -803:
    case -802:
    case -801:
    case -718:
    case -717:
    case -716:
    case -715:
    case -714:
    case -713:
    case -712:
    case -711:
    case -708:
    case -707:
    case -706:
    case -705:
    case -704:
    case -703:
    case -702:
    case -701:
    case -600:
    case -506:
    case -505:
    case -504:
    case -503:
    case -502:
    case -501:
    case -500:
    case -407:
    case -406:
    case -405:
    case -404:
    case -403:
    case -402:
    case -401:
    case -400:
    case -300:
    case -227:
    case -226:
    case -225:
    case -224:
    case -223:
    case -222:
    case -221:
    case -220:
    case -219:
    case -218:
    case -217:
    case -216:
    case -215:
    case -214:
    case -213:
    case -212:
    case -211:
    case -210:
    case -209:
    case -208:
    case -207:
    case -206:
    case -205:
    case -204:
    case -203:
    case -201:
    case -200:
    case -117:
    case -116:
    case -115:
    case -114:
    case -113:
    case -112:
    case -111:
    case -110:
    case -109:
    case -108:
    case -107:
    case -106:
    case -105:
    case -104:
    case -103:
    case -102:
    case -101:
    case -100:
    case -5:
    case -4:
    case -3:
    case -2:
    case -1:
    case 0:
      return true;
    default:
      return false;
  }
}


// @@protoc_insertion_point(namespace_scope)
}  // namespace NFrame
namespace google {
namespace protobuf {
}  // namespace protobuf
}  // namespace google

// @@protoc_insertion_point(global_scope)
