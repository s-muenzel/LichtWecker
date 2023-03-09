// #include <ArduinoJson.h>
#include "WebS.h"

#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <ezTime.h>

#include "NTP.h"
#include "OTA.h"
#include "Sonnenaufgang.h"
#include "Speicher.h"
#include "Zugangsinfo.h"
#include "main.h"

ESP8266WebServer server(80);
NTP_Helfer *ntp_p;
Sonnenaufgang *sa_p;
Speicher *sp_p;
OTA *ota_p;

bool __Admin_Mode_An;

///////// Hilfsfunktionen
// format bytes
String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + " B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + " KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + " MB";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + " GB";
  }
}
// Stunde aus Text
uint8_t parseZeit_Stunde(String s) { return min(23, max(0, int(s.toInt()))); }
// Minute aus Text
uint8_t parseZeit_Minute(String s) {
  return min(59, max(0, int(s.substring(3).toInt())));
}

void handleWeckzeit() {
  D_PRINTF(LOG_DEBUG, "Webaufruf /");
  char temp[2000];
  time_t w[7];
  bool a[7];
  for (int i = 0; i < 7; i++) {
    w[i] = sp_p->Weckzeit(i);
    a[i] = sp_p->Wecker_An(i);
  }
  snprintf(
      temp, 2000,
      "<html><head><meta charset='UTF-8'><link rel='stylesheet' type='text/css' href='style.css'></head>\
<body><form action='/Setze_WZ' method='POST'><span><div class='Tag'>Wecker an <input type='checkbox' name='Aktiv' %s></div><hr>\
<div class='Tag'><div>Montag</div><input type='time' name='Mo' value='%02d:%02d'><input type='checkbox' name='AnMo' %s></div>\
<div class='Tag'><div>Dienstag</div><input type='time' name='Di' value='%02d:%02d'><input type='checkbox' name='AnDi' %s></div>\
<div class='Tag'><div>Mittwoch</div><input type='time' name='Mi' value='%02d:%02d'><input type='checkbox' name='AnMi' %s></div>\
<div class='Tag'><div>Donnerstag</div><input type='time' name='Do' value='%02d:%02d'><input type='checkbox' name='AnDo' %s></div>\
<div class='Tag'><div>Freitag</div><input type='time' name='Fr' value='%02d:%02d'><input type='checkbox' name='AnFr' %s></div>\
<div class='Tag'><div>Samstag</div><input type='time' name='Sa' value='%02d:%02d'><input type='checkbox' name='AnSa' %s></div>\
<div class='Tag'><div>Sonntag</div><input type='time' name='So' value='%02d:%02d'><input type='checkbox' name='AnSo' %s></div>\
</span><span><input type='submit' name='ok' value='ok'></span></form></body></html>",
      sp_p->Wecker_Aktiv() ? "checked" : "", hour(w[2]), minute(w[2]),
      a[2] ? "checked" : "", hour(w[3]), minute(w[3]), a[3] ? "checked" : "",
      hour(w[4]), minute(w[4]), a[4] ? "checked" : "", hour(w[5]), minute(w[5]),
      a[5] ? "checked" : "", hour(w[6]), minute(w[6]), a[6] ? "checked" : "",
      hour(w[0]), minute(w[0]), a[0] ? "checked" : "", hour(w[1]), minute(w[1]),
      a[1] ? "checked" : "");
  server.send(200, "text/html", temp);
}

void handleSnooze() {
  D_PRINTF(LOG_DEBUG, "Webaufruf /Snooze");
  if (sa_p->Laeuft()) {
    D_PRINTF(LOG_DEBUG, "Snooze");
    sa_p->Snooze();
    server.sendHeader("Location", "/");
    server.send(303, "text/html", "Location: /");
  } else {
    server.sendHeader("Location", "/");
    server.send(303, "text/html", "Location: /");
  }
}

