#include <avr/io.h>
#include <avr/pgmspace.h>
#include "pmenunode.h"
#include "dispatch.h"

class PMenu
{
  private:
  int numElements;
  int menuIdx;
  const PMenuNode *nodes;
  const char* title;
  Adafruit_ST7735 *tft; 
  Dispatch *dispatcher;
  
  public:
  PMenu();
  PMenu(const PMenuNode*, int, const char*, Adafruit_ST7735*, Dispatch*);
  void PreviousNode();
  void NextNode();
  void SelectNode();
  void render();
  void renderEntry(int);
};

