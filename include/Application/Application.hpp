#ifndef ARTCHITECT_APPLICATION_HPP
#define ARTCHITECT_APPLICATION_HPP

#include <iostream>
#include <memory>

#include <QApplication>

#include "UpdatableWidgetWindow.hpp"
#include "Workspace.hpp"

class Application {
public:
    Application(int argc, char *argv[]): application(argc, argv) {
        window = std::make_shared<UpdatableWidgetWindow<Application>>(this);
        window->resize(600, 400);

        side_panel.setStyleSheet("background-color: rgb(43, 45, 48); border: 1px solid black;");

        main_layout = std::make_shared<QGridLayout>(window.get());
        main_layout->addWidget(&side_panel, 0, 0, 2, 1);

        workspace = std::make_shared<Workspace>(main_layout);

        updateWidth();

        window->setLayout(main_layout.get());
        window->show();
    }

    void addLine(const std::string& line) {
        workspace->addElement("structure", QString(line.c_str()));
    }

    void updateWidth() {
        side_panel.setFixedWidth(window->width() / 7);
    }
private:
    QApplication                                        application;
    std::shared_ptr<QGridLayout>                        main_layout;
    QWidget                                             side_panel;
    std::shared_ptr<UpdatableWidgetWindow<Application>> window;
    std::shared_ptr<Workspace>                          workspace;
};

#endif //ARTCHITECT_APPLICATION_HPP
