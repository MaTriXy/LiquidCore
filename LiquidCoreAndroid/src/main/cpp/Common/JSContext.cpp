//
// JSContext.cpp
//
// LiquidPlayer project
// https://github.com/LiquidPlayer
//
// Created by Eric Lange
//
/*
 Copyright (c) 2016 - 2018 Eric Lange. All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 - Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.

 - Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "Common/JSContext.h"
#include "Common/JSValue.h"
#include "Common/Macros.h"
#include <boost/make_shared.hpp>

boost::shared_ptr<JSContext> JSContext::New(boost::shared_ptr<ContextGroup> isolate, Local<Context> val)
{
    auto p = boost::make_shared<JSContext>(isolate, val);
    isolate->Manage(p);
    return p;
}

JSContext::JSContext(boost::shared_ptr<ContextGroup> isolate, Local<Context> val) {
    m_isolate = isolate;
    m_context.Reset(isolate->isolate(), val);
    m_isDefunct = false;
}

JSContext::~JSContext() {
    Dispose();
};

void JSContext::Dispose() {
    if (!m_isDefunct) {
        m_isDefunct = true;

        m_set_mutex.lock();
        for (auto it = m_value_set.begin(); it != m_value_set.end(); ++it) {
            auto p = boost::atomic_load<JSValue>(&(*it));
            p.reset();
        }
        m_value_set.clear();
        m_set_mutex.unlock();

        m_context.Reset();
        {
            boost::shared_ptr<ContextGroup> isolate = m_isolate;
            isolate.reset();
        }
    }
}

void JSContext::retain(boost::shared_ptr<JSValue> value) {
    m_set_mutex.lock();
    m_value_set.push_back(value);
    m_set_mutex.unlock();
}

boost::shared_ptr<JSValue> JSContext::Global() {
    Local<v8::Value> global = Value()->Global();
    boost::shared_ptr<JSContext> ctx = shared_from_this();
    return JSValue::New(ctx, global);
}
