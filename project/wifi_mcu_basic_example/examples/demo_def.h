#ifndef __DEMO_DEF_H__
#define __DEMO_DEF_H__

// PASS test
#define DEMO_GET_URL_README     "http://192.168.122.149:9090/readme.txt"
#define DEMO_GET_URL_PDF        "http://192.168.122.149:9090/resources/c.pdf"
#define DEMO_GET_URL_MP3        "http://192.168.122.149:9090/resources/music.mp3"
#define DEMO_GET_URL_SHANGHAI   "http://t.weather.sojson.com/api/weather/city/101020100"
#define DEMO_GET_URL_TIANJIN    "http://t.weather.sojson.com/api/weather/city/101030100"
#define DEMO_GET_CNBLOGS        "https://www.cnblogs.com/"

// HTTPS without CA verify, result is a long json string.
#define DEMO_GET_URL_MUSIC      "https://anime-music.jijidown.com/api/v2/music"

// FAILED test
#define DEMO_GET_BAIDU          "https://www.baidu.com/"

#define DEMO_GET_ACTIVE_URL     DEMO_GET_URL_PDF

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus




#ifdef __cplusplus
}
#endif // !__cplusplus
#endif // !__DEMO_DEF_H__
