// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: E_Pk1v1.proto

#ifndef PROTOBUF_E_5fPk1v1_2eproto__INCLUDED
#define PROTOBUF_E_5fPk1v1_2eproto__INCLUDED

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
void  protobuf_AddDesc_E_5fPk1v1_2eproto();
void protobuf_AssignDesc_E_5fPk1v1_2eproto();
void protobuf_ShutdownFile_E_5fPk1v1_2eproto();

class E_Pk1v1Rank;
class Sheet_Pk1v1Rank;
class E_Pk1v1Reward;
class Sheet_Pk1v1Reward;
class E_Pk1v1Dailyreward;
class Sheet_Pk1v1Dailyreward;

// ===================================================================

class E_Pk1v1Rank : public ::google::protobuf::Message {
 public:
  E_Pk1v1Rank();
  virtual ~E_Pk1v1Rank();

  E_Pk1v1Rank(const E_Pk1v1Rank& from);

  inline E_Pk1v1Rank& operator=(const E_Pk1v1Rank& from) {
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
  static const E_Pk1v1Rank& default_instance();

  void Swap(E_Pk1v1Rank* other);

  // implements Message ----------------------------------------------

  E_Pk1v1Rank* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const E_Pk1v1Rank& from);
  void MergeFrom(const E_Pk1v1Rank& from);
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

  // optional int32 m_id = 1;
  inline bool has_m_id() const;
  inline void clear_m_id();
  static const int kMIdFieldNumber = 1;
  inline ::google::protobuf::int32 m_id() const;
  inline void set_m_id(::google::protobuf::int32 value);

  // optional int32 m_mark = 2;
  inline bool has_m_mark() const;
  inline void clear_m_mark();
  static const int kMMarkFieldNumber = 2;
  inline ::google::protobuf::int32 m_mark() const;
  inline void set_m_mark(::google::protobuf::int32 value);

  // optional int32 m_scoremin = 3;
  inline bool has_m_scoremin() const;
  inline void clear_m_scoremin();
  static const int kMScoreminFieldNumber = 3;
  inline ::google::protobuf::int32 m_scoremin() const;
  inline void set_m_scoremin(::google::protobuf::int32 value);

  // optional int32 m_scoremax = 4;
  inline bool has_m_scoremax() const;
  inline void clear_m_scoremax();
  static const int kMScoremaxFieldNumber = 4;
  inline ::google::protobuf::int32 m_scoremax() const;
  inline void set_m_scoremax(::google::protobuf::int32 value);

  // optional int32 m_sucessscore = 5;
  inline bool has_m_sucessscore() const;
  inline void clear_m_sucessscore();
  static const int kMSucessscoreFieldNumber = 5;
  inline ::google::protobuf::int32 m_sucessscore() const;
  inline void set_m_sucessscore(::google::protobuf::int32 value);

  // optional int32 m_defeatscore = 6;
  inline bool has_m_defeatscore() const;
  inline void clear_m_defeatscore();
  static const int kMDefeatscoreFieldNumber = 6;
  inline ::google::protobuf::int32 m_defeatscore() const;
  inline void set_m_defeatscore(::google::protobuf::int32 value);

  // optional int32 m_sucessbox = 7;
  inline bool has_m_sucessbox() const;
  inline void clear_m_sucessbox();
  static const int kMSucessboxFieldNumber = 7;
  inline ::google::protobuf::int32 m_sucessbox() const;
  inline void set_m_sucessbox(::google::protobuf::int32 value);

  // optional int32 m_defeatbox = 8;
  inline bool has_m_defeatbox() const;
  inline void clear_m_defeatbox();
  static const int kMDefeatboxFieldNumber = 8;
  inline ::google::protobuf::int32 m_defeatbox() const;
  inline void set_m_defeatbox(::google::protobuf::int32 value);

  // optional int32 m_tiebox = 9;
  inline bool has_m_tiebox() const;
  inline void clear_m_tiebox();
  static const int kMTieboxFieldNumber = 9;
  inline ::google::protobuf::int32 m_tiebox() const;
  inline void set_m_tiebox(::google::protobuf::int32 value);

  // @@protoc_insertion_point(class_scope:proto_ff.E_Pk1v1Rank)
 private:
  inline void set_has_m_id();
  inline void clear_has_m_id();
  inline void set_has_m_mark();
  inline void clear_has_m_mark();
  inline void set_has_m_scoremin();
  inline void clear_has_m_scoremin();
  inline void set_has_m_scoremax();
  inline void clear_has_m_scoremax();
  inline void set_has_m_sucessscore();
  inline void clear_has_m_sucessscore();
  inline void set_has_m_defeatscore();
  inline void clear_has_m_defeatscore();
  inline void set_has_m_sucessbox();
  inline void clear_has_m_sucessbox();
  inline void set_has_m_defeatbox();
  inline void clear_has_m_defeatbox();
  inline void set_has_m_tiebox();
  inline void clear_has_m_tiebox();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::int32 m_id_;
  ::google::protobuf::int32 m_mark_;
  ::google::protobuf::int32 m_scoremin_;
  ::google::protobuf::int32 m_scoremax_;
  ::google::protobuf::int32 m_sucessscore_;
  ::google::protobuf::int32 m_defeatscore_;
  ::google::protobuf::int32 m_sucessbox_;
  ::google::protobuf::int32 m_defeatbox_;
  ::google::protobuf::int32 m_tiebox_;

  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(9 + 31) / 32];

  friend void  protobuf_AddDesc_E_5fPk1v1_2eproto();
  friend void protobuf_AssignDesc_E_5fPk1v1_2eproto();
  friend void protobuf_ShutdownFile_E_5fPk1v1_2eproto();

  void InitAsDefaultInstance();
  static E_Pk1v1Rank* default_instance_;
};
// -------------------------------------------------------------------

