### **Device-Suche**

Um die ID von neuen 1-Wire-Geräten herauszufinden, kann man diese mit dem 1-Wire-Anschluss verbinden. Daraufhin wird über das KO20 "IDs unbekannter Geräte" die ID dieses Gerätes ausgegeben. Dazu muss die Einstellung "neue Geräte-Id auf den Bus senden" ausgewählt sein.

Die ausgegebene ID ist vom Typ DPT16 und kann im Gruppenmonitor ausgelesen werden. Das Telegramm ist einfach zu erkennen, es hat rechts in der Spalte Info folgendes Aussehen:
![Unbekannte Id](pics/UnbekannteID.png)
Dabei ist der Wert des Telegramms wichtig, im angezeigten Beispiel sind es die Zeichen ganz am Ende der Zeile hinter dem senkrechten Strich "|":

    108BE4A8030800

Diese Id ist später für die ID-Eingabe wichtig. Dabei bilden die ersten beiden Stellen (im Beispiel die 10) die Gerätefamilie ab. Die Folgenden Stellen müssen dann über passende Auswahlfelder eingegeben werden. Die obige ID sieht dann in der Applikation so aus:
![Beispiel-ID](pics/beispiel-id.png)

Sobald man alle neuen Geräte an den 1-Wire-Bus angeschlossen hat, sollte man die Einstellung "neue Geräte ignorieren" auswählen und nur so in den produktiven Betrieb gehen.

Die Suche von neuen Geräten auf dem 1-Wire-Bus kostet (relativ zu einer Geräteabfrage) sehr viel Zeit und ist zudem noch Timing-kritisch. Bei eingeschalteter Geräte-Suche ist es sehr wahrscheinlich, dass Signalisierungen von iButton oder Inputs von IO-Bausteinen stark verspätet (>2 Sekunden) oder gar nicht ankommen. Das entsprechende gilt für Outputs.

Daher gilt ganz klar die Empfehlung, dass die Suche nur für den Anschluss neuer Geräte aktiviert werden sollte.

