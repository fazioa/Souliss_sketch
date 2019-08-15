#include <Arduino.h>
#include "constants.h"
#include "display.h"
#include "language.h"
#include "ntp.h"
#include "Ucglib.h"
#include "menu.h"
#include "preferences.h"

int ibaseH = 0;
boolean bTopicsPageEnabled = false;
String strVoid = "    "; //4 cifre
int iPortion = 0;
int integerPartNumber;
String sTemp;
int iLenght;


int getHeightPortion(Ucglib_ILI9341_18x240x320_HWSPI *ucg) {
  return ucg->getHeight() / 3 ;
}

void printNumber(Ucglib_ILI9341_18x240x320_HWSPI ucg, float fVal, String sUnity, String sText) {
  
  if (fVal == 0) {
    ucg.print("----"); //4 caratteri
  } else {
    integerPartNumber = (int) fVal;
    sTemp = integerPartNumber;
    iLenght = 4 - sTemp.length();
    sTemp = "";

    if (fVal - integerPartNumber > 0) {
      //aggiunge gli spazi necessari per occupare sempre lo spazion di 4 caratteri
      for (int i = 2; i < iLenght; i++) //considero lo spazio necessario al punto ed al carattere decimale
      {
        sTemp = sTemp + " ";
      }
      //se esiste la parte decimale, ne stampa una sola cifra
      ucg.print(sTemp);
      ucg.print(arrotonda(fVal), 1);

    } else {
      //aggiunge gli spazi necessari per occupare sempre lo spazion di 4 caratteri
      for (int i = 0; i < iLenght; i++)
      {
        sTemp = sTemp + " ";
      }
      sTemp = sTemp + integerPartNumber;
      // omette la parte decimale se è uguale a zero
      ucg.print(sTemp);
    }
  }
  ucg.setFont(FONT_BIG_MIN_50_PERCENT);
  ucg.print(sUnity);
  ucg.print(sText);
}

float fPrecC1;
void display_print_C1(Ucglib_ILI9341_18x240x320_HWSPI ucg, float fVal) {
  C1_COLOR();
  ucg.setFontMode(UCG_FONT_MODE_SOLID);
  ucg.setFont(FONT_BIG);

  ucg.setFontPosBaseline();
  //calcola ingombro testo
  String str = "    "; //4 cifre
  const char *c = str.c_str();
  iPortion = getHeightPortion(&ucg);
  ibaseH = ucg.getHeight()  - iPortion * 2 - iPortion / 2 + ucg.getFontAscent() / 2;
  ucg.setPrintPos(5 , ibaseH);
  //  ucg.clearScreen();

  printNumber(ucg, fVal, C1_AREA_UNITY, C1_AREA_TEXT);
  // Serial.print("C1_AREA: "); Serial.print(fVal); Serial.print(C1_AREA_UNITY); Serial.println(C1_AREA_TEXT);
}

float fPrecC2;
void display_print_C2(Ucglib_ILI9341_18x240x320_HWSPI ucg, float fVal) {
  C2_COLOR();
  ucg.setFontMode(UCG_FONT_MODE_SOLID);
  ucg.setFont(FONT_BIG);
  ucg.setFontPosBaseline();
  //calcola ingombro testo
  const char *c = strVoid.c_str();

  iPortion = getHeightPortion(&ucg);
  ibaseH = ucg.getHeight()  - iPortion - iPortion / 2 + ucg.getFontAscent() / 2;
  ucg.setPrintPos(5 , ibaseH);
  //   ucg.clearScreen();

  printNumber(ucg, fVal, C2_AREA_UNITY, C2_AREA_TEXT);

  //  Serial.print("C2_AREA: "); Serial.print(fVal); Serial.print(C2_AREA_UNITY); Serial.println(C2_AREA_TEXT);
}


