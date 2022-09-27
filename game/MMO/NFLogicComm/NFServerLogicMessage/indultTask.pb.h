// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: indultTask.proto

#ifndef PROTOBUF_indultTask_2eproto__INCLUDED
#define PROTOBUF_indultTask_2eproto__INCLUDED

#include <string>

#include <google/protobuf/stubs/common.h>

#if GOOGLE_PROTOBUF_VERSION < 2005000
#error This file was generated by a newer version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please update
#error your headers.
#endif
#if 2005000 < GOOGLE_PROTOBUF_MIN_PROTOC_VERSION
#error This file was generated by an older version of protoc which is
#error incompatible with your Protocol Buffer headers.  Please
#error regenerate this file with a newer version of protoc.
#endif

#include <google/protobuf/generated_message_util.h>
#include <google/protobuf/message.h>
#include <google/protobuf/repeated_field.h>
#include <google/protobuf/extension_set.h>
#include <google/protobuf/unknown_field_set.h>
#include "yd_fieldoptions.pb.h"
// @@protoc_insertion_point(includes)

namespace proto_ff {

// Internal implementation detail -- do not call these.
void  protobuf_AddDesc_indultTask_2eproto();
void protobuf_AssignDesc_indultTask_2eproto();
void protobuf_ShutdownFile_indultTask_2eproto();

class indultTaskindultTask;
class Sheet_indultTaskindultTask;
class indultTasktask;
class Sheet_indultTasktask;

// ===================================================================

class indultTaskindultTask : public ::google::protobuf::Message {
 public:
  indultTaskindultTask();
  virtual ~indultTaskindultTask();

  indultTaskindultTask(const indultTaskindultTask& from);

  inline indultTaskindultTask& operator=(const indultTaskindultTask& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const indultTaskindultTask& default_instance();

  void Swap(indultTaskindultTask* other);

  // implements Message ----------------------------------------------

  indultTaskindultTask* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const indultTaskindultTask& from);
  void MergeFrom(const indultTaskindultTask& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional int32 id = 1;
  inline bool has_id() const;
  inline void clear_id();
  static const int kIdFieldNumber = 1;
  inline ::google::protobuf::int32 id() const;
  inline void set_id(::google::protobuf::int32 value);

  // optional int32 order = 2;
  inline bool has_order() const;
  inline void clear_order();
  static const int kOrderFieldNumber = 2;
  inline ::google::protobuf::int32 order() const;
  inline void set_order(::google::protobuf::int32 value);

  // optional int32 functionUnlock = 3;
  inline bool has_functionunlock() const;
  inline void clear_functionunlock();
  static const int kFunctionUnlockFieldNumber = 3;
  inline ::google::protobuf::int32 functionunlock() const;
  inline void set_functionunlock(::google::protobuf::int32 value);

  // optional int64 boxid = 4;
  inline bool has_boxid() const;
  inline void clear_boxid();
  static const int kBoxidFieldNumber = 4;
  inline ::google::protobuf::int64 boxid() const;
  inline void set_boxid(::google::protobuf::int64 value);

  // optional string taskID = 5;
  inline bool has_taskid() const;
  inline void clear_taskid();
  static const int kTaskIDFieldNumber = 5;
  inline const ::std::string& taskid() const;
  inline void set_taskid(const ::std::string& value);
  inline void set_taskid(const char* value);
  inline void set_taskid(const char* value, size_t size);
  inline ::std::string* mutable_taskid();
  inline ::std::string* release_taskid();
  inline void set_allocated_taskid(::std::string* taskid);

  // optional int32 taskNum = 6;
  inline bool has_tasknum() const;
  inline void clear_tasknum();
  static const int kTaskNumFieldNumber = 6;
  inline ::google::protobuf::int32 tasknum() const;
  inline void set_tasknum(::google::protobuf::int32 value);

  // @@protoc_insertion_point(class_scope:proto_ff.indultTaskindultTask)
 private:
  inline void set_has_id();
  inline void clear_has_id();
  inline void set_has_order();
  inline void clear_has_order();
  inline void set_has_functionunlock();
  inline void clear_has_functionunlock();
  inline void set_has_boxid();
  inline void clear_has_boxid();
  inline void set_has_taskid();
  inline void clear_has_taskid();
  inline void set_has_tasknum();
  inline void clear_has_tasknum();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::int32 id_;
  ::google::protobuf::int32 order_;
  ::google::protobuf::int64 boxid_;
  ::google::protobuf::int32 functionunlock_;
  ::google::protobuf::int32 tasknum_;
  ::std::string* taskid_;

  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(6 + 31) / 32];

