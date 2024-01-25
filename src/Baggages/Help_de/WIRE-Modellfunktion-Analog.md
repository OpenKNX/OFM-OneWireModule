### Modellfunktion

Für ein Gateway zum KNX ist es schwer, ein solch generisches Gerät zur Verfügung zu stellen, da man üblicherweise komplizierte Formeln braucht, um den eingehenden Analogwert in einen entsprechenden digitalen Wert umzuwandeln. Dies ist um so komplizierter, als dass der gemessene Analogwert immer von der Spannung abhängt, die gerade am DS2438 anliegt und von der Wärme des Chips selbst.

Man kann den DS2438 auf verschiedene Arten verwenden. Über die Auswahlliste Modellfunktion können diese Verwendungsarten festgelegt werden.


Zum einen kann man alle gemessenen Werte des Sensors auf den KNX-Bus senden und in einer KNX-Logik den passenden finalen Sensorwert berechnen. Dazu legt man für einen DS2438 bis zu 4 Geräte an (alle mit der selben ID), jedes Gerät bekommt aber eine andere Modellfunktion und sendet somit verschiedene Werte auf den KNX-Bus:

* Temperatur (On Chip) - sendet die gemessene Temperatur des Bausteins zum Zeitpunkt der Messung
* Spannung vom Chip (VDD) - sendet die Spannung, die am Baustein anliegt, zum Zeitpunkt der Messung
* Spannung vom AD-Wandler - sendet die Spannung, die vom eigentlichen Sensor gemessen wurde (der eigentliche Analogwert).
* Spannung vom 2. AD-Wandler (VSens) - Der DS2438 enthält noch einen 2. AD-Wandler, dessen Spannung kann hier gesendet werden.

Des weiteren kann man einige wenige, bereits fertige 1-Wire-Geräte, die den DS2438 nutzen, an das Modul anschließen. Für diese Geräte ist die Konvertierungsformel bereits implementiert und gibt den konvertierten Wert auf dem zugehörigen KO aus.

* Temperatur (NTC) - ein einfacher NTC ist am Baustein angeschlossen und dessen Wert wird als Temperatur interpretiert.
* Luftfeuchte - ein HIDxxxx wird zur Luftfeuchtemessung verwendet. Die implementierte Formel erlaubt es, die Luftfeuchte vom Multisensor der Firma WireGate korrekt auszuwerten.
* Helligkeit - Der Lichtsensor der Firma WireGate wird hier ausgewertet.
