# pico-sd-ctrl

## Descrição
Controlador de cartão SD para uso no projeto [FPGA-SOM](https://github.com/PCS-Poli-USP/projeto-final-projeto-43) de LabDigi A, 2023, usando uma Raspberry Pi Pico, interagindo com uma FPGA [DE0-CV](https://www.terasic.com.tw/cgi-bin/page/archive.pl?Language=English&CategoryNo=163&No=921) da Terasic por meio do protocolo SPI, onde o MCU é o escravo.

Por meio de faixas em `.wav` armazenadas no diretório *raíz* do cartão SD, o MCU transmite os dados da faixa atual para a placa FPGA, onde ela será tocada por meio de um DAC de 1 bit.

Para controlar qual faixa está sendo enviada, a placa envia algum dos possíveis comandos abaixo, um byte, por meio da linha MOSI:

### Comandos
- `0x00`: OK (ignorar).
- `0x01`: PAUSE (para de enviar a faixa).
- `0x02`: SKIP (começa a transmitir a próxima faixa).
- `0x03`: RETURN (passa para a faixa anterior).

Alternativamente, é possível colocar o Chip Select em alto, pois a Pico é ativa em baixo, para para de enviar a faixa.

## Recursos utilizados

- 1 Raspberry Pi Pico. Nesse caso, será utilizado uma [RP2040-Zero](https://www.waveshare.com/wiki/RP2040-Zero).
- 2 resistores de 10K ohms para pull-up.
- 1 módulo leitor de cartão SD.

## Firmware

Siga as instruções de [Getting started with Raspberry Pi Pico](https://datasheets.raspberrypi.org/pico/getting-started-with-pico.pdf) para instalar a SDK.

Instale as dependências com o comando abaixo (esse exemplo assume que seu sistema usa o `apt`):

```console
$ sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi libstdc++-arm-none-eabi-newlib
```

Clone o repositório e monte o executável, substituindo `path_da_sdk` para o caminho da SDK da pico e `nome_da_placa` para a Pico utilizada:

```console
$ git clone https://github.com/gabrielpzt/pico-sd-ctrl.git
$ cmake -S. -Bbuild -DPICO_SDK_PATH=path_da_sdk -DBOARD_NAME=nome_da_placa
$ cd build
$ cmake --build .
```

## Bibliotecas utilizadas
- [no-OS-FatFS-SD-SPI-RPi-Pico](https://github.com/carlk3/no-OS-FatFS-SD-SPI-RPi-Pico01), para a interface com o cartão SD.
- [FatFs](http://elm-chan.org/fsw/ff/00index_e.html), para interagir com o FAT32.
