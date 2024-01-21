/* Solar Monitor v0.1.1 alfa ManuUniverso 2024

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
0.1.1 01/14/24 Added touch function with submenu

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

   The libraries Available directly from the Arduino IDE library manager.
   -XPT2046_Touchscreen.h
   -Adafruit_ILI9341.h
   -Adafruit_GFX.h
   -Fonts/FreeSansBold9pt7b.h No need to download, it is included in the previous ones.
   -Fonts/FreeSansBold18pt7b.h No need to download, it is included in the previous ones.
   Pin & Config:
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

#define UARTRate1 19200                                    /// VE.Direct Inverter
#define UART1rx 27                                         /// VE.Direct Inverter TX---> RX Esp32
#define UART1tx 14                                         /// VE.Direct Inverter RX---> TX Esp32 en este codigo no se usa (solo se hace lectura con RXEsp32)
#define UARTRate2 19200                                    /// VE.Direct MPPT  no es necesario asignar, usar los pines previsto RX2 y TX2 en el ESP32
#define ResetLoopDefault 200                               /// VE.Direct Data Reset Cuenta atras para reseteo      los 3 deben ser igual  ResetMPTData ResetIVTData ResetLoopDefault 
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
#define msLoop 15                                          /// Delay ms Loop
#define CirclRad  3                                        /// Imagen Animacion Bola tamaño Radio
#define CirclColor 0x4C5A                                  /// Imagen Animacion Bola Color Azul Victron Energy efecto de la bola animada por el cable
#define TextColorBlue 0x4C5A                               /// Imagen Text Color Azul Victron Energy para borrar texto con fondo azul
#define LedRad 4                                           /// Imagen Led Estado tamaño Radio ( simula un led en el display )
#define LedMPPTX 221                                       /// Imagen Menu Home MPPT Led estado Cursor eje X
#define LedMPPTY 115                                       /// Imagen Menu Home MPPT Led estado Cursor eje Y
#define LedIVTX 155                                        /// Imagen Menu Home Inverter Led estado Cursor eje X
#define LedIVTY 32                                         /// Imagen Menu Home Inverter Led estado Cursor eje Y
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
#define IVTAmpInLoss 0.09                                  /// Inverter para poder estimar el Amp DC estimado aproximado con perdida de 0.09 = 9%  
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
#define MPTVbatX 225                                       /// Imagen Text Menu Home MPPT Vbat Cursor Eje  X
#define MPTVbatY 158                                       /// Imagen Text Menu Home MPPT Vbat Cursor Eje  Y
#define IVTVbatX 145                                       /// Imagen Text Menu Home Inverter Vbat Cursor Eje  X
#define IVTVbatY 81                                        /// Imagen Text Menu Home Inverter Vbat Cursor Eje  Y
#define MPTVpvX 218                                        /// Imagen Text Menu Home MPPT Vpv Cursor Eje  X
#define MPTVpvY 226                                        /// Imagen Text Menu Home MPPT Vpv Cursor Eje  Y
#define MPTAmpX 281                                        /// Imagen Text Menu Home MPPT Amp pv Cursor Eje  X
#define MPTAmpY 158                                        /// Imagen Text Menu Home MPPT Amp pv Cursor Eje  Y
#define MPTPpvX 225                                        /// Imagen Text Menu Home MPPT Watt pv Cursor Eje  X
#define MPTPpvY 196                                        /// Imagen Text Menu Home MPPT Watt pv Cursor Eje  Y
#define MPTkWhX 266                                        /// Imagen Text Menu Home MPPT kWh pv Cursor Eje  X
#define MPTkWhY 226                                        /// Imagen Text Menu Home MPPT kWh pv Cursor Eje  Y
#define MPTWmaxX 230                                       /// Imagen Text Menu Home MPPT Watt Max pv Cursor Eje  X
#define MPTWmaxY 138                                       /// Imagen Text Menu Home MPPT Watt Max pv Cursor Eje  Y
#define MPTStatX 239                                       /// Imagen Text Menu Home MPPT Estado Cursor Eje  X
#define MPTStatY 121                                       /// Imagen Text Menu Home MPPT Estado Cursor Eje  Y 
#define IVTWarnX 107                                       /// Imagen Text Menu Home Inverter Warning Cursor Eje  X
#define IVTWarnY 203                                       /// Imagen Text Menu Home Inverter Warning Cursor Eje  Y
#define IVTACIX 122                                        /// Imagen Text Menu Home Inverter AC Amp eje X
#define IVTACIY 21                                         /// Imagen Text Menu Home Inverter AC Amp eje Y
#define IVTACVX 160                                        /// Imagen Text Menu Home Inverter AC Volt eje X
#define IVTACVY 21                                         /// Imagen Text Menu Home Inverter AC Volt eje Y
#define IVTACWX 230                                        /// Imagen Text Menu Home Inverter AC AV(aproximadamente Watt) eje X
#define IVTACWY 65                                         /// Imagen Text Menu Home Inverter AC AV(aproximadamente Watt) eje Y
#define IVTAmpInX 162                                      /// Imagen Text Menu Home Inverter Amp input estimado aproximado eje X
#define IVTAmpInY 117                                      /// Imagen Text Menu Home Inverter Amp input estimado aproximado eje Y
#define MPTVbatCL     "C1L1"                               /// Imagen Text SubMenu MPPT Columna Linea V bat seleccionar la columna y linea donde se mostrara cada dato
#define MPTiCL        "C1L2"                               /// Imagen Text SubMenu MPPT Columna Linea A bat
#define MPTVpvCL      "C1L3"                               /// Imagen Text SubMenu MPPT Columna Linea V pv
#define MPTPpvCL      "C1L4"                               /// Imagen Text SubMenu MPPT Columna Linea W pv
#define MPTkWhHistCL  "C1L5"                               /// Imagen Text SubMenu MPPT Columna Linea kWh historico
#define MPTkWhCL      "C1L6"                               /// Imagen Text SubMenu MPPT Columna Linea kWh
#define MPTWmaxCL     "C1L7"                               /// Imagen Text SubMenu MPPT Columna Linea Wmax
//#define xxxx        "C1L8"                               /// Imagen Text SubMenu MPPT no se usa en esta version version del codigo
#define MPTcsCL       "C2L1"                               /// Imagen Text SubMenu MPPT Columna Linea Estado  
#define MPTfwCL       "C2L2"                               /// Imagen Text SubMenu MPPT Columna Linea Firwmare
#define MPTpidCL      "C2L3"                               /// Imagen Text SubMenu MPPT Columna Linea Producto id
#define MPTserCL      "C2L4"                               /// Imagen Text SubMenu MPPT Columna Linea numero de serie
#define MPTerrCL      "C2L5"                               /// Imagen Text SubMenu MPPT Columna Linea Errores
#define MPTkWhAyerCL  "C2L6"                               /// Imagen Text SubMenu MPPT Columna Linea kWh ayer
#define MPTWmaxAyerCL "C2L7"                               /// Imagen Text SubMenu MPPT Columna Linea Wmax ayer
//#define xxxx        "C1L8"                               /// Imagen Text SubMenu MPPT no se usa en esta version del codigo
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
#define IVTVbatCL "C1L1"                                   /// Imagen Text SubMenu Inverter Columna Linea V Bat
#define IVTARCL "C1L2"                                     /// Imagen Text SubMenu Inverter Columna Linea Alarm
#define IVTcsCL "C1L3"                                     /// Imagen Text SubMenu Inverter Columna Linea Estado
#define IVTfwCL "C1L4"                                     /// Imagen Text SubMenu Inverter Columna Linea Firmware
#define IVTpidCL "C1L5"                                    /// Imagen Text SubMenu Inverter Columna Linea Product ID
#define IVTserCL "C1L6"                                    /// Imagen Text SubMenu Inverter Columna Linea Numero de Serie NS
#define IVTmodeCL "C1L7"                                   /// Imagen Text SubMenu Inverter Columna Linea Modo
#define IVTACVCL "C2L1"                                    /// Imagen Text SubMenu Inverter Columna Linea AC Volt
#define IVTACICL "C2L2"                                    /// Imagen Text SubMenu Inverter Columna Linea AC Amp
#define IVTACSCL "C2L3"                                    /// Imagen Text SubMenu Inverter Columna Linea AC AV (aproximadamente Watt)
#define IVTWarnCL "C2L4"                                   /// Imagen Text SubMenu Inverter Columna Linea Warning
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
#define MenuInvertermode2Msg "Mode ON"                     /// Imagen Text SubMenu Inverter Modo 2 ON
#define MenuInvertermode4Msg "Mode OFF"                    /// Imagen Text SubMenu Inverter Modo 4 OFF
#define MenuInvertermode5Msg "Mode ECO"                    /// Imagen Text SubMenu Inverter MOdo 5 ECO
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
#define BATSerialNumbr "PPTxxxxxxxxxx"                  /// Imagen Text SubMenu Bateria Numero de Serie
#define BATCell "IFpP/13/141/238/[(3P3S)2S+3P2S]M/0+50/95" /// Imagen Text SubMenu Bateria Celda tipologia estructura

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
String Origin;                       /// VE.Direct Lectura de datos organizado por Origen   MPPT o Inverter
String label;                        /// VE.Direct Lectura de datos tipo de lectura         V, VPV, PPV, AC V, AC I, ALARM, WARM, ... 
String val;                          /// VE.Direct Lectura de datos valor de lectura        26.4v, error4, ...
String MPTErrorStrg;                 /// MPPT String Error 
String MPTErrorStrgmsg;              /// MPPT Msg Error
String MPTCSStrg;                    /// MPPT String Estado
String MPTCSStrgmsg;                 /// MPPT Msg Estado
String MPTSERStrg;                   /// MPPT String Numero de Serie del producto 
String MPTpidStrg;                   /// MPPT String ID del Producto
String MPTfwStrg;                    /// MPPT String Firmware
//String MPTHSDSStrg;                /// MPPT String Dias secuencia [ no usado en esta version ]
String MPTWmaxAyerStrg;              /// MPPT String Wmax Ayer
String MPTkWhAyerStrg;               /// MPPT String kWh Ayer 
String MPTVbatStrg;                  /// MPPT String Volt Bat 
String MPTVpvStrg;                   /// MPPT String Volt Solar panel
String MPTPpvStrg;                   /// MPPT String Watt Solar panel
String MPTAmpStrg;                   /// MPPT String Amp
String MPTkWhStrg;                   /// MPPT String kWh HOY
String MPTWmaxStrg;                  /// MPPT String W max HOY
String MPTkWhHistStrg;               /// MPPT String kWh historico
String MenuCL;                       /// Imagen Text Linea y Columna elegida                                    (no usado en Menu Home, solo submenu Solar, Charger, Inverter Bateria ..)
String MenuPresent="Home";           /// Menu en Display actualmente
String MenuSelected;                 /// Menu Solicitado al tocar la pantalla

int MovSpeed= 4;                     /// Imagen Animacion Bola Animada Velociad de movimiento depende tambien del loop -->  pixel por loop
int IVTCS;                           /// Inverter Estado integral
int IVTMode;                         /// Inverter Modo integral
int IVTACV;                          /// Inverter AC Volt
int IVTACI;                          /// Inverter AC Amp
int IVTACS;                          /// Inverter AV "aproximadamente Watt AC"
int IVTWarn;                         /// Inverter Warning
int IVTAR;                           /// Inverter Alarm
int16_t IVTAmpIn;                    /// Inverter Amp Input (DC) Estimados aproximado 9% perdida esto no tiene en cuenta carga inductiva o resistiva
float IVTVbat;                       /// Inverter Bateria Volt
float MPTVbat;                       /// MPPT Bateria Volt   float para poder calcular el % correctamente necesita decimales
int16_t MPTVpv;                      /// MPPT Solar Volt paneles
int16_t MPTPpv;                      /// MPPT Watt Solar panel
int16_t MPTAmp;                      /// MPPT Amp
float MPTkWh;                        /// MPPT kWh HOY
int16_t MPTWmax;                     /// MPPT W max HOY
int MPTkWhHist;                      /// MPPT kWh Historico
int MPTError;                        /// MPPT error int                                           
int MPTCS;                           /// MPPT estado int
// int MPTHSDS;                      /// MPPT dias secuencia no es usa en esta version del codidgo
int MPTWmaxAyer;                     /// MPPT Wmax Ayer int
int MPTkWhAyer;                      /// MPPT kWh Ayer int
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

bool AnimSolOut=false;               /// Imagen Animacion interruptor de activacion Solar Output
bool AnimIVTOutput=false;            /// Imagen Animacion interruptor de activacion Inverter-->Load Output
bool AnimIVTInput=false;             /// Imagen Animacion interruptor de activacion Inverter Input
bool AnimChargIVT=false;             /// Imagen Animacion interruptor de activacion Charger-->Inverter
bool AnimChargBat=false;             /// Imagen Animacion interruptor de activacion Charger-->Bateria
bool AnimBatOut=false;               /// Imagen Animacion interruptor de activacion Bateria Output
bool AnimBatIn=false;                /// Imagen Animacion interruptor de activacion Bateria Input

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC,TFT_RST);  /// Display ili9341 pin IMAGEN
XPT2046_Touchscreen ts(CS_PIN, TIRQ_PIN);                         /// Display ili9341 pin TOUCH   Param 2 - Touch IRQ Pin - interrupt enabled polling

void SerialIni(){
  //Serial.begin(38400);
  Serial1.begin(UARTRate1,SERIAL_8N1,UART1rx,UART1tx);  // Serial 1 OK reasignado RX en GPIO27 D27 [ y TX en GPIO14 D14 no probado ]
  Serial2.begin(UARTRate2,SERIAL_8N1);                  // Serial 2 MPPT     ( se pueden cambiar los pines en la config esp32) 
}
void TextSetup(){
  tft.setTextColor(ILI9341_WHITE);        // Mostrar "No Hay Contact"
  //tft.setTextSize(3);                   //     no se usa con fonts
}
void TFTSetIni(){
  tft.begin();        // activar la imagen on rotation   se puede subir a tft.begin(40000000) para evitar parpadeos 
  tft.setRotation(3); // 0 1 2 3 0º 90º 180º 270º
  ts.begin();         // activar touch misma rotation que la imagen
  ts.setRotation(3);
}
void HomeBackGround(){//
  tft.drawRGBBitmap(0, 2, HomeBitmap, HOME_WIDTH,HOME_HEIGHT);
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
  tft.fillCircle(BATFinX, BATStrtY, CirclRad,CirclColor); // discharge
  if(BATStrtX<BATFinX){
    BATFinX= (BATFinX - MovSpeed);
    }else{
    BATFinX=BATFinXDef;
  }
}
void AnimBatInput(){//
  tft.drawRGBBitmap(93, 146, BATInvrtBitmap, BATInvrt_WIDTH,BATInvrt_HEIGHT);
  tft.fillCircle(BATStrtX, BATStrtY, CirclRad,CirclColor); // charge
  if(BATStrtX<BATFinX){
      BATStrtX= (BATStrtX + MovSpeed);
    }else{
      BATStrtX=BATStrtXDef;
  }
}
void TextSize1(){
  tft.setFont(TxtSize1);
}
void TextSize2(){
  tft.setFont(TxtSize2);
}
void DataResetMPT(){   
  MPTVbat=0;      // Volt Bat   float para poder calcular el % correctamente
  MPTVpv=0;       // Volt Solar paneles
  MPTPpv=0;       // Watt Solar panel
  MPTAmp=0;       // Amp
  MPTCS=7;      // CS7 no es un valor del mppt posible, anoto como valor de sin lectura reset
  AnimSolOut=false;
  HomeBackGround();
  ResetMPTData=ResetLoopDefault;  // dato recibido correctamente no resetar los datos
}
void DataResetIVT(){
  IVTVbat=0;      // Volt Bat
  IVTACV=0;       // AC Volt
  IVTACI=0;       // AC Amp
  IVTACS=0;       // AC Watt
  IVTAmpIn=0;     // DC input Inverter Estimados aproximado 9% perdida
  AnimIVTOutput=false;
  AnimIVTInput=false;
  HomeBackGround();
  ResetIVTData=ResetLoopDefault;  // dato recibido correctamente no resetar los datos
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
void CheckData(){
  if(Origin=="MPTnodata"){           // error de lectura del MPPT UART2 Serial2
    ResetMPTData=(ResetMPTData-1);
    if(ResetMPTData<1){
      DataResetMPT();               // resetear datos de pantalla
    }else{}
   }else{
  }
  if(Origin=="IVTnodata"){          // error de lectura del Inverter UART1 Serial1
    ResetIVTData=(ResetIVTData-1);
    if(ResetIVTData<1){
      DataResetIVT();                // resetear datos de pantalla
    }else{}
   }else{
  }
  if(label == "V"){         // Battery Volt  mV     NO ANIM
    if(Origin=="MPPT"){
        TextSize1();
        tft.setTextColor(ILI9341_ORANGE);
        tft.setCursor(MPTVbatX, MPTVbatY);  // Mostrar valor en pantalla Watt
        tft.print(MPTVbat,1);
        tft.print("V");
        MPTVbatStrg=val;
        MPTVbat=MPTVbatStrg.toInt();
        MPTVbat=(MPTVbat*milliTo); //mV to V
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(MPTVbatX, MPTVbatY);  // Mostrar valor en pantalla Watt
        tft.print(MPTVbat,1);
        tft.print("V");
      }else{
    }
    if(Origin=="Inverter"){
        TextSize1();
        tft.setTextColor(ILI9341_ORANGE);
        tft.setCursor(IVTVbatX, IVTVbatY);  // Mostrar valor en pantalla Watt
        tft.print(IVTVbat,1);
        tft.print("V");
        IVTVbatStrg=val;
        IVTVbat=IVTVbatStrg.toInt();
        IVTVbat=(IVTVbat*milliTo); //mV to V
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(IVTVbatX, IVTVbatY);  // Mostrar valor en pantalla Watt
        tft.print(IVTVbat,1);
        tft.print("V"); 
      }else{   
    }  
  }
  else if(label == "VPV"){  //Panel Volt mV
    if(Origin=="MPPT"){
        TextSize1();
        tft.setTextColor(ILI9341_ORANGE);
        tft.setCursor(MPTVpvX, MPTVpvY);  // Mostrar valor en pantalla Watt
        tft.print(MPTVpv);
        tft.print("V");
        MPTVpvStrg=val;
        MPTVpv=MPTVpvStrg.toInt();
        if(MPTVpv<0){
         MPTVpv=(MPTVpv*-1); // no se porque a veces el valor sale negativo supongo que al pasar de string a int lo hace no siempre bien
        }else{}
        MPTVpv=(MPTVpv*milliTo); // mV to V
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(MPTVpvX, MPTVpvY);  // Mostrar valor en pantalla Watt
        tft.print(MPTVpv);
        tft.print("V");
      }
  }
  else if(label == "PPV"){  // Panel Power W
    if(Origin=="MPPT"){
        TextSize2();
        tft.setTextColor(ILI9341_ORANGE);
        tft.setCursor(MPTPpvX, MPTPpvY);  // Mostrar valor en pantalla Watt
        tft.print(MPTPpv);
        tft.print("W");
        MPTPpvStrg=val;
        MPTPpv=MPTPpvStrg.toInt();
        if(MPTPpv>1){
          AnimSolOut=true;
        }else{
          AnimSolOut=false;
        }
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(MPTPpvX, MPTPpvY);  // Mostrar valor en pantalla Watt
        tft.print(MPTPpv);
        tft.print("W");
      }
  }
  else if(label == "I"){    // mA Battery mA
    if(Origin=="MPPT"){  
        TextSize1();
        tft.setTextColor(ILI9341_ORANGE);
        tft.setCursor(MPTAmpX, MPTAmpY);  // Mostrar valor en pantalla Watt
        tft.print(MPTAmp);
        tft.print("A");
        MPTAmpStrg=val;
        MPTAmp=MPTAmpStrg.toInt();
        MPTAmp=(MPTAmp*milliTo);  //mA to A
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(MPTAmpX, MPTAmpY);  // Mostrar valor en pantalla Watt
        tft.print(MPTAmp);
        tft.print("A");
      }
  }
  else if(label == "H20"){  // 0.01kWh HOY
    if(Origin=="MPPT"){  
        TextSize1();
        tft.setTextColor(ILI9341_ORANGE);
        tft.setCursor(MPTkWhX, MPTkWhY);  // Mostrar valor en pantalla Watt
        tft.print(MPTkWh,2);
        MPTkWhStrg=val;
        MPTkWh=MPTkWhStrg.toInt();
        MPTkWh=(MPTkWh); // *0.01 para kWh   *1 para Wh   
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(MPTkWhX, MPTkWhY);  // Mostrar valor en pantalla Watt
        tft.print(MPTkWh,2);
      }
  }
  else if(label == "H21"){ // Wmax HOY
    if(Origin=="MPPT"){  
        TextSize1();
        tft.setTextColor(ILI9341_ORANGE);
        tft.setCursor(MPTWmaxX, MPTWmaxY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmax);
        tft.print("Wmax");
        MPTWmaxStrg=val;
        MPTWmax=MPTWmaxStrg.toInt();
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(MPTWmaxX,MPTWmaxY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmax);
        tft.print("Wmax");
      }
  }
  else if(label == "CS"){ // Estado operativo
    if(Origin=="MPPT"){
      MPTCSStrg=val;
      MPTCS=MPTCSStrg.toInt();
      if(MPTCS==0){                                              // OFF
       tft.fillCircle(LedMPPTX, LedMPPTY, LedRad,ILI9341_BLACK);
        }
        else if(MPTCS==2){                                       // FAULT MPPT & Inverter off hasta hacer Reset
        tft.fillCircle(LedMPPTX, LedMPPTY, LedRad,ILI9341_RED);
        } 
        else if(MPTCS==3){                                       // bulk
        tft.fillCircle(LedMPPTX, LedMPPTY, LedRad,ILI9341_BLUE);
        }
        else if(MPTCS==4){                                       // Absorption
        tft.fillCircle(LedMPPTX, LedMPPTY, LedRad,ILI9341_YELLOW);
        }
        else if(MPTCS==5){                                       // Float
        tft.fillCircle(LedMPPTX, LedMPPTY, LedRad,ILI9341_GREEN);
        }
    }
  }
  else if(label=="MODE"){  // INverter Modo ON OFF ECO
    if(Origin=="Inverter"){
      IVTModeStrg=val;
      IVTMode=IVTModeStrg.toInt();
      if(IVTMode==2){                                        // inverter ON
      tft.fillCircle(LedIVTX, LedIVTY, LedRad,ILI9341_GREEN);
      }
      else if(IVTMode==4){                                  // inverter OFF
      tft.fillCircle(LedIVTX, LedIVTY, LedRad,ILI9341_WHITE);
      }
      else if(IVTMode==5){                                  // inverter ECO
      tft.fillCircle(LedIVTX, LedIVTY, LedRad,ILI9341_BLUE);
      }
    }
  }
  else if(label=="WARN"){    // INVERTER Alarm Razon
    if(Origin=="Inverter"){
    tft.fillCircle(LedIVTX, LedIVTY, LedRad,ILI9341_RED);
      if(IVTWarnStrg=="1"){ // volta bajo
        TextSize1();
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(IVTWarnX, IVTWarnY);  // 
        tft.print(IVTWarnStrg);
        IVTWarnStrg=Warm1;
        tft.setTextColor(ILI9341_RED);
        tft.setCursor(IVTWarnX,IVTWarnY);  //
        tft.print(IVTWarnStrg);
      }
      else if(IVTWarnStrg=="2"){ //Volt alto
        TextSize1();
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(IVTWarnX, IVTWarnY);  // 
        tft.print(IVTWarnStrg);
        IVTWarnStrg=Warm2;
        tft.setTextColor(ILI9341_RED);
        tft.setCursor(IVTWarnX,IVTWarnY);  // 
         tft.print(IVTWarnStrg);
      }
      else if(IVTWarnStrg=="32"){ // Temperatura baja
        TextSize1();
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(IVTWarnX, IVTWarnY);  
        tft.print(IVTWarnStrg);
        IVTWarnStrg=Warm32;
        tft.setTextColor(ILI9341_RED);
        tft.setCursor(IVTWarnX,IVTWarnY);  
        tft.print(IVTWarnStrg);
      }
      else if(IVTWarnStrg=="64"){ // Temperatura alta
        TextSize1();
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(IVTWarnX, IVTWarnY);  
        tft.print(IVTWarnStrg);
        IVTWarnStrg=Warm64;
        tft.setTextColor(ILI9341_RED);
        tft.setCursor(IVTWarnX,IVTWarnY);  
        tft.print(IVTWarnStrg);
      }
      else if(IVTWarnStrg=="256"){ // Sobrecarca se load demasiado alto
        TextSize1();
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(IVTWarnX, IVTWarnY);  
        tft.print(IVTWarnStrg);
        IVTWarnStrg=Warm256;
        tft.setTextColor(ILI9341_RED);
        tft.setCursor(IVTWarnX,IVTWarnY);  
        tft.print(IVTWarnStrg);
      }
      else if(IVTWarnStrg=="512"){ // rizado de volt continuos de la batteria ruido
        TextSize1();
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(IVTWarnX, IVTWarnY);  
        tft.print(IVTWarnStrg);
        IVTWarnStrg=Warm512;
        tft.setTextColor(ILI9341_RED);
        tft.setCursor(IVTWarnX,IVTWarnY);  
        tft.print(IVTWarnStrg);
      }
      else if(IVTWarnStrg=="1024"){ // AC Volt demasiado bajo
        TextSize1();
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(IVTWarnX, IVTWarnY);  
        tft.print(IVTWarnStrg);
       IVTWarnStrg=Warm1024;
        tft.setTextColor(ILI9341_RED);
        tft.setCursor(IVTWarnX,IVTWarnY);  
        tft.print(IVTWarnStrg);
      }
      else if(IVTWarnStrg=="2048"){ // AC volt demasiado alto
        TextSize1();
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(IVTWarnX, IVTWarnY);  
        tft.print(IVTWarnStrg);
        IVTWarnStrg=Warm2048;
        tft.setTextColor(ILI9341_RED);
        tft.setCursor(IVTWarnX,IVTWarnY);  
        tft.print(IVTWarnStrg);
      }
    }
  }
  else if(label=="AC_OUT_V"){  // AC volt 0.01V
    if(Origin=="Inverter"){
        TextSize1();
        tft.setTextColor(TextColorBlue);
        tft.setCursor(IVTACVX, IVTACVY);  // Mostrar valor en pantalla Watt
        tft.print(IVTACV);
        tft.print("V");
        IVTACVStrg=val;
        IVTACV=IVTACVStrg.toInt();
        IVTACV=(IVTACV*centesiTo);
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(IVTACVX, IVTACVY);  // Mostrar valor en pantalla Watt
        tft.print(IVTACV);
        tft.print("V");
    }
  }
  else  if(label=="AC_OUT_I"){ // AC amp 0.1A
    if(Origin=="Inverter"){
        TextSize1();
        tft.setTextColor(TextColorBlue);
        tft.setCursor(IVTACIX, IVTACIY);  // Mostrar valor en pantalla Watt
        tft.print(IVTACI);
        tft.print("A");
        IVTACIStrg=val;
        IVTACI=IVTACIStrg.toInt();
        IVTACI=(IVTACI*decimTo);
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(IVTACIX, IVTACIY);  // Mostrar valor en pantalla Watt
        tft.print(IVTACI);
        tft.print("A");
    }
  }
  else  if(label=="AC_OUT_S"){ // AC amp 0.1A
    if(Origin=="Inverter"){
        TextSize2();
        tft.setTextColor(ILI9341_GREEN);
        tft.setCursor(IVTACWX, IVTACWY);  // Mostrar valor en pantalla Watt
        tft.print(IVTACS);
        tft.print("W");
        IVTACSStrg=val;
        IVTACS=IVTACSStrg.toInt();
        IVTACS=(IVTACS);
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(IVTACWX, IVTACWY);  // Mostrar valor en pantalla Watt
        tft.print(IVTACS);
        tft.print("W");
      }
       if(IVTACS>2){
        AnimIVTInput=true;
      }else{
        AnimIVTInput=false;
    }
  }
  else{
      ResetMPTData=(ResetMPTData-1);
      ResetIVTData=(ResetIVTData-1);
    if(ResetMPTData<1){
      DataResetMPT();  
      }else{     
    }
    if(ResetIVTData<1){
      DataResetIVT();  
      }else{
    }
  }
}
void InvrtAmpImput(){   /// Amp input Inverter Estimado
  if((IVTACS>1)&&(IVTVbat>1)){
        TextSize1();
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(IVTAmpInX, IVTAmpInY);  // Mostrar valor en pantalla Watt
        tft.print(IVTAmpIn);
        tft.print("A");
        IVTAmpIn=((IVTACS / IVTVbat)*IVTAmpInLoss); // Amp inpout estimado con perdida estimada
        tft.setTextColor(ILI9341_BLACK);
        tft.setCursor(IVTAmpInX, IVTAmpInY);  // Mostrar valor en pantalla Watt
        tft.print(IVTAmpIn);
        tft.print("A");
  }else{}
}
void BatPorcentCalc(){  /// Menu Home Barras Nivel Bateria
  if(MPTVbat<BatLevel1){                                         // x , y, ancho hacia derecha , alto hacia abajo, color
    tft.fillRect(20, 134, 70, 72 , TextColorBlue);               // Nivel de bateria muy bajo borrar todo slo recuatros verde del nivel
  }
  if(MPTVbat>BatLevel1){
    tft.fillRoundRect(20, 204, 70, 15 ,3, ILI9341_GREENYELLOW); // Nivel 1 encender recuadro verde del nivel 1
  }  
  if(MPTVbat>BatLevel2){
    tft.fillRoundRect(20, 184, 70, 15 , 3,ILI9341_GREENYELLOW);  // Nivel 2...
  }
  if(MPTVbat>BatLevel3){
    tft.fillRoundRect(20, 164, 70, 15 , 3,ILI9341_GREENYELLOW);  // Nivel 3...
  }
  if(MPTVbat>BatLevel4){
    tft.fillRoundRect(20, 144, 70, 15 , 3 ,ILI9341_GREENYELLOW); // Nivel 4....
  }
}
void SwitchAnim(){     /// Animacion activar desactivar
  if(AnimSolOut==true){
    AnimSolarOut();
  }else{
  }
  if(AnimIVTOutput==true){
    AnimInverterOuput();
  }else{
  }
  if(AnimIVTInput==true){
    AnimInverterInput();
  }else{
  }
  if(AnimChargIVT==true){
    AnimChrgInvrt();
  }else{
  }
    if(AnimChargBat==true){
    AnimChrgBat();
  }else{
  }
  if(AnimBatOut==true){
    AnimBatOuput();
  }else{
  }
  if(AnimBatIn==true){
    AnimBatInput();
  }else{
  }
}
void Touched(){        /// Touched YES
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
}
void TouchAsk(){       /// Touched ?
  if (ts.touched()) {
    TS_Point p = ts.getPoint();
    TchX=p.x;
    TchY=p.y; 
    Touched();
  }else{}
}
void SetColumnLine(){  /// Convertir de C1L1 a Columna 1 Fila 1
  if(MenuCL=="C1L1"){
    tft.setCursor(MenuColumn1X,MenuColumn1Y);  // Mostrar valor en pantalla Watt
  }
  else if(MenuCL=="C1L2"){
    tft.setCursor(MenuColumn1X,(MenuColumn1Y+MenuColumn1NextLine));  // Mostrar valor en pantalla Watt
  }
  else if(MenuCL=="C1L3"){
    tft.setCursor(MenuColumn1X,(MenuColumn1Y+(MenuColumn1NextLine*2)));  // Mostrar valor en pantalla Watt
  }
  else if(MenuCL=="C1L4"){
    tft.setCursor(MenuColumn1X,(MenuColumn1Y+(MenuColumn1NextLine*3)));  // Mostrar valor en pantalla Watt
  }
  else if(MenuCL=="C1L5"){
    tft.setCursor(MenuColumn1X,(MenuColumn1Y+(MenuColumn1NextLine*4)));  // Mostrar valor en pantalla Watt
  }
  else if(MenuCL=="C1L6"){
    tft.setCursor(MenuColumn1X,(MenuColumn1Y+(MenuColumn1NextLine*5)));  // Mostrar valor en pantalla Watt
  }
  else if(MenuCL=="C1L7"){
    tft.setCursor(MenuColumn1X,(MenuColumn1Y+(MenuColumn1NextLine*6)));  // Mostrar valor en pantalla Watt
  }
  else if(MenuCL=="C1L8"){
    tft.setCursor(MenuColumn1X,(MenuColumn1Y+(MenuColumn1NextLine*7)));  // Mostrar valor en pantalla Watt
  }
  else if(MenuCL=="C2L1"){
    tft.setCursor(MenuColumn2X,MenuColumn2Y);  // Mostrar valor en pantalla Watt
  }
  else if(MenuCL=="C2L2"){
    tft.setCursor(MenuColumn2X,(MenuColumn2Y+MenuColumn2NextLine));  // Mostrar valor en pantalla Watt
  }
  else if(MenuCL=="C2L3"){
    tft.setCursor(MenuColumn2X,(MenuColumn2Y+(MenuColumn2NextLine*2)));  // Mostrar valor en pantalla Watt
  }
  else if(MenuCL=="C2L4"){
    tft.setCursor(MenuColumn2X,(MenuColumn2Y+(MenuColumn2NextLine*3)));  // Mostrar valor en pantalla Watt
  }
  else if(MenuCL=="C2L5"){
    tft.setCursor(MenuColumn2X,(MenuColumn2Y+(MenuColumn2NextLine*4)));  // Mostrar valor en pantalla Watt
  }
  else if(MenuCL=="C2L6"){
    tft.setCursor(MenuColumn2X,(MenuColumn2Y+(MenuColumn2NextLine*5)));  // Mostrar valor en pantalla Watt
  }
  else if(MenuCL=="C2L7"){
    tft.setCursor(MenuColumn2X,(MenuColumn2Y+(MenuColumn2NextLine*6)));  // Mostrar valor en pantalla Watt
  }
  else if(MenuCL=="C1L8"){
    tft.setCursor(MenuColumn2X,(MenuColumn2Y+(MenuColumn2NextLine*7)));  // Mostrar valor en pantalla Watt
  }
}
void Home(){           ///
  ReadSerial1();
  if(label == "V"){         // Battery Volt  mV     NO ANIM
    if(Origin=="MPPT"){
        ResetMPTData=ResetLoopDefault;  // dato recibido correctamente no resetar los datos
        TextSize1();
        tft.setTextColor(ILI9341_ORANGE);
        tft.setCursor(MPTVbatX, MPTVbatY);  // Mostrar valor en pantalla Watt
        tft.print(MPTVbat,1);
        tft.print("V");        MPTVbatStrg=val;
        MPTVbat=MPTVbatStrg.toInt();
        MPTVbat=(MPTVbat*0.001);
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(MPTVbatX, MPTVbatY);  // Mostrar valor en pantalla Watt
        tft.print(MPTVbat,1);
        tft.print("V");
      }else{/*
        ResetMPTData=(ResetMPTData-1);
        if(ResetMPTData<1){
           DataResetMPT();
        }else{  
      }*/ 
    }
    if(Origin=="Inverter"){
        ResetIVTData=ResetLoopDefault;  // dato recibido correctamente no resetar los datos
        TextSize1();
        tft.setTextColor(ILI9341_ORANGE);
        tft.setCursor(IVTVbatX, IVTVbatY);  // Mostrar valor en pantalla Watt
        tft.print(IVTVbat,1);
        tft.print("V");
        IVTVbatStrg=val;
        IVTVbat=IVTVbatStrg.toInt();
        IVTVbat=(IVTVbat*0.001);
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(IVTVbatX, IVTVbatY);  // Mostrar valor en pantalla Watt
        tft.print(IVTVbat,1);
        tft.print("V");    
        }else{
        ResetIVTData=(ResetIVTData-1);
        if(ResetIVTData<1){
           DataResetMPT();
        }else{
      }   
    }  
  }
  else if(label == "VPV"){  //Panel Volt mV
    if(Origin=="MPPT"){
        TextSize1();
        tft.setTextColor(ILI9341_ORANGE);
        tft.setCursor(MPTVpvX, MPTVpvY);  // Mostrar valor en pantalla Watt
        tft.print(MPTVpv);
        tft.print("V");
        MPTVpvStrg=val;
        MPTVpv=MPTVpvStrg.toInt();
        if(MPTVpv<0){
         MPTVpv=(MPTVpv*-1); // no se porque a veces el valor sale negativo supongo que al pasar de sting a int lo hace no siempre bien
        }else{}
        MPTVpv=(MPTVpv*0.001);
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(MPTVpvX, MPTVpvY);  // Mostrar valor en pantalla Watt
        tft.print(MPTVpv);
        tft.print("V");
      }
  }
  else if(label == "PPV"){  // Panel Power W
    if(Origin=="MPPT"){
        TextSize2();
        tft.setTextColor(ILI9341_ORANGE);
        tft.setCursor(MPTPpvX, MPTPpvY);  // Mostrar valor en pantalla Watt
        tft.print(MPTPpv);
        tft.print("W");
        MPTPpvStrg=val;
        MPTPpv=MPTPpvStrg.toInt();
        if(MPTPpv>1){
          AnimSolOut=true;
        }else{
          AnimSolOut=false;
        }
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(MPTPpvX, MPTPpvY);  // Mostrar valor en pantalla Watt
        tft.print(MPTPpv);
        tft.print("W");
      }
  }
  else if(label == "I"){    // mA Battery mA
    if(Origin=="MPPT"){  
        TextSize1();
        tft.setTextColor(ILI9341_ORANGE);
        tft.setCursor(MPTAmpX, MPTAmpY);  // Mostrar valor en pantalla Watt
        tft.print(MPTAmp);
        tft.print("A");
        MPTAmpStrg=val;
        MPTAmp=MPTAmpStrg.toInt();
        MPTAmp=(MPTAmp*0.001);
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(MPTAmpX, MPTAmpY);  // Mostrar valor en pantalla Watt
        tft.print(MPTAmp);
        tft.print("A");
      }
  }
  else if(label == "H20"){  // 0.01kWh HOY
    if(Origin=="MPPT"){  
        TextSize1();
        tft.setTextColor(ILI9341_ORANGE);
        tft.setCursor(MPTkWhX, MPTkWhY);  // Mostrar valor en pantalla Watt
        tft.print(MPTkWh,2);
        MPTkWhStrg=val;
        MPTkWh=MPTkWhStrg.toInt();
        MPTkWh=(MPTkWh); // 0.01kWh   
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(MPTkWhX, MPTkWhY);  // Mostrar valor en pantalla Watt
        tft.print(MPTkWh,2);
      }
  }
  else if(label == "H21"){ // Wmax HOY
    if(Origin=="MPPT"){  
        TextSize1();
        tft.setTextColor(ILI9341_ORANGE);
        tft.setCursor(MPTWmaxX, MPTWmaxY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmax);
        tft.print("Wmax");
        MPTWmaxStrg=val;
        MPTWmax=MPTWmaxStrg.toInt();
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(MPTWmaxX,MPTWmaxY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmax);
        tft.print("Wmax");
      }
  }
  else if(label == "CS"){ // Estado operativo
    if(Origin=="MPPT"){
      MPTCSStrg=val;
      MPTCS=MPTCSStrg.toInt();
      if(MPTCS==0){ // OFF
       tft.fillCircle(LedMPPTX, LedMPPTY, LedRad,ILI9341_BLACK);
        }
        else if(MPTCS==2){ // FAULT MPPT & Inverter off hasta hacer Reset
        tft.fillCircle(LedMPPTX, LedMPPTY, LedRad,ILI9341_RED);
        } 
        else if(MPTCS==3){ // bulk
        tft.fillCircle(LedMPPTX, LedMPPTY, LedRad,ILI9341_BLUE);
        }
        else if(MPTCS==4){ // Absorption
        tft.fillCircle(LedMPPTX, LedMPPTY, LedRad,ILI9341_YELLOW);
        }
        else if(MPTCS==5){ // Float
        tft.fillCircle(LedMPPTX, LedMPPTY, LedRad,ILI9341_GREEN);
        }
    }
  }
  else if(label=="MODE"){  // INverter Modo ON OFF ECO
    if(Origin=="Inverter"){
      IVTModeStrg=val;
      IVTMode=IVTModeStrg.toInt();
      if(IVTMode==2){ // inverter ON
      tft.fillCircle(LedIVTX, LedIVTY, LedRad,ILI9341_GREEN);
      }
      else if(IVTMode==4){ // inverter OFF
      tft.fillCircle(LedIVTX, LedIVTY, LedRad,ILI9341_WHITE);
      }
      else if(IVTMode==5){ // inverter ECO
      tft.fillCircle(LedIVTX, LedIVTY, LedRad,ILI9341_BLUE);
      }
    }
  }
  else if(label=="WARN"){    // INVERTER Alarm Razon
    if(Origin=="Inverter"){
    tft.fillCircle(LedIVTX, LedIVTY, LedRad,ILI9341_GREEN);
      if(IVTWarnStrg=="1"){ // volta bajo
        TextSize1();
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(IVTWarnX, IVTWarnY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmaxStrg);
        MPTWmaxStrg=Warm1;
        tft.setTextColor(ILI9341_RED);
        tft.setCursor(IVTWarnX,IVTWarnY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmaxStrg);
      }
      else if(IVTWarnStrg=="2"){ //Volt alto
        TextSize1();
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(IVTWarnX, IVTWarnY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmaxStrg);
  
        MPTWmaxStrg=Warm2;
        tft.setTextColor(ILI9341_RED);
        tft.setCursor(IVTWarnX,IVTWarnY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmaxStrg);
      }
      else if(IVTWarnStrg=="32"){ // Temperatura baja
        TextSize1();
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(IVTWarnX, IVTWarnY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmaxStrg);
        MPTWmaxStrg=Warm32;
        tft.setTextColor(ILI9341_RED);
        tft.setCursor(IVTWarnX,IVTWarnY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmaxStrg);
      }
      else if(IVTWarnStrg=="64"){ // Temperatura alta
        TextSize1();
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(IVTWarnX, IVTWarnY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmaxStrg);
        MPTWmaxStrg=Warm64;
        tft.setTextColor(ILI9341_RED);
        tft.setCursor(IVTWarnX,IVTWarnY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmaxStrg);
      }
      else if(IVTWarnStrg=="256"){ // Sobrecarca se load demasiado alto
        TextSize1();
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(IVTWarnX, IVTWarnY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmaxStrg);
        MPTWmaxStrg=Warm256;
        tft.setTextColor(ILI9341_RED);
        tft.setCursor(IVTWarnX,IVTWarnY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmaxStrg);
      }
      else if(IVTWarnStrg=="512"){ // rizado de volt continuos de la batteria ruido
        TextSize1();
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(IVTWarnX, IVTWarnY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmaxStrg);
        MPTWmaxStrg=Warm512;
        tft.setTextColor(ILI9341_RED);
        tft.setCursor(IVTWarnX,IVTWarnY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmaxStrg);
      }
      else if(IVTWarnStrg=="1024"){ // AC Volt demasiado bajo
        TextSize1();
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(IVTWarnX, IVTWarnY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmaxStrg);
        MPTWmaxStrg=Warm1024;
        tft.setTextColor(ILI9341_RED);
        tft.setCursor(IVTWarnX,IVTWarnY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmaxStrg);
      }
      else if(IVTWarnStrg=="2048"){ // AC volt demasiado alto
        TextSize1();
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(IVTWarnX, IVTWarnY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmaxStrg);
        MPTWmaxStrg=Warm2048;
        tft.setTextColor(ILI9341_RED);
        tft.setCursor(IVTWarnX,IVTWarnY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmaxStrg);
      }
    }
  }
  else if(label=="AC_OUT_V"){  // AC volt 0.01V
    if(Origin=="Inverter"){
        TextSize1();
        tft.setTextColor(TextColorBlue);
        tft.setCursor(IVTACVX, IVTACVY);  // Mostrar valor en pantalla Watt
        tft.print(IVTACV);
        tft.print("V");
        IVTACVStrg=val;
        IVTACV=IVTACVStrg.toInt();
        IVTACV=(IVTACV*0.01);
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(IVTACVX, IVTACVY);  // Mostrar valor en pantalla Watt
        tft.print(IVTACV);
        tft.print("V");
    }
  }
  else  if(label=="AC_OUT_I"){ // AC amp 0.1A
    if(Origin=="Inverter"){
        TextSize1();
        tft.setTextColor(TextColorBlue);
        tft.setCursor(IVTACIX, IVTACIY);  // Mostrar valor en pantalla Watt
        tft.print(IVTACI);
        tft.print("A");
        IVTACIStrg=val;
        IVTACI=IVTACVStrg.toInt();
        IVTACI=(IVTACV*0.1);
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(IVTACIX, IVTACIY);  // Mostrar valor en pantalla Watt
        tft.print(IVTACI);
        tft.print("A");
    }
  }
  else  if(label=="AC_OUT_S"){ // AC amp 0.1A
    if(Origin=="Inverter"){
        TextSize2();
        tft.setTextColor(ILI9341_GREEN);
        tft.setCursor(IVTACWX, IVTACWY);  // Mostrar valor en pantalla Watt
        tft.print(IVTACS);
        tft.print("W");
        IVTACSStrg=val;
        IVTACS=IVTACSStrg.toInt();
        IVTACS=(IVTACS*0.1);
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(IVTACWX, IVTACWY);  // Mostrar valor en pantalla Watt
        tft.print(IVTACS);
        tft.print("W");
      }
       if(IVTACS>2){
        AnimIVTInput=true;
      }else{
        AnimIVTInput=false;
    }
  }
  else{
      ResetMPTData=(ResetMPTData-1);
      ResetIVTData=(ResetIVTData-1);
    if(ResetMPTData<1){
      DataResetMPT();  
      }else{     
    }
    if(ResetIVTData<1){
      DataResetIVT();  
      }else{
    }
  }
  ReadSerial2();
    if(label == "V"){         // Battery Volt  mV     NO ANIM
    if(Origin=="MPPT"){
        ResetMPTData=ResetLoopDefault;  // dato recibido correctamente no resetar los datos
        TextSize1();
        tft.setTextColor(ILI9341_ORANGE);
        tft.setCursor(MPTVbatX, MPTVbatY);  // Mostrar valor en pantalla Watt
        tft.print(MPTVbat,1);
        tft.print("V");
        MPTVbatStrg=val;
        MPTVbat=MPTVbatStrg.toInt();
        MPTVbat=(MPTVbat*0.001);
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(MPTVbatX, MPTVbatY);  // Mostrar valor en pantalla Watt
        tft.print(MPTVbat,1);
        tft.print("V");
      }else{
        ResetMPTData=(ResetMPTData-1);
        if(ResetMPTData<1){
           DataResetMPT();
        }else{  
      } 
    }
    if(Origin=="Inverter"){
        ResetIVTData=ResetLoopDefault;  // dato recibido correctamente no resetar los datos
        TextSize1();
        tft.setTextColor(ILI9341_ORANGE);
        tft.setCursor(IVTVbatX, IVTVbatY);  // Mostrar valor en pantalla Watt
        tft.print(IVTVbat,1);
        tft.print("V");
        IVTVbatStrg=val;
        IVTVbat=IVTVbatStrg.toInt();
        IVTVbat=(IVTVbat*0.001);
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(IVTVbatX, IVTVbatY);  // Mostrar valor en pantalla Watt
        tft.print(IVTVbat,1);
        tft.print("V");
        ResetIVTData=ResetLoopDefault;      
        }else{
        ResetIVTData=(ResetIVTData-1);
        if(ResetIVTData<1){
           DataResetMPT();
        }else{
      }   
    }  
  }
  else if(label == "VPV"){  //Panel Volt mV
    if(Origin=="MPPT"){
        TextSize1();
        tft.setTextColor(ILI9341_ORANGE);
        tft.setCursor(MPTVpvX, MPTVpvY);  // Mostrar valor en pantalla Watt
        tft.print(MPTVpv);
        tft.print("V");
        MPTVpvStrg=val;
        MPTVpv=MPTVpvStrg.toInt();
        if(MPTVpv<0){
         MPTVpv=(MPTVpv*-1); // no se porque a veces el valor sale negativo supongo que al pasar de sting a int lo hace no siempre bien
        }else{}
        MPTVpv=(MPTVpv*0.001);
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(MPTVpvX, MPTVpvY);  // Mostrar valor en pantalla Watt
        tft.print(MPTVpv);
        tft.print("V");
      }
  }
  else if(label == "PPV"){  // Panel Power W
    if(Origin=="MPPT"){
        TextSize2();
        tft.setTextColor(ILI9341_ORANGE);
        tft.setCursor(MPTPpvX, MPTPpvY);  // Mostrar valor en pantalla Watt
        tft.print(MPTPpv);
        tft.print("W");
        MPTPpvStrg=val;
        MPTPpv=MPTPpvStrg.toInt();
        if(MPTPpv>1){
          AnimSolOut=true;
        }else{
          AnimSolOut=false;
        }
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(MPTPpvX, MPTPpvY);  // Mostrar valor en pantalla Watt
        tft.print(MPTPpv);
        tft.print("W");
      }
  }
  else if(label == "I"){    // mA Battery mA
    if(Origin=="MPPT"){  
        TextSize1();
        tft.setTextColor(ILI9341_ORANGE);
        tft.setCursor(MPTAmpX, MPTAmpY);  // Mostrar valor en pantalla Watt
        tft.print(MPTAmp);
        tft.print("A");
        MPTAmpStrg=val;
        MPTAmp=MPTAmpStrg.toInt();
        MPTAmp=(MPTAmp*0.001);
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(MPTAmpX, MPTAmpY);  // Mostrar valor en pantalla Watt
        tft.print(MPTAmp);
        tft.print("A");
      }
  }
  else if(label == "H20"){  // 0.01kWh HOY
    if(Origin=="MPPT"){  
        TextSize1();
        tft.setTextColor(ILI9341_ORANGE);
        tft.setCursor(MPTkWhX, MPTkWhY);  // Mostrar valor en pantalla Watt
        tft.print(MPTkWh,2);
        MPTkWhStrg=val;
        MPTkWh=MPTkWhStrg.toInt();
        MPTkWh=(MPTkWh); // 0.01kWh   
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(MPTkWhX, MPTkWhY);  // Mostrar valor en pantalla Watt
        tft.print(MPTkWh,2);
      }
  }
  else if(label == "H21"){ // Wmax HOY
    if(Origin=="MPPT"){  
        TextSize1();
        tft.setTextColor(ILI9341_ORANGE);
        tft.setCursor(MPTWmaxX, MPTWmaxY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmax);
        tft.print("Wmax");
        MPTWmaxStrg=val;
        MPTWmax=MPTWmaxStrg.toInt();
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(MPTWmaxX,MPTWmaxY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmax);
        tft.print("Wmax");
      }
  }
  else if(label == "CS"){ // Estado operativo
    if(Origin=="MPPT"){
      MPTCSStrg=val;
      MPTCS=MPTCSStrg.toInt();
      if(MPTCS==0){ // OFF
       tft.fillCircle(LedMPPTX, LedMPPTY, LedRad,ILI9341_BLACK);
        }
        else if(MPTCS==2){ // FAULT MPPT & Inverter off hasta hacer Reset
        tft.fillCircle(LedMPPTX, LedMPPTY, LedRad,ILI9341_RED);
        } 
        else if(MPTCS==3){ // bulk
        tft.fillCircle(LedMPPTX, LedMPPTY, LedRad,ILI9341_BLUE);
        }
        else if(MPTCS==4){ // Absorption
        tft.fillCircle(LedMPPTX, LedMPPTY, LedRad,ILI9341_YELLOW);
        }
        else if(MPTCS==5){ // Float
        tft.fillCircle(LedMPPTX, LedMPPTY, LedRad,ILI9341_GREEN);
        }
    }
  }
  else if(label=="MODE"){  // INverter Modo ON OFF ECO
    if(Origin=="Inverter"){
      IVTModeStrg=val;
      IVTMode=IVTModeStrg.toInt();
      if(IVTMode==2){ // inverter ON
      tft.fillCircle(LedIVTX, LedIVTY, LedRad,ILI9341_GREEN);
      }
      else if(IVTMode==4){ // inverter OFF
      tft.fillCircle(LedIVTX, LedIVTY, LedRad,ILI9341_WHITE);
      }
      else if(IVTMode==5){ // inverter ECO
      tft.fillCircle(LedIVTX, LedIVTY, LedRad,ILI9341_BLUE);
      }
    }
  }
  else if(label=="WARN"){    // INVERTER Alarm Razon
    if(Origin=="Inverter"){
    tft.fillCircle(LedIVTX, LedIVTY, LedRad,ILI9341_GREEN);
      if(IVTWarnStrg=="1"){ // volta bajo
        TextSize1();
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(IVTWarnX, IVTWarnY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmaxStrg);
        MPTWmaxStrg=Warm1;
        tft.setTextColor(ILI9341_RED);
        tft.setCursor(IVTWarnX,IVTWarnY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmaxStrg);
      }
      else if(IVTWarnStrg=="2"){ //Volt alto
        TextSize1();
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(IVTWarnX, IVTWarnY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmaxStrg);
  
        MPTWmaxStrg=Warm2;
        tft.setTextColor(ILI9341_RED);
        tft.setCursor(IVTWarnX,IVTWarnY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmaxStrg);
      }
      else if(IVTWarnStrg=="32"){ // Temperatura baja
        TextSize1();
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(IVTWarnX, IVTWarnY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmaxStrg);
        MPTWmaxStrg=Warm32;
        tft.setTextColor(ILI9341_RED);
        tft.setCursor(IVTWarnX,IVTWarnY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmaxStrg);
      }
      else if(IVTWarnStrg=="64"){ // Temperatura alta
        TextSize1();
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(IVTWarnX, IVTWarnY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmaxStrg);
        MPTWmaxStrg=Warm64;
        tft.setTextColor(ILI9341_RED);
        tft.setCursor(IVTWarnX,IVTWarnY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmaxStrg);
      }
      else if(IVTWarnStrg=="256"){ // Sobrecarca se load demasiado alto
        TextSize1();
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(IVTWarnX, IVTWarnY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmaxStrg);
        MPTWmaxStrg=Warm256;
        tft.setTextColor(ILI9341_RED);
        tft.setCursor(IVTWarnX,IVTWarnY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmaxStrg);
      }
      else if(IVTWarnStrg=="512"){ // rizado de volt continuos de la batteria ruido
        TextSize1();
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(IVTWarnX, IVTWarnY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmaxStrg);
        MPTWmaxStrg=Warm512;
        tft.setTextColor(ILI9341_RED);
        tft.setCursor(IVTWarnX,IVTWarnY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmaxStrg);
      }
      else if(IVTWarnStrg=="1024"){ // AC Volt demasiado bajo
        TextSize1();
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(IVTWarnX, IVTWarnY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmaxStrg);
        MPTWmaxStrg=Warm1024;
        tft.setTextColor(ILI9341_RED);
        tft.setCursor(IVTWarnX,IVTWarnY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmaxStrg);
      }
      else if(IVTWarnStrg=="2048"){ // AC volt demasiado alto
        TextSize1();
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(IVTWarnX, IVTWarnY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmaxStrg);
        MPTWmaxStrg=Warm2048;
        tft.setTextColor(ILI9341_RED);
        tft.setCursor(IVTWarnX,IVTWarnY);  // Mostrar valor en pantalla Watt
        tft.print(MPTWmaxStrg);
      }
    }
  }
  else if(label=="AC_OUT_V"){  // AC volt 0.01V
    if(Origin=="Inverter"){
        TextSize1();
        tft.setTextColor(TextColorBlue);
        tft.setCursor(IVTACVX, IVTACVY);  // Mostrar valor en pantalla Watt
        tft.print(IVTACV);
        tft.print("V");
        IVTACVStrg=val;
        IVTACV=IVTACVStrg.toInt();
        IVTACV=(IVTACV*0.01);
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(IVTACVX, IVTACVY);  // Mostrar valor en pantalla Watt
        tft.print(IVTACV);
        tft.print("V");
    }
  }
  else  if(label=="AC_OUT_I"){ // AC amp 0.1A
    if(Origin=="Inverter"){
        TextSize1();
        tft.setTextColor(TextColorBlue);
        tft.setCursor(IVTACIX, IVTACIY);  // Mostrar valor en pantalla Watt
        tft.print(IVTACI);
        tft.print("A");
        IVTACIStrg=val;
        IVTACI=IVTACVStrg.toInt();
        IVTACI=(IVTACV*0.1);
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(IVTACIX, IVTACIY);  // Mostrar valor en pantalla Watt
        tft.print(IVTACI);
        tft.print("A");
    }
  }
  else  if(label=="AC_OUT_S"){ // AC amp 0.1A
    if(Origin=="Inverter"){
        TextSize2();
        tft.setTextColor(ILI9341_GREEN);
        tft.setCursor(IVTACWX, IVTACWY);  // Mostrar valor en pantalla Watt
        tft.print(IVTACS);
        tft.print("W");
        IVTACSStrg=val;
        IVTACS=IVTACSStrg.toInt();
        IVTACS=(IVTACV*0.1);
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(IVTACWX, IVTACWY);  // Mostrar valor en pantalla Watt
        tft.print(IVTACS);
        tft.print("W");
      }
       if(IVTACS>2){
        AnimIVTInput=true;
      }else{
        AnimIVTInput=false;
    }
  }
  else{
      ResetMPTData=(ResetMPTData-1);
      ResetIVTData=(ResetIVTData-1);
    if(ResetMPTData<1){
      DataResetMPT();  
      }else{     
    }
    if(ResetIVTData<1){
      DataResetIVT();  
      }else{
    }
  }
  BatPorcentCalc(); // Calculo del % de bateri
  InvrtAmpImput();  // Calcul deAmp input estimado del inverter
  SwitchAnim();     // Activar animaciones si es necesario
}
void Solar(){          ///
  ReadSerial2();
  TextSize1();
  tft.setTextColor(MenuTxtColorRst);
  if(Origin=="MPPT"){
    if(label == "V"){             // Columna1 Linea1 Battery Volt  mV     NO ANIM
      MenuCL=MPTVbatCL; SetColumnLine();
      tft.print(MPTVbat,1);
      tft.print("V");
      MPTVbatStrg=val;
      MPTVbat=MPTVbatStrg.toInt(); 
      MPTVbat=(MPTVbat*milliTo);
      tft.setTextColor(MenuTxtColor);
      MenuCL=MPTVbatCL; SetColumnLine();
      tft.print(MPTVbat,1);
      tft.print("V");
    }
    else if(label == "VPV"){      // Columna1 Linea2 Panel Volt mV
      MenuCL=MPTVpvCL; SetColumnLine();
        tft.print(MPTVpv);
        tft.print("V");
        MPTVpvStrg=val;
        MPTVpv=MPTVpvStrg.toInt();
          if(MPTVpv<0){
            MPTVpv=(MPTVpv*-1); // no se porque a veces el valor sale negativo supongo que al pasar de sting a int lo hace no siempre bien
          }else{}
        MPTVpv=(MPTVpv*milliTo);
        tft.setTextColor(MenuTxtColor);
      MenuCL=MPTVpvCL; SetColumnLine();
        tft.print(MPTVpv);
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
      tft.print(IVTVbat,1);
      tft.print("V");
      IVTVbatStrg=val;
      IVTVbat=IVTVbatStrg.toInt(); 
      IVTVbat=(IVTVbat*milliTo);
      tft.setTextColor(MenuTxtColor);
      MenuCL=IVTVbatCL; SetColumnLine();
      tft.print(IVTVbat,1);
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
      tft.print(IVTACV);
      tft.print("Vac");
      IVTACVStrg=val;
      IVTACV=IVTACVStrg.toInt();
      IVTACV=(IVTACV*centesiTo);         //0.01 centesimo  CentesiTo
      tft.setTextColor(MenuTxtColor);
      MenuCL=IVTACVCL; SetColumnLine();
      tft.print(IVTACV);
      tft.print("Vac");
    }
    else if(label == "AC_OUT_I"){      // Columna1 Linea2 Panel Volt mV
      MenuCL=IVTACICL; SetColumnLine();
      tft.print(IVTACI);
      tft.print("Iac");
      IVTACIStrg=val;
      IVTACI=IVTACIStrg.toInt();
      IVTACI=(IVTACI*decimTo);         //0.01 centesimo  CentesiTo
      tft.setTextColor(MenuTxtColor);
      MenuCL=IVTACICL; SetColumnLine();
      tft.print(IVTACI);
      tft.print("Iac");
    }
    else if(label == "AC_OUT_S"){      // Columna1 Linea2 Panel Volt mV
      MenuCL=IVTACSCL; SetColumnLine();
      tft.print(IVTACS);
      tft.print("VA");
      IVTACSStrg=val;
      IVTACS=IVTACSStrg.toInt();
      tft.setTextColor(MenuTxtColor);
      MenuCL=IVTACSCL; SetColumnLine();
      tft.print(IVTACS);
      tft.print("VA");
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
        tft.print(IVTVbat,1);
        tft.print("V");
        IVTVbatStrg=val;
        IVTVbat=IVTVbatStrg.toInt(); 
        IVTVbat=(IVTVbat*milliTo);
        tft.setTextColor(MenuTxtColor);
        MenuCL=BATivtVbatCL; SetColumnLine();
        tft.print(batIVTVbattag);
        tft.print(IVTVbat,1);
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
      tft.print(MPTVbat,1);
      tft.print("V");
      MPTVbatStrg=val;
      MPTVbat=MPTVbatStrg.toInt(); 
      MPTVbat=(MPTVbat*milliTo);
      tft.setTextColor(MenuTxtColor);
      MenuCL=BATmptVbatCL; SetColumnLine();
      tft.print(batMPTVbattag);
      tft.print(MPTVbat,1);
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
void Charger(){
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
      tft.print(IVTACV);
      tft.print("Vac");
      IVTACVStrg=val;
      IVTACV=IVTACVStrg.toInt();
      IVTACV=(IVTACV*centesiTo);         //0.01 centesimo  CentesiTo
      tft.setTextColor(MenuTxtColor);
      MenuCL=IVTACVCL; SetColumnLine();
      tft.print(IVTACV);
      tft.print("Vac");
    }
    else if(label == "AC_OUT_I"){      // Columna1 Linea2 Panel Volt mV
      MenuCL=IVTACICL; SetColumnLine();
      tft.print(IVTACI);
      tft.print("Iac");
      IVTACIStrg=val;
      IVTACI=IVTACIStrg.toInt();
      IVTACI=(IVTACI*decimTo);         //0.01 centesimo  CentesiTo
      tft.setTextColor(MenuTxtColor);
      MenuCL=IVTACICL; SetColumnLine();
      tft.print(IVTACI);
      tft.print("Iac");
    }
    else if(label == "AC_OUT_S"){      // Columna1 Linea2 Panel Volt mV
      MenuCL=IVTACSCL; SetColumnLine();
      tft.print(IVTACS);
      tft.print("VA");
      IVTACSStrg=val;
      IVTACS=IVTACSStrg.toInt();
      tft.setTextColor(MenuTxtColor);
      MenuCL=IVTACSCL; SetColumnLine();
      tft.print(IVTACS);
      tft.print("VA");
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
  if(MenuSelected==MenuPresent){
  }else{
    if(MenuSelected=="Home"){
        tft.drawRGBBitmap(0, 2, HomeBitmap, HOME_WIDTH,HOME_HEIGHT);
        MenuPresent="Home";
    }else if(MenuSelected=="Solar"){
        tft.drawRGBBitmap(0, 2, SolarBackGround, SolarMenu_WIDTH,SolarMenu_HEIGHT);
        MenuPresent="Solar";
    }else if(MenuSelected=="Bat"){
        tft.drawRGBBitmap(0, 2, BatBackGround, BatMenu_WIDTH,BatMenu_HEIGHT);
        MenuPresent="Bat";
    }else if(MenuSelected=="Inverter"){
        tft.drawRGBBitmap(0, 2, InvrtBackGround, InvrtMenu_WIDTH,InvrtMenu_HEIGHT);
        MenuPresent="Inverter";
    }else if(MenuSelected=="Charger"){
        tft.drawRGBBitmap(0, 2, ChrgBackGround, ChrgMenu_WIDTH,ChrgMenu_HEIGHT);
        MenuPresent="Charger";
    }else if(MenuSelected=="Load"){
      tft.drawRGBBitmap(0, 2, LoadBackGround, LoadMenu_WIDTH,LoadMenu_HEIGHT);
       MenuPresent="Load";
    }
  }
}

void setup(){
  SerialIni();
  TFTSetIni();
  HomeBackGround();
  TextSetup();
}
void loop(){
  TouchAsk();
  MenuCheck();
delay(msLoop);
}