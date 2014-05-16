#include "ram.h"

struct config
{
  std::string biosfn;
};

config parseconfig(char* cfgfn)
{
  config res;
  FILE* fp = fopen(cfgfn,"r");
  while(!feof(fp))
  {
    std::string property;
    int c = fgetc(fp);
    while(c != '=')
    {
      if(c == EOF)
      {
        fclose(fp);
        return res;
      }
      if(c != ' ' && c != '\t' && c != '\r' && c != '\n')
      {
        property.push_back(c);
      }
      
      c = fgetc(fp);
    }
    std::string value;
    c = fgetc(fp);
    while(c != ';')
    {
      if(c == EOF)
      {
        fclose(fp);
        return res;
      }
      if(c != ' ' && c != '\t' && c != '\r' && c != '\n')
      {
        value.push_back(c);
      }
      
      c = fgetc(fp);
    }
    
    if(property == "bios")
    {
      res.biosfn = value;
    }
  }
  
  fclose(fp);
  
  return res;
}

int main(int ac, char** av)
{
  printf("PCEmu v1-prealpha\n");
  
  if(ac < 2) return 1;
  
  config cfg = parseconfig(av[1]);
  
  FILE* biosfp = fopen(cfg.biosfn.c_str(),"rb");
  
  fseek(biosfp,0,SEEK_END);
  long size = ftell(biosfp);
  
  fseek(biosfp,0,SEEK_SET);
  
  fread(RAM::RAM + (0x100000 - size),1,size,biosfp);
  
  CPU808X::init();
  
  for(;;)
  {
    CPU808X::tick();
  }
}