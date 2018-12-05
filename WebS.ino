#include "WebS.h"

#include "Sonnenaufgang.h"
#include "Speicher.h"

#include <FS.h>
#include <ESP8266WebServer.h>

ESP8266WebServer server(80);

bool __Admin_Mode_An;


///////// Hilfsfunktionen
//format bytes
String formatBytes(size_t bytes) {
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  } else {
    return String(bytes / 1024.0 / 1024.0 / 1024.0) + "GB";
  }
}
//Stunde aus Text
uint8_t parseZeit_Stunde(String s) {
  return min(23, max(0, int(s.toInt())));
}
//Minute aus Text
uint8_t parseZeit_Minute(String s) {
  return min(59, max(0, int(s.substring(3).toInt())));
}


void handleRoot() {
  Serial.println("handleRoot");
  if (SPIFFS.exists("/top.htm")) {
    File file = SPIFFS.open("/top.htm", "r");
    server.streamFile(file, "text/html");
    file.close();
  } else {
    server.send(404, "text/plain", "file error");
  }
}

void handleCSS() {
  Serial.println("handleCSS");
  if (SPIFFS.exists("/style.css")) {
    File file = SPIFFS.open("/style.css", "r");
    server.streamFile(file, "text/css");
    file.close();
  } else {
    server.send(404, "text/plain", "file error");
  }
}

void handleWeckzeit() {
  char temp[2000];
  time_t t = now(); // Store the current time in time
  time_t w[7];
  bool a[7];
  for (int i = 0; i < 7; i++) {
    w[i] = __WZ.Weckzeit(i);
    a[i] = __WZ.Wecker_An(i);
  }
  Serial.printf("Webaufruf / um %2d:%02d:%02d\n", hour(t), minute(t), second(t));
  snprintf(temp, 2000,
           "<html><head><link rel='stylesheet' type='text/css' href='style.css'></head>\
<body><form action='/Setze_WZ' method='POST'><span><div class='Tag'>Wecker an <input type='checkbox' name='Aktiv' %s></div><hr>\
<div class='Tag'><div>Montag</div><input type='time' name='Mo' value='%02d:%02d'><input type='checkbox' name='AnMo' %s></div>\
<div class='Tag'><div>Dienstag</div><input type='time' name='Di' value='%02d:%02d'><input type='checkbox' name='AnDi' %s></div>\
<div class='Tag'><div>Mittwoch</div><input type='time' name='Mi' value='%02d:%02d'><input type='checkbox' name='AnMi' %s></div>\
<div class='Tag'><div>Donnerstag</div><input type='time' name='Do' value='%02d:%02d'><input type='checkbox' name='AnDo' %s></div>\
<div class='Tag'><div>Freitag</div><input type='time' name='Fr' value='%02d:%02d'><input type='checkbox' name='AnFr' %s></div>\
<div class='Tag'><div>Samstag</div><input type='time' name='Sa' value='%02d:%02d'><input type='checkbox' name='AnSa' %s></div>\
<div class='Tag'><div>Sonntag</div><input type='time' name='So' value='%02d:%02d'><input type='checkbox' name='AnSo' %s></div>\
</span><span><input type='submit' name='ok' value='ok'></span></form></body></html>",
           __WZ.Wecker_Aktiv() ? "checked" : "",
           hour(w[2]), minute(w[2]), a[2] ? "checked" : "",
           hour(w[3]), minute(w[3]), a[3] ? "checked" : "",
           hour(w[4]), minute(w[4]), a[4] ? "checked" : "",
           hour(w[5]), minute(w[5]), a[5] ? "checked" : "",
           hour(w[6]), minute(w[6]), a[6] ? "checked" : "",
           hour(w[0]), minute(w[0]), a[0] ? "checked" : "",
           hour(w[1]), minute(w[1]), a[1] ? "checked" : ""
          );
  server.send(200, "text/html", temp);
}

