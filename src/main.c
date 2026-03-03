#include "main.h"
#include "test_containers.h"

int main() {
    test_containers();

    engineInitialization();
    engineRun();
    engineShutdown();
}
