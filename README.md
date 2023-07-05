# pico-sd-ctrl

## Descrição
Controlador de cartão SD para uso no projeto [FPGA-SOM](https://github.com/PCS-Poli-USP/projeto-final-projeto-43) de LabDigi A, 2023, usando uma Raspberry Pi Pico, interagindo com uma FPGA [DE0-CV](https://www.terasic.com.tw/cgi-bin/page/archive.pl?Language=English&CategoryNo=163&No=921) da Terasic por meio de pinos GPIO.

Por meio de faixas em `.wav` armazenadas no diretório *raíz* do cartão SD, o MCU usa um DAC de 1 bit para reproduzi-las.

Para controlar qual faixa está sendo enviada, a placa coloca um dos três GPIO configurados nela em alto, acarretando as seguintes funções:

### Funcionalidades
- PAUSE (para de enviar a faixa).
- SKIP (começa a transmitir a próxima faixa).
- RETURN (passa para a faixa anterior).

## Recursos utilizados

- 1 Raspberry Pi Pico. Nesse caso, será utilizado uma [RP2040-Zero](https://www.waveshare.com/wiki/RP2040-Zero).
- 1 módulo leitor de cartão SD.
- Filtro passa-baixas para remover o ruído (detalhado no projeto).

## Firmware

Siga as instruções de [Getting started with Raspberry Pi Pico](https://datasheets.raspberrypi.org/pico/getting-started-with-pico.pdf) para instalar a SDK.

Instale as dependências com o comando abaixo (esse exemplo assume que seu sistema usa o `apt`):

```console
$ sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib
```

Clone o repositório e monte o executável, substituindo `path_da_sdk` para o caminho da SDK da pico e `nome_da_placa` para a Pico utilizada:

```console
$ git clone https://github.com/gabrielpzt/pico-sd-ctrl.git
$ mkdir build && cd build
$ cmake .. -DPICO_SDK_PATH=path_da_sdk -DPICO_BOARD=nome_da_placa
$ cmake --build .
```

## Bibliotecas utilizadas
- [no-OS-FatFS-SD-SPI-RPi-Pico](https://github.com/carlk3/no-OS-FatFS-SD-SPI-RPi-Pico/tree/d587243b63c19ddb540e9ae20b2d74eed1921382), para a interface com o cartão SD.
- [FatFs](http://elm-chan.org/fsw/ff/00index_e.html), para interagir com o FAT32.
