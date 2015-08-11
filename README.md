# TX5823
FPV Sender mit Kanalwahl und konfiguration über SPI. Integriertes OSD mit MAX7456 und Spannungswandlern für 5V, 3,3V und Vorbereitung für 12V, über einen Eingangsspannungsbereich von 8V bis 25.2V.
Das OSD ist kompatibel zu minimosd und wird mit dessen Firmware betrieben und mit der entsprechenden Software konfiguriert.
Die Kanalwahl folgt über einen freien Empfängerkanal.
Zusätzlich ist ein Infrarotauslöser für Canonkameras vorgesehen.

-------------------
Status:
- schaltplan fertig
- layout fertig
- platine bestellt
- Bauteile bestellt
- platine bestückt
- Tests erfolgreich

-------------------
Steckerbelegung 3-reihige Pinleiste(Orientierung Pinleiste nach links, oben 1):

untere Reihe, mittlere Reihe, obere Reihe

1. GND, NC, Video in
2. GND, NC, Sound in
3. GND, NC, PWM in
4. Infrator LED -, NC, Infrarot LED +
5. GND, 5V out, NC
6. GND. 5V out, NC
7. GND, 12V out (optional), NC
8. GND, 12V out (optional), NC
 

-------------------

Steckerbelegung mavlink Stecker

1. DTR (für Reset über USB)
2. TX (Atmega Daten Senden)
3. RX (Atmega Daten Empfangen)
4. 5/3,3V
5. GND
