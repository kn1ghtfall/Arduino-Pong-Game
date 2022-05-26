# Arduino-Pong-Game

Autor: Coconu Robert-Iulian

## Descriere aplicatie
La pornirea jocului ecranul este activat si este pornit jocul PONG in varianta impotriva peretelui. Prin miscarea joystick-ului utilizatorul controleaza o bara laterala cu care incearca sa nu lase mingea sa treaca de el.

Prin apasarea joystick-ului se intra in meniul de setari, pentru trecerea la urmatoarea setare se apasa din nou pe buton. Buzzerul emite sunete atunci cand mingea atinge peretii sau iese in afara arei definite de ecran.

## Hardware Design
Lista componente:
- ARDUINO UNO R3 ATMEGA328P
- ECRAN OLED 0.95”
- BREADBOARD
- MODUL JOYSTICK PS2 COMPATIBIL ARDUINO
- MODUL BUZZER ACTIV
- FIRE

![project]([http://url/to/img.png](https://github.com/kn1ghtfall/Arduino-Pong-Game/blob/main/circuit.png))

![circuit_diagram](http://url/to/img.png)

## Software Design

Aplicatia dispune de optiunile urmatoare: SOUND, PAD_SPEED, BALL_SPEED, SKEW, PAD_SIZE, MULTIPLAYER. Aceste optiuni sunt integrate intr-o `struct` ce contine numele, valoarea minima, valoarea maxima, valoarea prin care incrementam si valoarea curenta a optiunii respective. De exemplu, un element al acestei structuri este de forma: `{“Ball-Speed”, 1, 19, 1, 4}`.

Pentru ciclarea prin meniul de optiuni folosim apasarea joystick-ului, pentru a previne fenomenul de debouncing tinem cont de timpul de la ultima apasare.

Pentru a verifica rezultatul jocului ( daca mingea gaseste un obstacol sau nu ) verificam pozitia acesteia in ecran. Pentru lovirea peretilor de sus si de jos vom inmulti cu -1 factorul cu care incrementam pozitia bilei pe Oy. Pentru lovirea cu paleta jucatorului vom schimba inversa directia pe Ox iar pe Oy vom decide directia in functie de pozitia bilei fata de mijlocul paletei.

## Concluzii

Aplicatia Pong implementata este complexa in felul in care utilizatorul poate sa modifice modul de functionare prin meniul pus la dispozitie. Acest proiect reprezinta o modalitate buna pentru familiarizarea cu comunicarea I2C, folosirea intrerperilor si a ecranelor OLED in aplicatii interactive.
