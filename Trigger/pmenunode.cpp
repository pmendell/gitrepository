#include <SPI.h>
#include "pmenunode.h"


PMenuNode::PMenuNode(const char* _label, const unsigned int _cmd, const unsigned int _arg)
{
  label = _label;
  cmd = _cmd;
  arg = _arg;

}
/*
const void PMenuNode::render() const
{
  char buf[50];
  //sprintf(buf, "ME: %s %u %u", label, command, arg );
  Serial.print( buf );
}
*/
const char* PMenuNode::getLabel() const
{
  return label;
}
const unsigned PMenuNode::getCmd() const
{
  return cmd;
}
const unsigned PMenuNode::getArg() const
{
  return arg;
}

