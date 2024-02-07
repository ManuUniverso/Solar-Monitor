/* Solar Monitor v1.1.0 ManuUniverso 2024

First stable version. Still some bug in the show display.
An ESP32 is used, data is read from the VE.Direct (Victron Energy brand) ports of the Inverter and MPPT. Then display the results on a color screen ili9341.
The touch screen available will allow you to enter the submenu of each device.
LDR dimming mode add
The comments are now mostly in English and some have yet to change.

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
0.1.0 01/01/2024 First tested OK
0.1.1 14/01/2024 Added touch function with submenu
0.1.2 23/01/2024 small bug & Current Sensor HCS LSP 20A for Charger (without ve direct)
1.0.0 30/01/2024 great changes and improvements, first stable version, add dimming mode LDR
1.1.0 06/02/2024 Add Relay Functions & bug fix bar battery level & text flicker fix & Current Sensor improvement

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
   - jpg diagrams & pin connection
     .ESP32 Pin image from https://www.LastMinuteEngineers.com
     .ili9341 image from https://bidouilleur.ca/ && https://www.youtube.com/watch?v=hcMU5H6vzxI
     .Current Sensor
     .LDR Dimming mode
     .Relay (Meross Hack) I contributed to this forum https://forum.bidouilleur.ca/index.php?topic=1222.msg8327#msg8327 

   The board Available direcetly from the Arduino IDE v2 
   -arduino-esp32    "Select USB correct"  & "node32s"
   The libraries Available directly from the Arduino IDE v2 library manager.
   -XPT2046_Touchscreen.h
   -Adafruit_ILI9341.h
   -Adafruit_GFX.h
   -Fonts/FreeSansBold9pt7b.h No need to download, it is included in the previous ones.
   -Fonts/FreeSansBold18pt7b.h No need to download, it is included in the previous ones.
   Pin & Config:
   -Current Sensor Model ZY264-20A HCS LSP 20A voltage supply will affect the reading value 
   -Current Sensor It is powered with 5V. Then a voltage divider with two resistors 2k7 and 4k7 make it safe to use for the esp32.This serves to adapt the logical level
   -Current Sensor you should not use the same isolators that are used in VE Direct.
   -Current Sensor 5V]---[2k7]---pin input ESP23---[4k7]---[GND
   -Current Sensor The value of the resistors varies depending on their quality, you must calibrate the RAW values in the code (search "calibra")
   -Current Sensor Pin 5V                        between this and output you should have 2k7
   -Current Sensor Pin GND                       between this and 5v you should have 2k7+4k7 = 7k4 aproxi
   -Current Sensor Pin Output ---> ESP32 GPIO33  between this and gnd you must have 4k7
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
   -Dimming mode  ---3.3v]---[LDR]---[LED Pin  (You only need to connect an LDR in series to the LED pin of the display as in the diagram)
   -Relay Pin 32  --- Relay  (Here you can use a PC817 type optocoupler for example to activate a relay to use as in my case modified smart plug)
   -Relay used with smart plug and charger, logically it should not be used with the 230 from Inverter. I know you know but the plugs are all the same and you have to verify that
   -Relay I am using a smart plug connected to remotely turn on the charger, in this link you can see how to access activation from outside for our esp32. Remember the optocoupler is necessary as an isolator !
   *****************************************************************************

      !! --> //***  They must be reviewed and established by the user for correct operation !

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
#define ResetLoopDefault 200                               /// VE.Direct Pin Data Reset  ResetMPTData ResetIVTData ResetLoopDefault
#define CurrSensPin 33                                     /// Current Sensor Pin charger control GPIO33 ADC1_5
#define PinRelay 32                                        /// Relay Pin D32 GPIO32
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
#define CirclRad  3                                        /// Image Animation BlueBall size 
#define CirclColor 0x4C5A                                  /// Image Animation BBlueBall size  Blue Victron Energy
#define TextColorBlue 0x4C5A                               /// Image Text Color Blue Victron Energy
#define TextColorRed 0xc1c5                                /// Image Text Color Red Charger
#define TextColorOrange 0xf4c2                             /// Image Text Color Orange MPPT
#define TextColorGreen 0x254b                              /// Image Text Color Green Load
#define TextColorWhite ILI9341_WHITE                       /// Image Text Color White 
#define TextColorBlack ILI9341_BLACK                       /// Image Text Color Black      
#define BatBarLevelColor ILI9341_GREENYELLOW               /// Battery Bar Level Color
#define MPTCSColor0 ILI9341_BLACK                          /// MPPT Color 0 OFF
#define MPTCSColor2 ILI9341_RED                            /// MPPT Color2 FAULT
#define MPTCSColor3 ILI9341_BLUE                           /// MPPT Color 3 BULK
#define MPTCSColor4 ILI9341_YELLOW                         /// MPPT Color 4 ABSORPTION
#define MPTCSColor5 ILI9341_GREEN                          /// MPPT Color 5 FLOAT
#define LedRad 4                                           /// Image Led Estado tamaño Radio ( simula un led en el display )
#define LedMPPTX 235                                       /// Image Menu Home MPPT Led estado Cursor eje X
#define LedMPPTY 116                                       /// Image Menu Home MPPT Led estado Cursor eje Y
#define LedIVTX 160                                        /// Image Menu Home Inverter Led estado Cursor eje X
#define LedIVTY 37                                         /// Image Menu Home Inverter Led estado Cursor eje Y
#define ChrgBatStrtX 56                                    /// Image Menu Home Animacion Bola de Charger a Battery eje X
#define ChrgBatFinY 118                                    /// Image Menu Home Animacion Bola de Charger a Battery eje Y
#define ChrgInvrtStrtY 52                                  /// Image Menu Home Animacion Bola de Charger a Inverter eje Y
#define ChrgInvrtFinX 116                                  /// Image Menu Home Animacion Bola de Charger a Inverter eje X
#define LoadStrtY 52                                       /// Image Menu Home Animacion Bola de Inverter a Load eje Y
#define LoadFinX 211                                       /// Image Menu Home Animacion Bola de Inverter a Load eje X
#define BATStrtY 150.5                                     /// Image Menu Home Animacion Bola de Bateria a Inverter eje Y
#define SolarStrtY 150.5                                   /// Image Menu Home Animacion Bola de Solar a Bateria eje Y
#define SolarFinX 170                                      /// Image Menu Home Animacion Bola de Solar a Bateria eje X
#define InvrtStrtX  160                                    /// Image Menu Home Animacion Bola de Solar+Bateria a Inverter eje X    
#define InvrtFinY 104                                      /// Image Menu Home Animacion Bola de Solar+Bateria a Inverter eje Y   
#define ChrgBatStrtYDef 94                                 /// Image Menu Home Animacion Bola de Charger a Bateria eje Y          Range
#define ChrgInvrtStrtXDef 108                              /// Image Menu Home Animacion Bola de Charger a Inverter eje X         Range
#define LoadStrtXDef 203                                   /// Image Menu Home Animacion Bola de Inverter a Load eje X            Range
#define BATStrtXDef 110                                    /// Image Menu Home Animacion Bola de Bateria a Inverter eje X         Range
#define BATFinXDef 150                                     /// Image Menu Home Animacion Bola de Solar a Bateria eje X            Range
#define SolarStrtXDef 210                                  /// Image Menu Home Animacion Bola de Solar Bateria Inverter eje X     Range
#define InvrtStrtYDef 141                                  /// Image Menu Home Animacion Bola de Solar Bateria Inverter eje Y     Range
#define TchHomeXmin 1600                                   /// Touch Go Back return to Home Cursor X Min   if ((p.x > TchHomeXmin && p.x < TchHomeXmax) && (p.y > TchHomeYmin && p.y < TchHomeYmax)) {
#define TchHomeXmax 2600                                   /// Touch Go Back return to Home Cursor X Max
#define TchHomeYmin 340                                    /// Touch Go Back return to Home Cursor Y Min
#define TchHomeYmax 1380                                   /// Touch Go Back return to Home Cursor Y Max
#define TchSolarXmin 360                                   /// Touch Go Menu Solar Cursor X min            if ((p.x > TchSolarXmin && p.x < TchSolarXmax) && (p.y > TchSolarYmin && p.y < TchSolarYmax)) {
#define TchSolarXmax 1400                                  /// Touch Go Menu Solar Cursor X max
#define TchSolarYmin 400                                   /// Touch Go Menu Solar Cursor Y min
#define TchSolarYmax 2230                                  /// Touch Go Menu Solar Cursor Y max
#define TchBatXmin 2870                                    /// Touch Go Menu Bateria Cursor X min          if ((p.x > TchBatXmin && p.x < TchBatXmax) && (p.y > TchBatYmin && p.y < TchBatYmax)) {
#define TchBatXmax 3850                                    /// Touch Go Menu Bateria Cursor X max 
#define TchBatYmin 350                                     /// Touch Go Menu Bateria Cursor Y min 
#define TchBatYmax 1820                                    /// Touch Go Menu Bateria Cursor Y max 
#define TchChrgXmin 2800                                   /// Touch Go Menu Charger Cursor X min          if ((p.x > ChrgXmin && p.x < ChrgXmax) && (p.y > ChrgYmin && p.y < ChrgYmax)) {
#define TchChrgXmax 3850                                   /// Touch Go Menu Charger Cursor X max
#define TchChrgYmin 2600                                   /// Touch Go Menu Charger Cursor Y min
#define TchChrgYmax 3740                                   /// Touch Go Menu Charger Cursor Y max
#define TchInvrtXmin 1670                                  /// Touch Go Menu Inverter Cursor X min         if ((p.x > TchInvrtXmin && p.x < TchInvrtXmax) && (p.y > TchInvrtYmin && p.y < 1TchInvrtYmax)) {
#define TchInvrtXmax 2560                                  /// Touch Go Menu Inverter Cursor X max 
#define TchInvrtYmin 2450                                  /// Touch Go Menu Inverter Cursor Y min 
#define TchInvrtYmax 3740                                  /// Touch Go Menu Inverter Cursor Y max 
#define TchLoadXmin 360                                    /// Touch Go Menu Load Cursor X min             if ((p.x > TchLoadXmin && p.x < TchLoadXmax) && (p.y > TchLoadYmin && p.y < TchLoadYmax)) {
#define TchLoadXmax 1400                                   /// Touch Go Menu Load Cursor X max 
#define TchLoadYmin 2600                                   /// TouchGo Menu Load Cursor Y min 
#define TchLoadYmax 3740                                   /// Touch Go Menu Load Cursor Y max
#define TchRelayXmin 2800                                  /// Touch Go Menu Relay Cursor X min             if ((p.x > TchRelayXmin && p.x < TchRelayXmax) && (p.y > TchRelayYmin && p.y < TchRelayYmax)) {
#define TchRelayXmax 3950                                  /// Touch Go Menu Relay Cursor X max 
#define TchRelayYmin 1825                                  /// TouchGo Menu Relay Cursor Y min 
#define TchRelayYmax 2595                                  /// Touch Go Menu Relay Cursor Y max
#define MenuColumn1X 20                                    /// Imagen Text SubMenu  Primera Linea de Texto en la primera columna eje X         
#define MenuColumn1Y 125                                   /// Imagen Text SubMenu Primera Linea de Texto en la primera columna eje Y         
#define MenuColumn1NextLine 15                             /// Imagen Text SubMenu     Siguiente Linea de Texto en la primera columna eje Y 
#define MenuColumn2X 130                                   /// Imagen Text SubMenu Primera Linea de Texto en la Segunda columna eje X       
#define MenuColumn2Y 125                                   /// Imagen Text SubMenu Primera Linea de Texto en la Seguna columna eje Y       
#define MenuColumn2NextLine 15                             /// Imagen Text SubMenu     Siguiente Linea de Texto en la Seguna columna eje Y
#define ChrgAmpX 33                                        /// Charger Curent Sensor Amp Value show display eje x
#define ChrgAmpY 47                                        /// Charger Curent Sensor Amp Value show display eje y    
#define ChrgAmpXtag 50                                     /// Charger Curent Sensor Tag "A" Home Menu   
#define ChrgAmpYtag 19                                     /// Charger Curent Sensor Tag "A" Home Menu  
#define BATAmpX 40                                         /// Imagen Text Menu Home Amp Charger to Battery Axis  X
#define BATAmpY 150                                        /// Imagen Text Menu Home Amp Charger to Battery Axis  Y
#define MPTVbatX 225                                       /// Imagen Text Menu Home MPPT Vbat Cursor Eje  X
#define MPTVbatY 155                                       /// Imagen Text Menu Home MPPT Vbat Cursor Eje  Y
#define IVTVbatX 140                                       /// Imagen Text Menu Home Inverter Vbat Cursor Eje  X
#define IVTVbatY 81                                        /// Imagen Text Menu Home Inverter Vbat Cursor Eje  Y
#define MPTVpvX 217                                        /// Imagen Text Menu Home MPPT Vpv Cursor Eje  X
#define MPTVpvY 226                                        /// Imagen Text Menu Home MPPT Vpv Cursor Eje  Y
#define MPTAmpX 225                                        /// Imagen Text Menu Home MPPT Amp pv Cursor Eje  X
#define MPTAmpY 173                                        /// Imagen Text Menu Home MPPT Amp pv Cursor Eje  Y
#define MPTPpvX 227                                        /// Imagen Text Menu Home MPPT Watt pv Cursor Eje  X
#define MPTPpvY 205                                        /// Imagen Text Menu Home MPPT Watt pv Cursor Eje  Y
#define MPTPpvXtag 257                                     /// Imagen Text Menu Home tag Watt X
#define MPTPpvYtag 123                                     /// Imagen Text Menu Home tag Watt Y
#define MPTkWhX 249                                        /// Imagen Text Menu Home MPPT kWh pv Cursor Eje  X
#define MPTkWhY 226                                        /// Imagen Text Menu Home MPPT kWh pv Cursor Eje  Y
#define MPTWmaxX 225                                       /// Imagen Text Menu Home MPPT Watt Max pv Cursor Eje  X
#define MPTWmaxY 138                                       /// Imagen Text Menu Home MPPT Watt Max pv Cursor Eje  Y
#define MPTStatX 239                                       /// Imagen Text Menu Home MPPT Estado Cursor Eje  X
#define MPTStatY 121                                       /// Imagen Text Menu Home MPPT Estado Cursor Eje  Y 
#define IVTWarnX 107                                       /// Imagen Text Menu Home Inverter Warning Cursor Eje  X
#define IVTWarnY 203                                       /// Imagen Text Menu Home Inverter Warning Cursor Eje  Y
#define IVTACIX 247                                        /// Imagen Text Menu Home Inverter AC Amp eje X
#define IVTACIY 76                                         /// Imagen Text Menu Home Inverter AC Amp eje Y
#define IVTACIXtag 252                                     /// Imagen Text Menu Home Inverter AC Amp eje X
#define IVTACIYtag 19                                      /// Imagen Text Menu Home Inverter AC Amp eje Y
#define IVTACVX 141                                        /// Imagen Text Menu Home Inverter AC Volt eje X
#define IVTACVY 21                                         /// Imagen Text Menu Home Inverter AC Volt eje Y
#define IVTACWX 227                                        /// Imagen Text Menu Home Inverter AC AV(aproximadamente Watt) eje X
#define IVTACWY 48                                         /// Imagen Text Menu Home Inverter AC AV(aproximadamente Watt) eje Y
#define IVTAmpInX 143                                      /// Imagen Text Menu Home Inverter Amp input estimado aproximado eje X
#define IVTAmpInY 61                                       /// Imagen Text Menu Home Inverter Amp input estimado aproximado eje Y
#define MenuBackgroundX 0                                  /// Imagen Background Menu set Cursor X   Home, Solar, Inverter, MPPT, etc..
#define MenuBackgroundY 2                                  /// Imagen Background Menu set Cursor Y
#define hX  3.5                                            /// Menu Home Animation Reset image7x8
#define hY  3                                              /// Menu Home Animation Reset image 
#define vX  3.5                                            /// Menu Home Animation Reset image 9x10
#define vY  5                                              /// Menu Home Animation Reset image 
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
#define CHRGCurrSensCL "C1L1"                              /// Image Text SubMenu Charger Amp
#define CHRGMPTVbatCL "C1L2"                               /// Image Text SubMenu Charger Volt Bat MPPT
#define BatBarLevels 4                                     /// Bar battery Level number 4
#define BatBarLevelRadius 3                                /// Bar battery Level round corner
#define BatBarLevelHeight 15                               /// Bar battery Level Height
#define BatBarLevelWidth 70                                /// Bar battery Level Width
#define BatBarLevelNextY 20                                /// Bar battery Level Space
#define BatBarLevel1y 215                                  /// Bar battery Level 1 axis Y
#define BatBarLevelx 20                                    /// Bar battery Level 0 axis X
#define BatBarLevel0y 150                                  /// Bar battery Level 0 axis y
#define BatBarLevel0w 70                                   /// Bar battery Level 0 Width
#define BatBarLevel0h 72                                   /// Bar battery Level 0 Height
#define DayHr 24                                           /// 24h one day It is used to calculate the maximum possible kWh in a day.
#define Zero 0                                             /// Zero
#define One 1                                              /// One
#define Two 2                                              /// Two
#define Tree 3                                             /// Tree
#define milliTo 0.001                                      /// 0.001
#define centesiTo 0.01                                     /// 0.01
#define decimTo 0.1                                        /// 0.1
#define decuTo 0.5                                         /// 5
#define decaTo 10                                          /// 10
#define msCurrSens 5                                       /// CurrentSensor number of samples to make the average
#define CurrSensAVG 24                                     /// Current Sensor Average Calcul
#define RelayMsgAuto "WiFi"                                /// Relay OFF && Available to use remotely if smart connected  (Meross For example)
#define RelayMsgON  "ON"                                   /// Relay ON
#define RelayMsgLowbat "BAJA"                              /// Automatically turns on when the battery is low, displays this message
#define RelayMsgHighW "Watt"                               /// Automatically turns on when the watt is high, displays this message
#define RelayMsgX 39                                       /// Text Relay State Axis X
#define RelayMsgY 70                                       /// Text Relay State Axis Y
#define RelayEraseW 64                                     /// Text Relay State Width erase
#define RelayEraseH 15                                     /// Text Relay State Height erase
#define RelayMsgEraseX 1                                   /// Text Relay State Axis X erase
#define RelayMsgEraseY 13                                  /// Text Relay State Axis Y erase
#define RelayLedX 15                                       /// Relay LED state Axis X
#define RelayLedY 14                                       /// Relay LED state Axis Y
#define LedRelayRad 5                                      /// Relay LED state circumference radius pixel
#define msRelayON 700                                      /// Relay ms minimum for change state
#define msRelayOFF 700                                     /// Relay ms minimum for change state
// If you do not want to use automatic relay functions, you can set the RelayVbatON 0 & RelayWivrtON 9999.
#define RelayVbatON 25.7                                   //*** Volt   Vbat <          the relay turn ON automatically                             It is advisable to set the value that you do not want to go under
#define RelayVbatOFF 25.8                                  //*** If RelayLowBat=true && Vbat >= this value         Relay turn OFF automatically     It is recommended that its value should be RelayVbatOn + 0.2 minimum !!
#define RelayWIvrtON 210                                   //*** Watt   VA inverter >   the relay turn ON automatically                              It is advisable to set the value that you do not want to exceed.
#define RelayWIvrtOFF 140                                  //*** If RelayHighW=true && VAinverter <= this value    Relay turn OFF automatically     It is recommended that its value should be RelayWivrtOn + 50 minimum !!
#define RelayLevelON HIGH                                  //*** Relay Pin Output level state ON  (some relays may need a low level to activate, here you can reverse the logic for that)
#define RelayLevelOFF LOW                                  //*** Relay Pin Output level state OFF (some relays may need a low level to activate, here you can reverse the logic for that)
// CurrSensRAW It is used to calibrate the current sensor (exemple voltage divider made with two resistors of 2k7 and 4k7 serie, to go from 5v to approximately 3.3v)
#define CurrSensRAWZero 1015                               //*** Current Sensor RAWvalue 0 Amp (affected by the sensor supply voltage)                                  look for lines "calibra"
#define CurrSensRAWUnit 0.0206                             //*** Current Sensor RAWvalue 1 Amp   1 / 48.45 = 0.0206 to avoid division step to multiplication factor     look for lines "calibra"
#define Warm1 "BATERIA BAJA"                               //*** Inverter Warning code 1 Msg nivel demasiado bajo de bateria
#define Warm2 "BATERIA ALTA"                               //*** Inverter Warning code 2 Msg nivel demasiado alto de bateria
#define Warm32 "TEMP BAJA"                                 //*** Inverter Warning code 32 Msg Temperatura demasaido baja
#define Warm64 "TEMP ALTA"                                 //*** Inverter Warning code 64 Msg Temperatura demasaido alta
#define Warm256 "SOBRECARGA"                               //*** Inverter Warning code 256 Msg Sobrecarga solicitado enel output AC
#define Warm512 "RUIDO ELECT"                              //*** Inverter Warning code 512 Msg Ruido electrico interferencia anormal (a menudo podría ser condensadores defectuosos)
#define Warm1024 "AC BAJO"                                 //*** Inverter Warning code 1024 Msg AC Volt demasiado bajo
#define Warm2048 "AC ALTO"                                 //*** Inverter Warning code 2048 Msg AC Volt demasaido alto
#define MenuSolarErr0Msg "Sin errores"                     //*** Image Text SubMenu Solar MSG                          
#define MenuSolarErr2Msg "V Bat alta"                      //*** Image Text SubMenu Solar MSG
#define MenuSolarErr17Msg "Temp alta"                      //*** Image Text SubMenu Solar MSG
#define MenuSolarErr18Msg "Amp alta"                       //*** Image Text SubMenu Solar MSG
#define MenuSolarErr19Msg "Amp invers"                     //*** Image Text SubMenu Solar MSG
#define MenuSolarErr20Msg "Bulk overtime"                  //*** Image Text SubMenu Solar MSG
#define MenuSolarErr21Msg "Amp Sens mal"                   //*** Image Text SubMenu Solar MSG
#define MenuSolarErr26Msg "Temp Terminales"                //*** Image Text SubMenu Solar MSG
#define MenuSolarErr33Msg "V PV alto"                      //*** Image Text SubMenu Solar MSG
#define MenuSolarErr34Msg "A PV alto"                      //*** Image Text SubMenu Solar MSG
#define MenuSolarErr38Msg "V Bat exceso"                   //*** Image Text SubMenu Solar MSG
#define MenuSolarErr116Msg "Calibration mal"               //*** Image Text SubMenu Solar MSG 
#define MenuSolarErr117Msg "Firmware mal"                  //*** Image Text SubMenu Solar MSG
#define MenuSolarErr119Msg "Config user mal"               //*** Image Text SubMenu Solar MSG
#define MenuSolarCS0Msg "OFF"                              //*** Image Text SubMenu Solar MSG                          
#define MenuSolarCS2Msg "MPPT Falla"                       //*** Image Text SubMenu Solar MSG
#define MenuSolarCS3Msg "Carga Inicial"                    //*** Image Text SubMenu Solar MSG
#define MenuSolarCS4Msg "Absorcion"                        //*** Image Text SubMenu Solar MSG
#define MenuSolarCS5Msg "Floatacion"                       //*** Image Text SubMenu Solar MSG
#define MenuInverterCS0Msg "OFF"                           //*** Image Text SubMenu Inverter MSg Estado 0  OFF     
#define MenuInverterCS1Msg "LOWPOWER"                      //*** Image Text SubMenu Inverter MSg Estado 1  Low Power 
#define MenuInverterCS3Msg "FAULT"                         //*** Image Text SubMenu Inverter MSg Estado 3  Fault ( necesita reinicio por parte del usuario)
#define MenuInverterCS4Msg "ON"                            //*** Image Text SubMenu Inverter MSg Estado 4  ON
#define MenuInverterAR1Msg "low voltage"                   //*** Image Text SubMenu Inverter Alarm 1 Low volt  
#define MenuInverterAR2Msg "high voltage"                  //*** Image Text SubMenu Inverter Alarm 2 High volt
#define MenuInverterAR32Msg "Low Temp"                     //*** Image Text SubMenu Inverter Alarm 32 Low Temperatura
#define MenuInverterAR64Msg "High Temp"                    //*** Image Text SubMenu Inverter Alarm 64 Alta temperatura 
#define MenuInverterAR256Msg "OverLoad"                    //*** Image Text SubMenu Inverter Alarm 256 Sobrecarga
#define MenuInverterAR512Msg "DC Ripple"                   //*** Image Text SubMenu Inverter Alarm 512 Ruido Interferencia en el circuito
#define MenuInverterAR1024Msg "Low V AC Out"               //*** Image Text SubMenu Inverter Alarm 1024 AC Volt muy bajo
#define MenuInverterAR2048Msg "High V AC out"              //*** Image Text SubMenu Inverter Alarm 2048 AC Volt muy alto
#define MenuInvertermode2Msg " ON"                         //*** Image Text SubMenu Inverter Modo 2 ON
#define MenuInvertermode4Msg " OFF"                        //*** Image Text SubMenu Inverter Modo 4 OFF
#define MenuInvertermode5Msg " ECO"                        //*** Image Text SubMenu Inverter MOdo 5 ECO
#define MenuInverterwarn1Msg "LOW VOTL"                    //*** Image Text SubMenu Inverter Warning 1 volt Bajo
#define MenuInverterwarn2Msg "HIGH VOLT"                   //*** Image Text SubMenu Inverter Warning 2 Volt alto
#define MenuInverterwarn32Msg "LOW TEMP"                   //*** Image Text SubMenu Inverter Warning 32 Temperatura Baja
#define MenuInverterwarn64Msg "HIGH TEMP"                  //*** Image Text SubMenu Inverter Warning 64 Temperatura Alta 
#define MenuInverterwarn256Msg "OverLoad"                  //*** Image Text SubMenu Inverter Warning 256 Sobracarga
#define MenuInverterwarn512Msg "DC Ripple"                 //*** Image Text SubMenu Inverter Warning 512 Ruido en la linea electrica
#define MenuInverterwarn1024Msg   "LOW V AC out"           //*** Image Text SubMenu Inverter Warning 1024 AC Volt muy bajo
#define MenuInverterwarn2048Msg "High V AC out"            //*** Image Text SubMenu Inverter Warning 2048 AC Volt muy alto
#define BATVolt "25.6V"                                    //*** Hardware Spec Imagen Text SubMenu Bateria Volt tipico                               
#define BATVCharg "28.4V"                                  //*** Hardware Spec Imagen Text SubMenu Bateria Volt para carga
#define BATAh "111Ah"                                      //*** Hardware Spec Imagen Text SubMenu Bateria Capacidad
#define BATkWh "2.8416kWh"                                 //*** Hardware Spec Imagen Text SubMenu Bateria kWh equivalentes      
#define BATTip "LiFePO4"                                   //*** Hardware Spec Imagen Text SubMenu Bateria tipo de tecnologia          
#define BATDate "2021-03-22"                               //*** Hardware Spec Imagen Text SubMenu Bateria Fecha de fabricacion
#define BATBrand "Pylontech"                               //*** Hardware Spec Imagen Text SubMenu Bateria Marca
#define BATModel "UP2500NB01V00101"                        //*** Hardware Spec Imagen Text SubMenu Bateria Modelo
#define BATSerialNumbr "PPTBP0xxxxxxxxxx"                  //*** Hardware Spec Imagen Text SubMenu Bateria Numero de Serie
#define BATCell "IFpP/13/141/238/[(3P3S)2S+3P2S]M/0+50/95" //*** Hardware Spec Imagen Text SubMenu Bateria Celda tipologia estructura
#define BatLevel1 25.6                                     //*** Hardware Spec Bateria Control volt bar level 1/4 
#define BatLevel2 26.0                                     //*** Hardware Spec Bateria Control volt bar level 2/4 
#define BatLevel3 26.4                                     //*** Hardware Spec Bateria Control volt bar level 3/4
#define BatLevel4 26.7                                     //*** Hardware Spec Bateria Control volt bar level 4/4 
#define MPTkWhHistAdd 103                                  //*** Hardware Spec MPPT kWh Lost by the user when resetting the counter (in my case 103kWh is missing which I will add each time to have the real historical value)
#define MPPTVmax 145                                       //*** Hardware Spec MPPT Spec Volt Max MPPT user defined  150 35
#define MPPTAmax 35                                        //*** Hardware Spec  MPPT Spec Amp  Max MPPT user defined  150 35
#define MPPTWmax 1000                                      //*** Hardware Spec  MPPT Spec Watt max depends on the type of voltage used 24V = 1000W limited
#define InverterVacmin 225.4                               //*** Hardware Spec  Inverter Spec 230 -2%
#define InverterVac 230                                    //*** Hardware Spec  Inverter Spec 230 -2%
#define InverterVacmax 234.6                               //*** Hardware Spec  Inverter Spec 230 +2%
#define InverterVbatmin 18.6                               //*** Hardware Spec  Inverter Spec volt battery min   Below this value it is understood that there is no battery connected.
#define InverterVbatmax 34                                 //*** Hardware Spec  Inverter Spec volt battery maxç
#define InverterIbatmax 70                                 //*** Hardware Spec  Inverter Spec Amp input max = wmax/vbatmax
#define InverterWmax 2400                                  //*** Hardware Spec  Inverter Spec VA max ( 3000VA +/- 2400W )
#define InverterEfimax 94                                  //*** Hardware Spec  Inverter Spec Efficiency Maxi 94%          (- =*0.94) (+ =*1.94) 
#define InverterLossmin 1.06                               //*** Hardware Spec  Inverter Spec minimum Loss 100-94= 6%    (- =*0.06) (+ =*1.06) 
#define ChargerImax 16.5                                   //*** Hardware Spec Charger Spec Amp max (16A max spec model Victron Energy BPC241642002)
#define ChargerImin 3.5                                    //*** Hardware Spec Charger Spec Amp min ( 4A min spec model Victron Energy BPC241642002)

#define msLoop 125                   //// Loop ms millis  (This may affect the flickering of text on the screen.)
unsigned long previousMillis = 0;    //// Delay
unsigned long currentMillis = 0;     //// Delay

String IVTCSStrgmsg;                 /// Inverter String Estado msg
String IVTARStrgmsg;                 /// Inverter String Alarmas msg
String IVTfwStrg;                    /// Inverter String Firmware
String IVTpidStrg;                   /// Inverter String Producto ID
String IVTSERStrg;                   /// Inverter String numero serie NS
String IVTmodeStrgmsg;               /// Inverter String Modo msg
String IVTWarnStrgmsg;               /// Inverter String Warm msg
String MPTErrorStrgmsg;              /// MPPT Msg Error
String MPTCSStrgmsg;                 /// MPPT Msg Estado
String MPTSERStrg;                   /// MPPT String Numero de Serie del producto 
String MPTpidStrg;                   /// MPPT String ID del Producto
String MPTfwStrg;                    /// MPPT String Firmware
String MenuCL="C1L1";                /// Imagen Text Linea y Columna elegida                                    (no usado en Menu Home, solo submenu Solar, Charger, Inverter Bateria ..)
String MenuPresent="Home";           /// Menu en Display actualmente
String MenuSelected="Home";          /// Menu Solicitado al tocar la pantalla
String Origin="MPP";                 /// VE.Direct Lectura de datos organizado por Origen   MPPT o Inverter
String label="V";                    /// VE.Direct Lectura de datos tipo de lectura         V, VPV, PPV, AC V, AC I, ALARM, WARM, ... 
String val="0";                      /// VE.Direct Lectura de datos valor de lectura        26.4v, error4, ...
String NewValueStrg="";              /// String      text format

float NewValue=0;                    /// int* 1     numerical format 
float  NewValuedecim=0;              /// int  0.1   
float  NewValuecentesi=0;            /// int  0.01  
float  NewValuemilli=0;              /// int  0.001 
float  NewValuedeca=0;               /// int  10 
uint16_t i=0;                        /// Genreal Use Used in counter
uint8_t MovSpeed= 1;                 /// Imagen Animacion Bola Animada Velociad de movimiento depende tambien del loop -->  pixel por loop
uint16_t IVTMode;                    /// Inverter Modo integral
float IVTACV;                        /// Inverter AC Volt
float IVTACI;                        /// Inverter AC Amp
uint16_t IVTACS;                     /// Inverter AV "aproximadamente Watt AC"
float IVTAmpIn;                      /// Inverter Amp Input (DC) Estimados aproximado 9% perdida esto no tiene en cuenta carga inductiva o resistiva
float IVTVbat;                       /// Inverter Bateria Volt
float BatLevelPresent;               /// Battery Bar Level Home Menu
float MPTVbat;                       /// MPPT Bateria Volt   float para poder calcular el % correctamente necesita decimales
float MPTVpv;                        /// MPPT Solar Volt paneles
uint16_t MPTPpv;                     /// MPPT Watt Solar panel
uint16_t MPTAmp;                     /// MPPT Amp
uint16_t MPTkWh;                     /// MPPT kWh HOY
uint16_t MPTWmax;                    /// MPPT W max HOY
int MPTkWhHist;                      /// MPPT kWh Historico                                       
uint8_t MPTCS;                       /// MPPT estado int
int MPTWmaxAyer;                     /// MPPT Wmax Ayer int
int MPTkWhAyer;                      /// MPPT kWh Ayer int
uint16_t ChrgBatStrtY = 94;          /// Imagen Menu Home Animacion Bola de Charger a Bateria eje Y     Down
uint16_t ChrgInvrtStrtX = 108;       /// Imagen Menu Home Animacion Bola de Charger a Inverter eje X    Right
uint16_t LoadStrtX = 203;            /// Imagen Menu Home Animacion Bola de Inverter a Load eje X       Right
uint16_t BATStrtX = 110;             /// Imagen Menu Home Animacion Bola de Bateria a Inverter eje X    Right
uint16_t BATFinX = 150;              /// Imagen Menu Home Animacion Bola de Solar a Bateria eje X       Left
uint16_t SolarStrtX = 210;           /// Imagen Menu Home Animacion Bola de SolarBat a Inverter eje X   Left
uint16_t InvrtStrtY = 141;           /// Imagen Menu Home Animacion Bola de BatSolar a Inverter eje Y   UP
uint8_t ResetMPTData=200;            /// VE.Direct Data Reset MPPT 
uint8_t ResetIVTData=200;            /// VE.Direct Data Reset Inverter
uint16_t TchX=0;                     /// Menu eje X Colocacion de background para el menu solicitado
uint16_t TchY=0;                     /// Menu eje Y ...
float CurrSens=0;                    /// Charger Current Sensr AMp Value show display buffer memoria
float BATAmp = 0;                    /// Battery Current Negative value is amps coming out of the battery. Positive value the battery is charging

bool AnimSolOut=false;               /// Imagen Animacion interruptor de activacion Solar Output
bool AnimIVTOutput=false;            /// Imagen Animacion interruptor de activacion Inverter-->Load Output
bool AnimIVTInput=false;             /// Imagen Animacion interruptor de activacion Inverter Input
bool AnimChargIVT=false;             /// Imagen Animacion interruptor de activacion Charger-->Inverter
bool AnimChargBat=false;             /// Imagen Animacion interruptor de activacion Charger-->Bateria
bool AnimBatOut=false;               /// Imagen Animacion interruptor de activacion Bateria Output
bool AnimBatIn=false;                /// Imagen Animacion interruptor de activacion Bateria Input
bool RelayStatePresent=false;        /// It will be used to buy the current state and the requested state by pressing the screen.
bool RelayStateSelect=false;         /// It will be used to buy the current state and the requested state by pressing the screen.
bool RelayShow=true;                 /// Update the relay status displayed on the screen.
bool RelayLowBat=false;              /// Indicates if the charger relay was activated due to the battery voltage being very low.  (related to:  RelayVbatON && RelayVbatOFF && RelayMsgLowbat)
bool RelayHighW=false;               /// Indicates if the charger relay was activated due to the inverter watt is too high.       (related to:  RelayWIvrtON && RelayWIvrtOFF && RelayMsgHighW)

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC,TFT_RST);  /// Display ili9341 pin IMAGEN
XPT2046_Touchscreen ts(CS_PIN, TIRQ_PIN);                         /// Display ili9341 pin TOUCH   Param 2 - Touch IRQ Pin - interrupt enabled polling

void SerialSet(){         // initial setup serial
  Serial1.begin(UARTRate1,SERIAL_8N1,UART1rx,UART1tx);  // Serial 1 OK reasignado RX en GPIO27 D27 [ y TX en GPIO14 D14 no probado ]
  Serial2.begin(UARTRate2,SERIAL_8N1);                  // Serial 2 MPPT     ( se pueden cambiar los pines en la config esp32) 
}
void PinSet(){            // initial setup pin esp32
  pinMode(CS_PIN,OUTPUT);                                // high level Display
  digitalWrite(CS_PIN,HIGH);                             // high level Display
  pinMode(TFT_CS,OUTPUT);                                // Output Display
  pinMode(CurrSensPin, INPUT);                           // Input Currrent Sensor
  analogReadResolution(12);                              // ADC 12 Bits

  pinMode(PinRelay, OUTPUT);                                     // Pin Relay output mode
  digitalWrite(PinRelay, RelayLevelOFF);                          // Pin Relay turn off 
}
void DisplaySet(){        // initial setup display
  tft.begin();        // can be raised to tft.begin(40000000) to avoid flickering   tft.begin(40000000); 
  tft.setRotation(3); // rotate screen  0 1 2 3  =  0º 90º 180º 270º
  ts.begin();         // activate the touch function of the display
  ts.setRotation(3);  // rotate screen touch should be the same as the rotated image (tft.setRotation)
}
void AnimSolarOut(){      // Menu Home Animation MPPT output 
  tft.drawRGBBitmap(SolarStrtX - (hX-1), SolarStrtY - hY, AnimRstH, AnimRstH_WIDTH,AnimRstH_HEIGHT);
  if(SolarStrtX>SolarFinX){
    SolarStrtX= (SolarStrtX - MovSpeed);
    }else{
    SolarStrtX=SolarStrtXDef;
  }
  tft.fillCircle(SolarStrtX, SolarStrtY, CirclRad,CirclColor);
}
void AnimInverterOuput(){ // Menu Home Animation INVERTER output
  tft.drawRGBBitmap(LoadStrtX - hX, LoadStrtY - hY, AnimRstH, AnimRstH_WIDTH,AnimRstH_HEIGHT);
  if(LoadStrtX<LoadFinX){
    LoadStrtX= (LoadStrtX + MovSpeed);
    }else{
    LoadStrtX=LoadStrtXDef;
  }
  tft.fillCircle(LoadStrtX, LoadStrtY, CirclRad,CirclColor);
}
void AnimInverterInput(){ // Menu Home Animation INVERTER input
  tft.drawRGBBitmap(InvrtStrtX - vX, InvrtStrtY - vY, AnimRstV, AnimRstV_WIDTH,AnimRstV_HEIGHT);
  if(InvrtStrtY>InvrtFinY){
      InvrtStrtY= (InvrtStrtY - MovSpeed);
    }else{
      InvrtStrtY=InvrtStrtYDef;
  }
  tft.fillCircle(InvrtStrtX, InvrtStrtY, CirclRad,CirclColor);
}
void AnimChrgInvrt(){     // Menu Home Animation CHARGER to INVERTER
  tft.drawRGBBitmap(ChrgInvrtStrtX - hX, ChrgInvrtStrtY - hY, AnimRstH, AnimRstH_WIDTH,AnimRstH_HEIGHT);
  if(ChrgInvrtStrtX<ChrgInvrtFinX){
    ChrgInvrtStrtX= (ChrgInvrtStrtX + MovSpeed);
    }else{
    ChrgInvrtStrtX=ChrgInvrtStrtXDef;
  }
  tft.fillCircle(ChrgInvrtStrtX, ChrgInvrtStrtY, CirclRad,CirclColor);
}
void AnimChrgBat(){       // Menu Home Animation CHARGER to BATTERY
  tft.drawRGBBitmap(ChrgBatStrtX - vX, ChrgBatStrtY - vY, AnimRstV, AnimRstV_WIDTH,AnimRstV_HEIGHT);
  if(ChrgBatStrtY<ChrgBatFinY){
    ChrgBatStrtY= (ChrgBatStrtY + MovSpeed);
    }else{
    ChrgBatStrtY=ChrgBatStrtYDef;
  }
  tft.fillCircle(ChrgBatStrtX, ChrgBatStrtY, CirclRad,CirclColor);
}
void AnimBatOuput(){      // Menu Home Animation BATTERY output
  tft.drawRGBBitmap(BATStrtX - hX, BATStrtY - hY, AnimRstH, AnimRstH_WIDTH,AnimRstH_HEIGHT);
  if(BATStrtX<BATFinX){
    BATStrtX= (BATStrtX + MovSpeed);
    }else{
      BATStrtX=BATStrtXDef;
  }
  tft.fillCircle(BATStrtX, BATStrtY, CirclRad,CirclColor); // charge
}
void AnimBatInput(){      // Menu Home Animation BATTERY input  ( notes added)
  tft.drawRGBBitmap(BATFinX - (hX-1), BATStrtY - hY, AnimRstH, AnimRstH_WIDTH,AnimRstH_HEIGHT); // delete the place of the previous animation blue ball
  if(BATStrtX<BATFinX){                                    // calculate new location of the blue ball
    BATFinX= (BATFinX - MovSpeed);                         // set how much spixel moves in each loop
    }else{
      BATFinX=BATFinXDef;                                  //end of the journey return from the beginning
  }
  tft.fillCircle(BATFinX, BATStrtY, CirclRad,CirclColor);  // draw a blue ball in its new location
}
void ReadSerial1(){       // Data from VE.Direct Inverter
  if (Serial1.available()){
    Origin="Inverter";
    label= Serial1.readStringUntil('\t'); // no se pueden usar  "x" hay qu eponer 'x'
    val = Serial1.readStringUntil('\n');     
    }else{
    Origin="IVTnodata";
  }    
}
void ReadSerial2(){       // Data from VE.Direct MPPT
  if (Serial2.available()){
    Origin="MPPT";
    label= Serial2.readStringUntil('\t');
    val = Serial2.readStringUntil('\n');     
    }else{  
    Origin="MPTnodata";    
  }
}   
void CurrentSensor(){     // Data Charger amper calcul from Current Sensor 
  NewValue=Zero;
  for( i = Zero; i < CurrSensAVG; i++) {          // read several times to have an average value
    NewValue += analogRead(CurrSensPin);
    delay(1);
  }

  NewValue = ( NewValue / (CurrSensAVG)) ;                          // RAW value for debug en calibration
  NewValue = (( NewValue - CurrSensRAWZero ) * CurrSensRAWUnit );   // RAW to Amp   deactivate this line with "//"" to see RAW on the screen and calibrate your current sensor calibration

  if(  NewValue < One ){
    NewValue=Zero;                                                // < 1 A  Is not taken into account
  }
}
void SetColumnLine(){     // SubMenu set cursor for print show display data
   // extract X Y axis position from C1L1
  int column, line;
  sscanf(MenuCL.c_str(), "C%dL%d", &column, &line);
  line--;                                          // start from line0
  if (column == 1) {
    tft.setCursor(MenuColumn1X, MenuColumn1Y + (MenuColumn1NextLine * line));
  } else if (column == 2) {
    tft.setCursor(MenuColumn2X, MenuColumn2Y + (MenuColumn2NextLine * line));
  }
}  
void HomePrintInt(const GFXfont *fonts, uint16_t colorold, uint16_t colornew, int x, int y, float old, float renew, uint8_t decim, String unit){   // Menu Home format int
  // font , backgrounb text color to erase, text color to write, Cursor axis X, Cursor axis Y, old value to erase, new value to write, figures after the decimal point, unit of measurement
  tft.setFont(fonts);
  tft.setTextColor(colorold,colorold);  // text color & text background color
  tft.setCursor(x, y);
  tft.print(old, decim);
  tft.print(unit);
  tft.setTextColor(colornew,colorold);
  tft.setCursor(x, y);
  tft.print(renew, decim);
  tft.print(unit);
}
void HomePrintStrg(const GFXfont *fonts, uint16_t colorold, uint16_t colornew, int x, int y, String oldStrg, String renewStrg, String tag) {       // Menu Home format String
  // font , backgrounb text color to erase, text color to write, Cursor axis X, Cursor axis Y, old value to erase, new value to write, figures after the decimal point, unit of measurement
  tft.setFont(fonts);
  tft.setTextColor(colorold,colorold);  // text color & text background color
  tft.setCursor(x, y);
  tft.print(tag);
  tft.print(oldStrg);
  tft.setTextColor(colornew,colorold);
  tft.setCursor(x, y);
  tft.print(tag);
  tft.print(renewStrg);
}
void SubMenuPrintInt(const GFXfont *fonts,uint16_t colorold, uint16_t colornew, float old, float renew, uint8_t decim, String unit){               // Sub Menu format int
  // font , backgrounb text color to erase, text color to write, SteColumnLine = Cursor axis X & Cursor axis Y, old value to erase, new value to write, figures after the decimal point, unit of measurement
  tft.setFont(fonts);
  tft.setTextColor(colorold,colorold);  // text color & text background color
  SetColumnLine();
  tft.print(old, decim);
  tft.print(unit);
  tft.setTextColor(colornew,colorold);
  SetColumnLine();
  tft.print(renew, decim);
  tft.print(unit);
}
void SubMenuPrintStrg(const GFXfont *fonts, uint16_t colorold, uint16_t colornew, String oldStrg, String renewStrg, String tag){                   // Sub Menu format String
  // font , backgrounb text color to erase, text color to write, Cursor axis X, Cursor axis Y, old value to erase, new value to write, figures after the decimal point, unit of measurement
  tft.setFont(fonts);
  tft.setTextColor(colorold,colorold);  // text color & text background color
  SetColumnLine();
  tft.print(tag);
  tft.print(oldStrg);
  tft.setTextColor(colornew,colorold);
  SetColumnLine();
  tft.print(tag);
  tft.print(renewStrg);
}
void Touched(){        // Touch Touched
    //create requested menu, set background & text, reset values, and mark requested menu as present menu
    if(MenuSelected=="Home"){
      tft.drawRGBBitmap(MenuBackgroundX, MenuBackgroundY, HomeBitmap, HOME_WIDTH,HOME_HEIGHT);  // set cursor X , y , select image, image widht, image height
      HomePrintStrg(TxtSize1,TextColorGreen, TextColorWhite, IVTACIXtag,IVTACIYtag, "VA", "VA", "");
      HomePrintStrg(TxtSize1,TextColorOrange,TextColorWhite, MPTPpvXtag, MPTPpvYtag, "W", "W", "");
      HomePrintStrg(TxtSize1,TextColorRed,TextColorWhite, ChrgAmpXtag, ChrgAmpYtag, "A", "A", "");
      ValueRst();
      MenuPresent="Home";
    }else if(MenuSelected=="Solar"){
      tft.drawRGBBitmap(MenuBackgroundX, MenuBackgroundY, SolarBackGround, SolarMenu_WIDTH,SolarMenu_HEIGHT);
      ValueRst();
      MenuPresent="Solar";
    }else if(MenuSelected=="Bat"){
      tft.drawRGBBitmap(MenuBackgroundX, MenuBackgroundY, BatBackGround, BatMenu_WIDTH,BatMenu_HEIGHT);
      ValueRst();
      MenuPresent="Bat";
    }else if(MenuSelected=="Inverter"){
      tft.drawRGBBitmap(MenuBackgroundX, MenuBackgroundY, InvrtBackGround, InvrtMenu_WIDTH,InvrtMenu_HEIGHT);
      ValueRst();
      MenuPresent="Inverter";
    }else if(MenuSelected=="Charger"){
      tft.drawRGBBitmap(MenuBackgroundX, MenuBackgroundY, ChrgBackGround, ChrgMenu_WIDTH,ChrgMenu_HEIGHT);
      ValueRst();
      MenuPresent="Charger";
    }else if(MenuSelected=="Load"){
      tft.drawRGBBitmap(MenuBackgroundX,MenuBackgroundY, LoadBackGround, LoadMenu_WIDTH,LoadMenu_HEIGHT);
      ValueRst();
    MenuPresent="Load";
  }

}
void TouchAsk(){       // Touch Did I touch a button on the screen?
  if (ts.touched()) {            // screen touched?
    TS_Point p = ts.getPoint();  // save point
    TchX=p.x;                    // save point axis X
    TchY=p.y;                    // save point axis Y
    if ((TchX > TchHomeXmin && TchX < TchHomeXmax) && (TchY > TchHomeYmin && TchY < TchHomeYmax)) {
      MenuSelected="Home";
    }else if ((TchX > TchSolarXmin && TchX < TchSolarXmax) && (TchY > TchSolarYmin &&TchY < TchSolarYmax)) {
        MenuSelected="Solar";
    } else if ((TchX > TchBatXmin && TchX < TchBatXmax) && (TchY > TchBatYmin && TchY < TchBatYmax)) {
        MenuSelected="Bat";
    }else if ((TchX > TchChrgXmin && TchX < TchChrgXmax) && (TchY > TchChrgYmin && TchY < TchChrgYmax)) {
        MenuSelected="Charger";
    }else if ((TchX> TchInvrtXmin && TchX < TchInvrtXmax) && (TchY > TchInvrtYmin && TchY < TchInvrtYmax)) {
        MenuSelected="Inverter";
    }else if ((TchX> TchLoadXmin && TchX < TchLoadXmax) && (TchY > TchLoadYmin && TchY < TchLoadYmax)) {
        MenuSelected="Load";
    }else if ((TchX> TchRelayXmin && TchX < TchRelayXmax) && (TchY > TchRelayYmin && TchY < TchRelayYmax)) {   // Relay Charger turn ON OFF
      RelayStateSelect=!RelayStatePresent;   
      RelayCheck();
    }
    if(MenuSelected!=MenuPresent){   //  if the present menu is different from the requested menu
      Touched();
    }
  }
}
void RelayCheck(){
  if(RelayStateSelect != RelayStatePresent){
    if(RelayStateSelect == true){
      RelayON();
    }else{
      RelayOFF();
    }
    RelayShow = true;   
  }
}
void RelayON(){
  digitalWrite(PinRelay, RelayLevelON);                           // Relay turn ON;
  RelayStatePresent=true;
  RelayStateSelect=true;
  RelayShowStat();
  delay(msRelayON);                                                       // avoid false button press                                           //   Delay set
}
void RelayOFF(){
  digitalWrite(PinRelay, RelayLevelOFF);                          // Relay turn OFF
  RelayStatePresent=false;
  RelayStateSelect=false;
  RelayLowBat=false;
  RelayHighW=false;
  RelayShowStat();
  delay(msRelayOFF);                                                       // avoid false button press   
}
void RelayShowStat(){
  if(RelayShow==true){                                                    // Update status only if there is a status change
    tft.fillRect(RelayMsgX-RelayMsgEraseX, RelayMsgY-RelayMsgEraseY, RelayEraseW, RelayEraseH , TextColorRed);    // erase old state
    if(RelayStatePresent==true){                                          // ON
      if(RelayLowBat==true){                                              
        tft.fillCircle(RelayLedX, RelayLedY, LedRelayRad,ILI9341_ORANGE);    // LED ORANGE VBat Low turn ON Charger Relay
        tft.setFont(TxtSize1);                                               // print show display new state Low Bat
        tft.setTextColor(TextColorOrange);  
        tft.setCursor(RelayMsgX, RelayMsgY);
        tft.print(RelayMsgLowbat);
      } else if ( RelayHighW==true ){
        tft.fillCircle(RelayLedX, RelayLedY, LedRelayRad,ILI9341_ORANGE);    // LED  ORANGE Watt High turn ON Charger Relay
        tft.setFont(TxtSize1);                                               // print show display new state High Watt
        tft.setTextColor(TextColorOrange);  
        tft.setCursor(RelayMsgX, RelayMsgY);
        tft.print(RelayMsgHighW);
      }else{
        tft.fillCircle(RelayLedX, RelayLedY, LedRelayRad,ILI9341_GREEN);     // LED GREEN
        tft.setFont(TxtSize1);                                               // print show display new state ON
        tft.setTextColor(TextColorGreen);  
        tft.setCursor(RelayMsgX, RelayMsgY);
        tft.print(RelayMsgON);
      }
    } else{                                                           // OFF
      tft.fillCircle(RelayLedX, RelayLedY, LedRelayRad,TextColorRed);        // LED RED + background RED = LED is OFF
      tft.setFont(TxtSize1);                                                 // print show display new state WiFi    (used for smart plug controlled from outside for example Meross)
      tft.setTextColor(TextColorWhite);  
      tft.setCursor(RelayMsgX, RelayMsgY);
      tft.print(RelayMsgAuto);
    }
   RelayShow=false;
  }
}
void RelayAuto(){
  if(!RelayStatePresent && MPTVbat>InverterVbatmin && MPTVbat<RelayVbatON ){  // If  InverterVbatmin < Vbat < RelayAutoON = low battery  // if InverterVbatmin > Vbat = no battery, do not activate auto relay
    RelayStateSelect=true;
    RelayLowBat=true;
    RelayCheck();
  }
  if(RelayStatePresent && RelayLowBat && MPTVbat>RelayVbatOFF ){              // If the relay was activated due to low battery && has returned to normal values 
    RelayStateSelect=false;
    RelayLowBat=false;
    RelayCheck();
  }
  if(!RelayStatePresent && IVTACS>=RelayWIvrtON){                             // If  InverterVbatmin < Vbat < RelayAutoON = low battery  // if InverterVbatmin > Vbat = no battery, do not activate auto relay
    RelayStateSelect=true;
    RelayHighW=true;
    RelayCheck();
  }
  if(RelayStatePresent && RelayHighW && IVTACS<=RelayWIvrtOFF){               // If the relay was activated due to high power && has returned to normal values
    RelayStateSelect=false;
    RelayHighW=false;
    RelayCheck();
  }   
}
void NewValueSet(){    // Data New Value Set 
  NewValueStrg = val;                   // RAW value to String
  NewValue = NewValueStrg.toInt();      // String to int
  NewValuedecim=(NewValue*decimTo);     // int to  / 0.1
  NewValuecentesi=(NewValue*centesiTo); // int to / 0.01
  NewValuemilli=(NewValue*milliTo);     // int to / 0.001
  NewValuedeca=(NewValue*decaTo);       // int to * 10
}
void HomeBatLevel() {  // Menu Home Battery Level    (from Vbat MPPT)
  if(abs(BatLevelPresent-MPTVbat)>=decimTo){                                                                           // only update if the change is equal to or higher than decimTo = 0.1 variance
    if (MPTVbat > BatLevel4) {
      tft.fillRoundRect(BatBarLevelx, BatBarLevel1y - (BatBarLevelNextY * 3), BatBarLevelWidth, BatBarLevelHeight, BatBarLevelRadius, BatBarLevelColor);  // Bar level 4 ON
      tft.fillRoundRect(BatBarLevelx, BatBarLevel1y - (BatBarLevelNextY * 2), BatBarLevelWidth, BatBarLevelHeight, BatBarLevelRadius, BatBarLevelColor);  // Bar level 3 ON
      tft.fillRoundRect(BatBarLevelx, BatBarLevel1y - BatBarLevelNextY, BatBarLevelWidth, BatBarLevelHeight, BatBarLevelRadius, BatBarLevelColor);        // Bar level 2 ON
      tft.fillRoundRect(BatBarLevelx, BatBarLevel1y, BatBarLevelWidth, BatBarLevelHeight, BatBarLevelRadius, BatBarLevelColor);                           // Bar level 1 ON
    } else if (MPTVbat > BatLevel3) {
      tft.fillRoundRect(BatBarLevelx, BatBarLevel1y - (BatBarLevelNextY * 3), BatBarLevelWidth, BatBarLevelHeight, BatBarLevelRadius, TextColorBlue);     // Bar level 4 OFF
      tft.fillRoundRect(BatBarLevelx, BatBarLevel1y - (BatBarLevelNextY * 2), BatBarLevelWidth, BatBarLevelHeight, BatBarLevelRadius, BatBarLevelColor);  // Bar level 3 ON
      tft.fillRoundRect(BatBarLevelx, BatBarLevel1y - BatBarLevelNextY, BatBarLevelWidth, BatBarLevelHeight, BatBarLevelRadius, BatBarLevelColor);        // Bar level 2 ON
      tft.fillRoundRect(BatBarLevelx, BatBarLevel1y, BatBarLevelWidth, BatBarLevelHeight, BatBarLevelRadius, BatBarLevelColor);                           // Bar level 1 ON
    } else if (MPTVbat > BatLevel2) {
      tft.fillRoundRect(BatBarLevelx, BatBarLevel1y - (BatBarLevelNextY * 3), BatBarLevelWidth, BatBarLevelHeight, BatBarLevelRadius, TextColorBlue);     // Bar level 4 OFF
      tft.fillRoundRect(BatBarLevelx, BatBarLevel1y - (BatBarLevelNextY * 2), BatBarLevelWidth, BatBarLevelHeight, BatBarLevelRadius, TextColorBlue);     // Bar level 3 OFF
      tft.fillRoundRect(BatBarLevelx, BatBarLevel1y - BatBarLevelNextY, BatBarLevelWidth, BatBarLevelHeight, BatBarLevelRadius, BatBarLevelColor);        // Bar level 2 ON
      tft.fillRoundRect(BatBarLevelx, BatBarLevel1y, BatBarLevelWidth, BatBarLevelHeight, BatBarLevelRadius, BatBarLevelColor);                           // Bar level 1 ON
    } else if (MPTVbat > BatLevel1) {
      tft.fillRoundRect(BatBarLevelx, BatBarLevel1y - (BatBarLevelNextY * 3), BatBarLevelWidth, BatBarLevelHeight, BatBarLevelRadius, TextColorBlue);     // Bar level 4 OFF
      tft.fillRoundRect(BatBarLevelx, BatBarLevel1y - (BatBarLevelNextY * 2), BatBarLevelWidth, BatBarLevelHeight, BatBarLevelRadius, TextColorBlue);     // Bar level 3 OFF
      tft.fillRoundRect(BatBarLevelx, BatBarLevel1y - BatBarLevelNextY, BatBarLevelWidth, BatBarLevelHeight, BatBarLevelRadius, TextColorBlue);           // Bar level 2 OFF
      tft.fillRoundRect(BatBarLevelx, BatBarLevel1y, BatBarLevelWidth, BatBarLevelHeight, BatBarLevelRadius, BatBarLevelColor);                           // Bar level 1 ON
    } else if(MPTVbat < BatLevel1){
      tft.fillRect(BatBarLevelx, BatBarLevel0y, BatBarLevel0w, BatBarLevel0h, TextColorBlue);                                                             // Bar level OFF very low battery voltage
    }
    BatLevelPresent=MPTVbat;
  }
}
void HomeCurrentS(){   // Menu Home Charger amper
  CurrentSensor();                      // refresh read current sensor
  if(abs(CurrSens - NewValue)>=decuTo){     //   decu = 0.5 variance
    if(NewValue < ChargerImin){
        NewValue=Zero;
        HomePrintInt(TxtSize2,TextColorRed,TextColorRed, ChrgAmpX, ChrgAmpY, CurrSens, NewValue, One , "");
      }else if( NewValue >= ChargerImin && NewValue <= ChargerImax ){  // deactivate this line with // to see RAW on the screen and calibrate your current sensor calibration
        HomePrintInt(TxtSize2,TextColorRed,TextColorWhite, ChrgAmpX, ChrgAmpY, CurrSens, NewValue, One , "");
      }
    CurrSens=NewValue;          
  }                                                          // deactivate this line with // to see RAW on the screen and calibrate your current sensor calibration
}
void HomeIVtAmpIn(){   // Menu Home Inverter amper calcul  (VA is not Watt but close to be able to estimate Amper )
  NewValue=((IVTACS / IVTVbat)*InverterLossmin);  // loss inverter add
    if(abs(IVTAmpIn - NewValue)>=decimTo){   // 0.1 dif
      if( NewValue > Zero && NewValue < InverterIbatmax ){
        HomePrintInt(TxtSize1,TextColorBlue,TextColorWhite, IVTAmpInX, IVTAmpInY, IVTAmpIn, NewValue, One , "A");
        IVTAmpIn=NewValue;
      }
    }
}
void HomeBatAmp(){     // Menu Home Battery amper calcul
  NewValue= (CurrSens+MPTAmp)-IVTAmpIn;
  if( abs(BATAmp - NewValue)>=decuTo){  // 0.5 dif
    HomePrintInt(TxtSize1,TextColorBlue,TextColorWhite, BATAmpX, BATAmpY, BATAmp, NewValue, One , "A");  
    BATAmp=NewValue;
  }
}
void HomeAnimAsk(){    // Menu Home Animation ask
  AnimSolOut     = MPTPpv  > One;
  AnimIVTOutput  = IVTACS  > One;
  AnimBatIn      = BATAmp  > Zero;
  AnimBatOut     = BATAmp  < Zero;
  AnimChargBat   = CurrSens > Zero;
  AnimChargIVT   = CurrSens > Zero;
  AnimIVTInput   = IVTAmpIn > Zero;

  if(MPTPpv<One){
    AnimSolOut=false;
    AnimBatIn=false;
    if(CurrSens>IVTAmpIn){
     AnimIVTInput=false;
    }
    if(CurrSens<IVTAmpIn){
     AnimChargBat=false;
    }
  }
  if(MPTPpv>One){
    if(MPTPpv>IVTAmpIn){
      AnimBatOut=false;
      AnimBatIn=true;
    }
  }

  if(MPTAmp<IVTAmpIn && CurrSens<One){
    AnimBatOut=true;
    AnimBatIn=false;
  }

  if(AnimSolOut==false){
    tft.drawRGBBitmap(SolarStrtX - (hX-1), SolarStrtY - hY, AnimRstH, AnimRstH_WIDTH,AnimRstH_HEIGHT);
  } 
  if(AnimIVTOutput==false){
      tft.drawRGBBitmap(LoadStrtX - hX, LoadStrtY - hY, AnimRstH, AnimRstH_WIDTH,AnimRstH_HEIGHT);
  }
  if(AnimIVTInput==false){
    tft.drawRGBBitmap(InvrtStrtX - vX, InvrtStrtY - vY, AnimRstV, AnimRstV_WIDTH,AnimRstV_HEIGHT);  
  }
  if(AnimChargIVT==false){  
    tft.drawRGBBitmap(ChrgInvrtStrtX - hX, ChrgInvrtStrtY - hY, AnimRstH, AnimRstH_WIDTH,AnimRstH_HEIGHT);
  }
  if(AnimChargBat==false){
    tft.drawRGBBitmap(ChrgBatStrtX - vX, ChrgBatStrtY - vY, AnimRstV, AnimRstV_WIDTH,AnimRstV_HEIGHT);
  }
  if(AnimBatIn==false){  
    tft.drawRGBBitmap(BATStrtX - hX, BATStrtY - hY, AnimRstH, AnimRstH_WIDTH,AnimRstH_HEIGHT);
  }
  if(AnimBatOut==false){
    tft.drawRGBBitmap(BATFinX - (hX-1), BATStrtY - hY, AnimRstH, AnimRstH_WIDTH,AnimRstH_HEIGHT);
  }
}
void HomeAnimSwitch(){ // Menu Home Animation Switchs
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
void ValueRst(){       // Data Value Reset
  MPTVbat =0;
  MPTPpv =0;
  MPTVpv =0;
  MPTAmp =0;
  MPTkWh =0;
  MPTWmax =0;
  MPTCS =0;
  IVTVbat =0;
  IVTMode =0;
  IVTACV =0;
  IVTACI =0;
  IVTACS =0;
  CurrSens =0;
  IVTAmpIn =0;
  BATAmp =0;
  IVTWarnStrgmsg="";
  MPTfwStrg="";
  MPTkWhHist=0;
  MPTpidStrg="";
  MPTSERStrg="";
  MPTErrorStrgmsg="";
  MPTCSStrgmsg="";
  MPTkWhAyer=0;
  MPTWmaxAyer=0;
  IVTSERStrg="";
  IVTpidStrg="";
  IVTfwStrg="";
  IVTCSStrgmsg="";
  IVTARStrgmsg="";
  BatLevelPresent=0;
  RelayShow=true;
}
void DataResetMPT(){   // Data error reset data
  Touched();
  ResetMPTData=ResetLoopDefault;  // correct data received, wrong data counter reset
}   
void DataResetIVT(){   // Data error reset data 
  Touched();
  ResetIVTData=ResetLoopDefault;  // correct data received, wrong data counter reset
}
void Home(){           // Menu Home
  ReadSerial2();
  if(Origin=="MPTnodata"){                                                  // turn on counter wrong data counter reset
    ResetMPTData-=1;
    if(ResetMPTData<1){
      DataResetMPT();  
    }
  }
  if(Origin=="MPPT"){
  NewValueSet();
    ResetMPTData=ResetLoopDefault;                                          // correct data received, wrong data counter reset
    if(label == "V" && abs(MPTVbat - NewValuemilli)>=decimTo){              // Battery Volt  mV   
      if( NewValuemilli > InverterVbatmin && NewValuemilli < InverterVbatmax ){           // Outside this range the inverter does not work
        HomePrintInt(TxtSize1,TextColorOrange,TextColorWhite, MPTVbatX, MPTVbatY, MPTVbat, NewValuemilli, One , "V");
        MPTVbat=NewValuemilli;
        HomeBatLevel(); // Calculo del % de bateri
      }
    }else if(label == "VPV" && abs(MPTVpv - NewValuemilli) >= decimTo){     //Volt PV
      if( NewValuemilli >= Zero && NewValuemilli < MPPTVmax ){
        HomePrintInt(TxtSize1,TextColorOrange,TextColorWhite, MPTVpvX, MPTVpvY, MPTVpv, NewValuemilli, Zero , "V");
        MPTVpv=NewValuemilli;
      }
    }else if(label == "PPV" && abs(MPTPpv - NewValue )>=One){               // Watt PV
      if( NewValue >= Zero && NewValue < MPPTWmax ){
        HomePrintInt(TxtSize2,TextColorOrange,TextColorWhite, MPTPpvX, MPTPpvY, MPTPpv, NewValue, Zero , "");
        MPTPpv=NewValue;
      }
    }else if(label == "I" && abs(MPTAmp - NewValuemilli)>=decuTo){          // I PV
      if( NewValuemilli >= Zero && NewValuemilli < MPPTAmax ){
        HomePrintInt(TxtSize1,TextColorOrange,TextColorWhite, MPTAmpX, MPTAmpY, MPTAmp, NewValuemilli, One , "A");
        MPTAmp=NewValuemilli;
      }
    }else if(label == "H20" && abs(MPTkWh-NewValuedeca)>=One){              // kWh
      if( NewValuedeca >= Zero && NewValuedeca < MPPTWmax*DayHr ){  // Wmax * 24h one day
        HomePrintInt(TxtSize1,TextColorOrange,TextColorWhite,MPTkWhX, MPTkWhY, MPTkWh, NewValuedeca, Zero , "Wh");
        MPTkWh=NewValuedeca;
      }
    }else if(label == "H21" && abs(MPTWmax-NewValue)>=One){                 // Wmax 
      if( NewValue >= Zero && NewValue < MPPTWmax ){
        HomePrintInt(TxtSize1,TextColorOrange,TextColorWhite,MPTWmaxX, MPTWmaxY, MPTWmax, NewValue, Zero , "Wmax");
        MPTWmax=NewValue;
      }
    }else if(label == "CS" && NewValue!= MPTCS){                            // State
      if( NewValue >= Zero && NewValue <= 5 ){
        if(NewValue==0){ // OFF
         tft.fillCircle(LedMPPTX, LedMPPTY, LedRad,ILI9341_BLACK);
        }else if(NewValue==2){ // FAULT MPPT & Inverter off hasta hacer Reset
          tft.fillCircle(LedMPPTX, LedMPPTY, LedRad,ILI9341_RED);
        }else if(NewValue==3){ // bulk
          tft.fillCircle(LedMPPTX, LedMPPTY, LedRad,ILI9341_BLUE);
        }else if(NewValue==4){ // Absorption
          tft.fillCircle(LedMPPTX, LedMPPTY, LedRad,ILI9341_YELLOW);
        }else if(NewValue==5){ // Float
          tft.fillCircle(LedMPPTX, LedMPPTY, LedRad,ILI9341_GREEN);
        }
      }
      MPTCS=NewValue;
    }
  }
  ReadSerial1();  
  if(Origin=="IVTnodata"){                                                   // turn on counter wrong data counter reset
    ResetIVTData=(ResetIVTData-One);
    if(ResetIVTData<One){
      DataResetIVT();  
    }
  }
  if(Origin=="Inverter"){
  NewValueSet();
    ResetIVTData=ResetLoopDefault;                                           // correct data received, wrong data counter reset
    if(label == "V" && abs(IVTVbat- NewValuemilli)>=decimTo){                // Battery Volt  
      if( NewValuemilli > InverterVbatmin && NewValuemilli < InverterVbatmax ){
        HomePrintInt(TxtSize1,TextColorBlue,TextColorWhite,IVTVbatX, IVTVbatY, IVTVbat, NewValuemilli, One , "V");          
        IVTVbat=NewValuemilli;
      }
    }else if(label=="MODE" && NewValue != IVTMode) {                         // Mode
      if( NewValue >= 2 && NewValue <= 5 ){
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
    }else if(label=="WARN" && NewValueStrg != IVTWarnStrgmsg ){              // Warm
      if( NewValueStrg=="1" || NewValueStrg=="2" || NewValueStrg=="32"|| NewValueStrg=="64" || NewValueStrg=="256" || NewValueStrg=="512" || NewValueStrg=="1024" || NewValueStrg=="248"){
          if(NewValueStrg=="1"){ // volta bajo
            NewValueStrg=Warm1;
            tft.fillCircle(LedIVTX, LedIVTY, LedRad,ILI9341_RED);
            HomePrintStrg(TxtSize1,TextColorWhite, TextColorRed, IVTWarnX,IVTWarnY, IVTWarnStrgmsg, NewValueStrg, "");
          }
          else if(NewValueStrg=="2"){ //Volt alto
            NewValueStrg=Warm2;
            tft.fillCircle(LedIVTX, LedIVTY, LedRad,ILI9341_RED);
            HomePrintStrg(TxtSize1,TextColorWhite, TextColorRed, IVTWarnX,IVTWarnY, IVTWarnStrgmsg ,NewValueStrg, "");
          }
          else if(NewValueStrg=="32"){ // Temperatura baja
            NewValueStrg=Warm32;
            tft.fillCircle(LedIVTX, LedIVTY, LedRad,ILI9341_RED);
            HomePrintStrg(TxtSize1,TextColorWhite, TextColorRed, IVTWarnX,IVTWarnY, IVTWarnStrgmsg ,NewValueStrg, "");
          }
          else if(NewValueStrg=="64"){ // Temperatura alta
            NewValueStrg=Warm64;
            tft.fillCircle(LedIVTX, LedIVTY, LedRad,ILI9341_RED);
            HomePrintStrg(TxtSize1,TextColorWhite, TextColorRed, IVTWarnX,IVTWarnY, IVTWarnStrgmsg, NewValueStrg, "");
          }
          else if(NewValueStrg=="256"){ // Sobrecarca se load demasiado alto
            NewValueStrg=Warm256;
            tft.fillCircle(LedIVTX, LedIVTY, LedRad,ILI9341_RED);
            HomePrintStrg(TxtSize1,TextColorWhite, TextColorRed, IVTWarnX,IVTWarnY, IVTWarnStrgmsg ,NewValueStrg, "");
          }
          else if(NewValueStrg=="512"){ // rizado de volt continuos de la batteria ruido
            NewValueStrg=Warm512;
            tft.fillCircle(LedIVTX, LedIVTY, LedRad,ILI9341_RED);
            HomePrintStrg(TxtSize1,TextColorWhite, TextColorRed, IVTWarnX,IVTWarnY, IVTWarnStrgmsg ,NewValueStrg, "");
          }
          else if(NewValueStrg=="1024"){ // AC Volt demasiado bajo
            NewValueStrg=Warm1024;
            tft.fillCircle(LedIVTX, LedIVTY, LedRad,ILI9341_RED);
            HomePrintStrg(TxtSize1,TextColorWhite, TextColorRed, IVTWarnX,IVTWarnY, IVTWarnStrgmsg, NewValueStrg, "");
          }
          else if(NewValueStrg=="2048"){ // AC volt demasiado alto
            NewValueStrg=Warm2048;
            tft.fillCircle(LedIVTX, LedIVTY, LedRad,ILI9341_RED);
            HomePrintStrg(TxtSize1,TextColorWhite, TextColorRed, IVTWarnX,IVTWarnY, IVTWarnStrgmsg ,NewValueStrg, "");
          }
      }                
      IVTWarnStrgmsg=NewValueStrg;
    }else if(label=="AC_OUT_V" && abs(IVTACV - NewValuecentesi)>=One) {      // V ac
      if( NewValuecentesi > InverterVacmin && NewValuecentesi < InverterVacmax ){
        HomePrintInt(TxtSize1,TextColorBlue,TextColorWhite,IVTACVX, IVTACVY, IVTACV, NewValuecentesi, Zero , "V");                
        IVTACV=NewValuecentesi;
      }
    }else if(label=="AC_OUT_I" && abs(IVTACI - NewValuedecim)>=decimTo){     // I ac
      if( NewValuedecim >= Zero && NewValuedecim <  InverterIbatmax ){
        HomePrintInt(TxtSize1,TextColorGreen,TextColorWhite,IVTACIX, IVTACIY,IVTACI, NewValuedecim, One , "A");   
        IVTACI=NewValuedecim;
      }
    }else if(label=="AC_OUT_S" && abs(IVTACS - NewValue)>=One){              // VA
      if( NewValue >= Zero && NewValue <  InverterWmax ){
        HomePrintInt(TxtSize2,TextColorGreen,TextColorWhite,IVTACWX, IVTACWY,IVTACS, NewValue, Zero , "");   
        IVTACS=NewValue;
      }
    }
  }
  HomeCurrentS();      // Amps measured by current sensor placed on the load
  HomeIVtAmpIn();      // Calcul deAmp input estimado del inverter
  HomeBatAmp();        // Estimated battery amps are negative for outgoing and positive for charging.
  HomeAnimAsk();       // Verify which animations should be activated
  HomeAnimSwitch();    // Activate required animations
  RelayShowStat();     // Relay State ON OFF AUTO
}
void Solar(){          // SubMenu Solar
  ReadSerial2();
  NewValueSet();
  if(label == "V"){          // Battery Volt  mV     
    MenuCL=MPTVbatCL; 
    SubMenuPrintInt(TxtSize1,TextColorWhite,TextColorBlack, MPTVbat, NewValuemilli, Two , "V");
    MPTVbat=NewValuemilli;
  } else if(label == "VPV"){          // Volt PV
    MenuCL=MPTVpvCL; 
    SubMenuPrintInt(TxtSize1,TextColorWhite,TextColorBlack, MPTVpv, NewValuemilli, Two , "V");
    MPTVpv=NewValuemilli;
  } else if(label == "PPV"){          // Watt PV
    MenuCL=MPTPpvCL;
    SubMenuPrintInt(TxtSize1,TextColorWhite,TextColorBlack, MPTPpv, NewValue, Zero , "W");
    MPTPpv=NewValue;
  } else if(label == "I"){          // I PV
    MenuCL=MPTiCL;
    SubMenuPrintInt(TxtSize1,TextColorWhite,TextColorBlack, MPTAmp, NewValuemilli, Two , "A");
    MPTAmp=NewValuemilli;
  } else if(label == "H19"){          // Historic kWh
    MenuCL=MPTkWhHistCL;
    NewValuecentesi=NewValuecentesi+MPTkWhHistAdd;
    SubMenuPrintInt(TxtSize1,TextColorWhite,TextColorBlack, MPTkWhHist, NewValuecentesi, Zero , "kWhT");
    MPTkWhHist=NewValuecentesi;
  } else if(label == "H20"){          // kWh
    MenuCL=MPTkWhCL;
    SubMenuPrintInt(TxtSize1,TextColorWhite,TextColorBlack, MPTkWh, NewValuedeca, Zero , "Wh");
    MPTkWh=NewValuedeca;
  } else if(label == "H21"){          // Wmax
    MenuCL=MPTWmaxCL;
    SubMenuPrintInt(TxtSize1,TextColorWhite,TextColorBlack, MPTWmax, NewValue, Zero , "Wmax");
    MPTWmax=NewValue;
  } else if(label == "FW"){          // Firmware version
    MenuCL=MPTfwCL;
    SubMenuPrintStrg(TxtSize1,TextColorWhite,TextColorBlack, MPTfwStrg, NewValueStrg,"FW.");
    MPTfwStrg=NewValueStrg;
  } else if(label == "PID"){          // Product ID
    MenuCL=MPTpidCL; 
    SubMenuPrintStrg(TxtSize1,TextColorWhite,TextColorBlack, MPTpidStrg, NewValueStrg,"PID.");
    MPTpidStrg=NewValueStrg;
  } else if(label == "SER#"){          // Serial number
    MenuCL=MPTserCL;
    SubMenuPrintStrg(TxtSize1,TextColorWhite,TextColorBlack, MPTSERStrg, NewValueStrg,"SN.");
    MPTSERStrg=NewValueStrg;
  } else if(label == "ERR"){          // Error
      if(NewValue==0){ // No error
        NewValueStrg=MenuSolarErr0Msg;
      }
      else if(NewValue==2){  // Bat too high
        NewValueStrg=MenuSolarErr2Msg;
      }
      else if(NewValue==17){ // Temp too high
        NewValueStrg=MenuSolarErr17Msg;
      }
      else if(NewValue==18){  // Over Current
        NewValueStrg=MenuSolarErr18Msg;
      }
      else if(NewValue==19){  // Current reverse
        NewValueStrg=MenuSolarErr19Msg;
      }
      else if(NewValue==20){  // bulk time limit      
        NewValueStrg=MenuSolarErr20Msg;
      }
      else if(NewValue==21){  // Current sensor fail
        NewValueStrg=MenuSolarErr21Msg;
      }
      else if(NewValue==26){  // Terminal OverHeated
        NewValueStrg=MenuSolarErr26Msg;
      }
      else if(NewValue==33){  // Volt too high PV
        NewValueStrg=MenuSolarErr33Msg;
      }
      else if(NewValue==34){  // Amp too high PV
        NewValueStrg=MenuSolarErr34Msg;
      }
      else if(NewValue==38){  // Excessive voltage battery                          
        NewValueStrg=MenuSolarErr38Msg;
      }
      else if(NewValue==116){ // Facturoy calibration lost  
        NewValueStrg=MenuSolarErr116Msg;
      }
      else if(NewValue==117){ // Firmware Invalid
        NewValueStrg=MenuSolarErr117Msg;
      }
      else if(NewValue==119){  // User Settings invalid
        NewValueStrg=MenuSolarErr119Msg;
      }
    MenuCL=MPTerrCL;
    SubMenuPrintStrg(TxtSize1,TextColorWhite,TextColorBlack, MPTErrorStrgmsg, NewValueStrg,"");
    MPTErrorStrgmsg=NewValueStrg;
  } else if(label == "CS"){     /// State
      if(NewValue==0){ // OFF
        NewValueStrg=MenuSolarCS0Msg;
      }
      else if(NewValue==2){ // FAULT MPPT & Inverter off hasta hacer Reset
        NewValueStrg=MenuSolarCS2Msg;
      } 
      else if(NewValue==3){ // bulk
        NewValueStrg=MenuSolarCS3Msg;
      }
      else if(NewValue==4){ // Absorption
        NewValueStrg=MenuSolarCS4Msg;;
      }
      else if(NewValue==5){ // Float
        NewValueStrg=MenuSolarCS5Msg;
      }
    MenuCL=MPTcsCL;
    SubMenuPrintStrg(TxtSize1,TextColorWhite,TextColorBlack, MPTCSStrgmsg, NewValueStrg,"");
    MPTCSStrgmsg=NewValueStrg;
  } else if(label == "H22"){    /// kWh yesterday
    MenuCL=MPTkWhAyerCL;
    SubMenuPrintInt(TxtSize1,TextColorWhite,TextColorBlack, MPTkWhAyer, NewValuedeca, Zero , "Ayer");
    MPTkWhAyer=NewValuedeca;
  } else if(label == "H23"){    /// Wmax yesterday
    MenuCL=MPTWmaxAyerCL;
    SubMenuPrintInt(TxtSize1,TextColorWhite,TextColorBlack, MPTWmaxAyer, NewValue, Zero , "Ayer");
    MPTWmaxAyer=NewValue;
  }else{}   
}
void Inverter(){       // SubMenu Inverter
  ReadSerial1();
  NewValueSet();
  if(label == "V" && abs(IVTVbat- NewValuemilli)>=centesiTo){          // Battery Volt  mV   
    MenuCL=IVTVbatCL; 
    SubMenuPrintInt(TxtSize1,TextColorWhite,TextColorBlack, IVTVbat, NewValuemilli, Two , "V");
    IVTVbat=NewValuemilli;
  } else if(label=="AR"){              /// Alarm
    MenuCL=IVTARCL;
    if(NewValue==0){      // LOW VOLTAGE){
      NewValueStrg=".";
    }else if(NewValue==1){      // LOW VOLTAGE
      NewValueStrg=MenuInverterAR1Msg;
    }else if(NewValue==2){ // HIGH VOLTAGE
      NewValueStrg=MenuInverterAR2Msg;
    }else if(NewValue==32){ // Low Temp
      NewValueStrg=MenuInverterAR32Msg;
    }else if(NewValue==64){ // High Temp
      NewValueStrg=MenuInverterAR64Msg;
    }else if(NewValue==256){ // OverLoad
      NewValueStrg=MenuInverterAR256Msg;
    }else if(NewValue==512){ // DC ripple
      NewValueStrg=MenuInverterAR512Msg;
    }else if(NewValue==1024){ // Low V AC out
      NewValueStrg=MenuInverterAR1024Msg;
    }else if(NewValue==2048){ // High V AC out
      NewValueStrg=MenuInverterAR2048Msg;
    }
    SubMenuPrintStrg(TxtSize1,TextColorWhite, TextColorBlack, IVTARStrgmsg, NewValueStrg, "");
    IVTARStrgmsg=NewValueStrg;
  } else if(label == "CS"){     /// State
    MenuCL=IVTcsCL; 
    if(NewValue==0){ // OFF
      NewValueStrg=MenuInverterCS0Msg;
    } else if(NewValue==1){ // LOW POWER
      NewValueStrg=MenuInverterCS1Msg;
    } else if(NewValue==2){ // FAULT
      NewValueStrg=MenuInverterCS3Msg;
    } else if(NewValue==9){ // INVERTER
      NewValueStrg=MenuInverterCS4Msg;
    }
    SubMenuPrintStrg(TxtSize1,TextColorWhite, TextColorBlack, IVTCSStrgmsg, NewValueStrg, "");
    IVTCSStrgmsg=NewValueStrg;
  } else if(label == "FW"){     /// Firmware
    MenuCL=IVTfwCL;    
    SubMenuPrintStrg(TxtSize1,TextColorWhite, TextColorBlack, IVTfwStrg, NewValueStrg, "FW.");
    IVTfwStrg=NewValueStrg;
  } else if(label == "PID"){    /// Product ID
    MenuCL=IVTpidCL;
    SubMenuPrintStrg(TxtSize1,TextColorWhite, TextColorBlack, IVTpidStrg, NewValueStrg, "PID.");
    IVTpidStrg=NewValueStrg;
  } else if(label == "SER#"){   /// Number Serie
    MenuCL=IVTserCL; 
    SubMenuPrintStrg(TxtSize1,TextColorWhite, TextColorBlack, IVTSERStrg, NewValueStrg, "SN.");
    IVTSERStrg=NewValueStrg;
  } else if(label=="MODE"){              /// Mode
      if(NewValue==0){      // INVERTER ON
        NewValueStrg=".";
      } else if(NewValue==2){      // INVERTER ON
        NewValueStrg=MenuInvertermode2Msg;
      } else if(NewValue==4){ // OFF
        NewValueStrg=MenuInvertermode4Msg;
      } else if(IVTMode==5){ // ECO
        NewValueStrg=MenuInvertermode5Msg;
      }   
    MenuCL=IVTmodeCL;
    SubMenuPrintStrg(TxtSize1,TextColorWhite, TextColorBlack, IVTmodeStrgmsg, NewValueStrg, "");
    IVTmodeStrgmsg=NewValueStrg;
  } else if(label == "AC_OUT_V"){      // V ac
      MenuCL=IVTACVCL; 
      SubMenuPrintInt(TxtSize1,TextColorWhite,TextColorBlack, IVTACV, NewValuecentesi, Two , "Vac");
      IVTACV=NewValuecentesi;
  } else if(label == "AC_OUT_I"){      // I ac
      MenuCL=IVTACICL; 
      SubMenuPrintInt(TxtSize1,TextColorWhite,TextColorBlack, IVTACI, NewValuedecim, Two , "Aac");
      IVTACI=NewValuedecim;
  } else if(label == "AC_OUT_S"){      // VA ac
      MenuCL=IVTACSCL;
      SubMenuPrintInt(TxtSize1,TextColorWhite,TextColorBlack, IVTACS, NewValue, Two , "VA");
      IVTACS=NewValue;
  } else if(label=="WARN"){              /// Warm
    if(NewValue==0){ 
        NewValueStrg=".";
      } else if(NewValue==1){      // LOW VOLTAGE
        NewValueStrg=MenuInverterwarn1Msg;
      } else if(NewValue==2){ // HIGH VOLTAGE
        NewValueStrg=MenuInverterwarn2Msg;
      } else if(NewValue==32){ // Low Temp
        NewValueStrg=MenuInverterwarn32Msg;
      } else if(NewValue==64){ // High Temp
        NewValueStrg=MenuInverterwarn64Msg;
      } else if(NewValue==256){ // OverLoad
        NewValueStrg=MenuInverterwarn256Msg;
      } else if(NewValue==512){ // DC ripple
        NewValueStrg=MenuInverterwarn512Msg;
      } else if(NewValue==1024){ // Low V AC out
        NewValueStrg=MenuInverterwarn1024Msg;
      } else if(NewValue==2048){ // High V AC out
        NewValueStrg=MenuInverterwarn2048Msg;
    }    
    MenuCL=IVTWarnCL;
    SubMenuPrintStrg(TxtSize1,TextColorWhite, TextColorBlack, IVTWarnStrgmsg, NewValueStrg, "");
    IVTWarnStrgmsg=NewValueStrg; 
  }
}
void Battery(){        // SubMenu Battery
  ReadSerial2();
  NewValueSet();
  if(label == "V" && abs(MPTVbat - NewValuemilli)>=centesiTo){          // Battery Volt  mV   
    MenuCL=BATmptVbatCL; 
    SubMenuPrintInt(TxtSize1,TextColorWhite,TextColorBlack, MPTVbat, NewValuemilli, Two , "Vmppt");
    MPTVbat=NewValuemilli;
  }
    tft.setFont(TxtSize1);             // display device specifications noted by user
    tft.setTextColor(TextColorBlack);
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
  NewValueSet();
  if(label == "V" && abs(IVTVbat- NewValuemilli)>=centesiTo){          // Battery Volt  mV     
    MenuCL=BATivtVbatCL; 
    SubMenuPrintInt(TxtSize1,TextColorWhite,TextColorBlack, IVTVbat, NewValuemilli, Two , "Vivrt");
    IVTVbat=NewValuemilli;
  }
}
void Charger(){        // SubMenu Charger
  ReadSerial2();
  NewValueSet();
    if(label == "V" && abs(MPTVbat - NewValuemilli)>=decimTo){          // Battery Volt  mV  
      MenuCL=CHRGMPTVbatCL; 
      if( NewValuemilli > InverterVbatmin && NewValuemilli < InverterVbatmax ){         
        SubMenuPrintInt(TxtSize1,TextColorWhite,TextColorBlack, MPTVbat, NewValuemilli, Two , "V");
        MPTVbat=NewValuemilli;
      }
    }

  CurrentSensor();
  MenuCL=CHRGCurrSensCL;
  if(abs(CurrSens - NewValue)>=decuTo){     //Charger Current Sensor 
    if( NewValue >= Zero && NewValue < ChargerImax ){
      SubMenuPrintInt(TxtSize1,TextColorWhite,TextColorBlack, CurrSens, NewValue, Two , "A");
      CurrSens=NewValue;             
    }
  }
}
void Load(){           // SubMenu Load
  ReadSerial1();
  NewValueSet();
  if(label=="AR"){                     // Alarm 
    MenuCL=IVTARCL;
    if(NewValue==0){      // LOW VOLTAGE){
      NewValueStrg=".";
    }else if(NewValue==1){      // LOW VOLTAGE
      NewValueStrg=MenuInverterAR1Msg;
    }else if(NewValue==2){ // HIGH VOLTAGE
      NewValueStrg=MenuInverterAR2Msg;
    }else if(NewValue==32){ // Low Temp
      NewValueStrg=MenuInverterAR32Msg;
    }else if(NewValue==64){ // High Temp
      NewValueStrg=MenuInverterAR64Msg;
    }else if(NewValue==256){ // OverLoad
      NewValueStrg=MenuInverterAR256Msg;
    }else if(NewValue==512){ // DC ripple
      NewValueStrg=MenuInverterAR512Msg;
    }else if(NewValue==1024){ // Low V AC out
      NewValueStrg=MenuInverterAR1024Msg;
    }else if(NewValue==2048){ // High V AC out
      NewValueStrg=MenuInverterAR2048Msg;
    }
    SubMenuPrintStrg(TxtSize1,TextColorWhite, TextColorBlack, IVTARStrgmsg, NewValueStrg, "");
    IVTARStrgmsg=NewValueStrg;
  } else if(label=="MODE"){            // Mode
    MenuCL=IVTmodeCL;
    if(NewValue==0){      // LOW VOLTAGE){
        NewValueStrg=".";
      }else if(NewValue==2){      // INVERTER ON
        NewValueStrg=MenuInvertermode2Msg;
      }else if(NewValue==4){ // OFF
        NewValueStrg=MenuInvertermode4Msg;
      }else if(NewValue==5){ // ECO
        NewValueStrg=MenuInvertermode5Msg;
    }   
    SubMenuPrintStrg(TxtSize1,TextColorWhite, TextColorBlack, IVTmodeStrgmsg, NewValueStrg, "");
    IVTmodeStrgmsg=NewValueStrg;
  } else if(label == "AC_OUT_V"){      // Vac
      MenuCL=IVTACVCL; 
      SubMenuPrintInt(TxtSize1,TextColorWhite,TextColorBlack, IVTACV, NewValuecentesi, Two , "  Vac");
      IVTACV=NewValuecentesi;
  } else if(label == "AC_OUT_I"){      // Iac
      MenuCL=IVTACICL; 
      SubMenuPrintInt(TxtSize1,TextColorWhite,TextColorBlack, IVTACI, NewValuedecim, Two , "     Aac");
      IVTACI=NewValuedecim;
  } else if(label == "AC_OUT_S"){      // VA
      MenuCL=IVTACSCL;
      SubMenuPrintInt(TxtSize1,TextColorWhite,TextColorBlack, IVTACS, NewValue, Two , "  VA");
      IVTACS=NewValue;
  } else if(label=="WARN"){            // Warm 
    if(NewValue==0){ 
        NewValueStrg=".";
      } else if(NewValue==1){      // LOW VOLTAGE
        NewValueStrg=MenuInverterwarn1Msg;
      } else if(NewValue==2){ // HIGH VOLTAGE
        NewValueStrg=MenuInverterwarn2Msg;
      } else if(NewValue==32){ // Low Temp
        NewValueStrg=MenuInverterwarn32Msg;
      } else if(NewValue==64){ // High Temp
        NewValueStrg=MenuInverterwarn64Msg;
      } else if(NewValue==256){ // OverLoad
        NewValueStrg=MenuInverterwarn256Msg;
      } else if(NewValue==512){ // DC ripple
        NewValueStrg=MenuInverterwarn512Msg;
      } else if(NewValue==1024){ // Low V AC out
        NewValueStrg=MenuInverterwarn1024Msg;
      } else if(NewValue==2048){ // High V AC out
        NewValueStrg=MenuInverterwarn2048Msg;
    }    
    MenuCL=IVTWarnCL;
    SubMenuPrintStrg(TxtSize1,TextColorWhite, TextColorBlack, IVTWarnStrgmsg, NewValueStrg, "");
    IVTWarnStrgmsg=NewValueStrg; 
  }
}
void MenuCheck(){      // Touch go to Menu
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

void setup(){
  SerialSet();
  PinSet();
  DisplaySet();
  Touched();
}

void loop(){  
  currentMillis = millis();                       // Delay start
  if (currentMillis - previousMillis >= msLoop) { // Delay required for screen and data update = msLoop
    previousMillis = currentMillis;               // Delay end & reset
     MenuCheck();                                 // Update present menu data
     TouchAsk();                                  // heck if any command on the screen was touched
     RelayAuto();                                 // check if it is necessary to activate the relay in case of low bat or high watt required
  }
}

