#ifndef WIFI_SETUP_H
#define WIFI_SETUP_H

#ifdef __cplusplus
extern "C" {
#endif

    void setup_wifi_via_airkiss(void);
    void setup_wifi_via_softap(void);
    void setup_wifi_via_smartconfig(void);
    void setup_wifi_direct(void);

#ifdef __cplusplus
}
#endif
#endif // !WIFI_SETUP_H
