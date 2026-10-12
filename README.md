# Seismotor: trabajo integrador

Trabajo final de **Programación de Microcontroladores** y trabajo integrador de **Protocolos de Comunicación en Sistemas Embebidos** (Especialización en Sistemas Embebidos).

El Seismotor es un estetoscopio de motor. Este firmware captura el sonido con un micrófono digital INMP441 y lo manda a la PC, que lo guarda como `.wav`. El pulsador B1 empieza y termina la grabación, y un LCD 1602 muestra el estado y los segundos grabados.

```
 INMP441 ──I2S──► NUCLEO-F446RE ──UART (ST-Link)──► PC: tools/record_wav.py → .wav
                       │
                       ├──I2C──► LCD 1602 (PCF8574)
                       └── B1
```

La explicación completa del código está en [`docs/como-funciona.md`](docs/como-funciona.md).

## Hardware

- NUCLEO-F446RE (reloj HSI de 16 MHz).
- Micrófono INMP441 (I2S).
- LCD 1602 con módulo I2C PCF8574 (dirección 0x27).

| INMP441 | Nucleo |
|---|---|
| SCK | PB13 (CN10-30) |
| WS | PB12 (CN10-16) |
| SD | PB15 (CN10-26) |
| L/R | GND (canal izquierdo) |
| VDD | **3V3** |
| GND | GND |

| LCD (PCF8574) | Nucleo |
|---|---|
| SCL | PB8 (D15) |
| SDA | PB9 (D14) |
| VCC | 5V |
| GND | GND |

## Periféricos

| Periférico | Uso | Configuración |
|---|---|---|
| I2S2 + DMA1 Stream3 | Micrófono | Master RX, Philips, 32 bits, 16 kHz; DMA circular a un doble buffer de 2 × 512 muestras (32 ms cada mitad) |
| USART2 | Audio a la PC | 500 000 baud, 8N1, por el puerto serie virtual del ST-Link |
| I2C1 | LCD | 100 kHz |
| GPIO PC13 | Pulsador B1 | Entrada, antirrebote de 40 ms |

## Estructura

```
Core/Inc, Core/Src     main.c (CubeMX) y app.c: la MEF de la aplicación
Drivers/API/Inc, Src   módulos propios, de la cátedra y de las prácticas
tools/record_wav.py    graba en la PC el audio que llega por la UART
docs/                  cómo funciona el código
inmp441.ioc            configuración de CubeMX
```

| Módulo | Origen | Qué hace |
|---|---|---|
| `app` | Propio | MEF: INIT → IDLE ⇄ STREAMING |
| `API_mic` + `API_mic_port` | Propio | Driver del micrófono por I2S con DMA |
| `API_lcd` + `API_lcd_port` | Cátedra (Ing. Israel Pavelek), adaptado | Driver del LCD 1602 por I2C |
| `API_uart` | Práctica 5, adaptado | Envío por USART2 |
| `API_debounce` | Práctica 4 | Antirrebote de B1 |
| `API_delay` | Práctica 3 | Retardos no bloqueantes |

## Drivers

Los dos drivers de periféricos externos están separados en un **driver** genérico y un **port**. Solo el port incluye la HAL; para usar el driver en otro microcontrolador alcanza con reescribir el port.

### Micrófono INMP441 (`API_mic`)

```c
bool_t micInit(void);              // arranca la captura I2S por DMA; queda corriendo siempre
bool_t micBlockReady(void);        // true si hay un bloque nuevo sin leer
const int16_t* micGetBlock(void);  // 512 muestras PCM de 16 bits (32 ms), o NULL
uint32_t micDropped(void);         // bloques que se pisaron sin leer
```

- **Port** (`API_mic_port`): `micPortStart(buf, frames, onHalf)` arranca `HAL_I2S_Receive_DMA` en modo circular. Los callbacks de la HAL (`HAL_I2S_RxHalfCpltCallback` y `HAL_I2S_RxCpltCallback`) están en el port y avisan al driver con el puntero a función `onHalf`.
- **Driver** (`API_mic`): anota qué mitad del buffer está lista y, cuando la aplicación la pide, toma el canal izquierdo, lo pasa de 24 a 16 bits con saturación y la marca como leída.

### LCD 1602 por I2C (`API_lcd`)

```c
_Bool Init_Lcd(void);                 // inicialización en modo 4 bits
void SacaTextoLcd(uint8_t *texto);    // escribe un texto desde la posición actual
void PosCaracHLcd(uint8_t posH);      // cursor en la línea de arriba
void PosCaracLLcd(uint8_t posL);      // cursor en la línea de abajo
void ClrLcd(void);                    // borra el display
```

- **Port** (`API_lcd_port`): `LCD_Write_Byte()` manda un byte al PCF8574 por I2C1 y `LCD_Delay()` hace las esperas.
- **Driver** (`API_lcd`): arma los nibbles con las señales RS, EN y la luz de fondo.
- **Cambio respecto del original:** se sacaron las esperas de 1 ms después de cada nibble. Cada escritura I2C ya tarda ~0.2 ms, y con las esperas cada carácter tardaba ~8 ms y se perdía audio.

## Compilar

El repo no incluye los archivos del IDE (`.project`, `.cproject`). Para abrirlo en STM32CubeIDE:

1. Abrir `inmp441.ioc` con STM32CubeMX (o CubeIDE) y generar el código con *Toolchain / IDE* = STM32CubeIDE. El código propio está en los bloques `USER CODE`, así que se conserva.
2. Importar la carpeta en CubeIDE (*File → Import → Existing Projects into Workspace*).
3. Agregar `Drivers/API/Inc` a los *include paths* del compilador (*Properties → C/C++ Build → Settings → MCU GCC Compiler → Include paths*).

## Grabar

1. Conectar la Nucleo por USB y flashear el firmware. El LCD muestra `B1 para grabar`.
2. En la PC (Python 3, sin dependencias externas):

   ```
   python3 tools/record_wav.py [segundos] [salida.wav]
   ```

   Sin argumentos, espera el audio y corta cuando deja de llegar.
3. Apretar B1 para empezar (el LCD muestra `Grabando` y los segundos) y B1 de nuevo para terminar.

Funciona en macOS (`/dev/cu.usbmodem*`) y en Linux (`/dev/ttyACM*`).
