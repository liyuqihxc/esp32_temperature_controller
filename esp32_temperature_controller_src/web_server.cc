#include <ESPAsyncWebServer.h>
#include "app.h"

const char index_html[] PROGMEM = R"rawliteral(<!DOCTYPE html><html><head><meta charset="utf-8"><link rel="shortcut icon" href="https://unpkg.com/@fortawesome/fontawesome-free@6.5.1/svgs/solid/microchip.svg"><meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no,minimal-ui"><title>ESP32 Temperature Controller</title><link rel="stylesheet" href="https://unpkg.com/vuetify@3.5.1/dist/vuetify.min.css"><link rel="stylesheet" href="https://unpkg.com/@mdi/font@5.9.55/css/materialdesignicons.min.css"></head><body><script type="importmap">{"imports": {"vue": "https://unpkg.com/vue@3/dist/vue.esm-browser.js","vuetify": "https://unpkg.com/vuetify@3.5.1/dist/vuetify.esm.js"}}</script><div class="hidden" id="app"><v-layout class="rounded rounded-md"><v-app-bar elevation="1"><v-app-bar-title>ESP32 Temperature Controller</v-app-bar-title><template v-slot:append><v-btn icon="mdi-cloud-upload"></v-btn></template></v-app-bar><v-main><v-container fluid><v-row align="center" justify="center"><v-col><v-card variant="flat"><v-card-item><div class="text-overline mb-1">Ambient temperature and humidity</div><v-row align="center" justify="center" style="min-height:100px"><v-col><div class="text-h6 text-center"><v-icon icon="mdi-home-thermometer-outline"></v-icon>35.7&deg;C</div></v-col><v-col><div class="text-h6 text-center"><v-icon icon="mdi-water-percent"></v-icon>57.9%</div></v-col></v-row></v-card-item></v-card></v-col></v-row><v-divider class="border-opacity-25"></v-divider><v-row align="center" justify="center"><v-col><v-card variant="flat"><v-card-item><div class="text-overline mb-1">Prob temperature</div><v-row align="end" justify="center" style="min-height:70px"><v-col><div class="text-h6 text-center"><v-icon icon="mdi-thermometer"></v-icon>35.7&deg;C</div></v-col><v-col><div class="text-h6 text-center"><v-icon icon="mdi-thermometer"></v-icon>N/A</div></v-col></v-row><v-row align="start" justify="center"><v-col><div class="text-caption text-center">Prob 1</div></v-col><v-col><div class="text-caption text-center">Prob 2</div></v-col></v-row></v-card-item></v-card></v-col></v-row><v-divider class="border-opacity-25"></v-divider><v-row align="center" justify="center"><v-col><v-card variant="flat"><v-card-item><div class="mb-1 text-overline">Heater</div><template v-slot:append><v-switch density="compact" hide-details><template v-slot:label><div class="text-caption">Heater Disabled</div></template></v-switch></template></v-card-item><v-card-text><v-row align="center" justify="center"><v-col><v-sheet class="text-h6 text-center"><v-icon icon="mdi-thermometer"></v-icon>35.7&deg;C</v-sheet></v-col><v-col><v-sheet class="text-h6 text-center"><v-icon icon="mdi-power-plug"></v-icon>57.9%</v-sheet></v-col></v-row><v-row align="start" justify="center"><v-col><div class="text-caption text-center">Fixed Temp Mode</div></v-col><v-col><div class="text-caption text-center">Fixed Power Mode</div></v-col></v-row></v-card-text></v-card></v-col></v-row></v-container></v-main></v-layout></div><style>.hidden{display:none!important}.layout{display:flex;flex-direction:column;padding:16px}.layout .title{margin:40px auto;font-size:1.25rem!important;font-weight:500;letter-spacing:.0125em!important;line-height:2rem}.a-input{font-size:1rem;line-height:1.5;margin-bottom:6px}.a-input-control label{margin:0 16px;font-size:.8em}.a-input-control input{border:1px solid #dcdcdc;padding:5px 16px;border-radius:25px;background:rgba(255,255,255,.1);margin-top:10px;width:100%;box-sizing:border-box}.a-input-control input:focus{outline:0;border-color:#569aff}.a-input-control.error input{border-color:red}.a-input-message{color:red;padding:0 16px;font-size:.7em}.divider{display:flex;align-items:center;text-align:center;width:90%;margin:0 auto;font-size:.9em;font-weight:400}.divider::after,.divider::before{content:'';flex:1;border-bottom:1px solid #dcdcdc}.divider:not(:empty)::before{margin-right:.25em}.divider:not(:empty)::after{margin-left:.25em}.a-button{width:100%;color:#fff;background-color:#06f;height:2.5em;border-radius:7px}.a-overlay{position:fixed;bottom:0;left:0;pointer-events:none;position:fixed;right:0;top:0;z-index:2000}.a-overlay .a-overlay-scrim{background:#000;bottom:0;left:0;opacity:.32;pointer-events:auto;position:fixed;right:0;top:0}.a-overlay .a-overlay-content{display:flex;flex-direction:column;align-items:center;justify-content:center;background-color:#fff;position:absolute;padding:16px}</style><div class="hidden layout" id="config"><div class="title text-center">Controller Setup</div><form id="settings"><div class="a-input"><div class="a-input-control"><label class="a-input-label" for="name">Controller Name</label> <input id="name" type="text" autofocus name="name"></div><div class="a-input-message"><span class="hidden">Controller name must be less than 30 characters</span></div></div><div class="divider">Wi-Fi credentials</div><div class="a-input"><div class="a-input-control"><label class="a-input-label" for="ssid">SSID</label> <input id="ssid" type="text" name="ssid"></div><div class="a-input-message"><span class="hidden">SSID is required</span> <span class="hidden">SSID can be any alphanumeric, case-sensitive entry from 2 to 32 characters</span></div></div><div class="a-input"><div class="a-input-control"><label class="a-input-label" for="ssid">Password</label> <input id="password" type="text" name="password"></div><div class="a-input-message"><span class="hidden">Password is required</span> <span class="hidden">Password can be 8 to 63 characters</span></div></div><button class="a-button" type="submit" style="margin-top:16px">Save</button></form><div style="margin-top:20px;font-size:.9em">Note:</div><div style="font-size:.75em;letter-spacing:.03em;font-weight:400;line-height:1.25rem">Controller will automatically reboot after settings were saved. If the specified network cannot be connected, the controller will start the default AP and let you modify those settings.</div><div class="a-overlay hidden"><div class="a-overlay-scrim"></div><div class="a-overlay-content"><div class="text-caption">Waiting for controller to boot up......</div><div class="text-caption mt-2">Will redirect in <span id="timer">15</span>s.</div></div></div></div><script>if("/config"===location.pathname){const e=document.getElementById("settings");e.onsubmit=function(t){e.querySelector("button").disabled=!0;const n=document.getElementById("name"),r=document.getElementById("ssid"),s=document.getElementById("password"),l=n.value.trim(),a=r.value.trim(),c=s.value.trim();if(function(){let e=!0;const t=".a-input-message span",o=t+":first-child",d=t+":nth-child(2)",i=n.parentElement;l&&l.length>30?(e=!1,i.classList.add("error"),i.parentElement.querySelector(t).classList.remove("hidden")):(i.classList.remove("error"),i.parentElement.querySelector(t).classList.add("hidden"));const m=r.parentElement;!a||a&&(a.length<2||a.length>32)?(e=!1,m.classList.add("error"),a?(m.parentElement.querySelector(o).classList.add("hidden"),m.parentElement.querySelector(d).classList.remove("hidden")):(m.parentElement.querySelector(o).classList.remove("hidden"),m.parentElement.querySelector(d).classList.add("hidden"))):(m.classList.remove("error"),m.parentElement.querySelectorAll(t).forEach((e=>{e.classList.add("hidden")})));const h=s.parentElement;return!c||c&&(c.length<8||c.length>63)?(e=!1,h.classList.add("error"),c?(h.parentElement.querySelector(o).classList.add("hidden"),h.parentElement.querySelector(d).classList.remove("hidden")):(h.parentElement.querySelector(o).classList.remove("hidden"),h.parentElement.querySelector(d).classList.add("hidden"))):(h.classList.remove("error"),h.parentElement.querySelectorAll(t).forEach((e=>{e.classList.add("hidden")}))),e}()){const t=new URLSearchParams;for(const n of new FormData(e))t.append(n[0],n[1].trim());fetch("/api/config?" + t.toString(),{method:"post"}).then((e=>{if(e.ok){const e=document.querySelector(".a-overlay");e.classList.remove("hidden");const t=e.querySelector(".a-overlay-content");t.style.top=screen.height/2-t.clientHeight+"px",t.style.left=(screen.width-t.clientWidth)/2+"px"}})).catch((e=>{alert("Please check you Wi-Fi connection.")}))}t.preventDefault()},document.getElementById("config").classList.remove("hidden")}</script><script type="module">import{createApp}from"vue";import{createVuetify}from"vuetify";const vuetify=createVuetify({theme:{defaultTheme:"custom",themes:{custom:{dark:!1,colors:{background:"#FFFFFF",surface:"#FFFFFF","surface-bright":"#FFFFFF","surface-light":"#EEEEEE","surface-variant":"#424242","on-surface-variant":"#EEEEEE",primary:"#1867C0","primary-darken-1":"#1F5592",secondary:"#48A9A6","secondary-darken-1":"#018786",error:"#B00020",info:"#2196F3",success:"#4CAF50",warning:"#FB8C00"},variables:{"border-color":"#000000","border-opacity":.12,"high-emphasis-opacity":.87,"medium-emphasis-opacity":.6,"disabled-opacity":.38,"idle-opacity":.04,"hover-opacity":.04,"focus-opacity":.12,"selected-opacity":.08,"activated-opacity":.12,"pressed-opacity":.12,"dragged-opacity":.08,"theme-kbd":"#212529","theme-on-kbd":"#FFFFFF","theme-code":"#F5F5F5","theme-on-code":"#000000"}}}}});"/"===location.pathname&&(createApp({data:()=>({drawer:null,drawerRight:null,right:!1,left:!1}),props:{source:String}}).use(vuetify).mount("#app"),document.getElementById("app").classList.remove("hidden"));</script></body></html>)rawliteral";