class Sheet_Pk1v1Rank : public ::google::protobuf::Message {
 public:
  Sheet_Pk1v1Rank();
  virtual ~Sheet_Pk1v1Rank();

  Sheet_Pk1v1Rank(const Sheet_Pk1v1Rank& from);

  inline Sheet_Pk1v1Rank& operator=(const Sheet_Pk1v1Rank& from) {
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
  static const Sheet_Pk1v1Rank& default_instance();

  void Swap(Sheet_Pk1v1Rank* other);

  // implements Message ----------------------------------------------

  Sheet_Pk1v1Rank* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const Sheet_Pk1v1Rank& from);
  void MergeFrom(const Sheet_Pk1v1Rank& from);
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

  // repeated .proto_ff.E_Pk1v1Rank E_Pk1v1Rank_List = 1;
  inline int e_pk1v1rank_list_size() const;
  inline void clear_e_pk1v1rank_list();
  static const int kEPk1V1RankListFieldNumber = 1;
  inline const ::proto_ff::E_Pk1v1Rank& e_pk1v1rank_list(int index) const;
  inline ::proto_ff::E_Pk1v1Rank* mutable_e_pk1v1rank_list(int index);
  inline ::proto_ff::E_Pk1v1Rank* add_e_pk1v1rank_list();
  inline const ::google::protobuf::RepeatedPtrField< ::proto_ff::E_Pk1v1Rank >&
      e_pk1v1rank_list() const;
  inline ::google::protobuf::RepeatedPtrField< ::proto_ff::E_Pk1v1Rank >*
      mutable_e_pk1v1rank_list();

  // @@protoc_insertion_point(class_scope:proto_ff.Sheet_Pk1v1Rank)
 private:

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::RepeatedPtrField< ::proto_ff::E_Pk1v1Rank > e_pk1v1rank_list_;

  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(1 + 31) / 32];

  friend void  protobuf_AddDesc_E_5fPk1v1_2eproto();
  friend void protobuf_AssignDesc_E_5fPk1v1_2eproto();
  friend void protobuf_ShutdownFile_E_5fPk1v1_2eproto();

  void InitAsDefaultInstance();
  static Sheet_Pk1v1Rank* default_instance_;
};
// -------------------------------------------------------------------

class E_Pk1v1Reward : public ::google::protobuf::Message {
 public:
  E_Pk1v1Reward();
  virtual ~E_Pk1v1Reward();

  E_Pk1v1Reward(const E_Pk1v1Reward& from);

  inline E_Pk1v1Reward& operator=(const E_Pk1v1Reward& from) {
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
  static const E_Pk1v1Reward& default_instance();

  void Swap(E_Pk1v1Reward* other);

  // implements Message ----------------------------------------------

  E_Pk1v1Reward* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const E_Pk1v1Reward& from);
  void MergeFrom(const E_Pk1v1Reward& from);
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

  // optional int32 m_id = 1;
  inline bool has_m_id() const;
  inline void clear_m_id();
  static const int kMIdFieldNumber = 1;
  inline ::google::protobuf::int32 m_id() const;
  inline void set_m_id(::google::protobuf::int32 value);

  // optional int32 m_type = 2;
  inline bool has_m_type() const;
  inline void clear_m_type();
  static const int kMTypeFieldNumber = 2;
  inline ::google::protobuf::int32 m_type() const;
  inline void set_m_type(::google::protobuf::int32 value);

  // optional int32 m_boxid = 3;
  inline bool has_m_boxid() const;
  inline void clear_m_boxid();
  static const int kMBoxidFieldNumber = 3;
  inline ::google::protobuf::int32 m_boxid() const;
  inline void set_m_boxid(::google::protobuf::int32 value);

  // repeated int32 m_typearg = 4;
  inline int m_typearg_size() const;
  inline void clear_m_typearg();
  static const int kMTypeargFieldNumber = 4;
  inline ::google::protobuf::int32 m_typearg(int index) const;
  inline void set_m_typearg(int index, ::google::protobuf::int32 value);
  inline void add_m_typearg(::google::protobuf::int32 value);
  inline const ::google::protobuf::RepeatedField< ::google::protobuf::int32 >&
      m_typearg() const;
  inline ::google::protobuf::RepeatedField< ::google::protobuf::int32 >*
      mutable_m_typearg();

