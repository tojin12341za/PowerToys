#include "pch.h"
#include "target_state.h"
#include "common/start_visible.h"
#include "keyboard_state.h"
#include "common/shared_constants.h"

TargetState::TargetState(int ms_delay) :
    delay(std::chrono::milliseconds(ms_delay)), thread(&TargetState::thread_proc, this)
{
}

bool TargetState::signal_event(unsigned vk_code, bool key_down)
{
    std::unique_lock lock(mutex);
    if (!events.empty() && events.back().key_down == key_down && events.back().vk_code == vk_code)
    {
        return false;
    }
    // Hide the overlay when WinKey + Shift + S is pressed. 0x53 is the VK code of the S key
    if (key_down && state == Shown && vk_code == 0x53 && (GetKeyState(VK_LSHIFT) || GetKeyState(VK_RSHIFT)))
    {
        // We cannot use normal hide() here, there is stuff that needs deinitialization.
        // It can be safely done when the user releases the WinKey.
        instance->quick_hide();
    }
    bool suppress = false;
    if (!key_down && (vk_code == VK_LWIN || vk_code == VK_RWIN) &&
        state == Shown &&
        std::chrono::system_clock::now() - signal_timestamp > std::chrono::milliseconds(300) &&
        !key_was_pressed)
    {
        suppress = true;
    }
    events.push_back({ key_down, vk_code });
    lock.unlock();
    cv.notify_one();
    if (suppress)
    {
        // Send a fake key-stroke to prevent the start menu from appearing.
        // We use 0xCF VK code, which is reserved. It still prevents the
        // start menu from appearing, but should not interfere with any
        // keyboard shortcuts.
        INPUT input[3] = { {}, {}, {} };
        input[0].type = INPUT_KEYBOARD;
        input[0].ki.wVk = 0xCF;
        input[0].ki.dwExtraInfo = CommonSharedConstants::KEYBOARDMANAGER_INJECTED_FLAG;
        input[1].type = INPUT_KEYBOARD;
        input[1].ki.wVk = 0xCF;
        input[1].ki.dwFlags = KEYEVENTF_KEYUP;
        input[1].ki.dwExtraInfo = CommonSharedConstants::KEYBOARDMANAGER_INJECTED_FLAG;
        input[2].type = INPUT_KEYBOARD;
        input[2].ki.wVk = vk_code;
        input[2].ki.dwFlags = KEYEVENTF_KEYUP;
        input[2].ki.dwExtraInfo = CommonSharedConstants::KEYBOARDMANAGER_INJECTED_FLAG;
        SendInput(3, input, sizeof(INPUT));
    }
    return suppress;
}

void TargetState::was_hidden()
{
    std::unique_lock<std::mutex> lock(mutex);
    state = Hidden;
    events.clear();
    lock.unlock();
    cv.notify_one();
}

void TargetState::exit()
{
    std::unique_lock lock(mutex);
    events.clear();
    state = Exiting;
    lock.unlock();
    cv.notify_one();
    thread.join();
}

KeyEvent TargetState::next()
{
    auto e = events.front();
    events.pop_front();
    return e;
}

void TargetState::handle_hidden()
{
    std::unique_lock lock(mutex);
    if (events.empty())
        cv.wait(lock);
    if (events.empty() || state == Exiting)
        return;
    auto event = next();
    if (event.key_down && (event.vk_code == VK_LWIN || event.vk_code == VK_RWIN))
    {
        state = Timeout;
        winkey_timestamp = std::chrono::system_clock::now();
    }
}

void TargetState::handle_shown()
{
    std::unique_lock lock(mutex);
    if (events.empty())
    {
        cv.wait(lock);
    }
    if (events.empty() || state == Exiting)
    {
        return;
    }
    auto event = next();
    if (event.key_down && (event.vk_code == VK_LWIN || event.vk_code == VK_RWIN))
    {
        return;
    }
    if (!event.key_down && (event.vk_code == VK_LWIN || event.vk_code == VK_RWIN) || !winkey_held())
    {
        state = Hidden;
        lock.unlock();
        return;
    }
    if (event.key_down)
    {
        key_was_pressed = true;
        lock.unlock();
        instance->on_held_press(event.vk_code);
    }
}

void TargetState::thread_proc()
{
    while (true)
    {
        switch (state)
        {
        case Hidden:
            handle_hidden();
            break;
        case Timeout:
            handle_timeout();
            break;
        case Shown:
            handle_shown();
            break;
        case Exiting:
        default:
            return;
        }
    }
}

void TargetState::handle_timeout()
{
    std::unique_lock lock(mutex);
    auto wait_time = delay - (std::chrono::system_clock::now() - winkey_timestamp);
    if (events.empty())
        cv.wait_for(lock, wait_time);
    if (state == Exiting)
        return;
    while (!events.empty())
    {
        auto event = events.front();
        if (event.key_down && (event.vk_code == VK_LWIN || event.vk_code == VK_RWIN))
            events.pop_front();
        else
            break;
    }
    if (!events.empty() || !only_winkey_key_held() || is_start_visible())
    {
        state = Hidden;
        return;
    }
    if (std::chrono::system_clock::now() - winkey_timestamp < delay)
        return;
    signal_timestamp = std::chrono::system_clock::now();
    key_was_pressed = false;
    state = Shown;
    lock.unlock();
    instance->on_held();
}

void TargetState::set_delay(int ms_delay)
{
    delay = std::chrono::milliseconds(ms_delay);
}
