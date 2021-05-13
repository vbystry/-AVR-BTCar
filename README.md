# BTCar

Program do sterowania zdalnego samochodzikiem wyposazonego w 3 czujniki analogowe
Szczegołowe załozenia konstrukcyjne znajdują sie poniżej
 
Samochodzik jest sterowany zdalnie przez bluetooth, przez cyfrowe urządzenie streujące.
Urządzenie sterujące przetwarza dane z czujnikow, samochodzik je tylko wysyła
 
Samochodzik moze mieć ustawiony poziom szybkości, jest to ustawiane przy starcie samochodzika,
pozwala to osobom ostrozniejszym zachować większą kontrolę pojazdu
 
Samochodzik skręca podobnie do czołgow


Założenia konstrukcyjne:

uC: Atmega88PA
Taktowanie zewn. oscylatorem 7.3728MHz
Filtrowanie zasilania przetwornika ADC (AREF=VCC)

Moduł BT: HC-05
Wyjście LED podłączone do PB0 procesora
Pin RXD podłączony do pinu TXD procesora
Pin TXD podłączony do pinu RXD procesora

Przyjmując następujące określenia:
Część sterująca- wejścia podłączone do tranzystorów, określające kierunek obrotów silnika
Część zasilająca- wyjście, które zasilone wprawia silnik w ruch

Dwa mostki H, każdy do sterowania pracą jednego silnika
- Mostek 1:
	- Część zasilająca podłączona do pinu OC0A procesora
	- Część sterująca podłączona do pinów PB1, PB2 procesora
- Mostek 2:
	- Część zasilająca podłączona do pinu OC0B procesora
	- Część sterująca podłączona do pinów PB3, PB4 procesora

Czujniki podłączone do pinów ADC0-2 procesora
