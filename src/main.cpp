#include "Application.hpp"

int main(int argc, char* argv[]) {
    return Application{{argv, argv + argc}}.run();
}
