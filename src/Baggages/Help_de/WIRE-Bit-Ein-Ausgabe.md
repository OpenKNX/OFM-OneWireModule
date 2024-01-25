### Bit-Ein-Ausgabe

Die definition der Ein- und Ausgabe wird vom KNX-Bus aus betrachtet:

* Ausgabe bedeutet, Werte vom KNX-Bus, die im KO landen, werden vom 1-Wire-Gerät an seinen Ausgängen ausgegeben. Somit ist das KO ein Eingang.
* Eingabe bedeutet, dass ein Signal an einem Anschluss vom 1-Wire-Gerät auf den KNX-Bus geschrieben wird. Somit ist das KO ein Ausgang.

Es ist wichtig zu beachten, dass man bitweise Ein- und Ausgänge einstellt, aber immer nur ein Byte auf den KNX-Bus verschickt wird, sobald sich ein Eingangswert ändert. Wenn also eine 7 auf den Bus gesendet wird, werden die Ausgänge 0, 1 und 2 gesetzt. Wird dann der Eingang 5 geändert, wird nicht der Wert 32 auf den Bus gesendet, sondern 39 (32 + 7). Es werden immer alle gesetzten Bits auf den Bus übertragen, egal ob diese Bits über den KNX-Bus oder über die externen Eingänge gesetzt wurden.