void handleStartStop() {
  time_t t = now(); // Store the current time in time
  Serial.printf("Webaufruf /StartStop um %2d:%2d:%2d\n", hour(t), minute(t), second(t));
  if ((server.args() == 1) && (server.argName(0) == "Schalten")) {
    if (server.arg(0) == "start") {
      Serial.printf("Starte Sonnenaufgang\n");
      __SA.Start();
      server.sendHeader("Location", "/");
      server.send(303, "text/html", "Location: /");
    } else {
      Serial.printf("Stoppe Sonnenaufgang\n");
      __SA.Stop();
      server.sendHeader("Location", "/");
      server.send(303, "text/html", "Location: /");
    }
  } else {
    Serial.printf("Fehler in Args\n");
    String message = "Fehler in den Args\n\n";
    for (uint8_t i = 0; i < server.args(); i++) {
      message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    server.send(400, "text/plain", message);
  }
}


void handleSetzeWeckzeit() {
  time_t t = now(); // Store the current time in time
  Serial.printf("Webaufruf /Setze_WZ um %2d:%2d:%2d\n", hour(t), minute(t), second(t));
  tmElements_t tmSet;
  tmSet.Year = 1;
  tmSet.Month = 1;
  tmSet.Day = 1;
  tmSet.Second = 0;
  bool _Aktiv = false;
  time_t w[7];
  bool a[7];
  bool u[7]; // Zeit für Wochentag gefunden..

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
  Serial.printf("Neue Weckzeiten:\nAktiv: %s\n", _Aktiv ? "Ja" : "Nein");
  __WZ.Wecker_Aktiv(_Aktiv);
  for (int i = 0; i < 7; i++) {
    if (u[i]) {
      Serial.printf("%d: %02d:%02d %s\n", i, hour(w[i]), minute(w[i]), a[i] ? "An" : "Aus");
      __WZ.setze_Weckzeit(i, w[i], a[i]);
    }
  }
  __WZ.speichern();
  server.sendHeader("Location", "/WeckZeit");
  server.send(303, "text/html", "Location:/WeckZeit");
}

void handleKonfig() {
  if (__Admin_Mode_An) {
    char temp[2000];
    time_t t = now(); // Store the current time in time
    Serial.printf("Webaufruf /Konfig um %2d:%02d:%02d\n", hour(t), minute(t), second(t));
    snprintf(temp, 2000,
             "<html><head><link rel='stylesheet' type='text/css' href='style.css'></head>\
<body><form action='/Setze_Konfig' method='POST'><span>\
<div class='Tag'>L&auml;nge<input type='number' min='0.5' max='2.0' step='0.1'  name='L' value='%f'></div>\
<div class='Tag'>Geschwindigkeit<input type='number' min='0.01' max='1.0' step='0.01' name='V' value='%f'></div>\
<div class='Tag'>Dauer Aufgang<input type='number' min='10' max='600' step='1' name='D' value='%f'></div>\
<div class='Tag'>Dauer Hell<input type='number' min='2' max='600' step='1'  name='N' value='%f'></div>\
<div class='Tag'>Snooze-Zeit<input type='number' min='10' max='900' step='1'  name='S' value='%f'></div>\
</span><span><input type='submit' name='ok' value='ok'></span></form></body></html>",
             __WZ.lese_SA_laenge(),
             __WZ.lese_SA_v(),
             __WZ.lese_SA_dauer(),
             __WZ.lese_SA_nachleuchten(),
             __WZ.lese_SA_snooze()
            );
    server.send(200, "text/html", temp);
  } else {
    char temp[2000];
    time_t t = now(); // Store the current time in time
    Serial.printf("Webaufruf /Konfig um %2d:%02d:%02d\n", hour(t), minute(t), second(t));
    snprintf(temp, 2000,
             "<html><head><link rel='stylesheet' type='text/css' href='style.css'></head>\
<body><span>\
<div class='Tag'>L&auml;nge: %f></div>\
<div class='Tag'>Geschwindigkeit: %f></div>\
<div class='Tag'>Dauer Aufgang: %f></div>\
<div class='Tag'>Dauer Hell: %f></div>\
<div class='Tag'>Snooze-Zeit: %f></div>\
</span><span></span></body></html>",
             __WZ.lese_SA_laenge(),
             __WZ.lese_SA_v(),
             __WZ.lese_SA_dauer(),
             __WZ.lese_SA_nachleuchten(),
             __WZ.lese_SA_snooze()
            );
    server.send(200, "text/html", temp);
  }
}

void handleSetzeKonfig() {
  time_t t = now(); // Store the current time in time
  Serial.printf("Webaufruf /Setze_Konfig um %2d:%2d:%2d\n", hour(t), minute(t), second(t));
  if (__Admin_Mode_An) {

    for (int i = 0; i < server.args(); i++) {
      if (server.argName(i) == "L") {
        float f = server.arg(i).toFloat();
        Serial.printf("Konfig: Laenge: %f\n", f);
        __WZ.setze_SA_laenge(f);
        __SA.Setze_Laenge(f);
      } else if (server.argName(i) == "V") {
        float f = server.arg(i).toFloat();
        Serial.printf("Konfig: V: %f\n", f);
        __WZ.setze_SA_v(f);
        __SA.Setze_v(f);
      } else if (server.argName(i) == "D") {
        float f = server.arg(i).toFloat();
        Serial.printf("Konfig: Dauer: %f\n", f);
        __WZ.setze_SA_dauer(f);
        __SA.Setze_Dauer(f);
      } else if (server.argName(i) == "N") {
        float f = server.arg(i).toFloat();
        Serial.printf("Konfig: Nachleuchten: %f\n", f);
        __WZ.setze_SA_nachleuchten(f);
        __SA.Setze_Nachleuchten(f);
      } else if (server.argName(i) == "S") {
        float f = server.arg(i).toFloat();
        Serial.printf("Konfig: Snooze: %f\n", f);
        __WZ.setze_SA_snooze(f);
        __SA.Setze_Snooze(f);
      }
    }
    __WZ.speichern();
    server.sendHeader("Location", "/Konfig");
    server.send(303, "text/html", "Location:/Konfig");
  } else {
    Serial.println("KEIN ADMIN MODE - tue nix\n");
    server.send(403, "text/plain", "Kein Admin-Mode!");
  }
}

void handleLokaleZeit() {
  char temp[1000];
  time_t t = now(); // Store the current time in time
  char style[60];
  switch (timeStatus()) {
    case timeNotSet:
      strncpy(style, "color:red;background-color:yellow;", 59);
      break;
    case timeNeedsSync:
      strncpy(style, "color:yellow;", 59);
      break;
    case timeSet:
    default:
      strncpy(style, "color:black;", 59);
      break;
  }
  snprintf(temp, 1000,
           "<html><head><meta http-equiv='refresh' content='10'/></head><body style='%s'>%2d:%02d:%02d</body></html>",
           style, hour(t), minute(t), second(t));
  server.send(200, "text/html", temp);
}

void handleFavIcon() {
  Serial.println("handleFavIcon");
  if (SPIFFS.exists("/favicon.ico")) {
    File file = SPIFFS.open("/favicon.ico", "r");
    server.streamFile(file, "image/x-icon");
    file.close();
  } else {
    server.send(404, "text/plain", "file error");
  }
  /*
    Serial.println("/favicon.ico");
    File file = SPIFFS.open("/favicon.ico", "r"); // FILE_READ ); // "rb"
    if (!file) { // isDir geht wohl nur auf ESP32 || file.isDirectory()) {
      Serial.println(" Verzeichnis?");
      server.send(404, "text/plain", "failed to open favicon.ico");
      return;
    }

    uint8_t buf[512]; // mehr als nötig:
    uint16_t size = file.size();
    if (size < 512) {
      size = file.read(buf, size);
      Serial.print(" FavIcon gelesen=");
      Serial.println(size);
      WiFiClient client = server.client();
      client.write( buf, size );
      server.send ( 200, "image/x-icon", "" );
      file.close();
      return;
    }
    Serial.println(" nicht gefunden");
    server.send(404, "text/plain", "read favicon.ico failed"); */
}

void handleNotFound() {
  time_t t = now(); // Store the current time in time
  Serial.printf("Webaufruf - unbekannte Seite %s um %2d:%02d:%02d\n", server.uri().c_str(), hour(t), minute(t), second(t));
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

void handleHochladen() {
  if (__Admin_Mode_An) {
    if (server.uri() != "/Hochladen") {
      return;
    }
    File fsUploadFile;
    HTTPUpload& upload = server.upload();
    if (upload.status == UPLOAD_FILE_START) {
      String filename = upload.filename;
      if (!filename.startsWith("/")) {
        filename = "/" + filename;
      }
      Serial.print("handleHochladen Name: "); Serial.println(filename);
      fsUploadFile = SPIFFS.open(filename, "w");
      filename = String();
    } else if (upload.status == UPLOAD_FILE_WRITE) {
      //Serial.print("handleHochladen Data: "); Serial.println(upload.currentSize);
      if (fsUploadFile) {
        fsUploadFile.write(upload.buf, upload.currentSize);
      }
    } else if (upload.status == UPLOAD_FILE_END) {
      if (fsUploadFile) {
        fsUploadFile.close();
      }
      Serial.print("handleHochladen Groesse: "); Serial.println(upload.totalSize);
    }
  }
}

void handleLoeschen() {
  if (__Admin_Mode_An) {
    if ((server.args() == 0) || (server.argName(0) != "datei")) {
      return server.send(500, "text/plain", "BAD ARGS");
    }
    String path = server.arg(0);
    Serial.println("handleLoeschen: " + path);
    if (path == "/") {
      return server.send(500, "text/plain", "BAD PATH");
    }
    if (!SPIFFS.exists(path)) {
      return server.send(404, "text/plain", "FileNotFound");
    }
    SPIFFS.remove(path);
    server.sendHeader("Location", "/Dateien");
    server.send(303, "text/html", "Location:/Dateien");
    path = String();
  } else {
    Serial.println("KEIN ADMIN MODE - tue nix\n");
    server.send(403, "text/plain", "Kein Admin-Mode!");
  }
}


void handleDateien() {
  Serial.println("Seite handleDateien");

  String output;
  output = "<html><head><link rel='stylesheet' type='text/css' href='style.css'></head><body>";
  if (__Admin_Mode_An) {
    output += String("<form action='/Hochladen' method='post' enctype='multipart/form-data'><span><div class='Tag'><input type='file' name='name'></div></span><span><input class='button' type='submit' value='Upload'></span></form>");
  }

  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
    File entry = dir.openFile("r");
    if (__Admin_Mode_An) {
      output += String("<form action='/Loeschen' method='post'>");
    }
    output += String("<div class='Tag'><span>");
    output += entry.name() + String("</span><span>") + formatBytes(entry.size());
    if (__Admin_Mode_An) {
      output += String("</span><input type='text' style='display:none' name='datei' value='") + entry.name();
      output += String("'></div></span><span><input class='button' type='submit' value='l&ouml;schen'></span></form>");
    } else {
      output += String("</span></div>");
    }
    entry.close();
  }
  output += "</body></html>";
  server.send(200, "text/html", output);
}
///////////

WebS::WebS() {
  __Admin_Mode_An = false;
}

void WebS::Beginn() {
  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
  }
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    Serial.printf("\n");
  }

  server.on("/",              handleRoot);          // Anzeige Weckzeiten und Möglichkeit Weckzeiten zu setzen. Auch Link zu Konfig
  server.on("/WeckZeit",      handleWeckzeit);      // Anzeige Weckzeiten und Möglichkeit Weckzeiten zu setzen. Auch Link zu Konfig
  server.on("/StartStop",     handleStartStop);     // nur noch zu Testzwecken
  server.on("/Setze_WZ",      handleSetzeWeckzeit); // Speichert die neuen Weckzeiten ab
  server.on("/Konfig",        handleKonfig);        // Zeigt die Konfig-Daten an
  server.on("/Setze_Konfig",  handleSetzeKonfig);   // Speichert die neuen Weckzeiten ab
  server.on("/LokaleZeit",    handleLokaleZeit);    // zeigt "nur" die aktuelle lokale Zeit an
  server.on("/Dateien",       handleDateien);        // Datei-Operationen (upload, delete)
  server.on("/Loeschen",      handleLoeschen);    // Delete (spezifische Datei)
  server.on("/Hochladen", HTTP_POST, []() { //first callback is called after the request has ended with all parsed arguments
    if (__Admin_Mode_An) {
      server.sendHeader("Location", "/Dateien");
      server.send(303, "text/html", "Location:/Dateien");
    } else {
      server.send(403, "text/plain", "Kein Admin-Mode!");
    }
  },  handleHochladen);        //second callback handles file uploads at that location
  server.on("/favicon.ico",   handleFavIcon);       // liefert das Favicon.ico
  server.on("/style.css",     handleCSS);           // liefert das Stylesheet
  server.onNotFound(          handleNotFound);      // Fallback

  server.begin();
}


void WebS::Admin_Mode() {
  __Admin_Mode_An = true;
}

void WebS::Tick() {
  server.handleClient();
}