float fPrecC3;
void display_print_C3(Ucglib_ILI9341_18x240x320_HWSPI ucg, float fVal) {
  C3_COLOR();
  ucg.setFontMode(UCG_FONT_MODE_SOLID);
  ucg.setFont(FONT_BIG);

  ucg.setFontPosBaseline();
  //calcola ingombro testo
  const char *c = strVoid.c_str();

  iPortion = getHeightPortion(&ucg);
  ibaseH = ucg.getHeight()  - iPortion / 2 + ucg.getFontAscent() / 2;
  ucg.setPrintPos(5 , ibaseH);
  //   ucg.clearScreen();

  printNumber(ucg, fVal, C3_AREA_UNITY, C3_AREA_TEXT);

  //  Serial.print("C3_AREA: "); Serial.print(fVal); Serial.print(C3_AREA_UNITY); Serial.println(C3_AREA_TEXT);
}
float fPrecC4;
void display_print_C4(Ucglib_ILI9341_18x240x320_HWSPI ucg, float fVal) {
  C4_COLOR();
  ucg.setFontMode(UCG_FONT_MODE_SOLID);
  ucg.setFont(FONT_BIG);

  ucg.setFontPosBaseline();
  //calcola ingombro testo
  String str = "    "; //4 cifre
  const char *c = str.c_str();
  iPortion = getHeightPortion(&ucg);
  ibaseH = ucg.getHeight()  - iPortion * 2 - iPortion / 2 + ucg.getFontAscent() / 2;
  ucg.setPrintPos(5 , ibaseH);
  //  ucg.clearScreen();

  printNumber(ucg, fVal, C4_AREA_UNITY, C4_AREA_TEXT);

  // Serial.print("C4_AREA: "); Serial.print(fVal); Serial.print(C4_AREA_UNITY); Serial.println(C4_AREA_TEXT);
}

float fPrecC5;
void display_print_C5(Ucglib_ILI9341_18x240x320_HWSPI ucg, float fVal) {
  C5_COLOR();
  ucg.setFontMode(UCG_FONT_MODE_SOLID);
  ucg.setFont(FONT_BIG);

  ucg.setFontPosBaseline();
  //calcola ingombro testo
  const char *c = strVoid.c_str();

  iPortion = getHeightPortion(&ucg);
  ibaseH = ucg.getHeight()  - iPortion - iPortion / 2 + ucg.getFontAscent() / 2;
  ucg.setPrintPos(5 , ibaseH);
  //   ucg.clearScreen();

  printNumber(ucg, fVal, C5_AREA_UNITY, C5_AREA_TEXT);

  //  Serial.print("C5_AREA: "); Serial.print(fVal); Serial.print(C5_AREA_UNITY); Serial.println(C5_AREA_TEXT);
}

float fPrecC6;
void display_print_C6(Ucglib_ILI9341_18x240x320_HWSPI ucg, float fVal) {
  C6_COLOR();
  ucg.setFontMode(UCG_FONT_MODE_SOLID);
  ucg.setFont(FONT_BIG);

  ucg.setFontPosBaseline();
  //calcola ingombro testo
  const char *c = strVoid.c_str();

  iPortion = getHeightPortion(&ucg);
  ibaseH = ucg.getHeight()  - iPortion / 2 + ucg.getFontAscent() / 2;
  ucg.setPrintPos(5 , ibaseH);
  //   ucg.clearScreen();


  printNumber(ucg, fVal, C6_AREA_UNITY, C6_AREA_TEXT);

  // Serial.print("C6_AREA: "); Serial.print(fVal); Serial.print(C6_AREA_UNITY); Serial.println(C6_AREA_TEXT);
}

float fC1_Value, fC2_Value, fC3_Value, fC4_Value, fC5_Value, fC6_Value;
float fPrecC1_Value, fPrecC2_Value, fPrecC3_Value, fPrecC4_Value, fPrecC5_Value, fPrecC6_Value;


