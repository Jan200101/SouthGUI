#include "ns_plugin.h"
#include "internal/logging.h"

#include "plugin.h"

Plugin* plugin = nullptr;

extern "C" __declspec(dllexport)
void PLUGIN_INIT(PluginInitFuncs* funcs, PluginNorthstarData* data)
{
    spdlog::default_logger()->sinks().pop_back();
    spdlog::default_logger()->sinks().push_back(std::make_shared<PluginSink>(funcs->logger, data->pluginHandle));

    plugin = new Plugin(funcs, data);
}

extern "C" __declspec(dllexport)
void PLUGIN_DEINIT()
{
    assert(plugin);

    delete plugin;
    plugin = nullptr;
}

extern "C" __declspec(dllexport)
void PLUGIN_INFORM_DLL_LOAD(PluginLoadDLL dll, void* data) {
    assert(plugin);

    switch (dll) {
        case PluginLoadDLL::ENGINE:
            plugin->LoadEngineData(data);
        case PluginLoadDLL::CLIENT:
            break;
        case PluginLoadDLL::SERVER:
            break;
        default:
            spdlog::warn("PLUGIN_INFORM_DLL_LOAD called with unknown type {}", (int)dll);
            break;
    }
}

extern "C" __declspec(dllexport)
void PLUGIN_INIT_SQVM_CLIENT(SquirrelFunctions* funcs)
{
    assert(plugin);
    plugin->LoadSQVMFunctions(ScriptContext::CLIENT, funcs);
}

extern "C" __declspec(dllexport)
void PLUGIN_INIT_SQVM_SERVER(SquirrelFunctions* funcs)
{
    assert(plugin);
    plugin->LoadSQVMFunctions(ScriptContext::SERVER, funcs);
}

extern "C" __declspec(dllexport)
void PLUGIN_INFORM_SQVM_CREATED(ScriptContext context, CSquirrelVM* sqvm)
{
    assert(plugin);
    plugin->LoadSQVM(context, sqvm);

    if (context == ScriptContext::UI)
        plugin->hook();

}

extern "C" __declspec(dllexport)
void PLUGIN_INFORM_SQVM_DESTROYED(ScriptContext context)
{
    assert(plugin);
    plugin->RemoveSQVM(context);
}

// There is no deinit logic for Plugins
// Recreate it using DllMain
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_DETACH:
        PLUGIN_DEINIT();

    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    }

    return TRUE;
}
