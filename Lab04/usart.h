#define BAUD(x) (((F_CPU/16)/x))

void USART_GetString(char * s);
void USART_PutString(char * s);
void USART_PutChar(char data);
void USART_Init(int baud);
char USART_GetChar(void);
char * Hex(char val, char*str);
