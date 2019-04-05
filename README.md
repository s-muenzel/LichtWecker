# LichtWecker #

## Intro ##

Lichtwecker soll zu gegebenen Zeiten langsam von dunkel/rötlich auf hell/weiss-bläuliches Licht aufblenden.
Die Einstellungen gehen über ein kleinew Web-Interface.

## Hardware ##

### Microcontroler ###
Der Code ist für ein ESP8266 entwickelt.
Entweder ein `Sonoff Basic`, bei den gängigen Versionen ist GPIO Pin 14 relativ einfach abgreifbar.
    Habe aber auch ein Exemplar erwischt, bei dem anstatt GPIO 14 ein GPIO Pin 2 an der Stelle auf dem PCB ist
	__ACHTUNG__: dann muss man ggfs. den Pin auf VCC hochziehen, sonst bootet der Sonoff nicht, der Wert des Pullup-Widerstands hängt von der verwendeten LED-Kette ab
    bei mir waren es ca 2k.

Alternativ geht auch ein `ESP 01(-S)`, dann mit Pegelkonverter. Da dann ohne Taster zum Snooze bzw. auschalten, sondern nur per Web.


### LEDs ###
Eine WS2812(xx), die Anzahl der LEDs ist konfigurierbar.

### Stromversorgung ###
Der WS2812B-Strip zieht bei 30 LEDs ca 0.9A bei 5V, bei mehr LEDs natürlich analog mehr Strom.

Daher habe ich bei einmal hinter dem `Sonoff Basic` ein LED-Netzteil gehängt. Bis das allerdings die Spannung liefert, dauert es. Daher gibt es einen Parameter,
wie lange verzögert wird bevor ein Signal an den Strip geschickt wird.

Bei einer anderen, einfacheren Ausführung (mit `ESP 01-S`) hängen Microcontroler und Strip an einem einfachen 2A-USB-Netzteil, da brauchst auch keine Verzögerung.

## Software ##

Generell sollten die verschiedenen Aufgabgen in eigenen Klassen versteckt sein.
Da bei dem Arduino-IDE wird relativ viel forwarding betrieben, globals sind auch überall verfügbar.
Das nutzt das Programm (unschöner weise) mehr oder weniger aus.

### Libraries ###

zum Betrieb sind einige externe Bibliotheken nötig:
#### ezTime ####
(by Rop Gonggrijp), aktuell: Version 0.7.10
holt die Zeit von einem NTP-Server (Default pool.ntp.org).
Kann auch Sommer-/Winterzeit, daher bessere Alternative als 
#### Time, Timelib ####
(by Michael Margolis), aktuell: 1.5.0
Standard-Arduino Time, unterstützt NTP, aber leider kein DST

#### EEPROM ####
Persistieren der Konfiguration (und Weckzeiten..), siehe Klasse Speicher
(by Ivan Grokhotkov), aktuell: Version 1.0.0

#### ESP8266WiFi, ESP8266mDNS.h ####
Wifi & Netzwerk
(by Simon Peter,...) (built-in wenn ESP8266 Support installiert wird)

#### ArduinoOTA, WiFiUdp ####
(by Ivan Grokhotkov,...) aktuell: Version 1.0.0 (built-in)
OTA Support

#### Adafruit_NeoPixel ####
Treiber für WS2812 - LEDs (by Adafruit), aktuell: Version 1.1.8

#### FS ####
Filesystem (SPIFS) für ESP8266 (built-in wenn ESP8266 Support installiert wird)

#### ESP8266WebServer ####
WebServer für ESP8266 (built-in wenn ESP8266 Support installiert wird)

### Klassen (Dateien) ###

#### LichtWecker.ino ####
Enthält `Setup()` und `loop()`.
Ausserdem alle `#includes` für die Klassen und die globalen Objekte.

#### Knopf.ino / .h ####
Wenn ein Taster an einem GPIO hängt, kann `Knopf` kurze bzw. lange Tastendrücke auswerten

#### NTP.ino / .h ####
NTP - Netzwerkzeit lesen

#### OTA.ino / .h ####
OTA - Over The Air Updates. Geht nur, wenn aktiv angeschaltet (bei der Implementierung in LichtWecker muss der Knopf sehr lange gedrückt werden.

#### Sonnenaufgang.ino / .h ####
Hier wird die LED-Kette angesteuert.

#### Speicher.ino / .h ####
Persistenz der Werte (sollen ja auch einen Reset oder Stromausfall überdauern)

#### WebS.ino / .h ####
Webserver - Hauptinteraktionsseite.
Analog zu OTA muss ein Admin-Mode eingeschaltet werden um Konfigurationswerte zu ändern  oder Dateien auf das SPIFS zu laden


### Andere Dateien ###

Auf dem Microcontroler ist ein Teil des Speichers als Filesystem reserviert (SPIFS).
Dort sind Dateien abgelegt, diese Dateien lassen sich über das Web-Interface hochladen oder löschen.

#### top.html ####
Die Hauptseite ("/"). Ist statisch und muss daher nicht im Programm zusammengebaut werden.
Sie verwendet stylesheet und javascript, die eigentlichen Inhalte werden per Frame dazugeladen.
Diese Frames werden im Programm erzeugt.

#### style.css ####
Damit's hübsch aussieht.

#### favicon.ico ####
Wollen alle Browser haben..



