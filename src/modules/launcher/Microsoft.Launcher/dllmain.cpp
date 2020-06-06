#include "pch.h"
#include <interface/powertoy_module_interface.h>
#include <interface/lowlevel_keyboard_event_data.h>
#include <interface/win_hook_event_data.h>
#include <common/settings_objects.h>
#include <common/common.h>
#include "trace.h"
#include "resource.h"

extern "C" IMAGE_DOS_HEADER __ImageBase;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
  switch (ul_reason_for_call) {
  case DLL_PROCESS_ATTACH:
    Trace::RegisterProvider();
    break;
  case DLL_THREAD_ATTACH:
  case DLL_THREAD_DETACH:
    break;
  case DLL_PROCESS_DETACH:
    Trace::UnregisterProvider();
    break;
  }
  return TRUE;
}


// These are the properties shown in the Settings page.
struct ModuleSettings {
} g_settings;

// Implement the PowerToy Module Interface and all the required methods.
class Microsoft_Launcher : public PowertoyModuleIface {
private:
  // The PowerToy state.
  bool m_enabled = false;

  // Load initial settings from the persisted values.
  void init_settings();

  // Handle to launch and terminate the launcher
  HANDLE m_hProcess;

  //contains the name of the powerToys
  std::wstring app_name;

public:
  // Constructor
  Microsoft_Launcher() {
    app_name = GET_RESOURCE_STRING(IDS_LAUNCHER_NAME);
    init_settings();
  };

  ~Microsoft_Launcher() {
      if (m_enabled)
      {
          TerminateProcess(m_hProcess, 1);
      }
      m_enabled = false;
  }

  // Destroy the powertoy and free memory
  virtual void destroy() override {
    delete this;
  }

  // Return the display name of the powertoy, this will be cached by the runner
  virtual const wchar_t* get_name() override {
      return app_name.c_str();
  }

  // Return array of the names of all events that this powertoy listens for, with
  // nullptr as the last element of the array. Nullptr can also be returned for empty
  // list.
  virtual const wchar_t** get_events() override {
    static const wchar_t* events[] = { nullptr };
    // Available events:
    // - ll_keyboard
    // - win_hook_event
    //
    // static const wchar_t* events[] = { ll_keyboard,
    //                                   win_hook_event,
    //                                   nullptr };

    return events;
  }

  // Return JSON with the configuration options.
  virtual bool get_config(wchar_t* buffer, int* buffer_size) override {
    HINSTANCE hinstance = reinterpret_cast<HINSTANCE>(&__ImageBase);

    // Create a Settings object.
    PowerToysSettings::Settings settings(hinstance, get_name());
    settings.set_description(GET_RESOURCE_STRING(IDS_LAUNCHER_SETTINGS_DESC));
    settings.set_overview_link(L"https://github.com/microsoft/PowerToys/blob/master/src/modules/launcher/README.md");

    return settings.serialize_to_buffer(buffer, buffer_size);
  }

  // Signal from the Settings editor to call a custom action.
  // This can be used to spawn more complex editors.
  virtual void call_custom_action(const wchar_t* action) override {
    static UINT custom_action_num_calls = 0;
    try {
      // Parse the action values, including name.
      PowerToysSettings::CustomActionObject action_object =
        PowerToysSettings::CustomActionObject::from_json_string(action);
    }
    catch (std::exception ex) {
      // Improper JSON.
    }
  }

  // Called by the runner to pass the updated settings values as a serialized JSON.
  virtual void set_config(const wchar_t* config) override {
    try {
      // Parse the input JSON string.
      PowerToysSettings::PowerToyValues values =
        PowerToysSettings::PowerToyValues::from_json_string(config);

      // If you don't need to do any custom processing of the settings, proceed
      // to persists the values calling:
      values.save_to_settings_file();
      // Otherwise call a custom function to process the settings before saving them to disk:
      // save_settings();
    }
    catch (std::exception ex) {
      // Improper JSON.
    }
  }

   // Enable the powertoy
  virtual void enable()
  {
      if (!is_process_elevated(false))
      {
          SHELLEXECUTEINFOW sei{ sizeof(sei) };
          sei.fMask = { SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI };
          sei.lpFile = L"modules\\launcher\\PowerLauncher.exe";
          sei.nShow = SW_SHOWNORMAL;
          ShellExecuteExW(&sei);

          m_hProcess = sei.hProcess;
      }
      else
      {
          std::wstring action_runner_path = get_module_folderpath();
          action_runner_path += L"\\action_runner.exe";

          // Set up the shared file from which to retrieve the PID of PowerLauncher
          HANDLE hMapFile = CreateFileMappingW(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(DWORD), POWER_LAUNCHER_PID_SHARED_FILE);
          if (hMapFile)
          {
              PDWORD pidBuffer = reinterpret_cast<PDWORD>(MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(DWORD)));
              if (pidBuffer)
              {
                  *pidBuffer = 0;
                  m_hProcess = NULL;

                  if (run_non_elevated(action_runner_path, L"-start_PowerLauncher", nullptr))
                  {
                      const int maxRetries = 20;
                      for (int retry = 0; retry < maxRetries; ++retry)
                      {
                          Sleep(50);
                          DWORD pid = *pidBuffer;
                          if (pid)
                          {
                              m_hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
                              break;
                          }
                      }
                  }
              }
              CloseHandle(hMapFile);
          }
      }

      m_enabled = true;
  }

  // Disable the powertoy
  virtual void disable()
  {
      if (m_enabled)
      {
          TerminateProcess(m_hProcess, 1);
      }

      m_enabled = false;
  }

  // Returns if the powertoys is enabled
  virtual bool is_enabled() override {
    return m_enabled;
  }

  // Handle incoming event, data is event-specific
  virtual intptr_t signal_event(const wchar_t* name, intptr_t data)  override {
    if (wcscmp(name, ll_keyboard) == 0) {
      auto& event = *(reinterpret_cast<LowlevelKeyboardEvent*>(data));
      // Return 1 if the keypress is to be suppressed (not forwarded to Windows),
      // otherwise return 0.
      return 0;
    }
    else if (wcscmp(name, win_hook_event) == 0) {
      auto& event = *(reinterpret_cast<WinHookEvent*>(data));
      // Return value is ignored
      return 0;
    }
    return 0;
  }

  /* Register helper class to handle system menu items related actions. */
  virtual void register_system_menu_helper(PowertoySystemMenuIface* helper) {}
  /* Handle action on system menu item. */
  virtual void signal_system_menu_action(const wchar_t* name) {}
};

// Load the settings file.
void Microsoft_Launcher::init_settings() {
  try {
    // Load and parse the settings file for this PowerToy.
    PowerToysSettings::PowerToyValues settings =
      PowerToysSettings::PowerToyValues::load_from_settings_file(get_name());

  }
  catch (std::exception ex) {
    // Error while loading from the settings file. Let default values stay as they are.
  }
}


extern "C" __declspec(dllexport) PowertoyModuleIface* __cdecl powertoy_create() {
  return new Microsoft_Launcher();
}
