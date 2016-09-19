#ifndef INCL_PMENUNODEH
#define INCL_PMENUNODEH
class PMenuNode
{
  public:
  const char* label;
  unsigned int cmd;
  unsigned int arg;
  
  public:
  PMenuNode(const char*, unsigned int, unsigned int);
  //const void render() const;
  const char* getLabel() const;
  const unsigned getCmd() const;
  const unsigned getArg() const;
};

#endif // INCL_PMENUNODEH
