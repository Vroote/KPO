#include <iostream>

#include "Di_container.h"
#include "app.h"

int main() {
    auto injector = makeInjector(std::cin, std::cout);
    auto app = injector.create<ConsoleApp>();
    app.run();
    return 0;
}
