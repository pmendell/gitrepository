#include <SPI.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include "pmenu.h"
#include "pmenunode.h"

#define MENU_LINESPACE 13
#define MENU_VOFFSET 8
#define MENU_PASSIVE_COLOUR ST7735_MAGENTA
#define MENU_SELECTED_COLOUR ST7735_YELLOW


PMenu::PMenu(const PMenuNode *_nodes, int length, const char* _title, Adafruit_ST7735 *_tft, Dispatch *_dispatcher )
{
  numElements = length;
  nodes = _nodes;
  title = _title;
  menuIdx = 0;
  tft = _tft;
  dispatcher = _dispatcher;


}

void PMenu::PreviousNode()
{
#ifdef _DIAG 
  Serial.print("menuIdx="); Serial.println(menuIdx);
#endif // _DIAG
  int prevIdx = menuIdx;
  if(--menuIdx < 0)
  {
#ifdef _DIAG 
    Serial.println("wrapping");
#endif // _DIAG
    menuIdx = numElements-1;
  }
  renderEntry(prevIdx);
  renderEntry(menuIdx);
#ifdef _DIAG 
  Serial.println( menuIdx );
#endif // _DIAG
}

void PMenu::NextNode()
{
#ifdef _DIAG 
  Serial.print("menuIdx="); Serial.println(menuIdx);
#endif // _DIAG
  int prevIdx = menuIdx;
   if(++menuIdx >= numElements)
  {
#ifdef _DIAG 
    Serial.println("wrapping");
#endif // _DIAG
    menuIdx = 0;
  } 
  renderEntry(prevIdx);
  renderEntry(menuIdx);
#ifdef _DIAG 
  Serial.println( menuIdx );
#endif // _DIAG
}

void PMenu::SelectNode()
{
#ifdef _DIAG 
  Serial.print("menuIdx="); Serial.println((int)this->menuIdx);
  Serial.print("SELECTING INDEX: " );
  Serial.println( (int)this->menuIdx );
#endif // _DIAG
  PMenuNode *ptr = (PMenuNode*) nodes;
  ptr+=menuIdx;
#ifdef _DIAG 
  Serial.print("FETCH:");
  Serial.println( ptr->label);
#endif // _DIAG
  dispatcher->setCommand(ptr->getCmd(), ptr->getArg());
  //dispatcher->setName("twoB");
  dispatcher->debug("PMenu::SelectNode");
  
}

void PMenu::render()
{
#ifdef _DIAG 
  Serial.println("Rendering nodes");
  Serial.println( numElements );
#endif // _DIAG
  tft->setCursor(0, MENU_VOFFSET);
  tft->setTextColor( ST7735_WHITE ); 
  tft->print(title);

  
  /*
  PMenuNode *ptr = (PMenuNode*)nodes;
  for(int i = 0; i < numElements; i++ )
  {
      ptr->render();
      Serial.println( ptr->getLabel());
      tft->setCursor(0, (i+2)*MENU_LINESPACE);
      tft->setTextColor( (i==menuIdx) ? MENU_SELECTED_COLOUR : MENU_PASSIVE_COLOUR ); 
      tft->print(ptr->getLabel());
      ptr++;
  }
  */
  for(int i = 0; i < numElements; i++ )
    renderEntry( i );
#ifdef _DIAG 
  Serial.print("menuIdx="); Serial.println(menuIdx);
  Serial.println("done");
#endif // _DIAG
}


void PMenu::renderEntry(int idx)
{
      if( idx >= 0 && idx < numElements )
      {
        PMenuNode *ptr = (PMenuNode*)nodes;
        ptr+= idx;
        tft->setCursor(0, ((idx+1)*MENU_LINESPACE)+MENU_VOFFSET);
        tft->setTextColor( (idx==menuIdx) ? MENU_SELECTED_COLOUR : MENU_PASSIVE_COLOUR ); 
        tft->print(ptr->getLabel());
      }  
}

