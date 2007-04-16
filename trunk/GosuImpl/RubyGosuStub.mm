#include <Gosu/Directories.hpp>
#include <Gosu/Utility.hpp>
#include <unistd.h>
#include <ruby.h>

int main()
{
    std::string resourcePath = Gosu::narrow(Gosu::resourcePrefix());
    std::string mainPath = resourcePath + "Main.rb";
    char* argv[] = { "ruby", const_cast<char*>(mainPath.c_str()) };
    chdir(resourcePath.c_str());

    ruby_init();    
    ruby_options(2, argv);
    ruby_run();
    return 0;
}
