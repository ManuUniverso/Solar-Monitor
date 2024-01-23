/* Solar Monitor v0.1.2 ManuUniverso 2024

It is the first version of the program, complete or practically complete.
This version is still unstable and may have bugs or incomplete functionality.

Using an ESP32, the data is read from the VE.Direct ports (Victron Energy brand) of the Inverter and MPPT.
Then display the results on a color ili9341 screen.
Taking advantage of the available touchscreen you can enter the submenu of each device.

Much of the comments are in Spanish, you can translate but, the symbolic name of 
variables is often quite intuitive and easy to associate.

========================================================================================================
======================= OPEN SOURCE LICENSE ============================================================
========================================================================================================

Created and designed by ManuUniverso

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
including, the rights to use, copy, modify, merge, publish, distribute, sublicense, and allow persons to whom the Software is provided to do so, subject to the following conditions:

The above copyright notice and this permission notice will be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING OUT OF, OUT OF OR IN CONNECTION WITH THE SOFTWARE
OR THE USE OR OTHER DEALINGS OF THE SOFTWARE.

............................................................................................................
Release history
............................................................................................................
Version Date Author Comment
0.1.0 01/01/24 First tested OK
0.1.1 14/01/24 Added touch function with submenu
0.1.2 23/01/24 small bug & Current Sensor HCS LSP 20A for Charger (without ve direct)

   *****************************************************************************
   The reading of the port was achieved thanks to the fantastic contribution of Âld Skânser:
   ------->https://www.youtube.com/@aldskanser7880
   ------->https://www.romlea.nl/Arduino%20MPPT/Page.htm

   This is an example of monitoring a solar energy system with products from
   the Victron Energy brand that has a VE.Direct port.
  
   Using two UART ports of the ESP32, VE.Direct data reading is carried out
   of the inverter and mppt.
   Attention, it is necessary to use a logic level converter circuit
   to adapt the logic level to the ESP32 3.3v.
   Attention, it is necessary to use an isolator circuit to separate correctly
   the GNDs to avoid ground loops or other problems.
   Using a 30pin ESP32 devkit v1 node32s & Isolator Based on chipset π122U31

   In this example an ili9341 SPI TFT LCD display is used.
   The touch part is made possible with the XP2046 resistive ADC chip.
   It can only detect one touch at a time.
   The SD card reader it has is independent and uses SPI but in
   This version of the code is not used.

   The libraries as well as diagrams should be provided with this code for their correct use.
   -"ResetAnim.h""MenuHome.h""MenuSolar.h""MenuCharger.h" "MenuInverter.h""MenuBat.h""MenuLoad.h"
   - SolarMonitor0.1.1a.jpg diagrams & pin connection
     .ESP32 Pin image from https://www.LastMinuteEngineers.com
     .ili9341 image from https://bidouilleur.ca/

   The board Available direcetly from the Arduino IDE v2 
   - arduino-esp32    "Select USB correct"  & "node32s"
   The libraries Available directly from the Arduino IDE v2 library manager.
   -XPT2046_Touchscreen.h
   -Adafruit_ILI9341.h
   -Adafruit_GFX.h
   -Fonts/FreeSansBold9pt7b.h No need to download, it is included in the previous ones.
   -Fonts/FreeSansBold18pt7b.h No need to download, it is included in the previous ones.
   Pin & Config:
   -Current Sensor Model ZY264-20A HCS LSP 20A voltage supply will affect the reading value 
   -Current Sensor Compatible with 5v but used with 3.3 it is capable of measuring current in both directions.
   -Current Sensor 3.3v will affect the reading value  you must calibrate the two values CurrSensRAW
   -Current Sensor GND ( from ESP32 )
   -Current Sensor OUT ---> GPIO33
   -ESP32 UART1 rate 19200             
   -ESP32 UART2 rate 19200                                                  
   -ESP32 Pin 27 UART1 RX <-- VE.Direct Inverter TX 
   -ESP32 Pin 2   --- TFT_DC    image
   -ESP32 Pin 15  --- TFT_CS    image
   -ESP32 Pin 23  --- TFT_MOSI  image
   -ESP32 Pin 18  --- TFT_CLK   image
   -ESP32 Pin 4   --- TFT_RST   image
   -ESP32 Pin 19  --- TFT_MISO  image
   -ESP32 Pin 21  --- CS_PIN    Touchscreen
   -ESP32 Pin 34  --- TIRQ_PIN  Touchscreen      
   *****************************************************************************
*/

#include <XPT2046_Touchscreen.h>                           // ili9341 Touch
#include "Adafruit_ILI9341.h"                              // ili9341 Imagen
#include "Adafruit_GFX.h"                                  // ili9341 Fuentes Rectangulos Circulos
#include <Fonts/FreeSansBold9pt7b.h>                       // ili9341 fuente   <Fonts/FreeSansBold12pt7b.h><Fonts/FreeSansBold24pt7b.h>	
#include <Fonts/FreeSansBold18pt7b.h>                      // ili9341 fuente   <Fonts/FreeSansBold12pt7b.h><Fonts/FreeSansBold24pt7b.h>	 
#include "ResetAnim.h"                                     // Imagen para las animaciones
#include "MenuHome.h"                                      // Imagen Background del menu Home
#include "MenuSolar.h"                                     // Imagen Background del menu Solar
#include "MenuCharger.h"                                   // Imagen Background del menu Charger
#include "MenuInverter.h"                                  // Imagen Backgorund del menu Inveter
#include "MenuBat.h"                                       // Imagen Background del menu Bateria
#include "MenuLoad.h"                                      // Imagen Background del menu Load

