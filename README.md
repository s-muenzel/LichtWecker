# LichtWecker #

## Intro ##

Lichtwecker soll zu gegebenen Zeiten langsam von dunkel/rötlich auf hell/weiss-bläuliches Licht aufblenden.
Die Einstellungen gehen über ein kleinew Web-Interface.

## Hardware ##

### Microcontroler ###
Der Code ist für ein ESP8266 entwickelt.
Entweder ein ´Sonoff Basic´, bei den gängigen Versionen ist GPIO Pin 14 relativ einfach abgreifbar.
    Habe aber auch ein Exemplar erwischt, bei dem anstatt GPIO 14 ein GPIO Pin 2 an der Stelle auf dem PCB ist
	_ACHTUNG_: dann muss man ggfs. den Pin auf VCC hochziehen, sonst bootet der Sonoff nicht, der Wert des Pullup-Widerstands hängt von der verwendeten LED-Kette ab
    bei mir waren es ca 2k.

Alternativ geht auch ein ESP 01(-S), dann mit Pegelkonverter. Da dann ohne Taster zum Snooze bzw. auschalten, sondern nur per Web.


### LEDs ###
Eine WS2812(xx), die Anzahl der LEDs iswt konfigurierbar.

### Stromversorgung ###
Der WS2812B-Strip zieht bei 30 LEDs ca 0.9A bei 5V, bei mehr LEDs natürlich analog mehr Strom.

Daher habe ich bei einmal hinter dem ´Sonoff Basic´ ein LED-Netzteil gehängt. Bis das allerdings die Spannung liefert, dauert es. Daher gibt es einen Parameter,
wie lange verzögert wird bevor ein Signal an den Strip geschickt wird.

Bei einer anderen, einfacheren Ausführung (mir ESP 01-S) hängen Microcontroler und Strip an einem einfachen 2A-USB-Netzteil, da brauchst auch keine Verzögerung.