void onInitialConfig(AsyncWebServerRequest *const);

void RegisterDebugWebRouter(AsyncWebServer& server)
{
  server.on("/api/preferences", HTTP_DELETE, [](AsyncWebServerRequest *request) {
    preferences.clear();
    request->send(204);
  });
}

void RegisterConfigWebRouter(AsyncWebServer& server)
{
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *const response = request->beginResponse(307);
    response->addHeader("Location", "/config");
    request->send(response);
  });
  server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", index_html);
  });
  server.on("/api/config", HTTP_POST, onInitialConfig);
}

void RegisterWebRouter(AsyncWebServer& server)
{
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", index_html);
  });
}

void onInitialConfig(AsyncWebServerRequest *const request)
{
  if (request->params() != 3) {
    request->send(400);
  }
  if (request->hasParam(PREF_APP_NAME)) {
    const AsyncWebParameter *const paramName = request->getParam(PREF_APP_NAME);
    if (!paramName->value().isEmpty()) {
      SERIAL_WRITE("name=");
      SERIAL_WRITE_LINE(paramName->value());
      preferences.putString(PREF_APP_NAME, paramName->value());
    }
  }
  if (!request->hasParam(PREF_WIFI_SSID) || !request->hasParam(PREF_WIFI_PASSWORD)) {
    request->send(400);
  }
  const AsyncWebParameter *const paramSsid = request->getParam(PREF_WIFI_SSID);
  const AsyncWebParameter *const paramPassword = request->getParam(PREF_WIFI_PASSWORD);
  if (paramSsid->value().isEmpty() || paramPassword->value().isEmpty()) {
    request->send(400);
  }
  SERIAL_WRITE("ssid=");
  SERIAL_WRITE_LINE(paramSsid->value());
  SERIAL_WRITE("password=");
  SERIAL_WRITE_LINE(paramPassword->value());
  preferences.putString(PREF_WIFI_SSID, paramSsid->value());
  preferences.putString(PREF_WIFI_PASSWORD, paramPassword->value());
  request->send(204);
  xTaskCreate([](void*){
    const String ssid = preferences.getString(PREF_WIFI_SSID, "");
    const String password = preferences.getString(PREF_WIFI_PASSWORD, "");
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    SERIAL_WRITE("Attempting to connect to hotspot ");
    SERIAL_WRITE_LINE(ssid);
    for (int i = 0; i < 15; i++)
    {
      if (WiFi.status() != WL_CONNECTED)
      {
        SERIAL_WRITE("Failed ");
        SERIAL_WRITE_LINE(i + 1);
        delay(1000);
        continue;
      }
      else
      {
        SERIAL_WRITE("Successfully connected to hotspot ");
        SERIAL_WRITE_LINE(ssid);
        esp_restart();
        return;
      }
    }
    preferences.remove(PREF_WIFI_SSID);
    preferences.remove(PREF_WIFI_PASSWORD);
    SERIAL_WRITE_LINE("Restarting default AP");
    WiFi.disconnect();
    WiFi.softAP(default_ap_ssid, default_ap_password);
    vTaskDelete(NULL);
  }, "TestSSIDTask", 4096, NULL, tskIDLE_PRIORITY, NULL);
}