#define UARTRate1 19200                                    /// VE.Direct Pin Inverter
#define UART1rx 27                                         /// VE.Direct Pin Inverter TX---> RX Esp32
#define UART1tx 14                                         /// VE.Direct Pin Inverter RX---> TX Esp32 en este codigo no se usa (solo se hace lectura con RXEsp32)
#define UARTRate2 19200                                    /// VE.Direct Pin MPPT  no es necesario asignar, usar los pines previsto RX2 y TX2 en el ESP32
#define ResetLoopDefault 400                               /// VE.Direct Pin Data Reset Cuenta atras para reseteo      los 3 deben ser igual  ResetMPTData ResetIVTData ResetLoopDefault
#define CurrSensPin 33                                     /// Current Sensor Pin charger control GPIO33 ADC1_5
#define CurrSensRAWZero 2899                               /// Current Sensor RAWvalue 0 Amp (afectado por la tension de alimentacion del sensor)
#define CurrSensRAWUnit 0.006                              /// Current Sensor RAWvalue 1 Amp (afectado por la tension de alimentacion del sensor) /147.1 por unidad que es lo mismo que multiplicar por eso 0.006...
#define TFT_DC 2                                           /// Display ili9341 pin para imagen
#define TFT_CS 15                                          /// Display ili9341 pin para imagen
#define TFT_MOSI 23                                        /// Display ili9341 pin para imagen (no se especifica en "Adafruit_ILI9341 tft" para evitar el [SPI Software limitado a 400Khz Max]; es mejor [SPI Material integrada puede alcanzar Mhz]
#define TFT_CLK 18                                         /// Display ili9341 pin para imagen  
#define TFT_RST 4                                          /// Display ili9341 pin para imagen
#define TFT_MISO 19                                        /// Display ili9341 pin para imagen
#define CS_PIN  21                                         /// Display ili9341 pin para Touch GPIO35
#define TIRQ_PIN  34                                       /// Display ili9341 pin para Touch GPIO34 [pendiente probar no usado especificamente en esta version]
#define TxtSize1 &FreeSansBold9pt7b                        /// Display ili9341 Fonts
#define TxtSize2 &FreeSansBold18pt7b                       /// Display ili9341 Fonts
#define CirclRad  3                                        /// Imagen Animacion Bola tamaño Radio
#define CirclColor 0x4C5A                                  /// Imagen Animacion Bola Color Azul Victron Energy efecto de la bola animada por el cable
#define TextColorBlue 0x4C5A                               /// Imagen Text Color Azul         Menu Home Azul Victron Energy
#define TextColorRed 0xc1c5                                /// Imagen Text Color Rojo         Menu Home de Charger
#define TextColorOrange 0xf4c2                             /// Imagen Text Color Naranja      Menu Home de Solar & MPPT
#define TextColorGreen 0x254b                              /// Imagen Text Color Verde        Menu Home de Load enchfes de casa alimantados por Inverter 0x256c 0x254b 
#define TextColorWhite ILI9341_WHITE                       /// Imagen Text Color Blanco       para escribir sobre fondo de colores o borrar sobre fondo blanco
#define TextColorBlack ILI9341_BLACK                       /// Imagen Text Color Black        escribir sobre fondo blanco
#define LedRad 4                                           /// Imagen Led Estado tamaño Radio ( simula un led en el display )
#define LedMPPTX 221                                       /// Imagen Menu Home MPPT Led estado Cursor eje X
#define LedMPPTY 115                                       /// Imagen Menu Home MPPT Led estado Cursor eje Y
#define LedIVTX 160                                        /// Imagen Menu Home Inverter Led estado Cursor eje X
#define LedIVTY 37                                         /// Imagen Menu Home Inverter Led estado Cursor eje Y
#define ChrgBatStrtX 56                                    /// Imagen Menu Home Animacion Bola de Charger a Battery eje X
#define ChrgBatFinY 129                                    /// Imagen Menu Home Animacion Bola de Charger a Battery eje Y
#define ChrgInvrtStrtY 52                                  /// Imagen Menu Home Animacion Bola de Charger a Inverter eje Y
#define ChrgInvrtFinX 126                                  /// Imagen Menu Home Animacion Bola de Charger a Inverter eje X
#define LoadStrtY 52                                       /// Imagen Menu Home Animacion Bola de Inverter a Load eje Y
#define LoadFinX 220                                       /// Imagen Menu Home Animacion Bola de Inverter a Load eje X
#define BATStrtY 150.5                                     /// Imagen Menu Home Animacion Bola de Bateria a Inverter eje Y
#define SolarStrtY 150.5                                   /// Imagen Menu Home Animacion Bola de Solar a Bateria eje Y
#define SolarFinX 162                                      /// Imagen Menu Home Animacion Bola de Solar a Bateria eje X
#define InvrtStrtX  160                                    /// Imagen Menu Home Animacion Bola de Solar+Bateria a Inverter eje X    
#define InvrtFinY 92                                       /// Imagen Menu Home Animacion Bola de Solar+Bateria a Inverter eje Y   
#define ChrgBatStrtYDef 83                                 /// Imagen Menu Home Animacion Bola de Charger a Bateria eje Y          Range
#define ChrgInvrtStrtXDef 99                               /// Imagen Menu Home Animacion Bola de Charger a Inverter eje X         Range
#define LoadStrtXDef 192                                   /// Imagen Menu Home Animacion Bola de Inverter a Load eje X            Range
#define BATStrtXDef 99                                     /// Imagen Menu Home Animacion Bola de Bateria a Inverter eje X         Range
#define BATFinXDef 162                                     /// Imagen Menu Home Animacion Bola de Solar a Bateria eje X            Range
#define SolarStrtXDef 220                                  /// Imagen Menu Home Animacion Bola de Solar Bateria Inverter eje X     Range
#define InvrtStrtYDef 150                                  /// Imagen Menu Home Animacion Bola de Solar Bateria Inverter eje Y     Range
#define TchHomeXmin 1600                                   /// Al tocar Go Back return to Home Cursor X Min   if ((p.x > TchHomeXmin && p.x < TchHomeXmax) && (p.y > TchHomeYmin && p.y < TchHomeYmax)) {
#define TchHomeXmax 2600                                   /// Al tocar Go Back return to Home Cursor X Max
#define TchHomeYmin 340                                    /// Al tocar Go Back return to Home Cursor Y Min
#define TchHomeYmax 1380                                   /// Al tocar Go Back return to Home Cursor Y Max
#define TchSolarXmin 360                                   /// Al tocar Go Menu Solar Cursor X min            if ((p.x > TchSolarXmin && p.x < TchSolarXmax) && (p.y > TchSolarYmin && p.y < TchSolarYmax)) {
#define TchSolarXmax 1400                                  /// Al tocar Go Menu Solar Cursor X max
#define TchSolarYmin 400                                   /// Al tocar Go Menu Solar Cursor Y min
#define TchSolarYmax 2230                                  /// Al tocar Go Menu Solar Cursor Y max
#define TchBatXmin 2870                                    /// Al tocar Go Menu Bateria Cursor X min          if ((p.x > TchBatXmin && p.x < TchBatXmax) && (p.y > TchBatYmin && p.y < TchBatYmax)) {
#define TchBatXmax 3850                                    /// Al tocar Go Menu Bateria Cursor X max 
#define TchBatYmin 350                                     /// Al tocar Go Menu Bateria Cursor Y min 
#define TchBatYmax 1820                                    /// Al tocar Go Menu Bateria Cursor Y max 
#define TchChrgXmin 2800                                   /// Al tocar Go Menu Charger Cursor X min          if ((p.x > ChrgXmin && p.x < ChrgXmax) && (p.y > ChrgYmin && p.y < ChrgYmax)) {
#define TchChrgXmax 3850                                   /// Al tocar Go Menu Charger Cursor X max
#define TchChrgYmin 2600                                   /// Al tocar Go Menu Charger Cursor Y min
#define TchChrgYmax 3740                                   /// Al tocar Go Menu Charger Cursor Y max
#define TchInvrtXmin 1670                                  /// Al tocar Go Menu Inverter Cursor X min         if ((p.x > TchInvrtXmin && p.x < TchInvrtXmax) && (p.y > TchInvrtYmin && p.y < 1TchInvrtYmax)) {
#define TchInvrtXmax 2560                                  /// Al tocar Go Menu Inverter Cursor X max 
#define TchInvrtYmin 2450                                  /// Al tocar Go Menu Inverter Cursor Y min 
#define TchInvrtYmax 3740                                  /// Al tocar Go Menu Inverter Cursor Y max 
#define TchLoadXmin 360                                    /// Al tocar Go Menu Load Cursor X min             if ((p.x > TchLoadXmin && p.x < TchLoadXmax) && (p.y > TchLoadYmin && p.y < TchLoadYmax)) {
#define TchLoadXmax 1400                                   /// Al tocar Go Menu Load Cursor X max 
#define TchLoadYmin 2600                                   /// Al tocar Go Menu Load Cursor Y min 
#define TchLoadYmax 3740                                   /// Al tocar Go Menu Load Cursor Y max 
#define VBatdataErrorL 18.4                                /// Bateria Control si el dato volt bat es inferiro se toma como un error de lectura
#define VBatdataErrorH 35.4                                /// Bateria Control si el dato volt bat es superior se toma como un error de lectura
#define LowBat 25.4                                        /// Bateria Control volt que considera minimo en su uso normal
#define HighBat 28.4                                       /// Bateria Control volt que considera maximo en su uso normal
#define BatLevel1 25.6                                     /// Bateria Control volt que considera 1/4 de nivel de carga
#define BatLevel2 26.0                                     /// Bateria Control volt que considera 2/4 de nivel de carga
#define BatLevel3 26.4                                     /// Bateria Control volt que considera 3/4 de nivel de carga
#define BatLevel4 26.7                                     /// Bateria Control volt que considera 4/4 de nivel de carga
#define MPTkWhHistAdd 103                                  /// MPPT kWh Perdidos por el usuario al poner a cero el contador (en mi caso falta 103kWh que sumare cada vez para tener el valor historico real)
#define milliTo 0.001                                      /// Bateria Volt & Amp RAW valor es en   mV & mA * 0.001 = V & A por ejemplo  
#define centesiTo 0.01                                     /// Inverter Usado con AC Volt & MPPT kWh
#define decimTo 0.1                                        /// Inverter Usado con AC Amp
#define IVTAmpInLoss 0.9                                   /// Inverter para poder estimar el Amp DC estimado aproximado con perdida de 0.9 = 9%  
#define Warm1 "BATERIA BAJA"                               /// Inverter Warning code 1 Msg nivel demasiado bajo de bateria
#define Warm2 "BATERIA ALTA"                               /// Inverter Warning code 2 Msg nivel demasiado alto de bateria
#define Warm32 "TEMP BAJA"                                 /// Inverter Warning code 32 Msg Temperatura demasaido baja
#define Warm64 "TEMP ALTA"                                 /// Inverter Warning code 64 Msg Temperatura demasaido alta
#define Warm256 "SOBRECARGA"                               /// Inverter Warning code 256 Msg Sobrecarga solicitado enel output AC
#define Warm512 "RUIDO ELECT"                              /// Inverter Warning code 512 Msg Ruido electrico interferencia anormal (a menudo podría ser condensadores defectuosos)
#define Warm1024 "AC BAJO"                                 /// Inverter Warning code 1024 Msg AC Volt demasiado bajo
#define Warm2048 "AC ALTO"                                 /// Inverter Warning code 2048 Msg AC Volt demasaido alto
#define MenuTxtColorRst ILI9341_WHITE                      /// Imagen Text Blanco sobre fondo blanco sera como Borrar
#define MenuTxtColor ILI9341_BLACK                         /// Imagen Text Negro  sobre fondo blanco sera como Escribir
#define MenuColumn1X 20                                    /// Imagen Text SubMenu  Primera Linea de Texto en la primera columna eje X         
#define MenuColumn1Y 125                                   /// Imagen Text SubMenu Primera Linea de Texto en la primera columna eje Y         
#define MenuColumn1NextLine 15                             /// Imagen Text SubMenu     Siguiente Linea de Texto en la primera columna eje Y 
#define MenuColumn2X 130                                   /// Imagen Text SubMenu Primera Linea de Texto en la Segunda columna eje X       
#define MenuColumn2Y 125                                   /// Imagen Text SubMenu Primera Linea de Texto en la Seguna columna eje Y       
#define MenuColumn2NextLine 15                             /// Imagen Text SubMenu     Siguiente Linea de Texto en la Seguna columna eje Y
#define ChrgAmpX 25                                        /// Charger Curent Sensor Amp Value show display eje x
#define ChrgAmpY 47                                        /// Charger Curent Sensor Amp Value show display eje y    
#define MPTVbatX 225                                       /// Imagen Text Menu Home MPPT Vbat Cursor Eje  X
#define MPTVbatY 155                                       /// Imagen Text Menu Home MPPT Vbat Cursor Eje  Y
#define IVTVbatX 140                                       /// Imagen Text Menu Home Inverter Vbat Cursor Eje  X
#define IVTVbatY 81                                        /// Imagen Text Menu Home Inverter Vbat Cursor Eje  Y
#define MPTVpvX 218                                        /// Imagen Text Menu Home MPPT Vpv Cursor Eje  X
#define MPTVpvY 226                                        /// Imagen Text Menu Home MPPT Vpv Cursor Eje  Y
#define MPTAmpX 275                                        /// Imagen Text Menu Home MPPT Amp pv Cursor Eje  X
#define MPTAmpY 155                                        /// Imagen Text Menu Home MPPT Amp pv Cursor Eje  Y
#define MPTPpvX 225                                        /// Imagen Text Menu Home MPPT Watt pv Cursor Eje  X
#define MPTPpvY 196                                        /// Imagen Text Menu Home MPPT Watt pv Cursor Eje  Y
#define MPTkWhX 263                                        /// Imagen Text Menu Home MPPT kWh pv Cursor Eje  X
#define MPTkWhY 226                                        /// Imagen Text Menu Home MPPT kWh pv Cursor Eje  Y
#define MPTWmaxX 225                                       /// Imagen Text Menu Home MPPT Watt Max pv Cursor Eje  X
#define MPTWmaxY 138                                       /// Imagen Text Menu Home MPPT Watt Max pv Cursor Eje  Y
#define MPTStatX 239                                       /// Imagen Text Menu Home MPPT Estado Cursor Eje  X
#define MPTStatY 121                                       /// Imagen Text Menu Home MPPT Estado Cursor Eje  Y 
#define IVTWarnX 107                                       /// Imagen Text Menu Home Inverter Warning Cursor Eje  X
#define IVTWarnY 203                                       /// Imagen Text Menu Home Inverter Warning Cursor Eje  Y
#define IVTACIX 225                                        /// Imagen Text Menu Home Inverter AC Amp eje X
#define IVTACIY 40                                         /// Imagen Text Menu Home Inverter AC Amp eje Y
#define IVTACVX 141                                        /// Imagen Text Menu Home Inverter AC Volt eje X
#define IVTACVY 21                                         /// Imagen Text Menu Home Inverter AC Volt eje Y
#define IVTACWX 222                                        /// Imagen Text Menu Home Inverter AC AV(aproximadamente Watt) eje X
#define IVTACWY 80                                         /// Imagen Text Menu Home Inverter AC AV(aproximadamente Watt) eje Y
#define IVTAmpInX 145                                      /// Imagen Text Menu Home Inverter Amp input estimado aproximado eje X
#define IVTAmpInY 61                                       /// Imagen Text Menu Home Inverter Amp input estimado aproximado eje Y
#define MPTVbatCL     "C1L1"                               /// Imagen Text SubMenu MPPT Columna Linea V bat seleccionar la columna y linea donde se mostrara cada dato
#define MPTiCL        "C1L2"                               /// Imagen Text SubMenu MPPT Columna Linea A bat
#define MPTVpvCL      "C1L3"                               /// Imagen Text SubMenu MPPT Columna Linea V pv
#define MPTPpvCL      "C1L4"                               /// Imagen Text SubMenu MPPT Columna Linea W pv
#define MPTkWhHistCL  "C1L5"                               /// Imagen Text SubMenu MPPT Columna Linea kWh historico
#define MPTkWhCL      "C1L6"                               /// Imagen Text SubMenu MPPT Columna Linea kWh
#define MPTWmaxCL     "C1L7"                               /// Imagen Text SubMenu MPPT Columna Linea Wmax
#define MPTcsCL       "C2L1"                               /// Imagen Text SubMenu MPPT Columna Linea Estado  
#define MPTfwCL       "C2L2"                               /// Imagen Text SubMenu MPPT Columna Linea Firwmare
#define MPTpidCL      "C2L3"                               /// Imagen Text SubMenu MPPT Columna Linea Producto id
#define MPTserCL      "C2L4"                               /// Imagen Text SubMenu MPPT Columna Linea numero de serie
#define MPTerrCL      "C2L5"                               /// Imagen Text SubMenu MPPT Columna Linea Errores
#define MPTkWhAyerCL  "C2L6"                               /// Imagen Text SubMenu MPPT Columna Linea kWh ayer
#define MPTWmaxAyerCL "C2L7"                               /// Imagen Text SubMenu MPPT Columna Linea Wmax ayer
#define MenuSolarErr0Msg "Sin errores"                     /// Imagen Text SubMenu Solar MSG                          
#define MenuSolarErr2Msg "V Bat alta"                      /// Imagen Text SubMenu Solar MSG
#define MenuSolarErr17Msg "Temp alta"                      /// Imagen Text SubMenu Solar MSG
#define MenuSolarErr18Msg "Amp alta"                       /// Imagen Text SubMenu Solar MSG
#define MenuSolarErr19Msg "Amp invers"                     /// Imagen Text SubMenu Solar MSG
#define MenuSolarErr20Msg "Bulk overtime"                  /// Imagen Text SubMenu Solar MSG
#define MenuSolarErr21Msg "Amp Sens mal"                   /// Imagen Text SubMenu Solar MSG
#define MenuSolarErr26Msg "Temp Terminales"                /// Imagen Text SubMenu Solar MSG
#define MenuSolarErr33Msg "V PV alto"                      /// Imagen Text SubMenu Solar MSG
#define MenuSolarErr34Msg "A PV alto"                      /// Imagen Text SubMenu Solar MSG
#define MenuSolarErr38Msg "V Bat exceso"                   /// Imagen Text SubMenu Solar MSG
#define MenuSolarErr116Msg "Calibration mal"               /// Imagen Text SubMenu Solar MSG 
#define MenuSolarErr117Msg "Firmware mal"                  /// Imagen Text SubMenu Solar MSG
#define MenuSolarErr119Msg "Config user mal"               /// Imagen Text SubMenu Solar MSG
#define MenuSolarCS0Msg "OFF"                              /// Imagen Text SubMenu Solar MSG                          
#define MenuSolarCS2Msg "MPPT Falla"                       /// Imagen Text SubMenu Solar MSG
#define MenuSolarCS3Msg "Carga Inicial"                    /// Imagen Text SubMenu Solar MSG
#define MenuSolarCS4Msg "Absorcion"                        /// Imagen Text SubMenu Solar MSG
#define MenuSolarCS5Msg "Floatacion"                       /// Imagen Text SubMenu Solar MSG
#define MenuSolarErrtag ""                                 /// Imagen Text SubMenu Solar MSG
#define MPTCStag ""                                        /// Imagen Text SubMenu Solar MSG
#define MenuSolarMPTsertag "NS: "                          /// Imagen Text SubMenu Solar MSG     
#define MenuSolarMPTpidtag "ID: "                          /// Imagen Text SubMenu Solar MSG
#define MenuSolarMPTfwtag "FW: "                           /// Imagen Text SubMenu Solar MSG
#define MenuSolarHSDStag "Dias: "                          /// Imagen Text SubMenu Solar MSG
#define MenuSolarMPTWmaxAytag "Ayer:"                      /// Imagen Text SubMenu Solar MSG
#define MenuSolarkWhAyertag "Ayer:"                        /// Imagen Text SubMenu Solar MSG
#define MenuSolarMPTkWhHisttag "T.kWh"                     /// Imagen Text SubMenu Solar MSG
#define IVTVbatCL "C1L2"                                   /// Imagen Text SubMenu Inverter Columna Linea V Bat
#define IVTARCL "C1L3"                                     /// Imagen Text SubMenu Inverter Columna Linea Alarm
#define IVTcsCL "C1L4"                                     /// Imagen Text SubMenu Inverter Columna Linea Estado
#define IVTfwCL "C1L5"                                     /// Imagen Text SubMenu Inverter Columna Linea Firmware
#define IVTpidCL "C1L6"                                    /// Imagen Text SubMenu Inverter Columna Linea Product ID
#define IVTserCL "C1L7"                                    /// Imagen Text SubMenu Inverter Columna Linea Numero de Serie NS
#define IVTmodeCL "C1L8"                                   /// Imagen Text SubMenu Inverter Columna Linea Modo
#define IVTACVCL "C2L2"                                    /// Imagen Text SubMenu Inverter Columna Linea AC Volt
#define IVTACICL "C2L3"                                    /// Imagen Text SubMenu Inverter Columna Linea AC Amp
#define IVTACSCL "C2L4"                                    /// Imagen Text SubMenu Inverter Columna Linea AC AV (aproximadamente Watt)
#define IVTWarnCL "C2L5"                                   /// Imagen Text SubMenu Inverter Columna Linea Warning
#define MenuInverterCS0Msg "OFF"                           /// Imagen Text SubMenu Inverter MSg Estado 0  OFF     
#define MenuInverterCS1Msg "LOWPOWER"                      /// Imagen Text SubMenu Inverter MSg Estado 1  Low Power 
#define MenuInverterCS3Msg "FAULT"                         /// Imagen Text SubMenu Inverter MSg Estado 3  Fault ( necesita reinicio por parte del usuario)
#define MenuInverterCS4Msg "ON"                            /// Imagen Text SubMenu Inverter MSg Estado 4  ON
#define MenuInverterAR1Msg "low voltage"                   /// Imagen Text SubMenu Inverter Alarm 1 Low volt  
#define MenuInverterAR2Msg "high voltage"                  /// Imagen Text SubMenu Inverter Alarm 2 High volt
#define MenuInverterAR32Msg "Low Temp"                     /// Imagen Text SubMenu Inverter Alarm 32 Low Temperatura
#define MenuInverterAR64Msg "High Temp"                    /// Imagen Text SubMenu Inverter Alarm 64 Alta temperatura 
#define MenuInverterAR256Msg "OverLoad"                    /// Imagen Text SubMenu Inverter Alarm 256 Sobrecarga
#define MenuInverterAR512Msg "DC Ripple"                   /// Imagen Text SubMenu Inverter Alarm 512 Ruido Interferencia en el circuito
#define MenuInverterAR1024Msg "Low V AC Out"               /// Imagen Text SubMenu Inverter Alarm 1024 AC Volt muy bajo
#define MenuInverterAR2048Msg "High V AC out"              /// Imagen Text SubMenu Inverter Alarm 2048 AC Volt muy alto
#define MenuInvertermode2Msg " ON"                         /// Imagen Text SubMenu Inverter Modo 2 ON
#define MenuInvertermode4Msg " OFF"                        /// Imagen Text SubMenu Inverter Modo 4 OFF
#define MenuInvertermode5Msg " ECO"                        /// Imagen Text SubMenu Inverter MOdo 5 ECO
#define MenuInverterwarn1Msg "LOW VOTL"                    /// Imagen Text SubMenu Inverter Warning 1 volt Bajo
#define MenuInverterwarn2Msg "HIGH VOLT"                   /// Imagen Text SubMenu Inverter Warning 2 Volt alto
#define MenuInverterwarn32Msg "LOW TEMP"                   /// Imagen Text SubMenu Inverter Warning 32 Temperatura Baja
#define MenuInverterwarn64Msg "HIGH TEMP"                  /// Imagen Text SubMenu Inverter Warning 64 Temperatura Alta 
#define MenuInverterwarn256Msg "OverLoad"                  /// Imagen Text SubMenu Inverter Warning 256 Sobracarga
#define MenuInverterwarn512Msg "DC Ripple"                 /// Imagen Text SubMenu Inverter Warning 512 Ruido en la linea electrica
#define MenuInverterwarn1024Msg   "LOW V AC out"           /// Imagen Text SubMenu Inverter Warning 1024 AC Volt muy bajo
#define MenuInverterwarn2048Msg "High V AC out"            /// Imagen Text SubMenu Inverter Warning 2048 AC Volt muy alto
#define MenuInverterfwtag "FW:"                            /// Imagen Text SubMenu Inverter Firmware
#define MenuIVTpidtag "PID:"                               /// Imagen Text SubMenu Inverter Prodcuto ID
#define MenuIVTsertag "SN:"                                /// Imagen Text SubMenu Inverter Numero de Serie NS
#define MenuInverterwarntag ""                             /// Imagen Text SubMenu Inverter etiqueta para Warning
#define MenuInverterARtag ""                               /// Imagen Text SubMenu Inverter etiqueta para Alarma
#define MenuInverterModetag "Mode:"                        /// Imagen Text SubMenu Inverter etiqueta para Modo
#define BATivtVbatCL "C1L1"                                /// Imagen Text SubMenu Bateria Columna Linea Volt 
#define BATkWhCL "C1L2"                                    /// Imagen Text SubMenu Bateria Columna Linea kWh 
#define BATTipCL "C1L3"                                    /// Imagen Text SubMenu Bateria Columna Linea Tipo ( LiFePo4 por ejemplo) 
#define BATDateCL "C1L4"                                   /// Imagen Text SubMenu Bateria Columna Linea Fecha de fabricacion 
#define BATBrandCL "C1L5"                                  /// Imagen Text SubMenu Bateria Columna Linea Marca 
#define BATModelCL "C1L6"                                  /// Imagen Text SubMenu Bateria Columna Linea Modelo
#define BATSerialNumbrCL "C1L7"                            /// Imagen Text SubMenu Bateria Columna Linea Numero de serie 
#define BATCellCL "C1L8"                                   /// Imagen Text SubMenu Bateria Columna Linea Cell Estrucura Paralelo Serie 
#define BATmptVbatCL "C2L1"                                /// Imagen Text SubMenu Bateria Columna Linea desde MPPT Volt medido
#define BATAhCL "C2L2"                                     /// Imagen Text SubMenu Bateria Columna Linea Capacidad Ah 
#define BATVoltCL "C2L3"                                   /// Imagen Text SubMenu Bateria Columna Linea Volt Tipico 
#define BATVChargCL "C2L4"                                 /// Imagen Text SubMenu Bateria Columna Linea Volt de carga 
#define batIVTVbattag "invt:"                              /// Imagen Text SubMenu Bateria Inverter Volt Bateria
#define batMPTVbattag "mppt:"                              /// Imagen Text SubMenu Bateria MPPT Volt Bateria
#define BATVolt "25.6V"                                    /// Imagen Text SubMenu Bateria Volt tipico                               
#define BATVCharg "28.4V"                                  /// Imagen Text SubMenu Bateria Volt para carga
#define BATAh "111Ah"                                      /// Imagen Text SubMenu Bateria Capacidad
#define BATkWh "2.8416kWh"                                 /// Imagen Text SubMenu Bateria kWh equivalentes      
#define BATTip "LiFePO4"                                   /// Imagen Text SubMenu Bateria tipo de tecnologia          
#define BATDate "2021-03-22"                               /// Imagen Text SubMenu Bateria Fecha de fabricacion
#define BATBrand "Pylontech"                               /// Imagen Text SubMenu Bateria Marca
#define BATModel "UP2500NB01V00101"                        /// Imagen Text SubMenu Bateria Modelo
#define BATSerialNumbr "PPTBP0xxxxxxxxxx"                  /// Imagen Text SubMenu Bateria Numero de Serie
#define BATCell "IFpP/13/141/238/[(3P3S)2S+3P2S]M/0+50/95" /// Imagen Text SubMenu Bateria Celda tipologia estructura
#define MenuBackgroundX 0                                  /// Imagen Background Menu set Cursor X   Home, Solar, Inverter, MPPT, etc..
#define MenuBackgroundY 2                                  /// Imagen Background Menu set Cursor Y
#define BatBarLevelColor ILI9341_GREENYELLOW // Barra de nivel de bateria Color
#define BatBarLevels 4                       // Barra de nivel de bateriaCantidad de barra de nivel 
#define BatBarLevelRadius 3                  // Barra de nivel de bateriaEsquina redondeadas con radio de 3 pixel
#define BatBarLevelHeight 15                 // Barra de nivel de bateria alto de cada barra de nivel
#define BatBarLevelWidth 70                  // Barra de nivel de bateriaancho de cada barra de nivel
#define BatBarLevelNextY 20                  // Barra de nivel de bateriaSeparacion entre las barra de nivel
#define BatBarLevel1y 204                    // Barra de nivel de baterialevel 1 eje y
#define BatBarLevelx 20                      // Barra de nivel de baterialevel 0 eje X
#define BatBarLevel0y 134                    // Barra de nivel de baterialevel 0 eje y
#define BatBarLevel0w 70                     // Barra de nivel de baterialevel 0 ancho
#define BatBarLevel0h 72                     // Barra de nivel de baterialevel 0 Alto
#define MPTCSColor0 ILI9341_BLACK            // MPPT estado color del falso led mostrado en pantalla para estado 0 OFF
#define MPTCSColor2 ILI9341_RED              // MPPT estado color del falso led mostrado en pantalla para estado 2 FAULT (1 no es un estado valido para mppt)
#define MPTCSColor3 ILI9341_BLUE             // MPPT estado color del falso led mostrado en pantalla para estado 3 BULK
#define MPTCSColor4 ILI9341_YELLOW           // MPPT estado color del falso led mostrado en pantalla para estado 4 ABSORPTION
#define MPTCSColor5 ILI9341_GREEN            // MPPT estado color del falso led mostrado en pantalla para estado 5 FLOAT
#define CHRGCurrSensCL "C1L1"                // Image Text SubMenu Charger Amp
#define CHRGMPTVbatCL "C1L2"                 // Image Text SubMenu Charger Volt Bat MPPT
#define BatAmpX 110                          // Image Text Menu Home Bateria Amp eje X
#define BatAmpY 142                          // Image Text Menu Home Bateria Amp eje Y
#define Zero 0                               // Image Text Menu Home Bateria Amp eje Y 
#define msLoop 125                           // Loop ms millis

