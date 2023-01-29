# ProgramowanieSystemowMikroprocesorowych
Program był pisany na mikrokontroler ATmega32A zamontowany na płytce uruchomieniowej ZL15AVR.

Zawartość:
- Obsługa zegara czasu rzeczywistego M41T00 przez interfejs I2C(TWI)
- Obsługa termometru cyfrowego TC77 przez interfejs SPI
- Obsługa potencjometru analogowego z wykorzystaniem ADC
- Obsługa wyświetlacza 16x2 HD44780
- Komunikacja z komputerem przez interfejs USART
- System Watchdog
- Tryb uśpienia

W miarę możliwości płytki uruchomieniowej całość została zrealizowana przy pomocy przerwań.

