// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: client_to_server.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "client_to_server.pb.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/once.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)

namespace proto_ff {

namespace {

const ::google::protobuf::EnumDescriptor* ClientToServerCmd_descriptor_ = NULL;

}  // namespace


void protobuf_AssignDesc_client_5fto_5fserver_2eproto() {
  protobuf_AddDesc_client_5fto_5fserver_2eproto();
  const ::google::protobuf::FileDescriptor* file =
    ::google::protobuf::DescriptorPool::generated_pool()->FindFileByName(
      "client_to_server.proto");
  GOOGLE_CHECK(file != NULL);
  ClientToServerCmd_descriptor_ = file->enum_type(0);
}

namespace {

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AssignDescriptors_once_);
inline void protobuf_AssignDescriptorsOnce() {
  ::google::protobuf::GoogleOnceInit(&protobuf_AssignDescriptors_once_,
                 &protobuf_AssignDesc_client_5fto_5fserver_2eproto);
}

void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
}

}  // namespace

void protobuf_ShutdownFile_client_5fto_5fserver_2eproto() {
}

void protobuf_AddDesc_client_5fto_5fserver_2eproto() {
  static bool already_here = false;
  if (already_here) return;
  already_here = true;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::yd_fieldoptions::protobuf_AddDesc_yd_5ffieldoptions_2eproto();
  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
    "\n\026client_to_server.proto\022\010proto_ff\032\025yd_f"
    "ieldoptions.proto*\220\003\n\021ClientToServerCmd\022"
    " \n\034CLIENT_TO_SERVER_LOGIN_BEGIN\020\001\022\030\n\024CLI"
    "ENT_TO_LOGIN_PING\020\002\022!\n\035CLIENT_TO_LOGIN_A"
    "CCOUNT_LOGIN\020\003\022\037\n\033CLIENT_TO_LOGIN_SELECT"
    "_ZONE\020\004\022\037\n\033CLIENT_TO_ZONE_GATEINFO_REQ\020\005"
    "\022\036\n\032CLIENT_TO_SERVER_LOGIN_END\020\t\022\030\n\024CLIE"
    "NT_TO_LOGIC_PING\020\n\022\027\n\023CLIENT_TO_ZONE_PIN"
    "G\020\013\022 \n\034CLIENT_TO_CENTER_LOGIN_BEGIN\020\022\022\032\n"
    "\026CLIENT_TO_CENTER_LOGIN\020\023\022%\n!CLIENT_TO_C"
    "ENTER_CREATE_CHARACTER\020\024\022\"\n\036CLIENT_TO_CE"
    "NTER_DEL_CHARACTER\020\025", 460);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "client_to_server.proto", &protobuf_RegisterTypes);
  ::google::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_client_5fto_5fserver_2eproto);
}

// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_client_5fto_5fserver_2eproto {
  StaticDescriptorInitializer_client_5fto_5fserver_2eproto() {
    protobuf_AddDesc_client_5fto_5fserver_2eproto();
  }
} static_descriptor_initializer_client_5fto_5fserver_2eproto_;
const ::google::protobuf::EnumDescriptor* ClientToServerCmd_descriptor() {
  protobuf_AssignDescriptorsOnce();
  return ClientToServerCmd_descriptor_;
}
bool ClientToServerCmd_IsValid(int value) {
  switch(value) {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 9:
    case 10:
    case 11:
    case 18:
    case 19:
    case 20:
    case 21:
      return true;
    default:
      return false;
  }
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace proto_ff

// @@protoc_insertion_point(global_scope)