String IVTCSStrg;                    /// Inverter String Estado
String IVTCSStrgmsg;                 /// Inverter String Estado msg
String IVTARStrg;                    /// Inverter String Alarmas
String IVTARStrgmsg;                 /// Inverter String Alarmas msg
String IVTfwStrg;                    /// Inverter String Firmware
String IVTpidStrg;                   /// Inverter String Producto ID
String IVTSERStrg;                   /// Inverter String numero serie NS
String IVTModeStrg;                  /// Inverter String Modo
String IVTmodeStrgmsg;               /// Inverter String Modo msg
String IVTACVStrg;                   /// Inverter String AC volt
String IVTACIStrg;                   /// Inverter String AC Amp
String IVTACSStrg;                   /// Inverter String AC Watt
String IVTWarnStrg;                  /// Inverter String Alarma
String IVTWarnStrgmsg;               /// Inverter String Warm msg
String IVTVbatStrg;                  /// Inverter StringBat V 
String Origin="MPPT";                /// VE.Direct Lectura de datos organizado por Origen   MPPT o Inverter
String label="V";                    /// VE.Direct Lectura de datos tipo de lectura         V, VPV, PPV, AC V, AC I, ALARM, WARM, ... 
String val="0";                      /// VE.Direct Lectura de datos valor de lectura        26.4v, error4, ...
String MPTErrorStrg;                 /// MPPT String Error 
String MPTErrorStrgmsg;              /// MPPT Msg Error
String MPTCSStrg;                    /// MPPT String Estado
String MPTCSStrgmsg;                 /// MPPT Msg Estado
String MPTSERStrg;                   /// MPPT String Numero de Serie del producto 
String MPTpidStrg;                   /// MPPT String ID del Producto
String MPTfwStrg;                    /// MPPT String Firmware
String MPTWmaxAyerStrg;              /// MPPT String Wmax Ayer
String MPTkWhAyerStrg;               /// MPPT String kWh Ayer 
String MPTVbatStrg;                  /// MPPT String Volt Bat 
String MPTVpvStrg;                   /// MPPT String Volt Solar panel
String MPTPpvStrg;                   /// MPPT String Watt Solar panel
String MPTAmpStrg;                   /// MPPT String Amp
String MPTkWhStrg;                   /// MPPT String kWh HOY
String MPTWmaxStrg;                  /// MPPT String W max HOY
String MPTkWhHistStrg;               /// MPPT String kWh historico

String MenuCL="C1L1";                       /// Imagen Text Linea y Columna elegida                                    (no usado en Menu Home, solo submenu Solar, Charger, Inverter Bateria ..)
String MenuPresent="Home";           /// Menu en Display actualmente
String MenuSelected="Home";                 /// Menu Solicitado al tocar la pantalla

