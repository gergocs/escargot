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

#include "Escargot.h"
#include "runtime/GlobalObject.h"
#include "runtime/Context.h"
#include "runtime/VMInstance.h"
#include "runtime/SymbolObject.h"
#include "runtime/NativeFunctionObject.h"
#include "runtime/ToStringRecursionPreventer.h"

namespace Escargot {

static Value builtinSymbolConstructor(ExecutionState& state, Value thisValue, size_t argc, Value* argv, Optional<Object*> newTarget)
{
    // If NewTarget is not undefined, throw a TypeError exception.
    if (newTarget.hasValue()) {
        ErrorObject::throwBuiltinError(state, ErrorObject::TypeError, "illegal constructor Symbol");
    }
    Optional<String*> descString = nullptr;
    // If description is undefined, let descString be undefined.
    if (!(argc == 0 || argv[0].isUndefined())) {
        // Else, let descString be ? ToString(description).
        descString = argv[0].toString(state);
    }
    // Return a new unique Symbol value whose [[Description]] value is descString.
    return new Symbol(descString);
}

#define RESOLVE_THIS_BINDING_TO_SYMBOL(NAME, OBJ, BUILT_IN_METHOD)                                                                                                                                                                                           \
    Symbol* NAME = nullptr;                                                                                                                                                                                                                                  \
    if (thisValue.isObject()) {                                                                                                                                                                                                                              \
        if (!thisValue.asObject()->isSymbolObject()) {                                                                                                                                                                                                       \
            ErrorObject::throwBuiltinError(state, ErrorObject::TypeError, state.context()->staticStrings().OBJ.string(), true, state.context()->staticStrings().BUILT_IN_METHOD.string(), ErrorObject::Messages::GlobalObject_CalledOnIncompatibleReceiver); \
        }                                                                                                                                                                                                                                                    \
        NAME = thisValue.asObject()->asSymbolObject()->primitiveValue();                                                                                                                                                                                     \
    } else if (thisValue.isSymbol()) {                                                                                                                                                                                                                       \
        NAME = thisValue.asSymbol();                                                                                                                                                                                                                         \
    } else {                                                                                                                                                                                                                                                 \
        ErrorObject::throwBuiltinError(state, ErrorObject::TypeError, state.context()->staticStrings().OBJ.string(), true, state.context()->staticStrings().BUILT_IN_METHOD.string(), ErrorObject::Messages::GlobalObject_CalledOnIncompatibleReceiver);     \
    }

static Value builtinSymbolToString(ExecutionState& state, Value thisValue, size_t argc, Value* argv, Optional<Object*> newTarget)
{
    RESOLVE_THIS_BINDING_TO_SYMBOL(S, Symbol, toString);
    return S->symbolDescriptiveString();
}

static Value builtinSymbolValueOf(ExecutionState& state, Value thisValue, size_t argc, Value* argv, Optional<Object*> newTarget)
{
    RESOLVE_THIS_BINDING_TO_SYMBOL(S, Symbol, valueOf);
    return Value(S);
}

static Value builtinSymbolToPrimitive(ExecutionState& state, Value thisValue, size_t argc, Value* argv, Optional<Object*> newTarget)
{
    RESOLVE_THIS_BINDING_TO_SYMBOL(S, Symbol, toPrimitive);
    return Value(S);
}

static Value builtinSymbolFor(ExecutionState& state, Value thisValue, size_t argc, Value* argv, Optional<Object*> newTarget)
{
    // Let stringKey be ? ToString(key).
    String* stringKey = argv[0].toString(state);
    return Symbol::fromGlobalSymbolRegistry(state.context()->vmInstance(), stringKey);
}

static Value builtinSymbolKeyFor(ExecutionState& state, Value thisValue, size_t argc, Value* argv, Optional<Object*> newTarget)
{
    // If Type(sym) is not Symbol, throw a TypeError exception.
    if (!argv[0].isSymbol()) {
        ErrorObject::throwBuiltinError(state, ErrorObject::TypeError, ErrorObject::Messages::GlobalObject_IllegalFirstArgument);
    }
    Symbol* sym = argv[0].asSymbol();
    // For each element e of the GlobalSymbolRegistry List (see 19.4.2.1),
    auto& list = state.context()->vmInstance()->globalSymbolRegistry();
    for (size_t i = 0; i < list.size(); i++) {
        // If SameValue(e.[[Symbol]], sym) is true, return e.[[Key]].
        if (list[i].symbol == sym) {
            return list[i].key.value();
        }
    }
    // Assert: GlobalSymbolRegistry does not currently contain an entry for sym.
    // Return undefined.
    return Value();
}

static Value builtinSymbolDescriptionGetter(ExecutionState& state, Value thisValue, size_t argc, Value* argv, Optional<Object*> newTarget)
{
    if (thisValue.isSymbol()) {
        if (!thisValue.asSymbol()->description().hasValue()) {
            return Value();
        }

        return thisValue.asSymbol()->description().value();

    } else if (thisValue.isObject() && thisValue.asObject()->isSymbolObject()) {
        if (!thisValue.asObject()->asSymbolObject()->primitiveValue()->description().hasValue()) {
            return Value();
        }

        return thisValue.asObject()->asSymbolObject()->primitiveValue()->description().value();

    } else {
        ErrorObject::throwBuiltinError(state, ErrorObject::Code::TypeError, "getter called on non-Symbol object");
    }
    return Value();
}

void GlobalObject::initializeSymbol(ExecutionState& state)
{
    ObjectPropertyNativeGetterSetterData* nativeData = new ObjectPropertyNativeGetterSetterData(true, false, true,
                                                                                                [](ExecutionState& state, Object* self, const Value& receiver, const EncodedValue& privateDataFromObjectPrivateArea) -> Value {
                                                                                                    ASSERT(self->isGlobalObject());
                                                                                                    return self->asGlobalObject()->symbol();
                                                                                                },
                                                                                                nullptr);

    defineNativeDataAccessorProperty(state, ObjectPropertyName(state.context()->staticStrings().Symbol), nativeData, Value(Value::EmptyValue));
}

void GlobalObject::installSymbol(ExecutionState& state)
{
    m_symbol = new NativeFunctionObject(state, NativeFunctionInfo(state.context()->staticStrings().Symbol, builtinSymbolConstructor, 0), NativeFunctionObject::__ForBuiltinConstructor__);
    m_symbol->setGlobalIntrinsicObject(state);

    m_symbol->defineOwnProperty(state, ObjectPropertyName(state.context()->staticStrings().stringFor),
                                ObjectPropertyDescriptor(new NativeFunctionObject(state, NativeFunctionInfo(state.context()->staticStrings().stringFor, builtinSymbolFor, 1, NativeFunctionInfo::Strict)),
                                                         (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));

    m_symbol->defineOwnProperty(state, ObjectPropertyName(state.context()->staticStrings().keyFor),
                                ObjectPropertyDescriptor(new NativeFunctionObject(state, NativeFunctionInfo(state.context()->staticStrings().keyFor, builtinSymbolKeyFor, 1, NativeFunctionInfo::Strict)),
                                                         (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));

    m_symbolPrototype = new Object(state);
    m_symbolPrototype->setGlobalIntrinsicObject(state, true);

    m_symbolPrototype->defineOwnProperty(state, ObjectPropertyName(state.context()->staticStrings().constructor), ObjectPropertyDescriptor(m_symbol, (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));

    m_symbolPrototype->defineOwnProperty(state, ObjectPropertyName(state.context()->staticStrings().toString),
                                         ObjectPropertyDescriptor(new NativeFunctionObject(state, NativeFunctionInfo(state.context()->staticStrings().toString, builtinSymbolToString, 0, NativeFunctionInfo::Strict)),
                                                                  (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));

    m_symbolPrototype->defineOwnProperty(state, ObjectPropertyName(state.context()->staticStrings().valueOf),
                                         ObjectPropertyDescriptor(new NativeFunctionObject(state, NativeFunctionInfo(state.context()->staticStrings().valueOf, builtinSymbolValueOf, 0, NativeFunctionInfo::Strict)),
                                                                  (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));

    m_symbolPrototype->defineOwnPropertyThrowsException(state, ObjectPropertyName(state, Value(state.context()->vmInstance()->globalSymbols().toPrimitive)),
                                                        ObjectPropertyDescriptor(new NativeFunctionObject(state, NativeFunctionInfo(AtomicString(state, String::fromASCII("[Symbol.toPrimitive]")), builtinSymbolToPrimitive, 1, NativeFunctionInfo::Strict)), (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::ConfigurablePresent)));

    m_symbolPrototype->defineOwnPropertyThrowsException(state, ObjectPropertyName(state, Value(state.context()->vmInstance()->globalSymbols().toStringTag)),
                                                        ObjectPropertyDescriptor(state.context()->staticStrings().Symbol.string(), (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::ConfigurablePresent)));

    {
        Value getter = new NativeFunctionObject(state, NativeFunctionInfo(state.context()->staticStrings().getDescription, builtinSymbolDescriptionGetter, 0, NativeFunctionInfo::Strict));
        JSGetterSetter gs(getter, Value());
        ObjectPropertyDescriptor desc(gs, ObjectPropertyDescriptor::ConfigurablePresent);
        m_symbolPrototype->defineOwnProperty(state, ObjectPropertyName(state, state.context()->staticStrings().description), desc);
    }


#define DECLARE_GLOBAL_SYMBOLS(name) \
    m_symbol->defineOwnProperty(state, ObjectPropertyName(state.context()->staticStrings().name), ObjectPropertyDescriptor(Value(state.context()->vmInstance()->globalSymbols().name), (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::NonEnumerablePresent | ObjectPropertyDescriptor::NonWritablePresent | ObjectPropertyDescriptor::NonConfigurablePresent)));
    DEFINE_GLOBAL_SYMBOLS(DECLARE_GLOBAL_SYMBOLS);

    m_symbol->setFunctionPrototype(state, m_symbolPrototype);

    m_symbolProxyObject = new SymbolObject(state, state.context()->vmInstance()->globalSymbols().iterator);

    redefineOwnProperty(state, ObjectPropertyName(state.context()->staticStrings().Symbol),
                        ObjectPropertyDescriptor(m_symbol, (ObjectPropertyDescriptor::PresentAttribute)(ObjectPropertyDescriptor::WritablePresent | ObjectPropertyDescriptor::ConfigurablePresent)));
}
} // namespace Escargot
