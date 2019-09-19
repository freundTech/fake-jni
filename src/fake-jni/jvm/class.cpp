#include "jni.h"

#include "fake-jni/jvm.h"

#include <vector>

#define _ERROR_ARBITRARY_CLASS \
throw std::runtime_error("FATAL: Cannot construct an arbitrary class with no native backing!");

namespace FakeJni {
 JClass::JClass(const char *name, uint32_t modifiers) noexcept :
  JObject(),
  constructV([](JavaVM * const, const char * const, va_list) -> JObject * {
   _ERROR_ARBITRARY_CLASS
  }),
  constructA([](JavaVM * const, const char * const, const jvalue *) -> JObject * {
   _ERROR_ARBITRARY_CLASS
  }),
  isArbitrary(true),
  className(name),
  modifiers(modifiers),
  parent(JObject::descriptor),
  functions{true},
  fields{true}
 {}

 bool JClass::registerMethod(JMethodID * const mid) const {
  if (isArbitrary) {
   if (mid->type == JMethodID::MEMBER_FUNC) {
    throw std::runtime_error(
     "FATAL: You cannot register a member function on an arbitrary class that cannot be instantiated!"
    );
   }
  }
  auto& functions_ref = const_cast<AllocStack<JMethodID *>&>(functions);
  const auto size = functions_ref.getSize();
  for (uint32_t i = 0; i < size; i++) {
   const auto reg_mid = functions_ref[i];
   if (*reg_mid == *mid) {
    if (strcmp(reg_mid->getName(), mid->getName()) == 0) {
     if (strcmp(reg_mid->getSignature(), mid->getSignature()) == 0) {
      return false;
     }
    }
   }
  }
  functions_ref.pushAlloc([](void *mid) { delete (JMethodID*)mid; }, mid);
  return true;
 }

 bool JClass::unregisterMethod(FakeJni::JMethodID * const mid) const noexcept {
  auto& functions_ref = const_cast<AllocStack<JMethodID *>&>(functions);
  const auto size = functions_ref.getSize();
  for (unsigned int i = 0; i < size; i++) {
   if (functions_ref[i] == mid) {
    functions_ref.removeAlloc(mid);
    return true;
   }
  }
  return false;
 }

 JMethodID * JClass::getMethod(const char * sig, const char * name) const noexcept {
  auto& functions_ref = const_cast<AllocStack<JMethodID *>&>(functions);
  const auto size = functions_ref.getSize();
  for (unsigned int i = 0; i < size; i++) {
   const auto& func = functions_ref[i];
   if (strcmp(name, func->getName()) == 0) {
    if (strcmp(sig, func->getSignature()) == 0) {
     return func;
    }
   }
  }
  return nullptr;
 }

 const AllocStack<JMethodID *>& JClass::getMethods() const noexcept {
  return functions;
 }

 bool JClass::registerField(JFieldID * const fid) const noexcept {
  auto& fields_ref = const_cast<AllocStack<JFieldID *>&>(fields);
  const auto size = fields_ref.getSize();
  for (uint32_t i = 0; i < size; i++) {
   const auto reg_fid = fields_ref[i];
   if (*reg_fid == *fid) {
    if (strcmp(reg_fid->getName(), fid->getName()) == 0) {
     if (strcmp(reg_fid->getSignature(), fid->getSignature()) == 0) {
      return false;
     }
    }
   }
  }
  fields_ref.pushAlloc([](void *fid) { delete (JFieldID*)fid; }, fid);
  return true;
 }

 bool JClass::unregisterField(FakeJni::JFieldID * const fid) const noexcept {
  auto& fields_ref = const_cast<AllocStack<JFieldID *>&>(fields);
  const auto size = fields_ref.getSize();
  for (unsigned int i = 0; i < size; i++) {
   if (fields_ref[i] == fid) {
    fields_ref.removeAlloc(fid);
    return true;
   }
  }
  return false;
 }

 JFieldID * JClass::getField(const char * name) const noexcept {
  auto& fields_ref = const_cast<AllocStack<JFieldID *>&>(fields);
  const auto size = fields_ref.getSize();
  for (unsigned int i = 0; i < size; i++) {
   const auto& field = fields_ref[i];
   if (strcmp(name, field->getName()) == 0) {
    return field;
   }
  }
  return nullptr;
 }

 JFieldID* JClass::getField(const char * sig, const char * name) const noexcept {
  auto& fields_ref = const_cast<AllocStack<JFieldID *>&>(fields);
  const auto size = fields_ref.getSize();
  for (unsigned int i = 0; i < size; i++) {
   const auto& field = fields_ref[i];
   if (strcmp(sig, field->getSignature()) == 0) {
    if (strcmp(name, field->getName()) == 0) {
     return field;
    }
   }
  }
  return nullptr;
 }

 const AllocStack<JFieldID *>& JClass::getFields() const noexcept {
  return fields;
 }

 const char * JClass::getName() const noexcept {
  return className;
 }

 JObject * JClass::newInstance(JavaVM * const vm, const char * const signature, va_list list) const {
  return constructV(vm, signature, list);
 }

 JObject * JClass::newInstance(JavaVM * const vm, const char * const signature, const jvalue * const values) const {
  return constructA(vm, signature, values);
 }
}

//Allocate JClass descriptor for JClass
DEFINE_NATIVE_DESCRIPTOR(FakeJni::JClass)END_NATIVE_DESCRIPTOR