  // @@protoc_insertion_point(class_scope:proto_ff.E_Pk1v1Reward)
 private:
  inline void set_has_m_id();
  inline void clear_has_m_id();
  inline void set_has_m_type();
  inline void clear_has_m_type();
  inline void set_has_m_boxid();
  inline void clear_has_m_boxid();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::int32 m_id_;
  ::google::protobuf::int32 m_type_;
  ::google::protobuf::RepeatedField< ::google::protobuf::int32 > m_typearg_;
  ::google::protobuf::int32 m_boxid_;

  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(4 + 31) / 32];

  friend void  protobuf_AddDesc_E_5fPk1v1_2eproto();
  friend void protobuf_AssignDesc_E_5fPk1v1_2eproto();
  friend void protobuf_ShutdownFile_E_5fPk1v1_2eproto();

  void InitAsDefaultInstance();
  static E_Pk1v1Reward* default_instance_;
};
// -------------------------------------------------------------------

class Sheet_Pk1v1Reward : public ::google::protobuf::Message {
 public:
  Sheet_Pk1v1Reward();
  virtual ~Sheet_Pk1v1Reward();

  Sheet_Pk1v1Reward(const Sheet_Pk1v1Reward& from);

  inline Sheet_Pk1v1Reward& operator=(const Sheet_Pk1v1Reward& from) {
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
  static const Sheet_Pk1v1Reward& default_instance();

  void Swap(Sheet_Pk1v1Reward* other);

  // implements Message ----------------------------------------------

  Sheet_Pk1v1Reward* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const Sheet_Pk1v1Reward& from);
  void MergeFrom(const Sheet_Pk1v1Reward& from);
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

  // repeated .proto_ff.E_Pk1v1Reward E_Pk1v1Reward_List = 1;
  inline int e_pk1v1reward_list_size() const;
  inline void clear_e_pk1v1reward_list();
  static const int kEPk1V1RewardListFieldNumber = 1;
  inline const ::proto_ff::E_Pk1v1Reward& e_pk1v1reward_list(int index) const;
  inline ::proto_ff::E_Pk1v1Reward* mutable_e_pk1v1reward_list(int index);
  inline ::proto_ff::E_Pk1v1Reward* add_e_pk1v1reward_list();
  inline const ::google::protobuf::RepeatedPtrField< ::proto_ff::E_Pk1v1Reward >&
      e_pk1v1reward_list() const;
  inline ::google::protobuf::RepeatedPtrField< ::proto_ff::E_Pk1v1Reward >*
      mutable_e_pk1v1reward_list();

  // @@protoc_insertion_point(class_scope:proto_ff.Sheet_Pk1v1Reward)
 private:

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::RepeatedPtrField< ::proto_ff::E_Pk1v1Reward > e_pk1v1reward_list_;

  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(1 + 31) / 32];

  friend void  protobuf_AddDesc_E_5fPk1v1_2eproto();
  friend void protobuf_AssignDesc_E_5fPk1v1_2eproto();
  friend void protobuf_ShutdownFile_E_5fPk1v1_2eproto();

  void InitAsDefaultInstance();
  static Sheet_Pk1v1Reward* default_instance_;
};
// -------------------------------------------------------------------

class E_Pk1v1Dailyreward : public ::google::protobuf::Message {
 public:
  E_Pk1v1Dailyreward();
  virtual ~E_Pk1v1Dailyreward();

  E_Pk1v1Dailyreward(const E_Pk1v1Dailyreward& from);

  inline E_Pk1v1Dailyreward& operator=(const E_Pk1v1Dailyreward& from) {
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
  static const E_Pk1v1Dailyreward& default_instance();

  void Swap(E_Pk1v1Dailyreward* other);

  // implements Message ----------------------------------------------

  E_Pk1v1Dailyreward* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const E_Pk1v1Dailyreward& from);
  void MergeFrom(const E_Pk1v1Dailyreward& from);
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

  // optional int32 m_id = 1;
  inline bool has_m_id() const;
  inline void clear_m_id();
  static const int kMIdFieldNumber = 1;
  inline ::google::protobuf::int32 m_id() const;
  inline void set_m_id(::google::protobuf::int32 value);

  // optional string m_name = 2;
  inline bool has_m_name() const;
  inline void clear_m_name();
  static const int kMNameFieldNumber = 2;
  inline const ::std::string& m_name() const;
  inline void set_m_name(const ::std::string& value);
  inline void set_m_name(const char* value);
  inline void set_m_name(const char* value, size_t size);
  inline ::std::string* mutable_m_name();
  inline ::std::string* release_m_name();
  inline void set_allocated_m_name(::std::string* m_name);

  // optional int32 m_times = 3;
  inline bool has_m_times() const;
  inline void clear_m_times();
  static const int kMTimesFieldNumber = 3;
  inline ::google::protobuf::int32 m_times() const;
  inline void set_m_times(::google::protobuf::int32 value);

