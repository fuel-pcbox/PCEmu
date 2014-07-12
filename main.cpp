#include "ram.h"
#include "io.h"
#include "pit.h"
#include "pic.h"

#include "interface.h"
#include "mda.h"

struct config
{
    std::string biosfn;
    std::string isa1;
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
        if(property == "isa1")
        {
            res.isa1 = value;
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
    
    fclose(biosfp);
    
    if(cfg.isa1 == "mda")
    {
        FILE* romfp = fopen("mda.rom","rb");
        
        fread(MDA::rom,1,0x2000,romfp);
        
        fclose(romfp);
    }
    
    for(int i = 0xA0000;i<0xBFFFF;i++)
    {
        RAM::RAM[i] = 0;
    }

    INTERFACE::init();
    
    MDA::init();

    CPU808X::init();
    PIC::init();
    PIT::init();

    bool quit;

    SDL_Event e;

    while(!quit)
    {
        CPU808X::tick();
        PIT::tick();
        
        for(int i = 0;i<4;i++) MDA::tick();
        
        while(SDL_PollEvent(&e))
        {
            if(e.type == SDL_QUIT) quit = true;
        }
    }
    
    SDL_Quit();
}