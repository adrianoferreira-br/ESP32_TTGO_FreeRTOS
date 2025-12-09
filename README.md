# ESP32_TTGO_FreeRTOS

* Testes de estudo com ESP32 + FreeRTOS *

## Firmware original da placa
https://github.com/Xinyuan-LilyGO/TTGO-T-Display
https://github.com/Xinyuan-LilyGO/TTGO-T-Display?spm=a2g0o.detail.1000023.2.5799BGU3BGU3y3

## Informação da placa:
https://www.smartkits.com.br/ttgo-t-display-v1-1-esp32-com-display-ips-1-14-colorido-16mb
https://lilygo.cc/products/lilygo%C2%AE-ttgo-t-display-1-14-inch-lcd-esp32-control-board?form=MG0AV3
![ESP32_TTGO_T-Display](doc/pinmap_t-display.jpg)

## Informações do processador
https://www.mouser.com/datasheet/2/891/Espressif_Systems_01292021_esp32-1991551.pdf
https://en.wikipedia.org/wiki/ESP32
informações [link](./doc/Espressif_Systems_01292021_esp32-1991551.pdf).
https://en.wikipedia.org/wiki/ESP32

## Configuração da placa no platformio.ini
https://docs.platformio.org/en/latest/boards/espressif32/lilygo-t-display.html
https://docs.platformio.org/en/latest/boards/espressif32/lilygo-t-display.html?utm_source=platformio&utm_medium=piohome
https://github.com/platformio/platformio-core?utm_source=platformio&utm_medium=piohome



## Display:

- Alterar arquivo 'User_Setup_Select.h', descomentar a linha: 
  ```
  #include <User_Setups/Setup25_TTGO_T_Display.h>
  ```
- Alterar a resolução, no arquivo 'User_Setup.h'
  ```
  #define ST7789_DRIVER  
  #define TFT_WIDTH  240 
  #define TFT_HEIGHT 320 
  ```
- 

## Informação do PlatformIO usando FreeRTOS:
https://docs.platformio.org/en/latest/tutorials/index.html


## Memória 16MB
Utilizar a seguinte informação no platformio.ini
```
 - board_upload.flash_size = 16MB
 - board_build.partitions = partitions.csv
```  
  Adicionar o arquivo partitions.csv no mesmo diretório do platformio.ini com a sequinte informação de particionamento:
```
#Name,   Type, SubType, Offset,  Size, Flags
nvs,      data, nvs,     0x9000,  0x5000,
otadata,  data, ota,     0xe000,  0x2000,
app0,     app,  ota_0,   0x10000, 0x700000,
app1,     app,  ota_1,   0x710000,0x700000,
spiffs,   data, spiffs,  0xE10000,0x1F0000,
```

comando detecta memória:
```
python3.11.exe C:\Users\Adriano\.platformio\packages\tool-esptoolpy\esptool.py flash_id
```


## Realizando o OTA

Tipos e Subtipos de Partições
Type 0: Indica uma partição de dados (data).
Subtype 16: Indica uma partição de aplicativo OTA (ota_0).

- Incluir no Platformio.ini informaçoes para gravação via OTA:
```
upload_protocol = espota
upload_port = 192.168.100.109  # Substitua pelo endereço IP do seu ESP32
upload_flags =
    --timeout=30 # Definir o tempo limite de conexão para 30 segundos
    --port=3232  # Definir a porta OTA para 3232
```

## teste com PIO
- colocar variável de ambiente para o path: C:\Users\NNNNNNNNNN\.platformio\penv\Scripts
```  
- C:\Users\Adriano\.platformio\penv\Scripts\pio run --target upload   (compila e faz upload para placa)
- C:\Users\Adriano\.platformio\penv\Scripts\pio run --target upload --upload-port <IP_ADDRESS_OF_ESP32>
```

## Mapeamento de Pinos

### **Pinos em Uso:**

| GPIO | Função | Sensor/Módulo | Tipo |
|------|--------|---------------|------|
| **4** | TFT_BL | Display Backlight | Saída |
| **5** | TFT_CS | Display Chip Select | Saída |
| **12** | BATIDA_PIN | Sensor de Batida (Prensa) | Entrada + Interrupção |
| **16** | TFT_RESET | Display Reset | Saída |
| **17** | TFT_DC | Display Data/Command | Saída |
| **18** | TFT_SCLK | Display SPI Clock | Saída |
| **19** | TFT_MOSI | Display SPI MOSI | Saída |
| **21** | DHTPIN | DHT22 (Temperatura/Umidade) | Entrada Digital |
| **23** | TFT_MISO | Display SPI MISO | Entrada |
| **26** | ULTRASONIC_TRIG | JSN-SR04T Trigger | Saída |
| **27** | ULTRASONIC_ECHO | JSN-SR04T Echo | Entrada |
| **32** | MLX90614_SDA | MLX90614 I2C Data | I2C |
| **33** | MLX90614_SCL | MLX90614 I2C Clock | I2C |
| **35** | BUTTON_35 / VBAT_PIN | Botão Config + Tensão Bateria | Entrada + ADC |
| **36** | SHUNT_ADC_PIN | Resistor Shunt (Corrente) | ADC (Input Only) |
| **37** | Pulse Counter | Fluxo Ar/Água (reservado) | Entrada |
| **38** | Sensor Reflexivo | (reservado no código) | Entrada |

### **Pinos Livres (Expostos na Placa):**

| GPIO | Características | Restrições |
|------|----------------|------------|
| **25** | ADC2, DAC | Conflita com WiFi ativo |
| **39 (VN)** | ADC1, Input Only | Sem pull-up/down interno |
| **0** | Boot Mode | Pull-up necessário |
| **2** | LED Onboard | ADC2, conflita com WiFi |
| **13** | GPIO genérico | - |
| **14** | GPIO genérico | - |
| **15** | GPIO genérico | - |
| **22** | GPIO genérico | Pode ser usado para I2C SCL |

### **Pinos Não Acessíveis (Uso Interno):**
- GPIO 1, 3 (UART TX/RX - USB Serial)
- GPIO 6-11 (Flash SPI - Memória Interna)
- GPIO 34 (não exposto fisicamente na placa)

### **Resumo de Sensores Configurados:**
1. ✅ Display TFT ST7789 (SPI - GPIOs 4, 5, 16, 17, 18, 19, 23)
2. ✅ DHT22 - Temperatura/Umidade (GPIO 21)
3. ✅ JSN-SR04T - Sensor Ultrassônico (GPIO 26/27)
4. ✅ MLX90614 - Temperatura Infravermelho (I2C GPIO 32/33)
5. ✅ Sensor de Batida - Prensa (GPIO 12)
6. ✅ Tensão da Bateria (GPIO 35 - ADC1)
7. ✅ Resistor Shunt - Medição de Corrente (GPIO 36 - ADC1)

## Notas adicionais
>[!NOTE]
> teste note

>[!WARNING]
> teste warning