  // optional int32 m_boxid = 4;
  inline bool has_m_boxid() const;
  inline void clear_m_boxid();
  static const int kMBoxidFieldNumber = 4;
  inline ::google::protobuf::int32 m_boxid() const;
  inline void set_m_boxid(::google::protobuf::int32 value);

  // @@protoc_insertion_point(class_scope:proto_ff.E_Pk1v1Dailyreward)
 private:
  inline void set_has_m_id();
  inline void clear_has_m_id();
  inline void set_has_m_name();
  inline void clear_has_m_name();
  inline void set_has_m_times();
  inline void clear_has_m_times();
  inline void set_has_m_boxid();
  inline void clear_has_m_boxid();

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::std::string* m_name_;
  ::google::protobuf::int32 m_id_;
  ::google::protobuf::int32 m_times_;
  ::google::protobuf::int32 m_boxid_;

  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(4 + 31) / 32];

  friend void  protobuf_AddDesc_E_5fPk1v1_2eproto();
  friend void protobuf_AssignDesc_E_5fPk1v1_2eproto();
  friend void protobuf_ShutdownFile_E_5fPk1v1_2eproto();

  void InitAsDefaultInstance();
  static E_Pk1v1Dailyreward* default_instance_;
};
// -------------------------------------------------------------------

class Sheet_Pk1v1Dailyreward : public ::google::protobuf::Message {
 public:
  Sheet_Pk1v1Dailyreward();
  virtual ~Sheet_Pk1v1Dailyreward();

  Sheet_Pk1v1Dailyreward(const Sheet_Pk1v1Dailyreward& from);

