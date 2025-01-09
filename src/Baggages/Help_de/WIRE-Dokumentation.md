### Dokumentation


Eine vollständige Applikationsbeschreibung ist unter folgendem Link verfügbar: https://github.com/OpenKNX/OFM-OneWireModule/blob/v1/doc/Applikationsbeschreibung-Wire.md


Diese Applikation erlaubt es, 1-Wire-Geräte, die über den 1-Wire-Bus mit der vorliegenden Hardware verbunden sind, passend zu parametrisieren und deren Werte auf den KNX-Bus zu senden. Falls die 1-Wire-Geräte Ausgaben erlauben, ist es auch möglich, KNX-Telegramminhalte an den Ausgängen der 1-Wire-Geräte verfügbar zu machen.

Auch wenn diese Anleitung für das WireGateway ist, wird die gleiche Applikation auch für das Sensormodul verwendet und kann hierfür herangezogen werden. 

Der 1-Wire-Bus ist so entworfen, dass alle angeschlossenen Geräte immer wieder abgefragt werden müssen, ob sie neue Werte haben (polling). Da diese Abfragen einige Zeit benötigen, ist der 1-Wire-Bus nicht dazu geeignet, kurzfristige Ereignisse (Zählimpulse, kurze Tastendrücke) zu erfassen oder unmittelbar auf so ein Ereignis zu reagieren. Aus diesem Grunde bietet die KNX-Applikation auch nur Funktionalitäten an, die sich für Sensorabfragen bzw. Fensterkontakte eignen, also alles keine zeitkritischen Ereignisse.