void handle24Aus() {
  D_PRINTF(LOG_DEBUG, "Webaufruf /24Aus");
  if (sa_p->Laeuft()) {
    D_PRINTF(LOG_DEBUG, "24Aus");
    sa_p->Stop();
  }
  server.sendHeader("Location", "/");
  server.send(303, "text/html", "Location: /");
}

void handleStart() {
  D_PRINTF(LOG_DEBUG, "Webaufruf /Start");
  if (__Admin_Mode_An) {
    D_PRINTF(LOG_DEBUG, "Starte Sonnenaufgang");
    sa_p->Nachricht(Sonnenaufgang::aufgang);
    server.sendHeader("Location", "/Konfig");
    server.send(303, "text/html", "Location: /Konfig");
  } else {
    server.send(403, "text/plain", "Kein Admin-Mode!");
  }
}

void handleReset() {
  D_PRINTF(LOG_DEBUG, "Webaufruf /Reset");
  if (__Admin_Mode_An) {
    D_PRINTF(LOG_DEBUG, "Resette JETZT");
    server.send(200, "text/plain", "Resetting - reload now");
    delay(100);
    ESP.restart();
  } else {
    server.send(403, "text/plain", "Kein Admin-Mode!");
  }
}

void handleAdmin() {
  D_PRINTF(LOG_DEBUG, "Webaufruf /Admin");
  if (!server.authenticate(admin_user, admin_pw)) {
    server.requestAuthentication(DIGEST_AUTH, "Admin-Mode",
                                 "Admin Mode failed");
  } else {
    // User authorisiert, Admin_Mode anschalgen
    __Admin_Mode_An = true;
    ota_p->Bereit();
    server.send(200, "text/plain", "Admin-Mode eingeschaltet");
  }
}

void handleSetzeWeckzeit() {
  D_PRINTF(LOG_DEBUG, "Webaufruf /Setze_WZ");
  tmElements_t tmSet;
  tmSet.Year = 1;
  tmSet.Month = 1;
  tmSet.Day = 1;
  tmSet.Second = 0;
  bool _Aktiv = false;
  time_t w[7];
  bool a[7];
  bool u[7];  // Zeit für Wochentag gefunden..

  for (int i = 0; i < 7; i++) {
    a[i] = false;
    u[7] = false;
  }

  for (int i = 0; i < server.args(); i++) {
    if (server.argName(i) == "Aktiv") {
      _Aktiv = true;
    } else if (server.argName(i) == "Mo") {
      u[2] = true;
      tmSet.Hour = parseZeit_Stunde(server.arg(i));
      tmSet.Minute = parseZeit_Minute(server.arg(i));
      w[2] = makeTime(tmSet);
    } else if (server.argName(i) == "Di") {
      u[3] = true;
      tmSet.Hour = parseZeit_Stunde(server.arg(i));
      tmSet.Minute = parseZeit_Minute(server.arg(i));
      w[3] = makeTime(tmSet);
    } else if (server.argName(i) == "Mi") {
      u[4] = true;
      tmSet.Hour = parseZeit_Stunde(server.arg(i));
      tmSet.Minute = parseZeit_Minute(server.arg(i));
      w[4] = makeTime(tmSet);
    } else if (server.argName(i) == "Do") {
      u[5] = true;
      tmSet.Hour = parseZeit_Stunde(server.arg(i));
      tmSet.Minute = parseZeit_Minute(server.arg(i));
      w[5] = makeTime(tmSet);
    } else if (server.argName(i) == "Fr") {
      u[6] = true;
      tmSet.Hour = parseZeit_Stunde(server.arg(i));
      tmSet.Minute = parseZeit_Minute(server.arg(i));
      w[6] = makeTime(tmSet);
    } else if (server.argName(i) == "Sa") {
      u[0] = true;
      tmSet.Hour = parseZeit_Stunde(server.arg(i));
      tmSet.Minute = parseZeit_Minute(server.arg(i));
      w[0] = makeTime(tmSet);
    } else if (server.argName(i) == "So") {
      u[1] = true;
      tmSet.Hour = parseZeit_Stunde(server.arg(i));
      tmSet.Minute = parseZeit_Minute(server.arg(i));
      w[1] = makeTime(tmSet);
    } else if (server.argName(i) == "AnMo") {
      a[2] = true;
    } else if (server.argName(i) == "AnDi") {
      a[3] = true;
    } else if (server.argName(i) == "AnMi") {
      a[4] = true;
    } else if (server.argName(i) == "AnDo") {
      a[5] = true;
    } else if (server.argName(i) == "AnFr") {
      a[6] = true;
    } else if (server.argName(i) == "AnSa") {
      a[0] = true;
    } else if (server.argName(i) == "AnSo") {
      a[1] = true;
    }
  }
  D_PRINTF(LOG_DEBUG, "Neue Weckzeiten: Aktiv: %s", _Aktiv ? "Ja" : "Nein");
  sp_p->Wecker_Aktiv(_Aktiv);
  for (int i = 0; i < 7; i++) {
    if (u[i]) {
      D_PRINTF(LOG_DEBUG, "%d: %02d:%02d %s", i, hour(w[i]), minute(w[i]),
               a[i] ? "An" : "Aus");
      sp_p->setze_Weckzeit(i, w[i], a[i]);
    }
  }
  sp_p->speichern();
  server.sendHeader("Location", "/WeckZeit");
  server.send(303, "text/html", "Location:/WeckZeit");
}