  friend void  protobuf_AddDesc_indultTask_2eproto();
  friend void protobuf_AssignDesc_indultTask_2eproto();
  friend void protobuf_ShutdownFile_indultTask_2eproto();

  void InitAsDefaultInstance();
  static indultTaskindultTask* default_instance_;
};
// -------------------------------------------------------------------

class Sheet_indultTaskindultTask : public ::google::protobuf::Message {
 public:
  Sheet_indultTaskindultTask();
  virtual ~Sheet_indultTaskindultTask();

  Sheet_indultTaskindultTask(const Sheet_indultTaskindultTask& from);

  inline Sheet_indultTaskindultTask& operator=(const Sheet_indultTaskindultTask& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const Sheet_indultTaskindultTask& default_instance();

  void Swap(Sheet_indultTaskindultTask* other);

  // implements Message ----------------------------------------------

  Sheet_indultTaskindultTask* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const Sheet_indultTaskindultTask& from);
  void MergeFrom(const Sheet_indultTaskindultTask& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // repeated .proto_ff.indultTaskindultTask indultTaskindultTask_List = 1;
  inline int indulttaskindulttask_list_size() const;
  inline void clear_indulttaskindulttask_list();
  static const int kIndultTaskindultTaskListFieldNumber = 1;
  inline const ::proto_ff::indultTaskindultTask& indulttaskindulttask_list(int index) const;
  inline ::proto_ff::indultTaskindultTask* mutable_indulttaskindulttask_list(int index);
  inline ::proto_ff::indultTaskindultTask* add_indulttaskindulttask_list();
  inline const ::google::protobuf::RepeatedPtrField< ::proto_ff::indultTaskindultTask >&
      indulttaskindulttask_list() const;
  inline ::google::protobuf::RepeatedPtrField< ::proto_ff::indultTaskindultTask >*
      mutable_indulttaskindulttask_list();

  // @@protoc_insertion_point(class_scope:proto_ff.Sheet_indultTaskindultTask)
 private:

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::RepeatedPtrField< ::proto_ff::indultTaskindultTask > indulttaskindulttask_list_;

  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(1 + 31) / 32];

  friend void  protobuf_AddDesc_indultTask_2eproto();
  friend void protobuf_AssignDesc_indultTask_2eproto();
  friend void protobuf_ShutdownFile_indultTask_2eproto();

  void InitAsDefaultInstance();
  static Sheet_indultTaskindultTask* default_instance_;
};
// -------------------------------------------------------------------

class indultTasktask : public ::google::protobuf::Message {
 public:
  indultTasktask();
  virtual ~indultTasktask();

  indultTasktask(const indultTasktask& from);

  inline indultTasktask& operator=(const indultTasktask& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const indultTasktask& default_instance();

  void Swap(indultTasktask* other);

  // implements Message ----------------------------------------------

  indultTasktask* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const indultTasktask& from);
  void MergeFrom(const indultTasktask& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // optional int32 id = 1;
  inline bool has_id() const;
  inline void clear_id();
  static const int kIdFieldNumber = 1;
  inline ::google::protobuf::int32 id() const;
  inline void set_id(::google::protobuf::int32 value);

  // optional int32 task = 2;
  inline bool has_task() const;
  inline void clear_task();
  static const int kTaskFieldNumber = 2;
  inline ::google::protobuf::int32 task() const;
  inline void set_task(::google::protobuf::int32 value);

  // optional string taskParam = 3;
  inline bool has_taskparam() const;
  inline void clear_taskparam();
  static const int kTaskParamFieldNumber = 3;
  inline const ::std::string& taskparam() const;
  inline void set_taskparam(const ::std::string& value);
  inline void set_taskparam(const char* value);
  inline void set_taskparam(const char* value, size_t size);
  inline ::std::string* mutable_taskparam();
  inline ::std::string* release_taskparam();
  inline void set_allocated_taskparam(::std::string* taskparam);

  // @@protoc_insertion_point(class_scope:proto_ff.indultTasktask)
 private:
  inline void set_has_id();
  inline void clear_has_id();
  inline void set_has_task();
  inline void clear_has_task();
  inline void set_has_taskparam();
  inline void clear_has_taskparam();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::int32 id_;
  ::google::protobuf::int32 task_;
  ::std::string* taskparam_;

  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(3 + 31) / 32];

