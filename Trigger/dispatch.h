#ifndef INCL_PDISPATCH
#define INCL_PDISPATCH
#include "pmenunode.h"

class Dispatch
{
  private:
  unsigned cmd;
  unsigned arg;
  boolean inputMode;
  int inputVar;
  unsigned long value;
  
  public:
  Dispatch();
  void setCommand( unsigned, unsigned);
  void clearCommand();
  void setInputMode(boolean);
  void setInputVar(unsigned);
  //void setValue(unsigned long);
 
  
  unsigned getCmd();
  unsigned getArg();
  boolean isInputMode();
  unsigned getInputVar();
  //unsigned long getValue();
  void debug(const char*);
  //void setName(char *);

};
#endif
