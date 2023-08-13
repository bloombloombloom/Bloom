#include <string>
#include <vector>

#include "Application.hpp"

int main(int argc, char* argv[]) {
    auto application = Application(std::vector<std::string>(argv, argv + argc));
    return application.run();
}