  friend void  protobuf_AddDesc_indultTask_2eproto();
  friend void protobuf_AssignDesc_indultTask_2eproto();
  friend void protobuf_ShutdownFile_indultTask_2eproto();

  void InitAsDefaultInstance();
  static indultTasktask* default_instance_;
};
// -------------------------------------------------------------------

class Sheet_indultTasktask : public ::google::protobuf::Message {
 public:
  Sheet_indultTasktask();
  virtual ~Sheet_indultTasktask();

  Sheet_indultTasktask(const Sheet_indultTasktask& from);

  inline Sheet_indultTasktask& operator=(const Sheet_indultTasktask& from) {
    CopyFrom(from);
    return *this;
  }

  inline const ::google::protobuf::UnknownFieldSet& unknown_fields() const {
    return _unknown_fields_;
  }

  inline ::google::protobuf::UnknownFieldSet* mutable_unknown_fields() {
    return &_unknown_fields_;
  }

  static const ::google::protobuf::Descriptor* descriptor();
  static const Sheet_indultTasktask& default_instance();

  void Swap(Sheet_indultTasktask* other);

  // implements Message ----------------------------------------------

  Sheet_indultTasktask* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const Sheet_indultTasktask& from);
  void MergeFrom(const Sheet_indultTasktask& from);
  void Clear();
  bool IsInitialized() const;

  int ByteSize() const;
  bool MergePartialFromCodedStream(
      ::google::protobuf::io::CodedInputStream* input);
  void SerializeWithCachedSizes(
      ::google::protobuf::io::CodedOutputStream* output) const;
  ::google::protobuf::uint8* SerializeWithCachedSizesToArray(::google::protobuf::uint8* output) const;
  int GetCachedSize() const { return _cached_size_; }
  private:
  void SharedCtor();
  void SharedDtor();
  void SetCachedSize(int size) const;
  public:

  ::google::protobuf::Metadata GetMetadata() const;

  // nested types ----------------------------------------------------

  // accessors -------------------------------------------------------

  // repeated .proto_ff.indultTasktask indultTasktask_List = 1;
  inline int indulttasktask_list_size() const;
  inline void clear_indulttasktask_list();
  static const int kIndultTasktaskListFieldNumber = 1;
  inline const ::proto_ff::indultTasktask& indulttasktask_list(int index) const;
  inline ::proto_ff::indultTasktask* mutable_indulttasktask_list(int index);
  inline ::proto_ff::indultTasktask* add_indulttasktask_list();
  inline const ::google::protobuf::RepeatedPtrField< ::proto_ff::indultTasktask >&
      indulttasktask_list() const;
  inline ::google::protobuf::RepeatedPtrField< ::proto_ff::indultTasktask >*
      mutable_indulttasktask_list();

  // @@protoc_insertion_point(class_scope:proto_ff.Sheet_indultTasktask)
 private:

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::RepeatedPtrField< ::proto_ff::indultTasktask > indulttasktask_list_;

  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(1 + 31) / 32];

  friend void  protobuf_AddDesc_indultTask_2eproto();
  friend void protobuf_AssignDesc_indultTask_2eproto();
  friend void protobuf_ShutdownFile_indultTask_2eproto();

  void InitAsDefaultInstance();
  static Sheet_indultTasktask* default_instance_;
};
// ===================================================================


// ===================================================================