void handleKonfig() {
  D_PRINTF(LOG_DEBUG, "Webaufruf /Konfig");
  char temp[2000];
  if (__Admin_Mode_An) {
    snprintf(
        temp, 2000,
        "<html><head><meta charset='UTF-8'><link rel='stylesheet' type='text/css' href='style.css'></head>\
<body><form action='/Setze_Konfig' method='POST'><span>\
<div class='Tag'>L&auml;nge<input type='number' min='0.5' max='2.0'step='0.1'  name='L' value='%f'></div>\
<div class='Tag'>Geschwindigkeit<input type='number' min='0.01' max='1.0' step='0.01' name='V' value='%f'></div>\
<div class='Tag'>Dauer Aufgang<input type='number' min='10' max='600' step='1' name='D' value='%f'></div>\
<div class='Tag'>Dauer Hell<input type='number' min='2' max='600' step='1' name='N' value='%f'></div>\
<div class='Tag'>Snooze-Zeit<input type='number' min='10' max='900' step='1'  name='S' value='%f'></div>\
<div class='Tag'>Schaltdauer Relais<input type='number' min='10' max='999' step='1'  name='R' value='%u'></div>\
<div class='Tag'>Hostname<input type='text' name='H' value='%s'></div>\
</span><span><input type='submit' name='ok' value='ok'></span></form>\
<form action='/Start'><span><div class='Tag'>Starte jetzt</div></span><span><input type='submit' name='ok' value='ok'></span></form>\
<form action='/Reset'><span><div class='Tag'>Neustart</div></span><span><input type='submit' name='ok' value='ok'></span></form>\
</body></html>",
        sp_p->SA_laenge(), sp_p->SA_v(), sp_p->SA_dauer(),
        sp_p->SA_nachleuchten(), sp_p->SA_snooze(),
        sp_p->SA_relais(), sp_p->SA_hostname());
  } else {
    snprintf(
        temp, 2000,
        "<html><head><meta charset='UTF-8'><link rel='stylesheet' type='text/css' href='style.css'></head>\
<body><span>\
<div class='Tag'>L&auml;nge:<span class='Rechts'>%3.1f m</span></div>\
<div class='Tag'>Geschwindigkeit:<span class='Rechts'>%4.2f m/s</span></div>\
<div class='Tag'>Dauer Aufgang:<span class='Rechts'>%4.0f s</span></div>\
<div class='Tag'>Dauer Hell:<span class='Rechts'>%4.0f s</span></div>\
<div class='Tag'>Snooze-Zeit:<span class='Rechts'>%4.0f s</span></div>\
<div class='Tag'>Schaltdauer Relais:<span class='Rechts'>%4u ms</span></div>\
<div class='Tag'>Hostname:<span class='Rechts'>%s</span></div>\
</span><span></span></body></html>",
        sp_p->SA_laenge(), sp_p->SA_v(), sp_p->SA_dauer(),
        sp_p->SA_nachleuchten(), sp_p->SA_snooze(),
        sp_p->SA_relais(), sp_p->SA_hostname());
  }
  // char temp2[1300];
  // File f =
  //     LittleFS.open(__Admin_Mode_An ? "/Konf_Adm.temp" : "/Konf_noA.temp",
  //     "r");//  f.read((uint8_t *)temp2, 1299);
  //  snprintf(temp, 2000, temp2, sp_p->SA_laenge(), sp_p->SA_v(),
  //  sp_p->SA_dauer(), sp_p->SA_nachleuchten(),
  //  sp_p->SA_snooze(), sp_p->SA_relais(),
  //  sp_p->SA_hostname());
  server.send(200, "text/html", temp);
}