  inline Sheet_Pk1v1Dailyreward& operator=(const Sheet_Pk1v1Dailyreward& from) {
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
  static const Sheet_Pk1v1Dailyreward& default_instance();

  void Swap(Sheet_Pk1v1Dailyreward* other);

  // implements Message ----------------------------------------------

  Sheet_Pk1v1Dailyreward* New() const;
  void CopyFrom(const ::google::protobuf::Message& from);
  void MergeFrom(const ::google::protobuf::Message& from);
  void CopyFrom(const Sheet_Pk1v1Dailyreward& from);
  void MergeFrom(const Sheet_Pk1v1Dailyreward& from);
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

  // repeated .proto_ff.E_Pk1v1Dailyreward E_Pk1v1Dailyreward_List = 1;
  inline int e_pk1v1dailyreward_list_size() const;
  inline void clear_e_pk1v1dailyreward_list();
  static const int kEPk1V1DailyrewardListFieldNumber = 1;
  inline const ::proto_ff::E_Pk1v1Dailyreward& e_pk1v1dailyreward_list(int index) const;
  inline ::proto_ff::E_Pk1v1Dailyreward* mutable_e_pk1v1dailyreward_list(int index);
  inline ::proto_ff::E_Pk1v1Dailyreward* add_e_pk1v1dailyreward_list();
  inline const ::google::protobuf::RepeatedPtrField< ::proto_ff::E_Pk1v1Dailyreward >&
      e_pk1v1dailyreward_list() const;
  inline ::google::protobuf::RepeatedPtrField< ::proto_ff::E_Pk1v1Dailyreward >*
      mutable_e_pk1v1dailyreward_list();

  // @@protoc_insertion_point(class_scope:proto_ff.Sheet_Pk1v1Dailyreward)
 private:

  ::google::protobuf::UnknownFieldSet _unknown_fields_;

  ::google::protobuf::RepeatedPtrField< ::proto_ff::E_Pk1v1Dailyreward > e_pk1v1dailyreward_list_;

  mutable int _cached_size_;
  ::google::protobuf::uint32 _has_bits_[(1 + 31) / 32];

  friend void  protobuf_AddDesc_E_5fPk1v1_2eproto();
  friend void protobuf_AssignDesc_E_5fPk1v1_2eproto();
  friend void protobuf_ShutdownFile_E_5fPk1v1_2eproto();

  void InitAsDefaultInstance();
  static Sheet_Pk1v1Dailyreward* default_instance_;
};
// ===================================================================


// ===================================================================

// E_Pk1v1Rank

// optional int32 m_id = 1;
inline bool E_Pk1v1Rank::has_m_id() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void E_Pk1v1Rank::set_has_m_id() {
  _has_bits_[0] |= 0x00000001u;
}
inline void E_Pk1v1Rank::clear_has_m_id() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void E_Pk1v1Rank::clear_m_id() {
  m_id_ = 0;
  clear_has_m_id();
}
inline ::google::protobuf::int32 E_Pk1v1Rank::m_id() const {
  return m_id_;
}
inline void E_Pk1v1Rank::set_m_id(::google::protobuf::int32 value) {
  set_has_m_id();
  m_id_ = value;
}

// optional int32 m_mark = 2;
inline bool E_Pk1v1Rank::has_m_mark() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void E_Pk1v1Rank::set_has_m_mark() {
  _has_bits_[0] |= 0x00000002u;
}
inline void E_Pk1v1Rank::clear_has_m_mark() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void E_Pk1v1Rank::clear_m_mark() {
  m_mark_ = 0;
  clear_has_m_mark();
}
inline ::google::protobuf::int32 E_Pk1v1Rank::m_mark() const {
  return m_mark_;
}
inline void E_Pk1v1Rank::set_m_mark(::google::protobuf::int32 value) {
  set_has_m_mark();
  m_mark_ = value;
}

// optional int32 m_scoremin = 3;
inline bool E_Pk1v1Rank::has_m_scoremin() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void E_Pk1v1Rank::set_has_m_scoremin() {
  _has_bits_[0] |= 0x00000004u;
}
inline void E_Pk1v1Rank::clear_has_m_scoremin() {
  _has_bits_[0] &= ~0x00000004u;
}
inline void E_Pk1v1Rank::clear_m_scoremin() {
  m_scoremin_ = 0;
  clear_has_m_scoremin();
}
inline ::google::protobuf::int32 E_Pk1v1Rank::m_scoremin() const {
  return m_scoremin_;
}
inline void E_Pk1v1Rank::set_m_scoremin(::google::protobuf::int32 value) {
  set_has_m_scoremin();
  m_scoremin_ = value;
}

// optional int32 m_scoremax = 4;
inline bool E_Pk1v1Rank::has_m_scoremax() const {
  return (_has_bits_[0] & 0x00000008u) != 0;
}
inline void E_Pk1v1Rank::set_has_m_scoremax() {
  _has_bits_[0] |= 0x00000008u;
}
inline void E_Pk1v1Rank::clear_has_m_scoremax() {
  _has_bits_[0] &= ~0x00000008u;
}
inline void E_Pk1v1Rank::clear_m_scoremax() {
  m_scoremax_ = 0;
  clear_has_m_scoremax();
}
inline ::google::protobuf::int32 E_Pk1v1Rank::m_scoremax() const {
  return m_scoremax_;
}
inline void E_Pk1v1Rank::set_m_scoremax(::google::protobuf::int32 value) {
  set_has_m_scoremax();
  m_scoremax_ = value;
}

// optional int32 m_sucessscore = 5;
inline bool E_Pk1v1Rank::has_m_sucessscore() const {
  return (_has_bits_[0] & 0x00000010u) != 0;
}
inline void E_Pk1v1Rank::set_has_m_sucessscore() {
  _has_bits_[0] |= 0x00000010u;
}
inline void E_Pk1v1Rank::clear_has_m_sucessscore() {
  _has_bits_[0] &= ~0x00000010u;
}
inline void E_Pk1v1Rank::clear_m_sucessscore() {
  m_sucessscore_ = 0;
  clear_has_m_sucessscore();
}
inline ::google::protobuf::int32 E_Pk1v1Rank::m_sucessscore() const {
  return m_sucessscore_;
}
inline void E_Pk1v1Rank::set_m_sucessscore(::google::protobuf::int32 value) {
  set_has_m_sucessscore();
  m_sucessscore_ = value;
}

// optional int32 m_defeatscore = 6;
inline bool E_Pk1v1Rank::has_m_defeatscore() const {
  return (_has_bits_[0] & 0x00000020u) != 0;
}
inline void E_Pk1v1Rank::set_has_m_defeatscore() {
  _has_bits_[0] |= 0x00000020u;
}
inline void E_Pk1v1Rank::clear_has_m_defeatscore() {
  _has_bits_[0] &= ~0x00000020u;
}
inline void E_Pk1v1Rank::clear_m_defeatscore() {
  m_defeatscore_ = 0;
  clear_has_m_defeatscore();
}
inline ::google::protobuf::int32 E_Pk1v1Rank::m_defeatscore() const {
  return m_defeatscore_;
}
inline void E_Pk1v1Rank::set_m_defeatscore(::google::protobuf::int32 value) {
  set_has_m_defeatscore();
  m_defeatscore_ = value;
}

// optional int32 m_sucessbox = 7;
inline bool E_Pk1v1Rank::has_m_sucessbox() const {
  return (_has_bits_[0] & 0x00000040u) != 0;
}
inline void E_Pk1v1Rank::set_has_m_sucessbox() {
  _has_bits_[0] |= 0x00000040u;
}
inline void E_Pk1v1Rank::clear_has_m_sucessbox() {
  _has_bits_[0] &= ~0x00000040u;
}
inline void E_Pk1v1Rank::clear_m_sucessbox() {
  m_sucessbox_ = 0;
  clear_has_m_sucessbox();
}
inline ::google::protobuf::int32 E_Pk1v1Rank::m_sucessbox() const {
  return m_sucessbox_;
}
inline void E_Pk1v1Rank::set_m_sucessbox(::google::protobuf::int32 value) {
  set_has_m_sucessbox();
  m_sucessbox_ = value;
}

// optional int32 m_defeatbox = 8;
inline bool E_Pk1v1Rank::has_m_defeatbox() const {
  return (_has_bits_[0] & 0x00000080u) != 0;
}
inline void E_Pk1v1Rank::set_has_m_defeatbox() {
  _has_bits_[0] |= 0x00000080u;
}
inline void E_Pk1v1Rank::clear_has_m_defeatbox() {
  _has_bits_[0] &= ~0x00000080u;
}
inline void E_Pk1v1Rank::clear_m_defeatbox() {
  m_defeatbox_ = 0;
  clear_has_m_defeatbox();
}
inline ::google::protobuf::int32 E_Pk1v1Rank::m_defeatbox() const {
  return m_defeatbox_;
}
inline void E_Pk1v1Rank::set_m_defeatbox(::google::protobuf::int32 value) {
  set_has_m_defeatbox();
  m_defeatbox_ = value;
}

// optional int32 m_tiebox = 9;
inline bool E_Pk1v1Rank::has_m_tiebox() const {
  return (_has_bits_[0] & 0x00000100u) != 0;
}
inline void E_Pk1v1Rank::set_has_m_tiebox() {
  _has_bits_[0] |= 0x00000100u;
}
inline void E_Pk1v1Rank::clear_has_m_tiebox() {
  _has_bits_[0] &= ~0x00000100u;
}
inline void E_Pk1v1Rank::clear_m_tiebox() {
  m_tiebox_ = 0;
  clear_has_m_tiebox();
}
inline ::google::protobuf::int32 E_Pk1v1Rank::m_tiebox() const {
  return m_tiebox_;
}
inline void E_Pk1v1Rank::set_m_tiebox(::google::protobuf::int32 value) {
  set_has_m_tiebox();
  m_tiebox_ = value;
}

// -------------------------------------------------------------------

// Sheet_Pk1v1Rank

// repeated .proto_ff.E_Pk1v1Rank E_Pk1v1Rank_List = 1;
inline int Sheet_Pk1v1Rank::e_pk1v1rank_list_size() const {
  return e_pk1v1rank_list_.size();
}
inline void Sheet_Pk1v1Rank::clear_e_pk1v1rank_list() {
  e_pk1v1rank_list_.Clear();
}
inline const ::proto_ff::E_Pk1v1Rank& Sheet_Pk1v1Rank::e_pk1v1rank_list(int index) const {
  return e_pk1v1rank_list_.Get(index);
}
inline ::proto_ff::E_Pk1v1Rank* Sheet_Pk1v1Rank::mutable_e_pk1v1rank_list(int index) {
  return e_pk1v1rank_list_.Mutable(index);
}
inline ::proto_ff::E_Pk1v1Rank* Sheet_Pk1v1Rank::add_e_pk1v1rank_list() {
  return e_pk1v1rank_list_.Add();
}
inline const ::google::protobuf::RepeatedPtrField< ::proto_ff::E_Pk1v1Rank >&
Sheet_Pk1v1Rank::e_pk1v1rank_list() const {
  return e_pk1v1rank_list_;
}
inline ::google::protobuf::RepeatedPtrField< ::proto_ff::E_Pk1v1Rank >*
Sheet_Pk1v1Rank::mutable_e_pk1v1rank_list() {
  return &e_pk1v1rank_list_;
}

// -------------------------------------------------------------------

// E_Pk1v1Reward

// optional int32 m_id = 1;
inline bool E_Pk1v1Reward::has_m_id() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void E_Pk1v1Reward::set_has_m_id() {
  _has_bits_[0] |= 0x00000001u;
}
inline void E_Pk1v1Reward::clear_has_m_id() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void E_Pk1v1Reward::clear_m_id() {
  m_id_ = 0;
  clear_has_m_id();
}
inline ::google::protobuf::int32 E_Pk1v1Reward::m_id() const {
  return m_id_;
}
inline void E_Pk1v1Reward::set_m_id(::google::protobuf::int32 value) {
  set_has_m_id();
  m_id_ = value;
}

// optional int32 m_type = 2;
inline bool E_Pk1v1Reward::has_m_type() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void E_Pk1v1Reward::set_has_m_type() {
  _has_bits_[0] |= 0x00000002u;
}
inline void E_Pk1v1Reward::clear_has_m_type() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void E_Pk1v1Reward::clear_m_type() {
  m_type_ = 0;
  clear_has_m_type();
}
inline ::google::protobuf::int32 E_Pk1v1Reward::m_type() const {
  return m_type_;
}
inline void E_Pk1v1Reward::set_m_type(::google::protobuf::int32 value) {
  set_has_m_type();
  m_type_ = value;
}

// optional int32 m_boxid = 3;
inline bool E_Pk1v1Reward::has_m_boxid() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void E_Pk1v1Reward::set_has_m_boxid() {
  _has_bits_[0] |= 0x00000004u;
}
inline void E_Pk1v1Reward::clear_has_m_boxid() {
  _has_bits_[0] &= ~0x00000004u;
}
inline void E_Pk1v1Reward::clear_m_boxid() {
  m_boxid_ = 0;
  clear_has_m_boxid();
}
inline ::google::protobuf::int32 E_Pk1v1Reward::m_boxid() const {
  return m_boxid_;
}
inline void E_Pk1v1Reward::set_m_boxid(::google::protobuf::int32 value) {
  set_has_m_boxid();
  m_boxid_ = value;
}

// repeated int32 m_typearg = 4;
inline int E_Pk1v1Reward::m_typearg_size() const {
  return m_typearg_.size();
}
inline void E_Pk1v1Reward::clear_m_typearg() {
  m_typearg_.Clear();
}
inline ::google::protobuf::int32 E_Pk1v1Reward::m_typearg(int index) const {
  return m_typearg_.Get(index);
}
inline void E_Pk1v1Reward::set_m_typearg(int index, ::google::protobuf::int32 value) {
  m_typearg_.Set(index, value);
}
inline void E_Pk1v1Reward::add_m_typearg(::google::protobuf::int32 value) {
  m_typearg_.Add(value);
}
inline const ::google::protobuf::RepeatedField< ::google::protobuf::int32 >&
E_Pk1v1Reward::m_typearg() const {
  return m_typearg_;
}
inline ::google::protobuf::RepeatedField< ::google::protobuf::int32 >*
E_Pk1v1Reward::mutable_m_typearg() {
  return &m_typearg_;
}

// -------------------------------------------------------------------

// Sheet_Pk1v1Reward

// repeated .proto_ff.E_Pk1v1Reward E_Pk1v1Reward_List = 1;
inline int Sheet_Pk1v1Reward::e_pk1v1reward_list_size() const {
  return e_pk1v1reward_list_.size();
}
inline void Sheet_Pk1v1Reward::clear_e_pk1v1reward_list() {
  e_pk1v1reward_list_.Clear();
}
inline const ::proto_ff::E_Pk1v1Reward& Sheet_Pk1v1Reward::e_pk1v1reward_list(int index) const {
  return e_pk1v1reward_list_.Get(index);
}
inline ::proto_ff::E_Pk1v1Reward* Sheet_Pk1v1Reward::mutable_e_pk1v1reward_list(int index) {
  return e_pk1v1reward_list_.Mutable(index);
}
inline ::proto_ff::E_Pk1v1Reward* Sheet_Pk1v1Reward::add_e_pk1v1reward_list() {
  return e_pk1v1reward_list_.Add();
}
inline const ::google::protobuf::RepeatedPtrField< ::proto_ff::E_Pk1v1Reward >&
Sheet_Pk1v1Reward::e_pk1v1reward_list() const {
  return e_pk1v1reward_list_;
}
inline ::google::protobuf::RepeatedPtrField< ::proto_ff::E_Pk1v1Reward >*
Sheet_Pk1v1Reward::mutable_e_pk1v1reward_list() {
  return &e_pk1v1reward_list_;
}

// -------------------------------------------------------------------

// E_Pk1v1Dailyreward

// optional int32 m_id = 1;
inline bool E_Pk1v1Dailyreward::has_m_id() const {
  return (_has_bits_[0] & 0x00000001u) != 0;
}
inline void E_Pk1v1Dailyreward::set_has_m_id() {
  _has_bits_[0] |= 0x00000001u;
}
inline void E_Pk1v1Dailyreward::clear_has_m_id() {
  _has_bits_[0] &= ~0x00000001u;
}
inline void E_Pk1v1Dailyreward::clear_m_id() {
  m_id_ = 0;
  clear_has_m_id();
}
inline ::google::protobuf::int32 E_Pk1v1Dailyreward::m_id() const {
  return m_id_;
}
inline void E_Pk1v1Dailyreward::set_m_id(::google::protobuf::int32 value) {
  set_has_m_id();
  m_id_ = value;
}

// optional string m_name = 2;
inline bool E_Pk1v1Dailyreward::has_m_name() const {
  return (_has_bits_[0] & 0x00000002u) != 0;
}
inline void E_Pk1v1Dailyreward::set_has_m_name() {
  _has_bits_[0] |= 0x00000002u;
}
inline void E_Pk1v1Dailyreward::clear_has_m_name() {
  _has_bits_[0] &= ~0x00000002u;
}
inline void E_Pk1v1Dailyreward::clear_m_name() {
  if (m_name_ != &::google::protobuf::internal::kEmptyString) {
    m_name_->clear();
  }
  clear_has_m_name();
}
inline const ::std::string& E_Pk1v1Dailyreward::m_name() const {
  return *m_name_;
}
inline void E_Pk1v1Dailyreward::set_m_name(const ::std::string& value) {
  set_has_m_name();
  if (m_name_ == &::google::protobuf::internal::kEmptyString) {
    m_name_ = new ::std::string;
  }
  m_name_->assign(value);
}
inline void E_Pk1v1Dailyreward::set_m_name(const char* value) {
  set_has_m_name();
  if (m_name_ == &::google::protobuf::internal::kEmptyString) {
    m_name_ = new ::std::string;
  }
  m_name_->assign(value);
}
inline void E_Pk1v1Dailyreward::set_m_name(const char* value, size_t size) {
  set_has_m_name();
  if (m_name_ == &::google::protobuf::internal::kEmptyString) {
    m_name_ = new ::std::string;
  }
  m_name_->assign(reinterpret_cast<const char*>(value), size);
}
inline ::std::string* E_Pk1v1Dailyreward::mutable_m_name() {
  set_has_m_name();
  if (m_name_ == &::google::protobuf::internal::kEmptyString) {
    m_name_ = new ::std::string;
  }
  return m_name_;
}
inline ::std::string* E_Pk1v1Dailyreward::release_m_name() {
  clear_has_m_name();
  if (m_name_ == &::google::protobuf::internal::kEmptyString) {
    return NULL;
  } else {
    ::std::string* temp = m_name_;
    m_name_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
    return temp;
  }
}
inline void E_Pk1v1Dailyreward::set_allocated_m_name(::std::string* m_name) {
  if (m_name_ != &::google::protobuf::internal::kEmptyString) {
    delete m_name_;
  }
  if (m_name) {
    set_has_m_name();
    m_name_ = m_name;
  } else {
    clear_has_m_name();
    m_name_ = const_cast< ::std::string*>(&::google::protobuf::internal::kEmptyString);
  }
}

// optional int32 m_times = 3;
inline bool E_Pk1v1Dailyreward::has_m_times() const {
  return (_has_bits_[0] & 0x00000004u) != 0;
}
inline void E_Pk1v1Dailyreward::set_has_m_times() {
  _has_bits_[0] |= 0x00000004u;
}
inline void E_Pk1v1Dailyreward::clear_has_m_times() {
  _has_bits_[0] &= ~0x00000004u;
}
inline void E_Pk1v1Dailyreward::clear_m_times() {
  m_times_ = 0;
  clear_has_m_times();
}
inline ::google::protobuf::int32 E_Pk1v1Dailyreward::m_times() const {
  return m_times_;
}
inline void E_Pk1v1Dailyreward::set_m_times(::google::protobuf::int32 value) {
  set_has_m_times();
  m_times_ = value;
}

// optional int32 m_boxid = 4;
inline bool E_Pk1v1Dailyreward::has_m_boxid() const {
  return (_has_bits_[0] & 0x00000008u) != 0;
}
inline void E_Pk1v1Dailyreward::set_has_m_boxid() {
  _has_bits_[0] |= 0x00000008u;
}
inline void E_Pk1v1Dailyreward::clear_has_m_boxid() {
  _has_bits_[0] &= ~0x00000008u;
}
inline void E_Pk1v1Dailyreward::clear_m_boxid() {
  m_boxid_ = 0;
  clear_has_m_boxid();
}
inline ::google::protobuf::int32 E_Pk1v1Dailyreward::m_boxid() const {
  return m_boxid_;
}
inline void E_Pk1v1Dailyreward::set_m_boxid(::google::protobuf::int32 value) {
  set_has_m_boxid();
  m_boxid_ = value;
}

// -------------------------------------------------------------------

// Sheet_Pk1v1Dailyreward

// repeated .proto_ff.E_Pk1v1Dailyreward E_Pk1v1Dailyreward_List = 1;
inline int Sheet_Pk1v1Dailyreward::e_pk1v1dailyreward_list_size() const {
  return e_pk1v1dailyreward_list_.size();
}
inline void Sheet_Pk1v1Dailyreward::clear_e_pk1v1dailyreward_list() {
  e_pk1v1dailyreward_list_.Clear();
}
inline const ::proto_ff::E_Pk1v1Dailyreward& Sheet_Pk1v1Dailyreward::e_pk1v1dailyreward_list(int index) const {
  return e_pk1v1dailyreward_list_.Get(index);
}
inline ::proto_ff::E_Pk1v1Dailyreward* Sheet_Pk1v1Dailyreward::mutable_e_pk1v1dailyreward_list(int index) {
  return e_pk1v1dailyreward_list_.Mutable(index);
}
inline ::proto_ff::E_Pk1v1Dailyreward* Sheet_Pk1v1Dailyreward::add_e_pk1v1dailyreward_list() {
  return e_pk1v1dailyreward_list_.Add();
}
inline const ::google::protobuf::RepeatedPtrField< ::proto_ff::E_Pk1v1Dailyreward >&
Sheet_Pk1v1Dailyreward::e_pk1v1dailyreward_list() const {
  return e_pk1v1dailyreward_list_;
}
inline ::google::protobuf::RepeatedPtrField< ::proto_ff::E_Pk1v1Dailyreward >*
Sheet_Pk1v1Dailyreward::mutable_e_pk1v1dailyreward_list() {
  return &e_pk1v1dailyreward_list_;
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

#endif  // PROTOBUF_E_5fPk1v1_2eproto__INCLUDED