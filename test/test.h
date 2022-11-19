#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"
#include "terminal/terminal.h"

int main(int argc, char** argv)
{
    terminal::init();
    auto context = doctest::Context(argc, argv);
    context.setOption("minimal", true);
    return context.run();
}
