#pragma once

#include <GeoIP.h>
#include "shared/settings/Settings.h"

namespace service
{
    class GeoLocation : public settings::Provider
    {
        using Timestamp = unsigned long;

    public:
        void begin();
        void update();

        void settingsBuild(sets::Builder& b) override;
        void settingsUpdate(sets::Updater& u) override;

    private:
        Timestamp m_fetchTS;

        float m_lat;
        float m_lon;
        long  m_utcOffset;
        char  m_region[256];
        char  m_city[256];
        char  m_tz[256];

        GeoIP m_geoip;
        location_t m_location;
    };

    extern GeoLocation geoLocation;
}
