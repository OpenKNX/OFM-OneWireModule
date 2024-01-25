### Device-Suche

Um die ID von neuen 1-Wire-Geräten herauszufinden, kann man diese mit dem 1-Wire-Anschluss verbinden. Daraufhin wird über das KO "IDs unbekannter Geräte" die ID dieses Gerätes ausgegeben. Dazu muss die Einstellung "neue Geräte-Id auf den Bus senden" ausgewählt sein.


Sobald man alle neuen Geräte an den 1-Wire-Bus angeschlossen hat, sollte man die Einstellung "neue Geräte ignorieren" auswählen und nur so in den produktiven Betrieb gehen.

Die Suche von neuen Geräten auf dem 1-Wire-Bus kostet (relativ zu einer Geräteabfrage) sehr viel Zeit und ist zudem noch Timing-kritisch. Bei eingeschalteter Geräte-Suche ist es sehr wahrscheinlich, dass Signalisierungen von iButton oder Inputs von IO-Bausteinen stark verspätet (>2 Sekunden) oder gar nicht ankommen. Das entsprechende gilt für Outputs.

Daher gilt ganz klar die Empfehlung, dass die Suche nur für den Anschluss neuer Geräte aktiviert werden sollte.