void displayTopics(Ucglib_ILI9341_18x240x320_HWSPI ucg, float fC1_Value, float fC2_Value, float fC3_Value) {
  setOnetime_clear_SetpointPage();
  if ( getUIChanged()) {
    ucg.clearScreen();
    #ifdef DEBUG_DEV
      Serial.println("TOPICS PAGE 1 - ClearScreen");
    #endif
    display_print_C1(ucg, fC1_Value);
    display_print_C2(ucg, fC2_Value);
    display_print_C3(ucg, fC3_Value);
    resetUIChanged();
  } else {
    if (fPrecC1_Value != fC1_Value) display_print_C1(ucg, fC1_Value);
    if (fPrecC2_Value != fC2_Value) display_print_C2(ucg, fC2_Value);
    if (fPrecC3_Value != fC3_Value) display_print_C3(ucg, fC3_Value);
  }


  fPrecC1_Value = fC1_Value;
  fPrecC2_Value = fC2_Value;
  fPrecC3_Value = fC3_Value;
}
void displayTopicsPage2(Ucglib_ILI9341_18x240x320_HWSPI ucg, float fC4_Value, float fC5_Value, float fC6_Value) {
  setOnetime_clear_SetpointPage();
  if ( getUIChanged()) {
    ucg.clearScreen();
    #ifdef DEBUG_DEV
      Serial.println("TOPICS PAGE 2 - ClearScreen");
    #endif
    display_print_C4(ucg, fC4_Value);
    display_print_C5(ucg, fC5_Value);
    display_print_C6(ucg, fC6_Value);
    resetUIChanged();
  } else {
    if (fPrecC4_Value != fC4_Value) display_print_C4(ucg, fC4_Value);
    if (fPrecC5_Value != fC5_Value) display_print_C5(ucg, fC5_Value);
    if (fPrecC6_Value != fC6_Value) display_print_C6(ucg, fC6_Value);
  }

  fPrecC4_Value = fC4_Value;
  fPrecC5_Value = fC5_Value;
  fPrecC6_Value = fC6_Value;
}