// indultTaskindultTask

// optional int32 id = 1;
inline bool indultTaskindultTask::has_id() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void indultTaskindultTask::set_has_id() {
  _has_bits_[0] |= 0x00000001u;
}
inline void indultTaskindultTask::clear_has_id() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void indultTaskindultTask::clear_id() {
  id_ = 0;
  clear_has_id();
}
inline ::google::protobuf::int32 indultTaskindultTask::id() const {
  return id_;
}
inline void indultTaskindultTask::set_id(::google::protobuf::int32 value) {
  set_has_id();
  id_ = value;
}

// optional int32 order = 2;
inline bool indultTaskindultTask::has_order() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void indultTaskindultTask::set_has_order() {
  _has_bits_[0] |= 0x00000002u;
}
inline void indultTaskindultTask::clear_has_order() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void indultTaskindultTask::clear_order() {
  order_ = 0;
  clear_has_order();
}
inline ::google::protobuf::int32 indultTaskindultTask::order() const {
  return order_;
}
inline void indultTaskindultTask::set_order(::google::protobuf::int32 value) {
  set_has_order();
  order_ = value;
}

// optional int32 functionUnlock = 3;
inline bool indultTaskindultTask::has_functionunlock() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void indultTaskindultTask::set_has_functionunlock() {
  _has_bits_[0] |= 0x00000004u;
}
inline void indultTaskindultTask::clear_has_functionunlock() {
  _has_bits_[0] &= ~0x00000004u;
}
inline void indultTaskindultTask::clear_functionunlock() {
  functionunlock_ = 0;
  clear_has_functionunlock();
}
inline ::google::protobuf::int32 indultTaskindultTask::functionunlock() const {
  return functionunlock_;
}
inline void indultTaskindultTask::set_functionunlock(::google::protobuf::int32 value) {
  set_has_functionunlock();
  functionunlock_ = value;
}

// optional int64 boxid = 4;
inline bool indultTaskindultTask::has_boxid() const {
  return (_has_bits_[0] & 0x00000008u) != 0;
}
inline void indultTaskindultTask::set_has_boxid() {
  _has_bits_[0] |= 0x00000008u;
}
inline void indultTaskindultTask::clear_has_boxid() {
  _has_bits_[0] &= ~0x00000008u;
}
inline void indultTaskindultTask::clear_boxid() {
  boxid_ = GOOGLE_LONGLONG(0);
  clear_has_boxid();
}
inline ::google::protobuf::int64 indultTaskindultTask::boxid() const {
  return boxid_;
}
inline void indultTaskindultTask::set_boxid(::google::protobuf::int64 value) {
  set_has_boxid();
  boxid_ = value;
}

// optional string taskID = 5;
inline bool indultTaskindultTask::has_taskid() const {
  return (_has_bits_[0] & 0x00000010u) != 0;
}
inline void indultTaskindultTask::set_has_taskid() {
  _has_bits_[0] |= 0x00000010u;
}
inline void indultTaskindultTask::clear_has_taskid() {
  _has_bits_[0] &= ~0x00000010u;
}
inline void indultTaskindultTask::clear_taskid() {
  if (taskid_ != &::google::protobuf::internal::kEmptyString) {
    taskid_->clear();
  }
  clear_has_taskid();
}
inline const ::std::string& indultTaskindultTask::taskid() const {
  return *taskid_;
}
inline void indultTaskindultTask::set_taskid(const ::std::string& value) {
  set_has_taskid();
  if (taskid_ == &::google::protobuf::internal::kEmptyString) {
    taskid_ = new ::std::string;
  }
  taskid_->assign(value);
}
inline void indultTaskindultTask::set_taskid(const char* value) {
  set_has_taskid();
  if (taskid_ == &::google::protobuf::internal::kEmptyString) {
    taskid_ = new ::std::string;
  }
  taskid_->assign(value);
}
inline void indultTaskindultTask::set_taskid(const char* value, size_t size) {
  set_has_taskid();
  if (taskid_ == &::google::protobuf::internal::kEmptyString) {
    taskid_ = new ::std::string;
  }
  taskid_->assign(reinterpret_cast<const char*>(value), size);
}
inline ::std::string* indultTaskindultTask::mutable_taskid() {
  set_has_taskid();
  if (taskid_ == &::google::protobuf::internal::kEmptyString) {
    taskid_ = new ::std::string;
  }
  return taskid_;
}
inline ::std::string* indultTaskindultTask::release_taskid() {
  clear_has_taskid();
  if (taskid_ == &::google::protobuf::internal::kEmptyString) {
    return NULL;
  } else {
    ::std::string* temp = taskid_;
    taskid_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
    return temp;
  }
}
inline void indultTaskindultTask::set_allocated_taskid(::std::string* taskid) {
  if (taskid_ != &::google::protobuf::internal::kEmptyString) {
    delete taskid_;
  }
  if (taskid) {
    set_has_taskid();
    taskid_ = taskid;
  } else {
    clear_has_taskid();
    taskid_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  }
}

