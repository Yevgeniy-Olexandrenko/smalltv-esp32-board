#include <GyverNTP.h>
#include "SettingsWebApp.h"
#include "drivers/onboard/SelfReboot.h"
#include "defines.h"

namespace service
{
    void SettingsWebApp::begin()
    {
        log_i("begin");
        settings::sets().onBuild([&](sets::Builder& b) { this->settingsBuild(b); });
        settings::sets().onUpdate([&](sets::Updater& u) { this->settingsUpdate(u); });
        settings::sets().onFocusChange([&]() { this->onFocusChange(settings::sets().focused()); });
        settings::sets().rtc.onSync([&](uint32_t unix) { NTP.sync(unix); });

        settings::sets().config.theme = sets::Colors::Aqua;
        settings::sets().config.updateTout = 1000;
        settings::sets().setProjectInfo("home page", WEBAPP_PROJECT_HOME);
        settings::sets().setPass("0000");

        m_currentTab = 1;
        m_connectedToPC = false;
        m_rebootRequest = false;
        m_rebootPending = false;
    }

    void SettingsWebApp::update()
    {
        settings::sets().tick();
        if (m_rebootPending)
        {
            m_rebootPending = false;
            driver::selfReboot.reboot();
        }
    }

    void SettingsWebApp::requestReboot(RebootCorfirmCB cb)
    {
        m_rebootConfirmCB = cb;
        m_rebootRequest = true;
    }

    void SettingsWebApp::settingsBuild(sets::Builder &b)
    {
        sets::GuestAccess g(b);
        if (m_connectedToPC)
        {
            b.Paragraph("Connected to PC", 
                "This device is connected to your computer as an external "
                "storage device. Please use \"Safely Remove\" or \"Eject\" "
                "operation to stop working with it!");
            return;
        }

        if (b.Tabs("SETS;MAIN;APPS", &m_currentTab))
        {
            b.reload();
            return;
        }

        switch(m_currentTab)
        {
        case 0: m_tabSets.settingsBuild(b); break;
        case 1: m_tabMain.settingsBuild(b); break;
        case 2: m_tabApps.settingsBuild(b); break;
        }

        if (b.Confirm("reboot_confirm"_h, "This operation requires a device reboot!"))
        {
            if (m_rebootConfirmCB)
                m_rebootConfirmCB(b.build.value.toBool());

            if (b.build.value.toBool())
            {
                m_rebootPending = true;
                settings::data().update();
                settings::sets().reload();
            }
        }
    }

    void SettingsWebApp::settingsUpdate(sets::Updater &u)
    {
        if (m_connectedToPC != driver::storage.isMSCRunning())
        {
            m_connectedToPC ^= true;
            settings::sets().reload();
            return;
        }

        switch(m_currentTab)
        {
        case 0: m_tabSets.settingsUpdate(u); break;
        case 1: m_tabMain.settingsUpdate(u); break;
        case 2: m_tabApps.settingsUpdate(u); break;
        }

        if (m_rebootRequest)
        {
            u.update("reboot_confirm"_h);
            m_rebootRequest = false;
        }
    }

    void SettingsWebApp::onFocusChange(bool f)
    {
        log_i("focus: %s", (f ? "YES" : "NO"));
    }

    SettingsWebApp settingsWebApp;
}