int MovSpeed= 1;                     /// Imagen Animacion Bola Animada Velociad de movimiento depende tambien del loop -->  pixel por loop
int IVTCS=99;                           /// Inverter Estado integral
int IVTMode=99;                         /// Inverter Modo integral
int IVTACV=99;                          /// Inverter AC Volt
float IVTACI=99;                        /// Inverter AC Amp
int IVTACS=99;                          /// Inverter AV "aproximadamente Watt AC"
int IVTWarn=99;                         /// Inverter Warning
int IVTAR=99;                           /// Inverter Alarm
float IVTAmpIn=99;                      /// Inverter Amp Input (DC) Estimados aproximado 9% perdida esto no tiene en cuenta carga inductiva o resistiva
float IVTVbat=99;                       /// Inverter Bateria Volt
float MPTVbat=99;                       /// MPPT Bateria Volt   float para poder calcular el % correctamente necesita decimales
int16_t MPTVpv=99;                      /// MPPT Solar Volt paneles
int16_t MPTPpv=99;                      /// MPPT Watt Solar panel
int16_t MPTAmp=99;                      /// MPPT Amp
int16_t MPTkWh=99;                      /// MPPT kWh HOY
int16_t MPTWmax=99;                     /// MPPT W max HOY
int MPTkWhHist=99;                      /// MPPT kWh Historico
int MPTError=99;                        /// MPPT error int                                           
int MPTCS=99;                           /// MPPT estado int
int MPTWmaxAyer=99;                     /// MPPT Wmax Ayer int
int MPTkWhAyer=99;                      /// MPPT kWh Ayer int
int ChrgBatStrtY = 83;               /// Imagen Menu Home Animacion Bola de Charger a Bateria eje Y     Down
int ChrgInvrtStrtX = 99;             /// Imagen Menu Home Animacion Bola de Charger a Inverter eje X    Right
int LoadStrtX = 192;                 /// Imagen Menu Home Animacion Bola de Inverter a Load eje X       Right
int BATStrtX = 99;                   /// Imagen Menu Home Animacion Bola de Bateria a Inverter eje X    Right
int BATFinX = 162;                   /// Imagen Menu Home Animacion Bola de Solar a Bateria eje X       Left
int SolarStrtX = 220;                /// Imagen Menu Home Animacion Bola de SolarBat a Inverter eje X   Left
int InvrtStrtY = 150;                /// Imagen Menu Home Animacion Bola de BatSolar a Inverter eje Y   UP
uint8_t ResetMPTData=200;            /// VE.Direct Data Reset MPPT Cuando fallas XX lecturas se resetean los datos      los 3 deben ser igual    ResetMPTData   ResetIVTData ResetLoopDefault 
uint8_t ResetIVTData=200;            /// VE.Direct Data Reset Inverter Cuando fallas XX lecturas se resetean los datos  los 3 deben ser igual    ResetMPTData   ResetIVTData ResetLoopDefault 
int TchX=0;                          /// Menu eje X Colocacion de background para el menu solicitado
int TchY=0;                          /// Menu eje Y ...
int CurrSens=0;                      /// Charger Current Sensr AMp Value show display buffer memoria
float BatAmp=99;

String NewValueStrg="";
float NewValue=0;

bool AnimSolOut=false;               /// Imagen Animacion interruptor de activacion Solar Output
bool AnimIVTOutput=false;            /// Imagen Animacion interruptor de activacion Inverter-->Load Output
bool AnimIVTInput=false;             /// Imagen Animacion interruptor de activacion Inverter Input
bool AnimChargIVT=false;             /// Imagen Animacion interruptor de activacion Charger-->Inverter
bool AnimChargBat=false;             /// Imagen Animacion interruptor de activacion Charger-->Bateria
bool AnimBatOut=false;               /// Imagen Animacion interruptor de activacion Bateria Output
bool AnimBatIn=false;                /// Imagen Animacion interruptor de activacion Bateria Input

unsigned long previousMillis = 0;    /// Delay          sin detencion 
unsigned long currentMillis=0;       /// Delay          sin detencion 

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC,TFT_RST);  /// Display ili9341 pin IMAGEN
XPT2046_Touchscreen ts(CS_PIN, TIRQ_PIN);                         /// Display ili9341 pin TOUCH   Param 2 - Touch IRQ Pin - interrupt enabled polling

