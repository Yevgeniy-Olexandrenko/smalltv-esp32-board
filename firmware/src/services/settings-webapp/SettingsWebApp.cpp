#include "SettingsWebApp.h"
#include "drivers/storage/Storage.h"

namespace service
{
    void SettingsWebApp::begin()
    {
        log_i("begin");
        settings::sets().onBuild([&](sets::Builder& b) { this->settingsBuild(b); });
        settings::sets().onUpdate([&](sets::Updater& u) { this->settingsUpdate(u); });
        settings::sets().onFocusChange([&]() { this->onFocusChange(settings::sets().focused()); });

        settings::sets().config.theme = sets::Colors::Aqua;
        settings::sets().config.updateTout = 1000;
        settings::sets().setPass("0000");

        m_currentTab = 1;
        m_connectedToPC = false;
        m_requestReboot = false;
    }

    void SettingsWebApp::update()
    {
        settings::sets().tick();
        if (m_requestReboot) ESP.restart();
    }

    void SettingsWebApp::requestDeviceReboot()
    {
        m_requestReboot = true;
        settings::data().update();
        settings::sets().reload();
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

        if (m_currentTab == 0)
            m_tabSets.settingsBuild(b);
        else if (m_currentTab == 2)
            m_tabApps.settingsBuild(b);
        else
        {
            m_tabMain.settingsBuild(b);
            if (b.Button("Reboot"))
            {
                requestDeviceReboot();
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
    }

    void SettingsWebApp::onFocusChange(bool f)
    {
        log_i("focus: %s", (f ? "YES" : "NO"));
    }

    SettingsWebApp settingsWebApp;
}
