/*
 * Copyright (c) 1997, 2013, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#ifndef SHARE_VM_OOPS_OOPSHIERARCHY_HPP
#define SHARE_VM_OOPS_OOPSHIERARCHY_HPP

#include "runtime/globals.hpp"
#include "utilities/globalDefinitions.hpp"

// OBJECT hierarchy. 对象层次结构
// This hierarchy is a representation hierarchy, i.e. if A is a superclass
// of B, A's representation is a prefix of B's representation.
// 层次结构，是一个表示形式的层次结构？例如，如果A是B的超类，那么 A的表示形式 是 B的表示形式 的前缀？

typedef juint narrowOop; // Offset instead of address for an oop within a java object. java对象中oop的偏移量而不是地址

// If compressed klass pointers then use narrowKlass. 如果压缩klass指针，则使用narrowKlass
typedef juint  narrowKlass;

typedef void* OopOrNarrowOopStar;
typedef class   markOopDesc*                markOop;

#ifndef CHECK_UNHANDLED_OOPS

typedef class oopDesc*                            oop;
typedef class   instanceOopDesc*            instanceOop;
typedef class   arrayOopDesc*                    arrayOop;
typedef class     objArrayOopDesc*            objArrayOop;
typedef class     typeArrayOopDesc*            typeArrayOop;

#else

// When CHECK_UNHANDLED_OOPS is defined, an "oop" is a class with a
// carefully chosen set of constructors and conversion operators to go
// to and from the underlying oopDesc pointer type.
//
// 当定义了CHECK_UNHANDLED_OOPS时，“oop”是一个带有一组精心选择的构造函数和转换操作符的类，用于与底层oopDesc指针类型进行转换。
//
// Because oop and its subclasses <type>Oop are class types, arbitrary
// conversions are not accepted by the compiler.  Applying a cast to
// an oop will cause the best matched conversion operator to be
// invoked returning the underlying oopDesc* type if appropriate.
// No copy constructors, explicit user conversions or operators of
// numerical type should be defined within the oop class. Most C++
// compilers will issue a compile time error concerning the overloading
// ambiguity between operators of numerical and pointer types. If
// a conversion to or from an oop to a numerical type is needed,
// use the inline template methods, cast_*_oop, defined below.
// 
// 因为oop及其子类 <type>Oop 是类类型，所以编译器不接受任意转换。
// 对oop进行类型转换，将导致调用最匹配的转换操作符，如果合适的话返回底层的oopDesc*类型。
// 不应该在oop类中定义：复制构造函数、显式用户转换或数值类型的运算符。
// 对于大多数的C++编译器，如果遇到数字和指针类型的运算符之间出现重载歧义，那么将会产生编译时错误。
// 如果需要进行oop和数值类型之间的互相转换，请使用下面定义的内联模板方法 cast_*_oop。
//
// Converting NULL to oop to Handle implicit is no longer accepted by the
// compiler because there are too many steps in the conversion.  Use Handle()
// instead, which generates less code anyway.
//
// 编译器不再接受将NULL转换为oop来进行隐式处理，因为在转换中需要太多步骤。
// 使用Handle()方法替代，这样可以生成更少的代码。

class Thread;
class PromotedObject;


class oop {
  oopDesc* _o;

  void register_oop();
  void unregister_oop();

  // friend class markOop;
public:
  void set_obj(const void* p)         {
    raw_set_obj(p);
    if (CheckUnhandledOops) register_oop();
  }
  void raw_set_obj(const void* p)     { _o = (oopDesc*)p; }

  oop()                               { set_obj(NULL); }
  oop(const oop& o)                   { set_obj(o.obj()); }
  oop(const volatile oop& o)          { set_obj(o.obj()); }
  oop(const void* p)                  { set_obj(p); }
  ~oop()                              {
    if (CheckUnhandledOops) unregister_oop();
  }

  oopDesc* obj()  const volatile      { return _o; }

  // General access
  oopDesc*  operator->() const        { return obj(); }
  bool operator==(const oop o) const  { return obj() == o.obj(); }
  bool operator==(void *p) const      { return obj() == p; }
  bool operator!=(const volatile oop o) const  { return obj() != o.obj(); }
  bool operator!=(void *p) const      { return obj() != p; }

  bool operator<(oop o) const         { return obj() < o.obj(); }
  bool operator>(oop o) const         { return obj() > o.obj(); }
  bool operator<=(oop o) const        { return obj() <= o.obj(); }
  bool operator>=(oop o) const        { return obj() >= o.obj(); }
  bool operator!() const              { return !obj(); }

  // Assignment
  oop& operator=(const oop& o)                            { _o = o.obj(); return *this; }
#ifndef SOLARIS
  volatile oop& operator=(const oop& o) volatile          { _o = o.obj(); return *this; }
#endif
  volatile oop& operator=(const volatile oop& o) volatile { _o = o.obj(); return *this; }

  // Explict user conversions
  operator void* () const             { return (void *)obj(); }
#ifndef SOLARIS
  operator void* () const volatile    { return (void *)obj(); }
#endif
  operator HeapWord* () const         { return (HeapWord*)obj(); }
  operator oopDesc* () const          { return obj(); }
  operator intptr_t* () const         { return (intptr_t*)obj(); }
  operator PromotedObject* () const   { return (PromotedObject*)obj(); }
  operator markOop () const           { return markOop(obj()); }

  operator address   () const         { return (address)obj(); }

  // from javaCalls.cpp
  operator jobject () const           { return (jobject)obj(); }
  // from javaClasses.cpp
  operator JavaThread* () const       { return (JavaThread*)obj(); }

#ifndef _LP64
  // from jvm.cpp
  operator jlong* () const            { return (jlong*)obj(); }
#endif

  // from parNewGeneration and other things that want to get to the end of
  // an oop for stuff (like ObjArrayKlass.cpp)
  operator oop* () const              { return (oop *)obj(); }
};

#define DEF_OOP(type)                                                      \
   class type##OopDesc;                                                    \
   class type##Oop : public oop {                                          \
     public:                                                               \
       type##Oop() : oop() {}                                              \
       type##Oop(const oop& o) : oop(o) {}                                 \
       type##Oop(const volatile oop& o) : oop(o) {}                        \
       type##Oop(const void* p) : oop(p) {}                                \
       operator type##OopDesc* () const { return (type##OopDesc*)obj(); }  \
       type##OopDesc* operator->() const {                                 \
            return (type##OopDesc*)obj();                                  \
       }                                                                   \
       type##Oop& operator=(const type##Oop& o) {                          \
            oop::operator=(o);                                             \
            return *this;                                                  \
       }                                                                   \
       NOT_SOLARIS(                                                        \
       volatile type##Oop& operator=(const type##Oop& o) volatile {        \
            (void)const_cast<oop&>(oop::operator=(o));                     \
            return *this;                                                  \
       })                                                                  \
       volatile type##Oop& operator=(const volatile type##Oop& o) volatile {\
            (void)const_cast<oop&>(oop::operator=(o));                     \
            return *this;                                                  \
       }                                                                   \
   };

DEF_OOP(instance);
DEF_OOP(array);
DEF_OOP(objArray);
DEF_OOP(typeArray);

#endif // CHECK_UNHANDLED_OOPS

// For CHECK_UNHANDLED_OOPS, it is ambiguous C++ behavior to have the oop
// structure contain explicit user defined conversions of both numerical
// and pointer type. Define inline methods to provide the numerical conversions.
template <class T> inline oop cast_to_oop(T value) {
  return (oop)(CHECK_UNHANDLED_OOPS_ONLY((void *))(value));
}
template <class T> inline T cast_from_oop(oop o) {
  return (T)(CHECK_UNHANDLED_OOPS_ONLY((void*))o);
}

// The metadata hierarchy is separate from the oop hierarchy

//      class MetaspaceObj
class   ConstMethod;
class   ConstantPoolCache;
class   MethodData;
//      class Metadata
class   Method;
class   ConstantPool;
//      class CHeapObj
class   CompiledICHolder;


// The klass hierarchy is separate from the oop hierarchy.

class Klass;
class   InstanceKlass;
class     InstanceMirrorKlass;
class     InstanceClassLoaderKlass;
class     InstanceRefKlass;
class   ArrayKlass;
class     ObjArrayKlass;
class     TypeArrayKlass;

#endif // SHARE_VM_OOPS_OOPSHIERARCHY_HPP
