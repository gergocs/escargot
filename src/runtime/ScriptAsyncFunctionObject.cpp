/*
 * Copyright (c) 2019-present Samsung Electronics Co., Ltd
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
#include "ScriptAsyncFunctionObject.h"
#include "runtime/Context.h"
#include "runtime/FunctionObjectInlines.h"
#include "runtime/PromiseObject.h"

namespace Escargot {

ScriptAsyncFunctionObject::ScriptAsyncFunctionObject(ExecutionState& state, Object* proto, InterpretedCodeBlock* codeBlock, LexicalEnvironment* outerEnvironment, EncodedValue thisValue, Object* homeObject)
    : ScriptFunctionObject(state, proto, codeBlock, outerEnvironment, false, false, true)
    , m_thisValue(thisValue)
    , m_homeObject(homeObject)
{
}

class ScriptAsyncFunctionObjectThisValueBinder {
public:
    Value operator()(ExecutionState& callerState, ExecutionState& calleeState, ScriptAsyncFunctionObject* self, const Value& thisArgument, bool isStrict)
    {
        Value thisValue = self->thisValue();
        if (thisValue.isEmpty()) {
            // OrdinaryCallBindThis ( F, calleeContext, thisArgument )
            // Let thisMode be the value of F’s [[ThisMode]] internal slot.
            // If thisMode is lexical, return NormalCompletion(undefined).
            // --> thisMode is always not lexcial because this is class ctor.
            // Let calleeRealm be the value of F’s [[Realm]] internal slot.
            // Let localEnv be the LexicalEnvironment of calleeContext.
            ASSERT(calleeState.context() == self->codeBlock()->context());

            if (isStrict) {
                // If thisMode is strict, let thisValue be thisArgument.
                return thisArgument;
            } else {
                // Else
                // if thisArgument is null or undefined, then
                // Let thisValue be calleeRealm.[[globalThis]]
                if (thisArgument.isUndefinedOrNull()) {
                    return calleeState.context()->globalObjectProxy();
                } else {
                    // Else
                    // Let thisValue be ToObject(thisArgument).
                    // Assert: thisValue is not an abrupt completion.
                    // NOTE ToObject produces wrapper objects using calleeRealm.
                    return thisArgument.toObject(calleeState);
                }
            }
        } else {
            ASSERT(self->interpretedCodeBlock()->isArrowFunctionExpression());
            return thisValue;
        }
    }
};

Value ScriptAsyncFunctionObject::call(ExecutionState& state, const Value& thisValue, const size_t argc, Value* argv)
{
    return FunctionObjectProcessCallGenerator::processCall<ScriptAsyncFunctionObject, false, false, false, ScriptAsyncFunctionObjectThisValueBinder, FunctionObjectNewTargetBinder, FunctionObjectReturnValueBinder>(state, this, thisValue, argc, argv, nullptr);
}

Value ScriptAsyncFunctionObject::construct(ExecutionState& state, const size_t argc, Value* argv, Object* newTarget)
{
    ExecutionState newState(m_codeBlock->context(), &state,
                            static_cast<LexicalEnvironment*>(nullptr), argc, argv, m_codeBlock->asInterpretedCodeBlock()->isStrict());
    ErrorObject::throwBuiltinError(newState, ErrorObject::TypeError, "Async function cannot be invoked with 'new'");
    ASSERT_NOT_REACHED();
    return Value();
}

class ScriptAsyncFunctionHelperFunctionObject : public NativeFunctionObject {
public:
    ScriptAsyncFunctionHelperFunctionObject(ExecutionState& state, NativeFunctionInfo info, ExecutionPauser* executionPauser, Object* source)
        : NativeFunctionObject(state, info)
        , m_executionPauser(executionPauser)
        , m_source(source)
    {
    }

    ExecutionPauser* m_executionPauser;
    Object* m_source;
};

// http://www.ecma-international.org/ecma-262/10.0/#await-fulfilled
static void awaitFulfilledFunctions(ExecutionState& state, ScriptAsyncFunctionHelperFunctionObject* F, const Value& value)
{
    // Let F be the active function object.
    // Let asyncContext be F.[[AsyncContext]].
    // Let prevContext be the running execution context.
    // Suspend prevContext.
    // Push asyncContext onto the execution context stack; asyncContext is now the running execution context.
    // Resume the suspended evaluation of asyncContext using NormalCompletion(value) as the result of the operation that suspended it.
    ExecutionPauser::start(state, F->m_executionPauser, F->m_source, value, false, false, F->m_source->isAsyncGeneratorObject() ? ExecutionPauser::StartFrom::AsyncGenerator : ExecutionPauser::StartFrom::Async);
    // Assert: When we reach this step, asyncContext has already been removed from the execution context stack and prevContext is the currently running execution context.
    // Return undefined.
}

static Value awaitFulfilledFunction(ExecutionState& state, Value thisValue, size_t argc, Value* argv, Optional<Object*> newTarget)
{
    ScriptAsyncFunctionHelperFunctionObject* self = (ScriptAsyncFunctionHelperFunctionObject*)state.resolveCallee();
    awaitFulfilledFunctions(state, self, argv[0]);
    return Value();
}

// http://www.ecma-international.org/ecma-262/10.0/#await-rejected
static void awaitRejectedFunctions(ExecutionState& state, ScriptAsyncFunctionHelperFunctionObject* F, const Value& reason)
{
    // Let F be the active function object.
    // Let asyncContext be F.[[AsyncContext]].
    // Let prevContext be the running execution context.
    // Suspend prevContext.
    // Push asyncContext onto the execution context stack; asyncContext is now the running execution context.
    // Resume the suspended evaluation of asyncContext using ThrowCompletion(reason) as the result of the operation that suspended it.
    ExecutionPauser::start(state, F->m_executionPauser, F->m_source, reason, false, true, F->m_source->isAsyncGeneratorObject() ? ExecutionPauser::StartFrom::AsyncGenerator : ExecutionPauser::StartFrom::Async);
    // Assert: When we reach this step, asyncContext has already been removed from the execution context stack and prevContext is the currently running execution context.
    // Return undefined.
}

static Value awaitRejectedFunction(ExecutionState& state, Value thisValue, size_t argc, Value* argv, Optional<Object*> newTarget)
{
    ScriptAsyncFunctionHelperFunctionObject* self = (ScriptAsyncFunctionHelperFunctionObject*)state.resolveCallee();
    awaitRejectedFunctions(state, self, argv[0]);
    return Value();
}

// http://www.ecma-international.org/ecma-262/10.0/#await
PromiseObject* ScriptAsyncFunctionObject::awaitOperationBeforePause(ExecutionState& state, ExecutionPauser* executionPauser, const Value& awaitValue, Object* source)
{
    // Let asyncContext be the running execution context.
    // Let promise be ? PromiseResolve(%Promise%, « value »).
    PromiseObject* promise = PromiseObject::promiseResolve(state, state.context()->globalObject()->promise(), awaitValue)->asPromiseObject();
    // Let stepsFulfilled be the algorithm steps defined in Await Fulfilled Functions.
    // Let onFulfilled be CreateBuiltinFunction(stepsFulfilled, « [[AsyncContext]] »).
    // Set onFulfilled.[[AsyncContext]] to asyncContext.
    FunctionObject* onFulfilled = new ScriptAsyncFunctionHelperFunctionObject(state, NativeFunctionInfo(AtomicString(), awaitFulfilledFunction, 1), executionPauser, source);

    // Let stepsRejected be the algorithm steps defined in Await Rejected Functions.
    // Let onRejected be CreateBuiltinFunction(stepsRejected, « [[AsyncContext]] »).
    // Set onRejected.[[AsyncContext]] to asyncContext.
    FunctionObject* onRejected = new ScriptAsyncFunctionHelperFunctionObject(state, NativeFunctionInfo(AtomicString(), awaitRejectedFunction, 1), executionPauser, source);

    // Perform ! PerformPromiseThen(promise, onFulfilled, onRejected).
    promise->then(state, onFulfilled, onRejected, Optional<PromiseReaction::Capability>());

    return promise;
}
} // namespace Escargot
