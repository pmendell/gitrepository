
#include <SPI.h>
#include "dispatch.h"

Dispatch::Dispatch()
{
  cmd = 0;
  arg = 0;
  inputVar = 0;
  value = 0;
  inputMode = false;
  //name = "def1";
}

void Dispatch::setCommand(unsigned _cmd, unsigned _arg)
{
  cmd = _cmd;
  arg = _arg;
  //name = "def2";
}
void Dispatch::setInputVar(unsigned v)
{
  inputVar = v;
}
/*
void Dispatch::setValue(unsigned long v)
{
  value = v;
}
*/
void Dispatch::clearCommand()
{
  cmd = 0;
  arg = 0;
  inputVar = 0;

}
void Dispatch::setInputMode(boolean b)
{
  inputMode = b;
}

unsigned Dispatch::getCmd()
{
  return cmd;
}
unsigned Dispatch::getArg()
{
  return arg;
}
boolean Dispatch::isInputMode()
{
  return inputMode;
}
unsigned Dispatch::getInputVar()
{
  return inputVar;
}

void Dispatch::debug(const char* s)
{
  char buf[64];
  sprintf(buf, "%s Disp: %02x  %02x  %02x %lu", s, cmd, arg, inputVar, value );
  Serial.println( buf );
}


