// yabridge: a Wine VST bridge
// Copyright (C) 2020  Robbert van der Helm
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <type_traits>

#include <bitsery/ext/std_optional.h>
#include <bitsery/traits/string.h>
#include <pluginterfaces/vst/ivsthostapplication.h>

#include "../common.h"
#include "base.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"

/**
 * Wraps around `IHostApplication` for serialization purposes. An instance of
 * this proxy object will be initialized on the Wine plugin host side after the
 * host passes an actual instance to the plugin, and all function calls made to
 * this proxy will be passed through to the actual object. This is used to proxy
 * both the host application context passed during `IPluginBase::intialize()` as
 * well as for the 'global' context in `IPluginFactory3::setHostContext()`.
 */
class YaHostApplication : public Steinberg::Vst::IHostApplication {
   public:
    /**
     * These are the arguments for constructing a `YaHostApplicationImpl`.
     */
    struct ConstructArgs {
        ConstructArgs();

        /**
         * Read arguments from an existing implementation.
         */
        ConstructArgs(Steinberg::IPtr<Steinberg::Vst::IHostApplication> context,
                      std::optional<size_t> owner_instance_id);

        /**
         * The unique instance identifier of the proxy object instance this host
         * context has been passed to and thus belongs to. If we are handling
         * When handling `IPluginFactory::setHostContext()` this will be empty.
         */
        std::optional<native_size_t> owner_instance_id;

        /**
         * For `IHostApplication::getName`.
         */
        std::optional<std::u16string> name;

        template <typename S>
        void serialize(S& s) {
            s.ext(owner_instance_id, bitsery::ext::StdOptional{},
                  [](S& s, native_size_t& instance_id) {
                      s.value8b(instance_id);
                  });
            s.ext(name, bitsery::ext::StdOptional{},
                  [](S& s, std::u16string& name) {
                      s.text2b(name, std::extent_v<Steinberg::Vst::String128>);
                  });
        }
    };

    /**
     * Instantiate this instance with arguments read from an actual host
     * context.
     *
     * @note Since this is passed as part of `IPluginBase::intialize()` and
     *   `IPluginFactory3::setHostContext()`, there are no direct `Construct` or
     *   `Destruct` messages. This object's lifetime is bound to that of the
     *   objects they are passed to. If those objects get dropped, then the host
     *   contexts should also be dropped.
     */
    YaHostApplication(const ConstructArgs&& args);

    /**
     * The lifetime of this object should be bound to the object we created it
     * for. When for instance the `Vst3PluginProxy` instance with id `n` gets
     * dropped, the corresponding `YaHostApplicationImpl` then that should also
     * be dropped.
     */
    virtual ~YaHostApplication();

    DECLARE_FUNKNOWN_METHODS

    // From `IHostApplication`
    tresult PLUGIN_API getName(Steinberg::Vst::String128 name) override;
    virtual tresult PLUGIN_API createInstance(Steinberg::TUID cid,
                                              Steinberg::TUID _iid,
                                              void** obj) override = 0;

   protected:
    ConstructArgs arguments;
};

#pragma GCC diagnostic pop
