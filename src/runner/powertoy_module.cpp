#include "pch.h"
#include "powertoy_module.h"

std::map<std::wstring, PowertoyModule>& modules()
{
    static std::map<std::wstring, PowertoyModule> modules;
    return modules;
}

PowertoyModule load_powertoy(const std::wstring& filename)
{
    auto handle = winrt::check_pointer(LoadLibraryW(filename.c_str()));
    auto create = reinterpret_cast<powertoy_create_func>(GetProcAddress(handle, "powertoy_create"));
    if (!create)
    {
        FreeLibrary(handle);
        winrt::throw_last_error();
    }
    auto module = create();
    if (!module)
    {
        FreeLibrary(handle);
        winrt::throw_last_error();
    }
    module->register_system_menu_helper(&SystemMenuHelperInstance());
    return PowertoyModule(module, handle);
}

json::JsonObject PowertoyModule::json_config() const
{
    int size = 0;
    module->get_config(nullptr, &size);
    std::wstring result;
    result.resize(size - 1);
    module->get_config(result.data(), &size);
    return json::JsonObject::Parse(result);
}

PowertoyModule::PowertoyModule(PowertoyModuleIface* module, HMODULE handle) :
    handle(handle), module(module)
{
    if (!module)
    {
        throw std::runtime_error("Module not initialized");
    }
    auto want_signals = module->get_events();
    if (want_signals)
    {
        for (; *want_signals; ++want_signals)
        {
            powertoys_events().register_receiver(*want_signals, module);
        }
    }
    if (SystemMenuHelperInstance().HasCustomConfig(module))
    {
        powertoys_events().register_system_menu_action(module);
    }
}
