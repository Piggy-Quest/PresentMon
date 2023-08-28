// Copyright (C) 2022 Intel Corporation
// SPDX-License-Identifier: MIT
#pragma once
#include "../AsyncEndpoint.h"
#include <Core/source/kernel/Kernel.h>
#include "../CefValues.h"

namespace p2c::client::util::async
{
    class EnumerateKeys : public AsyncEndpoint
    {
    public:
        static constexpr std::string GetKey() { return "enumerateKeys"; }
        EnumerateKeys() : AsyncEndpoint{ AsyncEndpoint::Environment::KernelTask } {}
        // {} => {keys: [{code: uint, text: string}]}
        Result ExecuteOnKernelTask(uint64_t uid, CefRefPtr<CefValue> pArgObj, kern::Kernel& kernel) const override
        {
            using namespace win;
            auto keyList = Key::EnumerateKeys();
            // remove modifiers like Ctrl from the list
            std::erase_if(keyList, [blacklist = ModSet::EnumerateMods()](const Key::Descriptor& kd) {
                return std::ranges::any_of(blacklist, [&kd](const ModSet::Descriptor& md) {
                    return kd.text.contains(md.text);
                });
            });
            auto keyListCef = MakeCefList(keyList.size());
            for (int i = 0; i < keyList.size(); i++)
            {
                auto& key = keyList[i];
                keyListCef->SetValue(i, MakeCefObject(
                    CefProp{ "code", key.code },
                    CefProp{ "text", std::move(key.text) }
                ));
            }
            return Result{ true, MakeCefObject(CefProp{ "keys", CefValueDecay(std::move(keyListCef)) }) };
        }
    };
}