/*
 * Copyright (c) 2002, 2012, Oracle and/or its affiliates. All rights reserved.
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

#ifndef SHARE_VM_GC_IMPLEMENTATION_SHARED_COLLECTORCOUNTERS_HPP
#define SHARE_VM_GC_IMPLEMENTATION_SHARED_COLLECTORCOUNTERS_HPP

#include "runtime/perfData.hpp"

// CollectorCounters is a holder class for performance counters
// that track a collector

class CollectorCounters: public CHeapObj<mtGC> {
  friend class VMStructs;

  private:
    PerfCounter*      _invocations;
    PerfCounter*      _time;
    PerfVariable*     _last_entry_time;
    PerfVariable*     _last_exit_time;

    // Constant PerfData types don't need to retain a reference.
    // However, it's a good idea to document them here.
    // PerfStringConstant*     _name;

    char*             _name_space;

  public:

    CollectorCounters(const char* name, int ordinal);

    ~CollectorCounters() {
      if (_name_space != NULL) FREE_C_HEAP_ARRAY(char, _name_space, mtGC);
    }

    inline PerfCounter* invocation_counter() const  { return _invocations; }

    inline PerfCounter* time_counter() const        { return _time; }

    inline PerfVariable* last_entry_counter() const { return _last_entry_time; }

    inline PerfVariable* last_exit_counter() const  { return _last_exit_time; }

    const char* name_space() const                  { return _name_space; }
};

// 用于辅助记录GC的次数
class TraceCollectorStats: public PerfTraceTimedEvent {

  protected:
    CollectorCounters* _c;

  public:
    inline TraceCollectorStats(CollectorCounters* c) :
           PerfTraceTimedEvent(c->time_counter(), c->invocation_counter()), // invocation_counter()计数器自增1
           _c(c) {

      if (UsePerfData) {
         _c->last_entry_counter()->set_value(os::elapsed_counter());
      }
    }

    inline ~TraceCollectorStats() {
      if (UsePerfData) _c->last_exit_counter()->set_value(os::elapsed_counter());
    }
};

#endif // SHARE_VM_GC_IMPLEMENTATION_SHARED_COLLECTORCOUNTERS_HPP
