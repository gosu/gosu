#include <Gosu/Directories.hpp>
#include <Gosu/Utility.hpp>
#include <unistd.h>
#include "ruby.h"

extern "C" void Init_gosu();

int main()
{
    std::string resourcePath = Gosu::wstringToUTF8(Gosu::resourcePrefix());
    chdir(resourcePath.c_str());

    std::string mainPath = resourcePath + "Main.rb";

    char* argv[] = { "ruby", const_cast<char*>(mainPath.c_str()), 0 };
    int argc = 2;
    
    char** argvPointer = argv;
    ruby_sysinit(&argc, &argvPointer);
    { 
        RUBY_INIT_STACK; 
        ruby_init();
        Init_gosu();
        rb_eval_string("$LOADED_FEATURES << 'gosu.bundle'");
        return ruby_run_node(ruby_options(argc, argv)); 
    }
}