// optional int32 taskNum = 6;
inline bool indultTaskindultTask::has_tasknum() const {
  return (_has_bits_[0] & 0x00000020u) != 0;
}
inline void indultTaskindultTask::set_has_tasknum() {
  _has_bits_[0] |= 0x00000020u;
}
inline void indultTaskindultTask::clear_has_tasknum() {
  _has_bits_[0] &= ~0x00000020u;
}
inline void indultTaskindultTask::clear_tasknum() {
  tasknum_ = 0;
  clear_has_tasknum();
}
inline ::google::protobuf::int32 indultTaskindultTask::tasknum() const {
  return tasknum_;
}
inline void indultTaskindultTask::set_tasknum(::google::protobuf::int32 value) {
  set_has_tasknum();
  tasknum_ = value;
}

// -------------------------------------------------------------------

// Sheet_indultTaskindultTask

// repeated .proto_ff.indultTaskindultTask indultTaskindultTask_List = 1;
inline int Sheet_indultTaskindultTask::indulttaskindulttask_list_size() const {
  return indulttaskindulttask_list_.size();
}
inline void Sheet_indultTaskindultTask::clear_indulttaskindulttask_list() {
  indulttaskindulttask_list_.Clear();
}
inline const ::proto_ff::indultTaskindultTask& Sheet_indultTaskindultTask::indulttaskindulttask_list(int index) const {
  return indulttaskindulttask_list_.Get(index);
}
inline ::proto_ff::indultTaskindultTask* Sheet_indultTaskindultTask::mutable_indulttaskindulttask_list(int index) {
  return indulttaskindulttask_list_.Mutable(index);
}
inline ::proto_ff::indultTaskindultTask* Sheet_indultTaskindultTask::add_indulttaskindulttask_list() {
  return indulttaskindulttask_list_.Add();
}
inline const ::google::protobuf::RepeatedPtrField< ::proto_ff::indultTaskindultTask >&
Sheet_indultTaskindultTask::indulttaskindulttask_list() const {
  return indulttaskindulttask_list_;
}
inline ::google::protobuf::RepeatedPtrField< ::proto_ff::indultTaskindultTask >*
Sheet_indultTaskindultTask::mutable_indulttaskindulttask_list() {
  return &indulttaskindulttask_list_;
}

// -------------------------------------------------------------------

// indultTasktask

// optional int32 id = 1;
inline bool indultTasktask::has_id() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void indultTasktask::set_has_id() {
  _has_bits_[0] |= 0x00000001u;
}
inline void indultTasktask::clear_has_id() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void indultTasktask::clear_id() {
  id_ = 0;
  clear_has_id();
}
inline ::google::protobuf::int32 indultTasktask::id() const {
  return id_;
}
inline void indultTasktask::set_id(::google::protobuf::int32 value) {
  set_has_id();
  id_ = value;
}

// optional int32 task = 2;
inline bool indultTasktask::has_task() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void indultTasktask::set_has_task() {
  _has_bits_[0] |= 0x00000002u;
}
inline void indultTasktask::clear_has_task() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void indultTasktask::clear_task() {
  task_ = 0;
  clear_has_task();
}
inline ::google::protobuf::int32 indultTasktask::task() const {
  return task_;
}
inline void indultTasktask::set_task(::google::protobuf::int32 value) {
  set_has_task();
  task_ = value;
}

