//
// Created by vasniktel on 08.09.2019.
//

#ifndef HELIUM_COMPILER_SRC_TYPE_HPP_
#define HELIUM_COMPILER_SRC_TYPE_HPP_

#include "absl/memory/memory.h"
#include "interner.hpp"

namespace helium {

class SingleType;
class ErrorType;

class TypeVisitor {
 public:
  virtual ~TypeVisitor() = default;
  virtual void Visit(SingleType&) = 0;
  virtual void Visit(ErrorType&) = 0;
};

enum class TypeKind {
  kSingle,
  kError
};

// TODO: make representation more efficient
class Type {
 public:
  virtual TypeKind GetKind() const = 0;
  virtual ~Type() = default;
  virtual bool Match(const Type* other) const = 0;

  virtual bool Match(const ::std::unique_ptr<Type>& other) const final {
    return Match(other.get());
  }

  virtual ::std::unique_ptr<Type> Copy() const = 0;
  virtual void Accept(TypeVisitor& visitor) = 0;
};

template <typename T>
bool Is(const Type* type);

template <typename T>
T* Cast(Type* type);

template <typename T>
const T* Cast(const Type* type);

class SingleType final : public Type {
  friend class Type;

  Interner::Data type_data_;

 public:
  SingleType() = delete;
  explicit SingleType(Interner::Data type_data)
  : type_data_(type_data)
  {}

  Interner::Data GetTypeData() const { return type_data_; }

  TypeKind GetKind() const override { return TypeKind::kSingle; }

  static bool ClassOf(const Type* type) {
    return type->GetKind() == TypeKind::kSingle;
  }

  bool Match(const Type* other) const override {
    if (const auto* type = Cast<SingleType>(other)) {
      return type->GetTypeData() == this->GetTypeData();
    }

    return false;
  }

  ::std::unique_ptr<Type> Copy() const override {
    return ::absl::make_unique<SingleType>(type_data_);
  }

  void Accept(TypeVisitor& visitor) override {
    visitor.Visit(*this);
  }
};

class ErrorType final : public Type {
 public:
  static bool ClassOf(const Type* type) {
    return type->GetKind() == TypeKind::kError;
  }

  TypeKind GetKind() const override {
    return TypeKind::kError;
  }

  bool Match(const Type* other) const override {
    return Is<ErrorType>(other);
  }

  ::std::unique_ptr<Type> Copy() const override {
    return ::absl::make_unique<ErrorType>();
  }

  void Accept(TypeVisitor& visitor) override {
    visitor.Visit(*this);
  }
};

template <typename T>
inline bool Is(const Type* type) {
  return type ? T::ClassOf(type) : false;
}

template <typename T>
inline bool Is(const std::unique_ptr<Type>& type) {
  return Is<T>(type.get());
}

template <typename T>
inline T* Cast(Type* type) {
  return Is<T>(type) ? static_cast<T*>(type) : nullptr;
}

template <typename T>
inline const T* Cast(const Type* type) {
  return Is<T>(type) ? static_cast<const T*>(type) : nullptr;
}

}

#endif //HELIUM_COMPILER_SRC_TYPE_HPP_
