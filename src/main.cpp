#include "Application/Application.hpp"


int main(int argc, char *argv[]) {
    Application app(argc, argv);

    return QApplication::exec();
}