void SerialIni(){
  Serial1.begin(UARTRate1,SERIAL_8N1,UART1rx,UART1tx);  // Serial 1 OK reasignado RX en GPIO27 D27 [ y TX en GPIO14 D14 no probado ]
  Serial2.begin(UARTRate2,SERIAL_8N1);                  // Serial 2 MPPT     ( se pueden cambiar los pines en la config esp32) 
}
void TextSetup(){
  tft.setTextColor(ILI9341_WHITE);        // Mostrar "No Hay Contact"
}
void TFTSetIni(){
  tft.begin();        // activar la imagen on rotation   se puede subir a tft.begin(40000000) para evitar parpadeos 
  tft.setRotation(3); // 0 1 2 3 0º 90º 180º 270º
  ts.begin();         // activar touch misma rotation que la imagen
  ts.setRotation(3);
}
void AnimSolarOut(){        /// APPV
  tft.drawRGBBitmap(157, 146, SolarOutBitmap, SolarOut_WIDTH,SolarOut_HEIGHT);
  tft.fillCircle(SolarStrtX, SolarStrtY, CirclRad,CirclColor); // sentido de la animacion invertida > y - en vez de < y +
  if(SolarStrtX>SolarFinX){
    SolarStrtX= (SolarStrtX - MovSpeed);
    }else{
    SolarStrtX=SolarStrtXDef;
  }
}
void AnimInverterOuput(){//
  tft.drawRGBBitmap(188, 48, InvrtOutBitmap, InvrtOut_WIDTH,InvrtOut_HEIGHT);
  tft.fillCircle(LoadStrtX, LoadStrtY, CirclRad,CirclColor);
  if(LoadStrtX<LoadFinX){
    LoadStrtX= (LoadStrtX + MovSpeed);
    }else{
    LoadStrtX=LoadStrtXDef;
  }
}
void AnimInverterInput(){//
     tft.drawRGBBitmap(156, 87, InvrtInBitmap, InvrtIn_WIDTH,InvrtIn_HEIGHT);
     tft.fillCircle(InvrtStrtX, InvrtStrtY, CirclRad,CirclColor);
        if(InvrtStrtY>InvrtFinY){
      InvrtStrtY= (InvrtStrtY - (MovSpeed * 1.9354838));
    }else{
      InvrtStrtY=InvrtStrtYDef;
    }

}
void AnimChrgInvrt(){//
  tft.drawRGBBitmap(94, 48, ChrgInvrtBitmap, ChrgInvrt_WIDTH,ChrgInvrt_HEIGHT);
  tft.fillCircle(ChrgInvrtStrtX, ChrgInvrtStrtY, CirclRad,CirclColor);
  if(ChrgInvrtStrtX<ChrgInvrtFinX){
    ChrgInvrtStrtX= (ChrgInvrtStrtX + MovSpeed);
    }else{
    ChrgInvrtStrtX=ChrgInvrtStrtXDef;
  }
}
void AnimChrgBat(){//
  tft.drawRGBBitmap(52, 78, ChrgBATBitmap, ChrgBAT_WIDTH,ChrgBAT_HEIGHT);
  tft.fillCircle(ChrgBatStrtX, ChrgBatStrtY, CirclRad,CirclColor);
  if(ChrgBatStrtY<ChrgBatFinY){
    ChrgBatStrtY= (ChrgBatStrtY + MovSpeed);
    }else{
    ChrgBatStrtY=ChrgBatStrtYDef;
  }
}
void AnimBatOuput(){//
  tft.drawRGBBitmap(93, 146, BATInvrtBitmap, BATInvrt_WIDTH,BATInvrt_HEIGHT);
  tft.fillCircle(BATStrtX, BATStrtY, CirclRad,CirclColor); // charge
  if(BATStrtX<BATFinX){
    BATStrtX= (BATStrtX + MovSpeed);
    }else{
      BATStrtX=BATStrtXDef;
  }
}
void AnimBatInput(){//
  tft.drawRGBBitmap(93, 146, BATInvrtBitmap, BATInvrt_WIDTH,BATInvrt_HEIGHT);
  tft.fillCircle(BATFinX, BATStrtY, CirclRad,CirclColor); // discharge
  if(BATStrtX<BATFinX){
    BATFinX= (BATFinX - MovSpeed);
    }else{
      BATFinX=BATFinXDef;
  }
}
void TextSize1(){
  tft.setFont(TxtSize1);
}
void TextSize2(){
  tft.setFont(TxtSize2);
}
void ReadSerial1(){ // Inverter
  if (Serial1.available()){
    Origin="Inverter";
    label= Serial1.readStringUntil('\t'); // no se pueden usar  "x" hay qu eponer 'x'
    val = Serial1.readStringUntil('\n');     
    }else{
    Origin="IVTnodata";
  }    
}
void ReadSerial2(){
  if (Serial2.available()){
    Origin="MPPT";
    label= Serial2.readStringUntil('\t'); // no se pueden usar  "x" hay qu eponer 'x'
    val = Serial2.readStringUntil('\n');     
    }else{  
    Origin="MPTnodata";    
  }
}
void CurrentSensor(){
  NewValue = analogRead(CurrSensPin);                      // Current Sensor sumar cada valor leido      
  NewValue = ((NewValue-CurrSensRAWZero)*CurrSensRAWUnit); // Current Sensor pasar de RAWvalue a Amp
  if(NewValue<Zero){
   NewValue=Zero;
  }else{}
}
void Touched(){          //// Touched YES
    if(MenuSelected=="Home"){
      tft.drawRGBBitmap(MenuBackgroundX, MenuBackgroundY, HomeBitmap, HOME_WIDTH,HOME_HEIGHT);  // set cursor X , y , select image, image widht, image height
      MPTVbat=99;
      MPTVpv=99;
      MPTPpv=99;
      MPTAmp=99;
      MPTkWh=99;
      MPTWmax=99;
      MPTCS=99;
      IVTVbat=99;
      IVTMode=99;
      IVTWarnStrg="";
      IVTACV=99;
      IVTACI=99;
      IVTACS=99;
      CurrSens=99;
      IVTAmpIn=99;
      BatAmp=99;
      AnimIVTOutput=false;
      AnimIVTInput=false;
      MenuPresent="Home";
    }else if(MenuSelected=="Solar"){
      tft.drawRGBBitmap(MenuBackgroundX, MenuBackgroundY, SolarBackGround, SolarMenu_WIDTH,SolarMenu_HEIGHT);
      MenuPresent="Solar";
    }else if(MenuSelected=="Bat"){
      tft.drawRGBBitmap(MenuBackgroundX, MenuBackgroundY, BatBackGround, BatMenu_WIDTH,BatMenu_HEIGHT);
      MenuPresent="Bat";
    }else if(MenuSelected=="Inverter"){
      tft.drawRGBBitmap(MenuBackgroundX, MenuBackgroundY, InvrtBackGround, InvrtMenu_WIDTH,InvrtMenu_HEIGHT);
      MenuPresent="Inverter";
    }else if(MenuSelected=="Charger"){
      tft.drawRGBBitmap(MenuBackgroundX, MenuBackgroundY, ChrgBackGround, ChrgMenu_WIDTH,ChrgMenu_HEIGHT);
      MenuPresent="Charger";
    }else if(MenuSelected=="Load"){
      tft.drawRGBBitmap(MenuBackgroundX,MenuBackgroundY, LoadBackGround, LoadMenu_WIDTH,LoadMenu_HEIGHT);
    MenuPresent="Load";
  }

}
void TouchAsk(){         //// Touched ?
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    TchX=p.x;
    TchY=p.y;
  if ((TchX > TchHomeXmin && TchX < TchHomeXmax) && (TchY > TchHomeYmin && TchY < TchHomeYmax)) {
    MenuSelected="Home";
  }
  else if ((TchX > TchSolarXmin && TchX < TchSolarXmax) && (TchY > TchSolarYmin &&TchY < TchSolarYmax)) {
    MenuSelected="Solar";
  }
  else if ((TchX > TchBatXmin && TchX < TchBatXmax) && (TchY > TchBatYmin && TchY < TchBatYmax)) {
    MenuSelected="Bat";
  }
  else if ((TchX > TchChrgXmin && TchX < TchChrgXmax) && (TchY > TchChrgYmin && TchY < TchChrgYmax)) {
    MenuSelected="Charger";
  }
  else if ((TchX> TchInvrtXmin && TchX < TchInvrtXmax) && (TchY > TchInvrtYmin && TchY < TchInvrtYmax)) {
    MenuSelected="Inverter";
  }
  else if ((TchX> TchLoadXmin && TchX < TchLoadXmax) && (TchY > TchLoadYmin && TchY < TchLoadYmax)) {
    MenuSelected="Load";
  }
  if(MenuSelected==MenuPresent){
    }else{ 
      Touched();
    }
  }
}
void HomeBatLevel() {     ///// Home Battery Level     basado en Vbat desde MPPT    Copilot Asist
  if (MPTVbat < BatLevel1) {
    tft.fillRect(BatBarLevelx, BatBarLevel0y, BatBarLevel0w, BatBarLevel0h , TextColorBlue);  // Nivel de bateria muy bajo borrar todo slo recuatros verde del nivel
  }
  int levels[] = {BatLevel1, BatLevel2, BatLevel3, BatLevel4};
  for (int i = 0; i < BatBarLevels; i++) {
    if (MPTVbat > levels[i]) {
      tft.fillRoundRect(BatBarLevelx, BatBarLevel1y - (i * BatBarLevelNextY), BatBarLevelWidth, BatBarLevelHeight ,BatBarLevelRadius, BatBarLevelColor);  // Nivel i+1 encender recuadro verde del nivel i+1
    }
  }

}
void HomeCurrentS(){                    ///// Home Currenst Sensor Charger
  CurrentSensor();                      // actualizar lectura del sensor de corrienteo
  if(abs(CurrSens - NewValue) < 1){     ///// Charger Current Sensor mismo valor 
  }else{                                ///// Yes no hace nada
    TextSize2();                        ///// No es igual, es necesario borrar y escribir el valor nuevo
    tft.setTextColor(TextColorRed);     ///// Escribir del mismo color que el fondo
    tft.setCursor(ChrgAmpX, ChrgAmpY);  ///// Cursor
    tft.print(CurrSens,0);              ///// B0rrar Current Sensor valor anterior
    tft.print("A");                     ///// Borrar Letra A
    tft.setTextColor(TextColorWhite);    ///// Escribir en blanco
    tft.setCursor(ChrgAmpX, ChrgAmpY);  ///// Cursor
    tft.print(NewValue,0);              ///// Escribir Nuevo valor
    tft.print("A");                     ///// Escribir Letra A
    CurrSens=NewValue;             ///// Establecer el nuevo valor con valor antiguo para la proxima verificacion
  }
  NewValue=Zero;                        ///// Resetear NewValue
}
void HomeIVtAmpIn(){            ///// Home Amp input Inverter Estimado
  NewValue=((IVTACS / IVTVbat)*IVTAmpInLoss); // Amp inpout estimado con perdida estimada
    if(NewValue==IVTAmpIn){
    }else{
        TextSize1();
        tft.setTextColor(TextColorBlue);
        tft.setCursor(IVTAmpInX, IVTAmpInY);  // Mostrar valor en pantalla Watt
        tft.print(IVTAmpIn,1);
        tft.print("A");
        tft.setTextColor(TextColorWhite);
        tft.setCursor(IVTAmpInX, IVTAmpInY);  // Mostrar valor en pantalla Watt
        tft.print(NewValue,1);
        tft.print("A");
        IVTAmpIn=NewValue;
    }
  NewValue=Zero;
}
void HomeBatAmp(){
  NewValue=(CurrSens+MPTAmp);
  NewValue=(NewValue-IVTAmpIn);
        if(NewValue==BatAmp){
        }else{
        TextSize1();
        tft.setTextColor(TextColorWhite);
        tft.setCursor(BatAmpX, BatAmpY);  // Mostrar valor en pantalla Watt
        tft.print(BatAmp,1);
        tft.print("A");
        if(NewValue<0){
          tft.setTextColor(TextColorOrange);
        }else{
          tft.setTextColor(TextColorBlue);
        }
        tft.setCursor(BatAmpX, BatAmpY);  // Mostrar valor en pantalla Watt
        tft.print(NewValue,1);
        tft.print("A");
        BatAmp=NewValue;
      } 
  NewValue=Zero;
}
void HomeAnimAsk(){             //  Copilot Asist ayuda
  AnimSolOut = MPTPpv > 0.5 ? true : false;
  AnimIVTOutput = IVTACS > 1 ? true : false;

  if(BatAmp>0){
      AnimBatIn=true;
      AnimBatOut=false;
  }else if(BatAmp<0){
      AnimBatIn=false;
      AnimBatOut=true;
  }else if(BatAmp==0){
      AnimBatIn=false;
      AnimBatOut=false;
  }

  if(CurrSens>0){
    if(CurrSens>IVTAmpIn){
      AnimChargBat=true;
      }else{
        if(MPTAmp>IVTAmpIn){
          AnimChargBat=true;
        }else{
          AnimChargBat=false;
        }
      }
    }else{
    AnimChargBat=false;
  }
  
  if(IVTAmpIn>CurrSens){
      AnimIVTInput=true;
    }else{   
    AnimIVTInput=false;
  }

  if (CurrSens>0){
    if(IVTAmpIn>MPTAmp){ 
      AnimChargIVT=true;
    }else{
    AnimChargIVT=false;
    }
  }

  if(CurrSens>0){
    if(MPTAmp>IVTAmpIn){
        AnimChargBat=true;
    }else{
      if(IVTAmpIn<CurrSens){
        AnimChargBat=true;
      }else{
        AnimChargBat=false;
      }
    }
  }
}
void HomeAnimSwitch(){          ///// Home Animaciones interruptores
  if(AnimSolOut==true){
    AnimSolarOut();
  }
  if(AnimIVTOutput==true){
    AnimInverterOuput();
  }
  if(AnimIVTInput==true){
    AnimInverterInput();
  }
  if(AnimChargIVT==true){
    AnimChrgInvrt();
  }
  if(AnimChargBat==true){
    AnimChrgBat();
  }
  if(AnimBatOut==true){
    AnimBatOuput();
  }
  if(AnimBatIn==true){
    AnimBatInput();
  }
}
void DataResetMPT(){   
  AnimSolOut=false;
  Touched();
  ResetMPTData=ResetLoopDefault;  // dato recibido correctamente no resetar los datos
}
void DataResetIVT(){
  Touched();
  ResetIVTData=ResetLoopDefault;  // dato recibido correctamente no resetar los datos
}
void Home(){            ////
  ReadSerial2();
  if(Origin=="MPTnodata"){
    ResetMPTData-=1;
    if(ResetMPTData<1){
      DataResetMPT();  
    }
  }
  if(Origin=="MPPT"){
    ResetMPTData=ResetLoopDefault;  // dato recibido correctamente no resetar los datos
    if(label == "V"){         // Battery Volt  mV     // Bateria Indicador de Nivel 
      NewValueStrg=val;
      NewValue=NewValueStrg.toInt();
      NewValue=(NewValue*milliTo);
      if(NewValue==MPTVbat){
        }else{
        TextSize1();
        tft.setTextColor(TextColorOrange);
        tft.setCursor(MPTVbatX, MPTVbatY);  // Mostrar valor en pantalla Watt
        tft.print(MPTVbat,1);
        tft.print("V");
        tft.setTextColor(TextColorWhite);
        tft.setCursor(MPTVbatX, MPTVbatY);  // Mostrar valor en pantalla Watt
        tft.print(NewValue,1);
        tft.print("V");
        MPTVbat=NewValue;
      }
      NewValue=Zero;
    HomeBatLevel(); // Calculo del % de bateri 
    }
    else if(label == "VPV"){  //Panel Volt mV
      NewValueStrg=val;
      NewValue=NewValueStrg.toInt();
      if(NewValue<Zero){
        NewValue=Zero; 
      }
      NewValue=(NewValue*milliTo);
      if(abs(MPTVpv - NewValue) < 1){
      }else{
        TextSize1();
        tft.setTextColor(TextColorOrange);
        tft.setCursor(MPTVpvX, MPTVpvY);  // Mostrar valor en pantalla Watt
        tft.print(MPTVpv,0);
        tft.print("V");
        tft.setTextColor(TextColorWhite);
        tft.setCursor(MPTVpvX, MPTVpvY);  // Mostrar valor en pantalla Watt
        tft.print(NewValue,0);
        tft.print("V");
        MPTVpv=NewValue;
      }
     NewValue=Zero;
    }
    else if(label == "PPV"){  // Panel Power W
      NewValueStrg=val;
      NewValue=NewValueStrg.toInt();
      if(NewValue==MPTPpv){
      }else{
        TextSize2();
        tft.setTextColor(TextColorOrange);
        tft.setCursor(MPTPpvX, MPTPpvY);  // Mostrar valor en pantalla Watt
        tft.print(MPTPpv,0);
        tft.print("W");        tft.setTextColor(TextColorWhite);
        tft.setCursor(MPTPpvX, MPTPpvY);  // Mostrar valor en pantalla Watt
        tft.print(NewValue,0);
        tft.print("W");
        MPTPpv=NewValue;
      }
     NewValue=Zero;
    }
    else if(label == "I"){    // mA Battery mA
      NewValueStrg=val;
      NewValue=NewValueStrg.toInt();
      NewValue=(NewValue*milliTo);
      if(NewValue==MPTAmp){
      }else{
        TextSize1();
        tft.setTextColor(TextColorOrange);
        tft.setCursor(MPTAmpX, MPTAmpY);  // Mostrar valor en pantalla Watt
        tft.print(MPTAmp,1);
        tft.print("A");
        tft.setTextColor(TextColorWhite);
        tft.setCursor(MPTAmpX, MPTAmpY);  // Mostrar valor en pantalla Watt
        tft.print(NewValue,1);
        tft.print("A");
        MPTAmp=NewValue;
      }
     NewValue=Zero;
    }
    else if(label == "H20"){  // 0.01kWh HOY
      NewValueStrg=val;
      NewValue=NewValueStrg.toInt();
      if(NewValue==MPTkWh){
      }else{
        TextSize1();
        tft.setTextColor(TextColorOrange);
        tft.setCursor(MPTkWhX, MPTkWhY);  // Mostrar valor en pantalla Watt
        tft.print(MPTkWh,0);
        tft.print("kWh");  
        tft.setTextColor(TextColorWhite);
        tft.setCursor(MPTkWhX, MPTkWhY);  // Mostrar valor en pantalla Watt
        tft.print(NewValue,0);
        tft.print("kWh");
        MPTkWh=NewValue;
      }
     NewValue=Zero;
    }
    else if(label == "H21"){ // Wmax HOY
      NewValueStrg=val;
      NewValue=NewValueStrg.toInt();
      if(NewValue==MPTWmax){
      }else{
        TextSize1();
        tft.setTextColor(TextColorOrange);
        tft.setCursor(MPTWmaxX, MPTWmaxY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmax,0);
        tft.print("Wmax");
        tft.setTextColor(TextColorWhite);
        tft.setCursor(MPTWmaxX,MPTWmaxY);  // Mostrar valor en pantalla Watt
        tft.print(NewValue,0);
        tft.print("Wmax");
        MPTWmax=NewValue;
      }
      NewValue=Zero;
    }
    else if(label == "CS"){ // Estado operativo
      NewValueStrg=val;
      NewValue=NewValueStrg.toInt();
      if(NewValue==MPTCS){
      }else{
        if(NewValue==0){ // OFF
         tft.fillCircle(LedMPPTX, LedMPPTY, LedRad,ILI9341_BLACK);
        }
        else if(NewValue==2){ // FAULT MPPT & Inverter off hasta hacer Reset
          tft.fillCircle(LedMPPTX, LedMPPTY, LedRad,ILI9341_RED);
        } 
        else if(NewValue==3){ // bulk
          tft.fillCircle(LedMPPTX, LedMPPTY, LedRad,ILI9341_BLUE);
        }
        else if(NewValue==4){ // Absorption
          tft.fillCircle(LedMPPTX, LedMPPTY, LedRad,ILI9341_YELLOW);
        }
        else if(NewValue==5){ // Float
          tft.fillCircle(LedMPPTX, LedMPPTY, LedRad,ILI9341_GREEN);
        }
      MPTCS=NewValue;
      }
     NewValue=Zero;
    }
  }
  ReadSerial1();  // AR (es como WARM) , CS (es como MODE),
  if(Origin=="IVTnodata"){
    ResetIVTData=(ResetIVTData-1);
    if(ResetIVTData<1){
      DataResetIVT();  
    }else{}
  }
  if(Origin=="Inverter"){
    ResetIVTData=ResetLoopDefault;  // dato recibido correctamente no resetar los datos
    if(label == "V"){         // Battery Volt  mV     NO ANIM
      NewValueStrg=val;
      NewValue=NewValueStrg.toInt();
      NewValue=(NewValue*milliTo);
      if(IVTVbat==NewValue){
        }else{
        TextSize1();
        tft.setTextColor(TextColorBlue);
        tft.setCursor(IVTVbatX, IVTVbatY);  // Mostrar valor en pantalla Watt
        tft.print(IVTVbat,1);
        tft.print("V");
        tft.setTextColor(TextColorWhite);
        tft.setCursor(IVTVbatX, IVTVbatY);  // Mostrar valor en pantalla Watt
        tft.print(NewValue,1);
        tft.print("V");
        IVTVbat=NewValue;
      }
      NewValue=Zero;   
    }
    else if(label=="MODE"){  // INverter Modo ON OFF ECO
      NewValueStrg=val;
      NewValue=NewValueStrg.toInt();
      if(IVTMode==NewValue){
        }else{
          if(NewValue==2){ // inverter ON
            tft.fillCircle(LedIVTX, LedIVTY, LedRad,ILI9341_GREEN);
          }
          else if(NewValue==4){ // inverter OFF
            tft.fillCircle(LedIVTX, LedIVTY, LedRad,ILI9341_BLACK);
          }
          else if(NewValue==5){ // inverter ECO
            tft.fillCircle(LedIVTX, LedIVTY, LedRad,ILI9341_BLUE);
          }
        IVTMode=NewValue;  
      }
      NewValue=Zero;
    }
    else if(label=="WARN"){    // INVERTER Alarm Razon
      NewValueStrg=val;
      if(NewValueStrg==IVTWarnStrg){
        }else{
          if(NewValueStrg=="1"){ // volta bajo
            NewValueStrg=Warm1;
            tft.fillCircle(LedIVTX, LedIVTY, LedRad,ILI9341_RED);
            TextSize1();
            tft.setTextColor(TextColorWhite);
            tft.setCursor(IVTWarnX, IVTWarnY);  // 
            tft.print(IVTWarnStrg);
            tft.setTextColor(TextColorRed);
            tft.setCursor(IVTWarnX,IVTWarnY);  //
            tft.print(NewValueStrg);
          }
          else if(NewValueStrg=="2"){ //Volt alto
            NewValueStrg=Warm2;
            tft.fillCircle(LedIVTX, LedIVTY, LedRad,ILI9341_RED);
            TextSize1();
            tft.setTextColor(TextColorWhite);
            tft.setCursor(IVTWarnX, IVTWarnY);  // 
            tft.print(IVTWarnStrg);
            tft.setTextColor(TextColorRed);
            tft.setCursor(IVTWarnX,IVTWarnY);  // 
            tft.print(NewValueStrg);
          }
          else if(NewValueStrg=="32"){ // Temperatura baja
            NewValueStrg=Warm32;
            tft.fillCircle(LedIVTX, LedIVTY, LedRad,ILI9341_RED);
            TextSize1();
            tft.setTextColor(TextColorWhite);
            tft.setCursor(IVTWarnX, IVTWarnY);  
            tft.print(IVTWarnStrg);
            tft.setTextColor(TextColorRed);
            tft.setCursor(IVTWarnX,IVTWarnY);  
            tft.print(NewValueStrg);
          }
          else if(NewValueStrg=="64"){ // Temperatura alta
            NewValueStrg=Warm64;
            tft.fillCircle(LedIVTX, LedIVTY, LedRad,ILI9341_RED);
            TextSize1();
            tft.setTextColor(TextColorWhite);
            tft.setCursor(IVTWarnX, IVTWarnY);  
            tft.print(IVTWarnStrg);
            tft.setTextColor(TextColorRed);
            tft.setCursor(IVTWarnX,IVTWarnY);  
            tft.print(NewValueStrg);
          }
          else if(NewValueStrg=="256"){ // Sobrecarca se load demasiado alto
            NewValueStrg=Warm256;
            tft.fillCircle(LedIVTX, LedIVTY, LedRad,ILI9341_RED);
            TextSize1();
            tft.setTextColor(TextColorWhite);
            tft.setCursor(IVTWarnX, IVTWarnY);  
            tft.print(IVTWarnStrg); 
            tft.setTextColor(TextColorRed);
            tft.setCursor(IVTWarnX,IVTWarnY);  
            tft.print(NewValueStrg);
          }
          else if(NewValueStrg=="512"){ // rizado de volt continuos de la batteria ruido
            NewValueStrg=Warm512;
            tft.fillCircle(LedIVTX, LedIVTY, LedRad,ILI9341_RED);
            TextSize1();
            tft.setTextColor(TextColorWhite);
            tft.setCursor(IVTWarnX, IVTWarnY);  
            tft.print(IVTWarnStrg);
            tft.setTextColor(TextColorRed);
            tft.setCursor(IVTWarnX,IVTWarnY);  
            tft.print(NewValueStrg);
          }
          else if(NewValueStrg=="1024"){ // AC Volt demasiado bajo
            NewValueStrg=Warm1024;
            tft.fillCircle(LedIVTX, LedIVTY, LedRad,ILI9341_RED);
            TextSize1();
            tft.setTextColor(TextColorWhite);
            tft.setCursor(IVTWarnX, IVTWarnY);  
            tft.print(IVTWarnStrg);
            tft.setTextColor(TextColorRed);
            tft.setCursor(IVTWarnX,IVTWarnY);  
            tft.print(NewValueStrg);
          }
          else if(NewValueStrg=="2048"){ // AC volt demasiado alto
            NewValueStrg=Warm2048;
            tft.fillCircle(LedIVTX, LedIVTY, LedRad,ILI9341_RED);
            TextSize1();
            tft.setTextColor(TextColorWhite);
            tft.setCursor(IVTWarnX, IVTWarnY);  
            tft.print(IVTWarnStrg);
            tft.setTextColor(TextColorRed);
            tft.setCursor(IVTWarnX,IVTWarnY);  
            tft.print(NewValueStrg);
          }
          NewValueStrg=IVTWarnStrg;
        }
       NewValue=Zero;
    }
    else if(label=="AC_OUT_V"){  // AC volt 0.01V
        NewValueStrg=val;
        NewValue=NewValueStrg.toInt();
        NewValue=(NewValue*centesiTo);
        if(NewValue==IVTACV){
          }else{
          TextSize1();
          tft.setTextColor(TextColorBlue);
          tft.setCursor(IVTACVX, IVTACVY);  // Mostrar valor en pantalla Watt
          tft.print(IVTACV,0);
          tft.print("V");
          tft.setTextColor(TextColorWhite);
          tft.setCursor(IVTACVX, IVTACVY);  // Mostrar valor en pantalla Watt
          tft.print(NewValue,0);
          tft.print("V");
          IVTACV=NewValue;
          }
       NewValue=Zero;
    }
    else if(label=="AC_OUT_I"){ // AC amp 0.1A
        NewValueStrg=val;
        NewValue=NewValueStrg.toInt();
        NewValue=(NewValue*decimTo);
        if(NewValue==IVTACI){
        }else{
          TextSize1();
          tft.setTextColor(TextColorGreen);
          tft.setCursor(IVTACIX, IVTACIY);  // Mostrar valor en pantalla Watt
          tft.print(IVTACI,1);
          tft.print("A");
          tft.setTextColor(TextColorWhite);
          tft.setCursor(IVTACIX, IVTACIY);  // Mostrar valor en pantalla WattCS
          tft.print(NewValue,1);
          tft.print("A");
          IVTACI=NewValue;
          }
        NewValue=Zero;
    }
    else if(label=="AC_OUT_S"){ // AC VA aproxi Watt
        NewValueStrg=val;
        NewValue=NewValueStrg.toInt();//decimTo
        if(NewValue==IVTACS){
        }else{
          TextSize2();
          tft.setTextColor(TextColorGreen);
          tft.setCursor(IVTACWX, IVTACWY);  // Mostrar valor en pantalla Watt
          tft.print(IVTACS,0);
          tft.print("W"); // VA = "aproximadamente" Watt en AC
          tft.setTextColor(TextColorWhite);
          tft.setCursor(IVTACWX, IVTACWY);  // Mostrar valor en pantalla Watt
          tft.print(NewValue,0);
          tft.print("W");  // VA = "aproximadamente" Watt en AC
        IVTACS=NewValue;
        }
       NewValue=Zero;
    }
  }
  HomeCurrentS();
  HomeIVtAmpIn();  // Calcul deAmp input estimado del inverter
  HomeBatAmp();
  HomeAnimAsk();
  HomeAnimSwitch();     // Activar animaciones si es necesario
}
void Solar(){          ///
  ReadSerial2();
  TextSize1();
  tft.setTextColor(MenuTxtColorRst);
  if(Origin=="MPPT"){
    if(label == "V"){             // Columna1 Linea1 Battery Volt  mV     NO ANIM
      MenuCL=MPTVbatCL; SetColumnLine();
      tft.print(MPTVbat,2);
      tft.print("V");
      MPTVbatStrg=val;
      MPTVbat=MPTVbatStrg.toInt(); 
      MPTVbat=(MPTVbat*milliTo);
      tft.setTextColor(MenuTxtColor);
      MenuCL=MPTVbatCL; SetColumnLine();
      tft.print(MPTVbat,2);
      tft.print("V");
    }
    else if(label == "VPV"){      // Columna1 Linea2 Panel Volt mV
      MenuCL=MPTVpvCL; SetColumnLine();
        tft.print(MPTVpv,0);
        tft.print("V");
        MPTVpvStrg=val;
        MPTVpv=MPTVpvStrg.toInt();
          if(MPTVpv<0){
            MPTVpv=(MPTVpv*-1); // no se porque a veces el valor sale negativo supongo que al pasar de sting a int lo hace no siempre bien
          }else{}
        MPTVpv=(MPTVpv*milliTo);
        tft.setTextColor(MenuTxtColor);
      MenuCL=MPTVpvCL; SetColumnLine();
        tft.print(MPTVpv,0);
        tft.print("V");
    }
    else if(label == "PPV"){      // Columna1 Linea3 Panel Power W
      MenuCL=MPTPpvCL; SetColumnLine();
        tft.print(MPTPpv);
        tft.print("W");
        MPTPpvStrg=val;
        MPTPpv=MPTPpvStrg.toInt();
          if(MPTPpv>1){
            AnimSolOut=true;
          }else{
            AnimSolOut=false;
          }
        tft.setTextColor(MenuTxtColor);
      MenuCL=MPTPpvCL; SetColumnLine();
        tft.print(MPTPpv);
        tft.print("W");
    }
    else if(label == "I"){        // Columna1 Linea4 mA Battery mA
      MenuCL=MPTiCL; SetColumnLine();
        tft.print(MPTAmp);
        tft.print("A");
        MPTAmpStrg=val;
        MPTAmp=MPTAmpStrg.toInt();
        MPTAmp=(MPTAmp*milliTo);
        tft.setTextColor(MenuTxtColor);
      MenuCL=MPTiCL; SetColumnLine();
        tft.print(MPTAmp);
        tft.print("A");
    }
    else if(label == "H19"){      // Columna1 Linea5 0.01kWh historic
      MenuCL=MPTkWhHistCL; SetColumnLine();
        tft.print(MPTkWhHist,0);
        tft.print(MenuSolarMPTkWhHisttag);
        MPTkWhHistStrg=val;
        MPTkWhHist=MPTkWhHistStrg.toInt();
        MPTkWhHist=((MPTkWhHist*centesiTo)+MPTkWhHistAdd); // 0.01kWh   
        tft.setTextColor(MenuTxtColor);
      MenuCL=MPTkWhHistCL; SetColumnLine();
        tft.print(MPTkWhHist,0);
        tft.print(MenuSolarMPTkWhHisttag);
    }
    else if(label == "H20"){      // Columna1 Linea6 0.01kWh HOY
      MenuCL=MPTkWhCL; SetColumnLine();
        tft.print(MPTkWh,0);
        tft.print("kWh");
        MPTkWhStrg=val;
        MPTkWh=MPTkWhStrg.toInt();
        MPTkWh=(MPTkWh*centesiTo); // 0.01kWh   
        tft.setTextColor(MenuTxtColor);
      MenuCL=MPTkWhCL; SetColumnLine();
        tft.print(MPTkWh,0);
        tft.print("kWh");
    }
    else if(label == "H21"){     // Columna1 Linea7 Wmax HOY
      MenuCL=MPTWmaxCL; SetColumnLine();
        tft.print(MPTWmax);
        tft.print("Wmax");
        MPTWmaxStrg=val;
        MPTWmax=MPTWmaxStrg.toInt();
        tft.setTextColor(MenuTxtColor);
      MenuCL=MPTWmaxCL; SetColumnLine();
        tft.print(MPTWmax);
        tft.print("Wmax");
    }
    else if(label == "FW"){     /// Columna2 Linea2 Firmware
      MenuCL=MPTfwCL; SetColumnLine();
        tft.print(MenuSolarMPTfwtag);
        tft.print(MPTfwStrg);
        MPTfwStrg=val;
        tft.setTextColor(MenuTxtColor);
      MenuCL=MPTfwCL; SetColumnLine();
        tft.print(MenuSolarMPTfwtag);  
        tft.print(MPTfwStrg);
    }
    else if(label == "PID"){    /// Columna2 Linea3 Product ID
      MenuCL=MPTpidCL; SetColumnLine();
        tft.print(MenuSolarMPTpidtag);
        tft.print(MPTpidStrg);
        MPTpidStrg=val;
        tft.setTextColor(MenuTxtColor);
      MenuCL=MPTpidCL; SetColumnLine();
        tft.print(MenuSolarMPTpidtag); 
        tft.print(MPTpidStrg);
    }
    else if(label == "SER#"){   /// Columna2 Linea4 Numero de serie producto
      MenuCL=MPTserCL; SetColumnLine();
      tft.print(MenuSolarMPTsertag); 
      tft.print(MPTSERStrg);
        MPTSERStrg=val;
      tft.setTextColor(MenuTxtColor);
      MenuCL=MPTserCL; SetColumnLine();
      tft.print(MenuSolarMPTsertag);
      tft.print(MPTSERStrg);
    }
    else if(label == "ERR"){    /// Columna2 Linea5 Error Code
      MenuCL=MPTerrCL; SetColumnLine();
      tft.print(MenuSolarErrtag);  
      tft.print(MPTErrorStrgmsg);
        MPTErrorStrg=val;
        MPTError=MPTErrorStrg.toInt();
      tft.setTextColor(MenuTxtColor);
      MenuCL=MPTerrCL; SetColumnLine();
      tft.print(MenuSolarErrtag); 
      if(MPTError==0){ // No error
        MPTErrorStrgmsg=MenuSolarErr0Msg;
      }
      else if(MPTError==2){  // Bat too high
        MPTErrorStrgmsg=MenuSolarErr2Msg;
      }
      else if(MPTError==17){ // Temp too high
        MPTErrorStrgmsg=MenuSolarErr17Msg;
      }
      else if(MPTError==18){  // Over Current
        MPTErrorStrgmsg=MenuSolarErr18Msg;
      }
      else if(MPTError==19){  // Current reverse
        MPTErrorStrgmsg=MenuSolarErr19Msg;
      }
      else if(MPTError==20){  // bulk time limit      
        MPTErrorStrgmsg=MenuSolarErr20Msg;
      }
      else if(MPTError==21){  // Current sensor fail
        MPTErrorStrgmsg=MenuSolarErr21Msg;
      }
      else if(MPTError==26){  // Terminal OverHeated
        MPTErrorStrgmsg=MenuSolarErr26Msg;
      }
      else if(MPTError==33){  // Volt too high PV
        MPTErrorStrgmsg=MenuSolarErr33Msg;
      }
      else if(MPTError==34){  // Amp too high PV
        MPTErrorStrgmsg=MenuSolarErr34Msg;
      }
      else if(MPTError==38){  // Excessive voltage battery                          
        MPTErrorStrgmsg=MenuSolarErr38Msg;
      }
      else if(MPTError==116){ // Facturoy calibration lost  
        MPTErrorStrgmsg=MenuSolarErr116Msg;
      }
      else if(MPTError==117){ // Firmware Invalid
        MPTErrorStrgmsg=MenuSolarErr117Msg;
      }
      else if(MPTError==119){  // User Settings invalid
        MPTErrorStrgmsg=MenuSolarErr119Msg;
      }
      tft.print(MPTErrorStrgmsg);
    }
    else if(label == "CS"){     /// Columna2 Linea6 Estado operativo
      MenuCL=MPTcsCL; SetColumnLine();
      tft.print(MPTCStag); 
      tft.print(MPTCSStrgmsg);
        MPTCSStrg=val;
        MPTCS=MPTCSStrg.toInt();
      tft.setTextColor(MenuTxtColor);
      MenuCL=MPTcsCL; SetColumnLine();
      tft.print(MPTCStag);  
      if(MPTCS==0){ // OFF
        MPTCSStrgmsg=MenuSolarCS0Msg;
      }
      else if(MPTCS==2){ // FAULT MPPT & Inverter off hasta hacer Reset
        MPTCSStrgmsg=MenuSolarCS2Msg;
      } 
      else if(MPTCS==3){ // bulk
        MPTCSStrgmsg=MenuSolarCS3Msg;
      }
      else if(MPTCS==4){ // Absorption
        MPTCSStrgmsg=MenuSolarCS4Msg;;
      }
      else if(MPTCS==5){ // Float
        MPTCSStrgmsg=MenuSolarCS5Msg;
      }
      tft.print(MPTCSStrgmsg);
    }
    else if(label == "H22"){    /// Columna2 Linea7 kWh ayer
      MenuCL=MPTkWhAyerCL; SetColumnLine();
      tft.print(MenuSolarkWhAyertag);
      tft.print(MPTkWhAyer);
      MPTkWhAyerStrg=val;
      MPTkWhAyer=MPTkWhAyerStrg.toInt();
      MPTkWhAyer=(MPTkWhAyer*centesiTo);
      tft.setTextColor(MenuTxtColor);
      MenuCL=MPTkWhAyerCL; SetColumnLine();
      tft.print(MenuSolarkWhAyertag);
      tft.print(MPTkWhAyer);
    }
    else if(label == "H23"){    /// Columna2 Linea8 Wmax Ayer
      MenuCL=MPTWmaxAyerCL; SetColumnLine();
        tft.print(MenuSolarMPTWmaxAytag);
        tft.print(MPTWmaxAyer);
        MPTWmaxAyerStrg=val;
        MPTWmaxAyer=MPTWmaxAyerStrg.toInt();
        tft.setTextColor(MenuTxtColor);
      MenuCL=MPTWmaxAyerCL; SetColumnLine();
        tft.print(MenuSolarMPTWmaxAytag);
        tft.print(MPTWmaxAyer);
    } 
  }else{}   
}
void Inverter(){       ///
  ReadSerial1();
  TextSize1();
  tft.setTextColor(MenuTxtColorRst);
  if(Origin=="Inverter"){
    if(label == "V"){                /// Columna1 Linea1 Battery Volt  mV     NO ANIM
      MenuCL=IVTVbatCL; SetColumnLine();
      tft.print(IVTVbat,3);
      tft.print("V");
      IVTVbatStrg=val;
      IVTVbat=IVTVbatStrg.toInt(); 
      IVTVbat=(IVTVbat*milliTo);
      tft.setTextColor(MenuTxtColor);
      MenuCL=IVTVbatCL; SetColumnLine();
      tft.print(IVTVbat,3);
      tft.print("V");
    }
    else if(label=="AR"){              /// INVERTER Alarm Razon
      MenuCL=IVTARCL; SetColumnLine();
      tft.print(MenuInverterARtag);  
      tft.print(IVTARStrgmsg);
      IVTARStrg=val;
      IVTAR=IVTARStrg.toInt();
      tft.setTextColor(MenuTxtColor);
      MenuCL=IVTARCL; SetColumnLine();
      tft.print(MenuInverterARtag);
      if(IVTAR==1){      // LOW VOLTAGE
        IVTARStrgmsg=MenuInverterAR1Msg;
      }
      else if(IVTAR==2){ // HIGH VOLTAGE
        IVTARStrgmsg=MenuInverterAR2Msg;
      }
      else if(IVTAR==32){ // Low Temp
        IVTARStrgmsg=MenuInverterAR32Msg;
      }
      else if(IVTAR==64){ // High Temp
        IVTARStrgmsg=MenuInverterAR64Msg;
      }
      else if(IVTAR==256){ // OverLoad
        IVTARStrgmsg=MenuInverterAR256Msg;
      }
      else if(IVTAR==512){ // DC ripple
        IVTARStrgmsg=MenuInverterAR512Msg;
      }
      else if(IVTAR==1024){ // Low V AC out
        IVTARStrgmsg=MenuInverterAR1024Msg;
      }
      else if(IVTAR==2048){ // High V AC out
        IVTARStrgmsg=MenuInverterAR2048Msg;
      }
      tft.print(IVTARStrgmsg);
    }
    else if(label == "CS"){     /// Columna2 Linea6 Estado operativo
      MenuCL=IVTcsCL; SetColumnLine();
      tft.print(IVTCSStrgmsg);
      IVTCSStrg=val;
      IVTCS=IVTCSStrg.toInt();
      tft.setTextColor(MenuTxtColor);
      MenuCL=IVTcsCL; SetColumnLine();
      if(IVTCS==0){ // OFF
        IVTCSStrgmsg=MenuInverterCS0Msg;
      }
      else if(IVTCS==1){ // LOW POWER
        IVTCSStrgmsg=MenuInverterCS1Msg;
      } 
      else if(IVTCS==2){ // FAULT
        IVTCSStrgmsg=MenuInverterCS3Msg;
      }
      else if(IVTCS==9){ // INVERTER
        IVTCSStrgmsg=MenuInverterCS4Msg;
      }
      tft.print(IVTCSStrgmsg);
    }
    else if(label == "FW"){     /// Columna2 Linea2 Firmware
      MenuCL=IVTfwCL; SetColumnLine();
      tft.print(MenuInverterfwtag);
      tft.print(IVTfwStrg);
      IVTfwStrg=val;
      tft.setTextColor(MenuTxtColor);
      MenuCL=IVTfwCL; SetColumnLine();
      tft.print(MenuInverterfwtag);  
      tft.print(IVTfwStrg);
    }
    else if(label == "PID"){    /// Columna2 Linea3 Product ID
      MenuCL=IVTpidCL; SetColumnLine();
        tft.print(MenuIVTpidtag);
        tft.print(IVTpidStrg);
        IVTpidStrg=val;
        tft.setTextColor(MenuTxtColor);
        MenuCL=IVTpidCL; SetColumnLine();
        tft.print(MenuIVTpidtag); 
        tft.print(IVTpidStrg);
    }
    else if(label == "SER#"){   /// Columna2 Linea4 Numero de serie producto
      MenuCL=IVTserCL; SetColumnLine();
      tft.print(MenuIVTsertag); 
      tft.print(IVTSERStrg);
      IVTSERStrg=val;
      tft.setTextColor(MenuTxtColor);
      MenuCL=IVTserCL; SetColumnLine();
      tft.print(MenuIVTsertag);
      tft.print(IVTSERStrg);
    }
    else if(label=="MODE"){              /// INVERTER Alarm Razon
      MenuCL=IVTmodeCL; SetColumnLine();
      tft.print(MenuInverterModetag);  
      tft.print(IVTmodeStrgmsg);
      IVTModeStrg=val;
      IVTMode=IVTModeStrg.toInt();
      tft.setTextColor(MenuTxtColor);
      MenuCL=IVTmodeCL; SetColumnLine();
      tft.print(MenuInverterModetag);
      if(IVTMode==2){      // INVERTER ON
        IVTmodeStrgmsg=MenuInvertermode2Msg;
      }
      else if(IVTMode==4){ // OFF
        IVTmodeStrgmsg=MenuInvertermode4Msg;
      }
      else if(IVTMode==5){ // ECO
        IVTmodeStrgmsg=MenuInvertermode5Msg;
      }   
      tft.print(IVTmodeStrgmsg);
    }
    else if(label == "AC_OUT_V"){      // Columna1 Linea2 Panel Volt mV
      MenuCL=IVTACVCL; SetColumnLine();
      tft.print(IVTACV,1);
      tft.print(" V AC");
      IVTACVStrg=val;
      IVTACV=IVTACVStrg.toInt();
      IVTACV=(IVTACV*centesiTo);         //0.01 centesimo  CentesiTo
      tft.setTextColor(MenuTxtColor);
      MenuCL=IVTACVCL; SetColumnLine();
      tft.print(IVTACV,1);
      tft.print(" V AC");
    }
    else if(label == "AC_OUT_I"){      // Columna1 Linea2 Panel Volt mV
      MenuCL=IVTACICL; SetColumnLine();
      tft.print(IVTACI,3);
      tft.print(" A AC");
      IVTACIStrg=val;
      IVTACI=IVTACIStrg.toInt();
      IVTACI=(IVTACI*decimTo);         
      tft.setTextColor(MenuTxtColor);
      MenuCL=IVTACICL; SetColumnLine();
      tft.print(IVTACI,3);
      tft.print(" A AC");
    }
    else if(label == "AC_OUT_S"){      // Columna1 Linea2 Panel Volt mV
      MenuCL=IVTACSCL; SetColumnLine();
      tft.print(IVTACS);
      tft.print(" VA");
      IVTACSStrg=val;
      IVTACS=IVTACSStrg.toInt();
      tft.setTextColor(MenuTxtColor);
      MenuCL=IVTACSCL; SetColumnLine();
      tft.print(IVTACS);
      tft.print(" VA");
    }
    else if(label=="WARN"){              /// INVERTER Alarm Razon
      MenuCL=IVTWarnCL; SetColumnLine();
      tft.print(MenuInverterwarntag);  
      tft.print(IVTWarnStrgmsg);
      IVTWarnStrg=val;
      IVTWarn=IVTWarnStrg.toInt();
      tft.setTextColor(MenuTxtColor);
      MenuCL=IVTWarnCL; SetColumnLine();
      tft.print(MenuInverterwarntag);
      if(IVTWarn==1){      // LOW VOLTAGE
        IVTWarnStrgmsg=MenuInverterwarn1Msg;
      } 
      else if(IVTWarn==2){ // HIGH VOLTAGE
        IVTWarnStrgmsg=MenuInverterwarn2Msg;
      }
      else if(IVTWarn==32){ // Low Temp
        IVTWarnStrgmsg=MenuInverterwarn32Msg;
      }
      else if(IVTWarn==64){ // High Temp
        IVTWarnStrgmsg=MenuInverterwarn64Msg;
      }
      else if(IVTWarn==256){ // OverLoad
        IVTWarnStrgmsg=MenuInverterwarn256Msg;
      }
      else if(IVTWarn==512){ // DC ripple
        IVTWarnStrgmsg=MenuInverterwarn512Msg;
      }
      else if(IVTWarn==1024){ // Low V AC out
        IVTWarnStrgmsg=MenuInverterwarn1024Msg;
      }
      else if(IVTWarn==2048){ // High V AC out
        IVTWarnStrgmsg=MenuInverterwarn2048Msg;
      }
      tft.print(IVTWarnStrgmsg);
    }
  }
}
void Battery(){        ///
    TextSize1();
    tft.setTextColor(MenuTxtColor);
    MenuCL=BATkWhCL; SetColumnLine();
    tft.print(BATkWh);
    MenuCL=BATTipCL; SetColumnLine();
    tft.print(BATTip);
    MenuCL=BATDateCL; SetColumnLine();
    tft.print(BATDate);
    MenuCL=BATBrandCL; SetColumnLine();
    tft.print(BATBrand);
    MenuCL=BATModelCL; SetColumnLine();
    tft.print(BATModel);
    MenuCL=BATSerialNumbrCL; SetColumnLine();
    tft.print(BATSerialNumbr);
    MenuCL=BATCellCL; SetColumnLine();
    tft.print(BATCell);
    MenuCL=BATAhCL; SetColumnLine();
    tft.print(BATAh);
    MenuCL=BATVoltCL; SetColumnLine();
    tft.print(BATVolt);
    MenuCL=BATVChargCL; SetColumnLine();
    tft.print(BATVCharg);
    ReadSerial1();
    tft.setTextColor(MenuTxtColorRst);
    if(label == "V"){                /// Columna1 Linea1 Battery Volt  mV     NO ANIM
        ResetIVTData=ResetLoopDefault;  // dato recibido correctamente no resetar los datos
        MenuCL=BATivtVbatCL; SetColumnLine();
        tft.print(batIVTVbattag);
        tft.print(IVTVbat,3);
        tft.print("V");
        IVTVbatStrg=val;
        IVTVbat=IVTVbatStrg.toInt(); 
        IVTVbat=(IVTVbat*milliTo);
        tft.setTextColor(MenuTxtColor);
        MenuCL=BATivtVbatCL; SetColumnLine();
        tft.print(batIVTVbattag);
        tft.print(IVTVbat,3);
        tft.print("V");
        if((IVTVbat<VBatdataErrorL)||(IVTVbat>VBatdataErrorH)){
          ResetIVTData=(ResetIVTData-1);
        if(ResetIVTData<1){           
          DataResetIVT();
          }else{  
        } 
      }
    }
    ReadSerial2();
    tft.setTextColor(MenuTxtColorRst);
    if(label == "V"){             // Columna1 Linea1 Battery Volt  mV     NO ANIM
      ResetMPTData=ResetLoopDefault;  // dato recibido correctamente no resetar los datos
      MenuCL=BATmptVbatCL; SetColumnLine();
      tft.print(batMPTVbattag);
      tft.print(MPTVbat,3);
      tft.print("V");
      MPTVbatStrg=val;
      MPTVbat=MPTVbatStrg.toInt(); 
      MPTVbat=(MPTVbat*milliTo);
      tft.setTextColor(MenuTxtColor);
      MenuCL=BATmptVbatCL; SetColumnLine();
      tft.print(batMPTVbattag);
      tft.print(MPTVbat,3);
      tft.print("V");
      if((MPTVbat<VBatdataErrorL)||(MPTVbat>VBatdataErrorH)){
        ResetMPTData=(ResetMPTData-1);
        if(ResetMPTData<1){           
          DataResetMPT();
          }else{  
        } 
      }
    }
}
void Charger(){  // todavia no hay datos pendiente implementar currento sensor
  ReadSerial2();
  TextSize1();
  tft.setTextColor(MenuTxtColorRst);
  if(Origin=="MPPT"){
    if(label == "V"){             // Columna1 Linea1 Battery Volt  mV     NO ANIM
      MenuCL=CHRGMPTVbatCL; SetColumnLine();
      tft.print(MPTVbat,3);
      tft.print("V");
      MPTVbatStrg=val;
      MPTVbat=MPTVbatStrg.toInt(); 
      MPTVbat=(MPTVbat*milliTo);
      tft.setTextColor(MenuTxtColor);
      MenuCL=CHRGMPTVbatCL; SetColumnLine();
      tft.print(MPTVbat,3);
      tft.print("V");
    }
  }
  CurrentSensor();
  if(CurrSens==NewValue){
  }else{
    TextSize1();
    tft.setTextColor(MenuTxtColorRst);
    MenuCL=CHRGCurrSensCL; SetColumnLine();
    tft.print(CurrSens,1);              ///// B0rrar Current Sensor valor anterior
    tft.print(" A");                     ///// Borrar Letra A
    tft.setTextColor(TextColorBlack);    ///// Escribir en blanco
    MenuCL=CHRGCurrSensCL; SetColumnLine();
    tft.print(NewValue,1);              ///// Escribir Nuevo valor
    tft.print(" A");                     ///// Escribir Letra A
    CurrSens=NewValue;             ///// Establecer el nuevo valor con valor antiguo para la proxima verificacion
  }
  NewValue=Zero;                        ///// Resetear NewValue
}
void Load(){
  ReadSerial1();
  TextSize1();
  tft.setTextColor(MenuTxtColorRst);
  if(Origin=="Inverter"){
    if(label=="AR"){              /// INVERTER Alarm Razon
     MenuCL=IVTARCL; SetColumnLine();
     tft.print(MenuInverterARtag);  
     tft.print(IVTARStrgmsg);
     IVTARStrg=val;
     IVTAR=IVTARStrg.toInt();
     tft.setTextColor(MenuTxtColor);
     MenuCL=IVTARCL; SetColumnLine();
     tft.print(MenuInverterARtag);
     if(IVTAR==1){      // LOW VOLTAGE
       IVTARStrgmsg=MenuInverterAR1Msg;
      }
      else if(IVTAR==2){ // HIGH VOLTAGE
        IVTARStrgmsg=MenuInverterAR2Msg;
      }
      else if(IVTAR==32){ // Low Temp
        IVTARStrgmsg=MenuInverterAR32Msg;
      }
      else if(IVTAR==64){ // High Temp
        IVTARStrgmsg=MenuInverterAR64Msg;
      }
      else if(IVTAR==256){ // OverLoad
        IVTARStrgmsg=MenuInverterAR256Msg;
      }
      else if(IVTAR==512){ // DC ripple
        IVTARStrgmsg=MenuInverterAR512Msg;
      }
      else if(IVTAR==1024){ // Low V AC out
        IVTARStrgmsg=MenuInverterAR1024Msg;
      }
      else if(IVTAR==2048){ // High V AC out
        IVTARStrgmsg=MenuInverterAR2048Msg;
      }
      tft.print(IVTARStrgmsg);
    }
    else if(label=="MODE"){              /// INVERTER Alarm Razon
      MenuCL=IVTmodeCL; SetColumnLine();
      tft.print(MenuInverterModetag);  
      tft.print(IVTmodeStrgmsg);
      IVTModeStrg=val;
      IVTMode=IVTModeStrg.toInt();
      tft.setTextColor(MenuTxtColor);
      MenuCL=IVTmodeCL; SetColumnLine();
      tft.print(MenuInverterModetag);
      if(IVTMode==2){      // INVERTER ON
        IVTmodeStrgmsg=MenuInvertermode2Msg;
      }
      else if(IVTMode==4){ // OFF
        IVTmodeStrgmsg=MenuInvertermode4Msg;
      }
      else if(IVTMode==5){ // ECO
        IVTmodeStrgmsg=MenuInvertermode5Msg;
      }   
      tft.print(IVTmodeStrgmsg);
    }
    else if(label == "AC_OUT_V"){      // Columna1 Linea2 Panel Volt mV
      MenuCL=IVTACVCL; SetColumnLine();
      tft.print(IVTACV,1);
      tft.print(" V AC");
      IVTACVStrg=val;
      IVTACV=IVTACVStrg.toInt();
      IVTACV=(IVTACV*centesiTo);         //0.01 centesimo  CentesiTo
      tft.setTextColor(MenuTxtColor);
      MenuCL=IVTACVCL; SetColumnLine();
      tft.print(IVTACV,1);
      tft.print(" V AC");
    }
    else if(label == "AC_OUT_I"){      // Columna1 Linea2 Panel Volt mV
      MenuCL=IVTACICL; SetColumnLine();
      tft.print(IVTACI,3);
      tft.print(" A AC");
      IVTACIStrg=val;
      IVTACI=IVTACIStrg.toInt();
      IVTACI=(IVTACI*decimTo);         
      tft.setTextColor(MenuTxtColor);
      MenuCL=IVTACICL; SetColumnLine();
      tft.print(IVTACI,3);
      tft.print(" A AC");
    }
    else if(label == "AC_OUT_S"){      // Columna1 Linea2 Panel Volt mV
      MenuCL=IVTACSCL; SetColumnLine();
      tft.print(IVTACS);
      tft.print(" VA");
      IVTACSStrg=val;
      IVTACS=IVTACSStrg.toInt();
      tft.setTextColor(MenuTxtColor);
      MenuCL=IVTACSCL; SetColumnLine();
      tft.print(IVTACS);
      tft.print(" VA");
    }
    else if(label=="WARN"){              /// INVERTER Alarm Razon
      MenuCL=IVTWarnCL; SetColumnLine();
      tft.print(MenuInverterwarntag);  
      tft.print(IVTWarnStrgmsg);
      IVTWarnStrg=val;
      IVTWarn=IVTWarnStrg.toInt();
      tft.setTextColor(MenuTxtColor);
      MenuCL=IVTWarnCL; SetColumnLine();
      tft.print(MenuInverterwarntag);
      if(IVTWarn==1){      // LOW VOLTAGE
        IVTWarnStrgmsg=MenuInverterwarn1Msg;
      } 
      else if(IVTWarn==2){ // HIGH VOLTAGE
        IVTWarnStrgmsg=MenuInverterwarn2Msg;
      }
      else if(IVTWarn==32){ // Low Temp
        IVTWarnStrgmsg=MenuInverterwarn32Msg;
      }
      else if(IVTWarn==64){ // High Temp
        IVTWarnStrgmsg=MenuInverterwarn64Msg;
      }
      else if(IVTWarn==256){ // OverLoad
        IVTWarnStrgmsg=MenuInverterwarn256Msg;
      }
      else if(IVTWarn==512){ // DC ripple
        IVTWarnStrgmsg=MenuInverterwarn512Msg;
      }
      else if(IVTWarn==1024){ // Low V AC out
        IVTWarnStrgmsg=MenuInverterwarn1024Msg;
      }
      else if(IVTWarn==2048){ // High V AC out
        IVTWarnStrgmsg=MenuInverterwarn2048Msg;
      }
      tft.print(IVTWarnStrgmsg);
    }
  }
}
void MenuCheck(){   // go to touched menu
  if(MenuPresent=="Home"){
      Home();
    }else if(MenuPresent=="Solar"){
      Solar();
    }else if(MenuPresent=="Bat"){
      Battery();
    }else if(MenuPresent=="Inverter"){
      Inverter();
    }else if(MenuPresent=="Charger"){
      Charger();
    }else if(MenuPresent=="Load"){
     Load();
  }
}
void SetColumnLine() {   //// Copilot Asist 
  int column, line;
  sscanf(MenuCL.c_str(), "C%dL%d", &column, &line);
  line--;                                          // start from line0
  if (column == 1) {
    tft.setCursor(MenuColumn1X, MenuColumn1Y + (MenuColumn1NextLine * line));
  } else if (column == 2) {
    tft.setCursor(MenuColumn2X, MenuColumn2Y + (MenuColumn2NextLine * line));
  }
}

void setup(){
  pinMode(CS_PIN,OUTPUT);                                // a nivel alto al iniciar
  digitalWrite(CS_PIN,HIGH);                             //  a nivel alto al iniciar
  pinMode(TFT_CS,OUTPUT);                                 // Configurar como salida
  pinMode(CurrSensPin, INPUT);                           // Configura el pin como entrada
  analogReadResolution(12);                              //  config 12 bits para el ADC
  SerialIni();
  TFTSetIni();
  Touched();
  TextSetup();
}

void loop(){  
  currentMillis = millis();
  if (currentMillis - previousMillis >= msLoop) {
    previousMillis = currentMillis;
     MenuCheck();
     TouchAsk();
  }
}