void displayTopicsHomePageLayout2(Ucglib_ILI9341_18x240x320_HWSPI ucg, float fC1_Value, float fC2_Value, float fC3_Value, float fC4_Value, float fC5_Value, float fC6_Value) {

  //Serial.println("displayTopics Home Page Layout2");
          
          ucg.setFontMode(UCG_FONT_MODE_SOLID);
          ucg.setFont(FONT_SMALL);
          ucg.setFontPosTop();
   
   if (fPrecC1_Value != fC1_Value) 
  {     
          
          ucg.setFont(ucg_font_helvB12_hf);
          C1_AREA_TEXT_COLOR_LAYOUT2();
          ucg.setPrintPos(175, 25);
          ucg.print(C1_AREA_TEXT);
          ucg.setFont(ucg_font_helvB14_hf);
          ucg.setPrintPos(195, 42);
          ucg.setColor(0,0,0); 
          ucg.print("          "); //clear old value
          ucg.setPrintPos(195, 42);
          C1_COLOR();
          ucg.print(fC1_Value);
          if (C1_AREA_UNITY == "°") {
            ucg.print((char)176);
          }
          else {
            ucg.print(C1_AREA_UNITY);
          }
          fPrecC1_Value = fC1_Value;
 }     
 if (fPrecC2_Value != fC2_Value)          
  { 
          ucg.setFont(ucg_font_helvB12_hf);
          C2_AREA_TEXT_COLOR_LAYOUT2();
          ucg.setPrintPos(200, 63);
          ucg.print(C2_AREA_TEXT);
          ucg.setFont(ucg_font_helvB14_hf);
          ucg.setPrintPos(210, 82);
          ucg.setColor(0,0,0); 
          ucg.print("          "); //clear old value
          ucg.setPrintPos(210, 82);
          C2_COLOR();
          ucg.print(fC2_Value);
          if (C2_AREA_UNITY == "°") {
            ucg.print((char)176);
          }
          else {
            ucg.print(C2_AREA_UNITY);
          }
          fPrecC2_Value = fC2_Value;
  }
  if (fPrecC3_Value != fC3_Value)          
  {     
          ucg.setFont(ucg_font_helvB12_hf);
          C3_AREA_TEXT_COLOR_LAYOUT2();
          ucg.setPrintPos(210, 102);
          ucg.print(C3_AREA_TEXT);
          ucg.setFont(ucg_font_helvB14_hf);
          ucg.setPrintPos(215, 120);
          ucg.setColor(0,0,0); 
          ucg.print("          "); //clear old value
          ucg.setPrintPos(215, 120);
          C3_COLOR();
          ucg.print(fC3_Value);
          if (C3_AREA_UNITY == "°") {
            ucg.print((char)176);
          }
          else {
            ucg.print(C3_AREA_UNITY);
          }
          fPrecC3_Value = fC3_Value;
  }
  
if (TOPICSPAGESNUMBER == 2) {

  if(fPrecC4_Value != fC4_Value)          
  { 
                ucg.setFont(ucg_font_helvB12_hf);
                C4_AREA_TEXT_COLOR_LAYOUT2();
                ucg.setPrintPos(205, 138);
                ucg.print(C4_AREA_TEXT);
                ucg.setFont(ucg_font_helvB14_hf);
                ucg.setPrintPos(205, 155);
                ucg.setColor(0,0,0); 
                ucg.print("          "); //clear old value
                ucg.setPrintPos(205, 155);
                C4_COLOR();
                ucg.print(fC4_Value);
                if (C4_AREA_UNITY == "°") {
                  ucg.print((char)176);
                }
                else {
                  ucg.print(C4_AREA_UNITY);
                }
                fPrecC4_Value = fC4_Value;
  }
  if (fPrecC5_Value != fC5_Value)          
  {              
                ucg.setFont(ucg_font_helvB12_hf);
                C5_AREA_TEXT_COLOR_LAYOUT2();
                ucg.setPrintPos(193, 173);
                ucg.print(C5_AREA_TEXT);
                ucg.setFont(ucg_font_helvB14_hf);
                ucg.setPrintPos(188, 190);
                ucg.setColor(0,0,0); 
                ucg.print("          "); //clear old value
                ucg.setPrintPos(188, 190);
                C5_COLOR();
                ucg.print(fC5_Value);
                if (C5_AREA_UNITY == "°") {
                  ucg.print((char)176);
                }
                else {
                  ucg.print(C5_AREA_UNITY);
                }
                fPrecC5_Value = fC5_Value;
  }
  if (fPrecC6_Value != fC6_Value)          
  {              
                ucg.setFont(ucg_font_helvB12_hf);
                C6_AREA_TEXT_COLOR_LAYOUT2();
                ucg.setPrintPos(170, 208);
                ucg.print(C6_AREA_TEXT);
                ucg.setFont(ucg_font_helvB14_hf);
                ucg.setPrintPos(160, 225);
                ucg.setColor(0,0,0); 
                ucg.print("          "); //clear old value
                ucg.setPrintPos(160, 225);
                C6_COLOR();
                ucg.print(fC6_Value);
                if (C6_AREA_UNITY == "°") {
                  ucg.print((char)176);
                }
                else {
                  ucg.print(C6_AREA_UNITY);
                }
                fPrecC6_Value = fC6_Value;
  }
 }
          
}