// optional string taskParam = 3;
inline bool indultTasktask::has_taskparam() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void indultTasktask::set_has_taskparam() {
  _has_bits_[0] |= 0x00000004u;
}
inline void indultTasktask::clear_has_taskparam() {
  _has_bits_[0] &= ~0x00000004u;
}
inline void indultTasktask::clear_taskparam() {
  if (taskparam_ != &::google::protobuf::internal::kEmptyString) {
    taskparam_->clear();
  }
  clear_has_taskparam();
}
inline const ::std::string& indultTasktask::taskparam() const {
  return *taskparam_;
}
inline void indultTasktask::set_taskparam(const ::std::string& value) {
  set_has_taskparam();
  if (taskparam_ == &::google::protobuf::internal::kEmptyString) {
    taskparam_ = new ::std::string;
  }
  taskparam_->assign(value);
}
inline void indultTasktask::set_taskparam(const char* value) {
  set_has_taskparam();
  if (taskparam_ == &::google::protobuf::internal::kEmptyString) {
    taskparam_ = new ::std::string;
  }
  taskparam_->assign(value);
}
inline void indultTasktask::set_taskparam(const char* value, size_t size) {
  set_has_taskparam();
  if (taskparam_ == &::google::protobuf::internal::kEmptyString) {
    taskparam_ = new ::std::string;
  }
  taskparam_->assign(reinterpret_cast<const char*>(value), size);
}
inline ::std::string* indultTasktask::mutable_taskparam() {
  set_has_taskparam();
  if (taskparam_ == &::google::protobuf::internal::kEmptyString) {
    taskparam_ = new ::std::string;
  }
  return taskparam_;
}
inline ::std::string* indultTasktask::release_taskparam() {
  clear_has_taskparam();
  if (taskparam_ == &::google::protobuf::internal::kEmptyString) {
    return NULL;
  } else {
    ::std::string* temp = taskparam_;
    taskparam_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
    return temp;
  }
}
inline void indultTasktask::set_allocated_taskparam(::std::string* taskparam) {
  if (taskparam_ != &::google::protobuf::internal::kEmptyString) {
    delete taskparam_;
  }
  if (taskparam) {
    set_has_taskparam();
    taskparam_ = taskparam;
  } else {
    clear_has_taskparam();
    taskparam_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  }
}

// -------------------------------------------------------------------

// Sheet_indultTasktask

// repeated .proto_ff.indultTasktask indultTasktask_List = 1;
inline int Sheet_indultTasktask::indulttasktask_list_size() const {
  return indulttasktask_list_.size();
}
inline void Sheet_indultTasktask::clear_indulttasktask_list() {
  indulttasktask_list_.Clear();
}
inline const ::proto_ff::indultTasktask& Sheet_indultTasktask::indulttasktask_list(int index) const {
  return indulttasktask_list_.Get(index);
}
inline ::proto_ff::indultTasktask* Sheet_indultTasktask::mutable_indulttasktask_list(int index) {
  return indulttasktask_list_.Mutable(index);
}
inline ::proto_ff::indultTasktask* Sheet_indultTasktask::add_indulttasktask_list() {
  return indulttasktask_list_.Add();
}
inline const ::google::protobuf::RepeatedPtrField< ::proto_ff::indultTasktask >&
Sheet_indultTasktask::indulttasktask_list() const {
  return indulttasktask_list_;
}
inline ::google::protobuf::RepeatedPtrField< ::proto_ff::indultTasktask >*
Sheet_indultTasktask::mutable_indulttasktask_list() {
  return &indulttasktask_list_;
}


// @@protoc_insertion_point(namespace_scope)

}  // namespace proto_ff

#ifndef SWIG
namespace google {
namespace protobuf {


}  // namespace google
}  // namespace protobuf
#endif  // SWIG

// @@protoc_insertion_point(global_scope)

#endif  // PROTOBUF_indultTask_2eproto__INCLUDED