#include "mdns_manager.h"
#include <ESPmDNS.h>
#include "../log_manager/log_manager.h"

static const char* TAG = "mDNS";

MdnsManager mdnsMgr;

bool MdnsManager::start(const char* hostname) {
    if (MDNS.begin(hostname)) {
        MDNS.addService("http", "tcp", 80);
        _active = true;
        LOG_INFO(TAG, "mDNS actif : http://%s.local", hostname);
        return true;
    }
    LOG_ERROR(TAG, "Échec démarrage mDNS");
    return false;
}