void alwaysdisplayTopicsHomePageLayout2(Ucglib_ILI9341_18x240x320_HWSPI ucg, float fC1_Value, float fC2_Value, float fC3_Value, float fC4_Value, float fC5_Value, float fC6_Value) {

  //Serial.println("displayTopics Home Page Layout2");
          
          ucg.setFontMode(UCG_FONT_MODE_SOLID);
          ucg.setFont(FONT_SMALL);
          ucg.setFontPosTop();
          ucg.setFont(ucg_font_helvB12_hf);
          C1_AREA_TEXT_COLOR_LAYOUT2();
          ucg.setPrintPos(175, 25);
          ucg.print(C1_AREA_TEXT);
          ucg.setFont(ucg_font_helvB14_hf);
          ucg.setPrintPos(195, 42);
          ucg.setColor(0,0,0); 
          ucg.print("          "); //clear old value
          ucg.setPrintPos(195, 42);
          C1_COLOR();
          ucg.print(fC1_Value);
          if (C1_AREA_UNITY == "°") {
            ucg.print((char)176);
          }
          else {
            ucg.print(C1_AREA_UNITY);
          }

          ucg.setFont(ucg_font_helvB12_hf);
          C2_AREA_TEXT_COLOR_LAYOUT2();
          ucg.setPrintPos(200, 63);
          ucg.print(C2_AREA_TEXT);
          ucg.setFont(ucg_font_helvB14_hf);
          ucg.setPrintPos(210, 82);
          ucg.setColor(0,0,0); 
          ucg.print("          "); //clear old value
          ucg.setPrintPos(210, 82);
          C2_COLOR();
          ucg.print(fC2_Value);
          if (C2_AREA_UNITY == "°") {
            ucg.print((char)176);
          }
          else {
            ucg.print(C2_AREA_UNITY);
          }

          ucg.setFont(ucg_font_helvB12_hf);
          C3_AREA_TEXT_COLOR_LAYOUT2();
          ucg.setPrintPos(210, 102);
          ucg.print(C3_AREA_TEXT);
          ucg.setFont(ucg_font_helvB14_hf);
          ucg.setPrintPos(215, 120);
          ucg.setColor(0,0,0); 
          ucg.print("          "); //clear old value
          ucg.setPrintPos(215, 120);
          C3_COLOR();
          ucg.print(fC3_Value);
          if (C3_AREA_UNITY == "°") {
            ucg.print((char)176);
          }
          else {
            ucg.print(C3_AREA_UNITY);
          }
  
if (TOPICSPAGESNUMBER == 2) {

                ucg.setFont(ucg_font_helvB12_hf);
                C4_AREA_TEXT_COLOR_LAYOUT2();
                ucg.setPrintPos(205, 138);
                ucg.print(C4_AREA_TEXT);
                ucg.setFont(ucg_font_helvB14_hf);
                ucg.setPrintPos(205, 155);
                ucg.setColor(0,0,0); 
                ucg.print("          "); //clear old value
                ucg.setPrintPos(205, 155);
                C4_COLOR();
                ucg.print(fC4_Value);
                if (C4_AREA_UNITY == "°") {
                  ucg.print((char)176);
                }
                else {
                  ucg.print(C4_AREA_UNITY);
                }

                ucg.setFont(ucg_font_helvB12_hf);
                C5_AREA_TEXT_COLOR_LAYOUT2();
                ucg.setPrintPos(193, 173);
                ucg.print(C5_AREA_TEXT);
                ucg.setFont(ucg_font_helvB14_hf);
                ucg.setPrintPos(188, 190);
                ucg.setColor(0,0,0); 
                ucg.print("          "); //clear old value
                ucg.setPrintPos(188, 190);
                C5_COLOR();
                ucg.print(fC5_Value);
                if (C5_AREA_UNITY == "°") {
                  ucg.print((char)176);
                }
                else {
                  ucg.print(C5_AREA_UNITY);
                }

                ucg.setFont(ucg_font_helvB12_hf);
                C6_AREA_TEXT_COLOR_LAYOUT2();
                ucg.setPrintPos(170, 208);
                ucg.print(C6_AREA_TEXT);
                ucg.setFont(ucg_font_helvB14_hf);
                ucg.setPrintPos(160, 225);
                ucg.setColor(0,0,0); 
                ucg.print("          "); //clear old value
                ucg.setPrintPos(160, 225);
                C6_COLOR();
                ucg.print(fC6_Value);
                if (C6_AREA_UNITY == "°") {
                  ucg.print((char)176);
                }
                else {
                  ucg.print(C6_AREA_UNITY);
                }
 }
          
}

boolean getTopicsPageEnabled() {
  return bTopicsPageEnabled;
}

void setTopicsPageEnabled() {
  bTopicsPageEnabled = true;
}

void resetTopicsPageEnabled() {
  bTopicsPageEnabled = false;
}

