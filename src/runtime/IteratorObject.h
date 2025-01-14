/*
 * Copyright (c) 2018-present Samsung Electronics Co., Ltd
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
 *  USA
 */

#ifndef __EscargotIteratorObject__
#define __EscargotIteratorObject__

#include "runtime/Object.h"

namespace Escargot {

class ArrayIteratorObject;
class StringIteratorObject;
class RegExpStringIteratorObject;
class MapIteratorObject;
class SetIteratorObject;
class IteratorObject;

class IteratorRecord : public PointerValue {
public:
    Object* m_iterator;
    EncodedValue m_nextMethod;
    bool m_done;

    virtual bool isIteratorRecord() const override
    {
        return true;
    }

    IteratorRecord(Object* iterator, EncodedValue nextMethod, bool done)
        : m_iterator(iterator)
        , m_nextMethod(nextMethod)
        , m_done(done)
    {
    }
};

class IteratorObject : public Object {
public:
    explicit IteratorObject(ExecutionState& state);
    explicit IteratorObject(ExecutionState& state, Object* proto);

    virtual bool isIteratorObject() const
    {
        return true;
    }

    ArrayIteratorObject* asArrayIteratorObject()
    {
        ASSERT(isArrayIteratorObject());
        return (ArrayIteratorObject*)this;
    }

    StringIteratorObject* asStringIteratorObject()
    {
        ASSERT(isStringIteratorObject());
        return (StringIteratorObject*)this;
    }

    RegExpStringIteratorObject* asRegExpStringIteratorObject()
    {
        ASSERT(isRegExpStringIteratorObject());
        return (RegExpStringIteratorObject*)this;
    }

    MapIteratorObject* asMapIteratorObject()
    {
        ASSERT(isMapIteratorObject());
        return (MapIteratorObject*)this;
    }

    SetIteratorObject* asSetIteratorObject()
    {
        ASSERT(isSetIteratorObject());
        return (SetIteratorObject*)this;
    }

    Value next(ExecutionState& state);

    virtual std::pair<Value, bool> advance(ExecutionState& state) = 0;

    static IteratorRecord* getIterator(ExecutionState& state, const Value& obj, const bool sync = true, const Value& func = Value(Value::EmptyValue));
    static Object* iteratorNext(ExecutionState& state, IteratorRecord* iteratorRecord, const Value& value = Value(Value::EmptyValue));
    static bool iteratorComplete(ExecutionState& state, Object* iterResult);
    static Value iteratorValue(ExecutionState& state, Object* iterResult);
    // this function return empty Optional value instead of Value(false)
    static Optional<Object*> iteratorStep(ExecutionState& state, IteratorRecord* iteratorRecord);
    static Value iteratorClose(ExecutionState& state, IteratorRecord* iteratorRecord, const Value& completionValue, bool hasThrowOnCompletionType);
    static Object* createIterResultObject(ExecutionState& state, const Value& value, bool done);
    // https://www.ecma-international.org/ecma-262/10.0/#sec-iterabletolist
    static ValueVectorWithInlineStorage iterableToList(ExecutionState& state, const Value& items, Optional<Value> method = Optional<Value>());
};
} // namespace Escargot

#endif
