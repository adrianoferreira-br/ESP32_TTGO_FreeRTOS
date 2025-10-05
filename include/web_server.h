#ifndef WEB_SERVER_H
#define WEB_SERVER_H


void setup_webserver();
void loop_webserver();
void handleRoot();
void handleConfigMQTT();
void handleReadings();
void handleInfo();
void handleOTA();




#endif // WEB_SERVER_H