void handleSetzeKonfig() {
  D_PRINTF(LOG_DEBUG, "Webaufruf /Setze_Konfig");
  if (__Admin_Mode_An) {
    for (int i = 0; i < server.args(); i++) {
      if (server.argName(i) == "L") {
        float f = server.arg(i).toFloat();
        D_PRINTF(LOG_DEBUG, "Konfig: Laenge: %f", f);
        sp_p->SA_laenge(f);
      } else if (server.argName(i) == "V") {
        float f = server.arg(i).toFloat();
        D_PRINTF(LOG_DEBUG, "Konfig: V: %f", f);
        sp_p->SA_v(f);
      } else if (server.argName(i) == "D") {
        float f = server.arg(i).toFloat();
        D_PRINTF(LOG_DEBUG, "Konfig: Dauer: %f", f);
        sp_p->SA_dauer(f);
      } else if (server.argName(i) == "N") {
        float f = server.arg(i).toFloat();
        D_PRINTF(LOG_DEBUG, "Konfig: Nachleuchten: %f", f);
        sp_p->SA_nachleuchten(f);
      } else if (server.argName(i) == "S") {
        float f = server.arg(i).toFloat();
        D_PRINTF(LOG_DEBUG, "Konfig: Snooze: %f", f);
        sp_p->SA_snooze(f);
      } else if (server.argName(i) == "R") {
        unsigned int n = server.arg(i).toInt();
        if (n > 5000) n = 100;
        D_PRINTF(LOG_DEBUG, "Konfig: Relais: %d", n);
        sp_p->SA_relais(n);
      } else if (server.argName(i) == "H") {
        D_PRINTF(LOG_DEBUG, "Konfig: Hostname: %s", server.arg(i).c_str());
        sp_p->SA_hostname(server.arg(i).c_str());
      }
    }
    sp_p->speichern();
    server.sendHeader("Location", "/Konfig");
    server.send(303, "text/html", "Location:/Konfig");
  } else {
    D_PRINTF(LOG_DEBUG, "KEIN ADMIN MODE - tue nix");
    server.send(403, "text/plain", "Kein Admin-Mode!");
  }
}

void handleStatus() {
  char temp[1000];
  time_t t = ntp_p->Jetzt();  // Store the current time in time
  snprintf(temp, 1000,
           "{ \"Zeit\" : \"%2d:%02d:%02d\", \"ZeitStatus\" : %d, \"Aktiv\" : "
           "%d, \"Admin\" : %d}",
           hour(t), minute(t), second(t), timeStatus(), sa_p->Laeuft(),
           __Admin_Mode_An);
  server.send(200, "application/json", temp);
}

void handleLogs() {
  if (!__Admin_Mode_An) {
    server.send(403, "text/plain", "kein Zugriff möglich");
    return;
  }
  //  DynamicJsonDocument doc(8192);
  // JsonArray list = doc.createNestedArray("list");
  // doc["h"] = get_log_puffer_h();
  // for (int8_t i = 0; i < LOGG_PUFFER_SIZE - 1; i++) {
  //   JsonObject list_entry = list.createNestedObject();
  //   list_entry["t"] = get_log_puffer_t(i);
  //   list_entry["s"] = get_log_puffer_s(i);
  //   list_entry["l"] = get_log_puffer_l(i);
  //   list_entry["m"] = get_log_puffer_m(i);
  // }
  // String temp;
  // serializeJson(doc, temp);
  String temp = "{\"list\":[";
  for (int8_t i = 0; i < LOGG_PUFFER_SIZE - 1; i++) {
    temp += "{\"t\":";
    temp += get_log_puffer_t(i);
    temp += ",\"s\":";
    temp += get_log_puffer_s(i);
    temp += ",\"l\":\"";
    temp += get_log_puffer_l(i);
    temp += "\",\"m\":\"";
    temp += get_log_puffer_m(i);
    temp += "\"}";
    if (i < LOGG_PUFFER_SIZE - 2) temp += ",";
  }
  temp += "],\"h\":";
  temp += get_log_puffer_h();
  temp += "}";
  server.send(200, "application/json", temp);
}

void handleNotFound() {
  D_PRINTF(LOG_DEBUG, "Webaufruf - unbekannte Seite %s", server.uri().c_str());
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ":" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

File fsUploadFile;
void handleHochladen() {
  if (__Admin_Mode_An) {
    if (server.uri() != "/Hochladen") {
      return;
    }
    HTTPUpload &upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      String filename = upload.filename;
      if (!filename.startsWith("/")) {
        filename = "/" + filename;
      }
      D_PRINTF(LOG_DEBUG, "handleHochladen Name: %s", filename.c_str());
      fsUploadFile = LittleFS.open(filename, "w");
      filename = String();
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      if (fsUploadFile) {
        fsUploadFile.write(upload.buf, upload.currentSize);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (fsUploadFile) {
        fsUploadFile.close();
      }
      D_PRINTF(LOG_DEBUG, "handleHochladen Groesse: %d", upload.totalSize);
    }
  }
}

void handleLoeschen() {
  if (__Admin_Mode_An) {
    if ((server.args() == 0) || (server.argName(0) != "datei")) {
      return server.send(500, "text/plain", "BAD ARGS");
    }
    String path = server.arg(0);
    D_PRINTF(LOG_DEBUG, "handleLoeschen: %s", path.c_str());
    if (path == "/") {
      return server.send(500, "text/plain", "BAD PATH");
    }
    if (!LittleFS.exists(path)) {
      return server.send(404, "text/plain", "FileNotFound");
    }
    LittleFS.remove(path);
    server.sendHeader("Location", "/Dateien");
    server.send(303, "text/html", "Location:/Dateien");
    path = String();
  } else {
    D_PRINTF(LOG_DEBUG, "KEIN ADMIN MODE - tue nix");
    server.send(403, "text/plain", "Kein Admin-Mode!");
  }
}

void handleDateien() {
  D_PRINTF(LOG_DEBUG, "Seite handleDateien");

  String output;
  output =
      "<html><head><meta charset='UTF-8'><link rel='stylesheet' "
      "type='text/css' href='style.css'></head><body>";
  if (__Admin_Mode_An) {
    output += String(
        "<form action='/Hochladen' method='post' "
        "enctype='multipart/form-data'><span><div class='Tag'><input "
        "type='file' name='name'></div></span><span><input class='button' "
        "type='submit' value='Upload'></span></form>");
  }

  Dir dir = LittleFS.openDir("/");
  while (dir.next()) {
    File entry = dir.openFile("r");
    if (__Admin_Mode_An) {
      output += String("<form action='/Loeschen' method='post'>");
    }
    output += String("<span><div class='Tag'><span>");
    output += entry.name() + String("</span><span class='Rechts'>") +
              formatBytes(entry.size());
    if (__Admin_Mode_An) {
      output += String(
                    "</span><input type='text' style='display:none' "
                    "name='datei' value='") +
                entry.name();
      output += String(
          "'></div></span><span><input class='button' type='submit' "
          "value='l&ouml;schen'></span></form>");
    } else {
      output += String("</span></div>");
    }
    D_PRINTF(LOG_DEBUG, "File '%s', Size %d", entry.name(), entry.size());
    entry.close();
  }
  output += "</body></html>";
  server.send(200, "text/html", output);
}
///////////

WebS::WebS() { __Admin_Mode_An = false; }

void WebS::Beginn(NTP_Helfer *ntp, Sonnenaufgang *sa, Speicher *sp, OTA *ota) {
  ntp_p = ntp;
  sa_p = sa;
  sp_p = sp;
  ota_p = ota;
  if (!LittleFS.begin()) {
    D_PRINTF(LOG_DEBUG, "Failed to mount file system");
  }
  {
    Dir dir = LittleFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      D_PRINTF(LOG_DEBUG, "FS File: %s, size: %s", fileName.c_str(),
               formatBytes(fileSize).c_str());
    }
  }

  //  Weckzeiten
  server.serveStatic(
      "/", LittleFS,
      "/top.htm");  // Anzeige Weckzeiten und Möglichkeit Weckzeiten
                    // zu setzen. Auch Link zu Konfig und Dateien
  server.on("/Snooze",
            handleSnooze);  // Falls Sonnenaufgang läuft --> Snooze aufrufen
  server.on("/24Aus",
            handle24Aus);  // Falls Sonnenaufgang läuft --> Stop aufrufen
  server.on(
      "/Status",
      handleStatus);  // liefert lokale Zeit, Admin- und Aktiv-Status per JSON
  server.on("/WeckZeit", handleWeckzeit);  // Anzeige Weckzeiten und Möglichkeit
                                           // Weckzeiten zu setzen
  server.on("/Setze_WZ",
            handleSetzeWeckzeit);  // Speichert die neuen Weckzeiten ab
  server.on("/Logs", handleLogs);  // Debug-Output
  server.serveStatic("/logs", LittleFS, "/logs.htm");  // Aufgehübscht
  server.on("/Konfig", handleKonfig);  // Zeigt die Konfig-Daten an
  server.on("/Setze_Konfig",
            handleSetzeKonfig);  // Speichert die neuen Weckzeiten ab
  server.on(
      "/Start",
      handleStart);  // nur noch zu Testzwecken - startet einen Sonnenaufgang
  server.on("/Reset",
            handleReset);  // Neustart, nötig wenn z.B. Hostname geändert wurde
                           // oder Admin-Mode zurückgesetzt werden soll
  server.on(
      "/Admin",
      handleAdmin);  // Admin-Mode einschalten (erforder Authentifizierung)
  server.on("/Dateien", handleDateien);    // Datei-Operationen (upload, delete)
  server.on("/Loeschen", handleLoeschen);  // Delete (spezifische Datei)
  server.on(
      "/Hochladen", HTTP_POST,
      []() {  // first callback is called after the request has ended with all
              // parsed arguments
        if (__Admin_Mode_An) {
          server.sendHeader("Location", "/Dateien");
          server.send(303, "text/html", "Location:/Dateien");
        } else {
          server.send(403, "text/plain", "Kein Admin-Mode!");
        }
      },
      handleHochladen);  // second callback handles file uploads at that
                         // location
  server.serveStatic("/favicon.ico", LittleFS,
                     "/favicon.ico");  // liefert das Favicon.ico
  server.serveStatic("/style.css", LittleFS,
                     "/style.css");   // liefert das Stylesheet
  server.onNotFound(handleNotFound);  // Fallback

  server.begin();
}

void WebS::Admin_Mode() { __Admin_Mode_An = true; }

void WebS::Tick() { server.handleClient(); }
