#ifndef ARTCHITECT_WORKSPACE_HPP
#define ARTCHITECT_WORKSPACE_HPP

#include <iostream>
#include <memory>

#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QMenu>
#include <QScrollArea>
#include <QLineEdit>

#include "ElementButton.hpp"
#include "CodeGenerator/Class.hpp"


class Workspace {
public:
    explicit Workspace(std::shared_ptr<QGridLayout>& parent_layout) {
        initScrollArea();
        initLinesPanel();
        initContent();
        initWorkspace();
        scroll_area.setWidget(&workspace_widget);

        addFileName(parent_layout);
        addEnding();

        parent_layout->addWidget(&scroll_area, 1, 1);
    }

    void addElement(const std::string& type, const QString& text = "", int parent_index = -1, ElementButton<Workspace>* parent = nullptr) {
        auto* element = new ElementButton<Workspace>(this, type);
        auto* font = new QFont("Arial", 14);
        element->setName(text);
        element->setFont(*font);
        element->setFixedWidth(element->sizeHint().width());

        std::string structs[4] = {"class", "structure", "component", "module"};
        if ((std::find(std::begin(structs), std::end(structs), element->getType()) != std::end(structs)) && line_counter != 0) {
            auto* layout_space_label = new QLabel();
            layout_space_label->setFont(*font); layout_space_label->setFixedHeight(element->sizeHint().height());
            content_layout.insertWidget(line_counter, layout_space_label);
            addLineNum(font);
            line_counter++;
        }
        if (element->getType() == "member") {
            auto* member_layout = new QGridLayout();
            auto* member_spacer = new QWidget();
            element->setMemberLayout(member_layout);
            element->setMemberParent(parent);
            member_spacer->setFixedWidth(40);
            member_layout->addWidget(member_spacer, 0, 0);
            member_layout->addWidget(element, 0, 1);
            member_layout->setColumnStretch(1, 1);
            member_layout->setAlignment(Qt::AlignLeft);
            content_layout.insertLayout(parent_index+1, member_layout);
        } else {
            content_layout.insertWidget(line_counter, element);
        }
        addLineNum(font);
        line_counter++;
    }

    QVBoxLayout& getContentLayout() {
        return content_layout;
    }
    int& getElementsAmount() {
        return line_counter;
    }
    [[nodiscard]] unsigned short getSelectMode() const {
        return select_mode;
    }

    void setSelectMode(unsigned short mode_id) {
        select_mode = mode_id;
    }
    static void selectElement(QWidget* element_widget) {
        if (element_widget->styleSheet().toStdString().empty()) {
            element_widget->setStyleSheet("background-color: blue");
        } else {
            element_widget->setStyleSheet("");
        }
    }
    void removeInvisibleElement() {
        if (select_mode != 0) {
            content_layout.removeWidget(content_layout.itemAt(line_counter)->widget());
            line_counter--;
            return;
        }
        for (int i = 0; i < line_counter; i++) {
            QLayoutItem* item = content_layout.itemAt(i);
            QWidget* widget = item->widget();
            if (item->controlTypes() != QSizePolicy::Label && item->controlTypes() != QSizePolicy::PushButton) {
                widget = item->layout()->itemAt(1)->widget();
            }
            if (!widget->isVisible()) {
                auto* btn = reinterpret_cast<ElementButton<Workspace>*>(widget);
                content_layout.removeWidget(widget);
                QLayoutItem* item_line = lines_panel.itemAt(line_counter-1);
                QWidget* item_widget = item_line->widget();
                auto* label = qobject_cast<QLabel*>(item_widget);
                lines_panel.removeWidget(label);
                delete label;
                line_counter--;
                if (btn->getType() != "member" && i != 0) {
                    QLayoutItem* item_line2 = lines_panel.itemAt(line_counter-1);
                    QWidget* item_widget2 = item_line2->widget();
                    auto* label2 = qobject_cast<QLabel*>(item_widget2);
                    lines_panel.removeWidget(label2);
                    delete label2;

                    QLayoutItem* empty_item = content_layout.itemAt(i-1);
                    QWidget* empty_item_widget = empty_item->widget();
                    auto* empty_label = qobject_cast<QLabel*>(empty_item_widget);
                    content_layout.removeWidget(empty_label);
                    delete empty_label;
                    line_counter--;

                    int j = 0;
                    while (j < line_counter) {
                        QLayoutItem* item_j = content_layout.itemAt(j);
                        if (item_j->controlTypes() != QSizePolicy::Label && item_j->controlTypes() != QSizePolicy::PushButton) {
                            widget = item_j->layout()->itemAt(1)->widget();
                            auto* btn_j = reinterpret_cast<ElementButton<Workspace>*>(widget);
                            if (btn_j->getMemberParent()->text() == btn->text()) {
                                QLayoutItem* member_number_item = lines_panel.itemAt(line_counter-1);
                                QWidget* member_number_widget = member_number_item->widget();
                                auto* member_number = qobject_cast<QLabel*>(member_number_widget);
                                lines_panel.removeWidget(member_number);
                                delete member_number;

                                QLayoutItem* member_item = content_layout.itemAt(j);
                                QWidget* member_item_widget = member_item->widget();
                                member_item_widget = member_item->layout()->itemAt(1)->widget();
                                delete member_item_widget;
                                line_counter--;
                            }
                        }
                        j++;
                    }
                }
                break;
            }
        }
    }
private:
    QPushButton                  button_add;
    std::vector<Element>         code_elements;
    QVBoxLayout                  content_layout;
    int                          line_counter = 0;
    QVBoxLayout                  lines_panel;
    QScrollArea                  scroll_area;
    unsigned short               select_mode = 0;
    std::shared_ptr<QGridLayout> workspace_layout;
    QWidget                      workspace_widget;

    void addLineNum(QFont* font) {
        auto* line_num = new QLabel;
        line_num->setStyleSheet(QString("color: rgb(59, 86, 96);"));
        line_num->setText(std::to_string(line_counter+1).c_str());
        line_num->setFont(*font);
        line_num->setMargin(5);
        line_num->setFixedWidth(line_num->sizeHint().width());

        lines_panel.insertWidget(line_counter, line_num);
        lines_panel.setAlignment(line_num, Qt::AlignRight);
    }

    static void addFileName(std::shared_ptr<QGridLayout>& parent_layout) {
        auto* label = new QLabel;
        auto* font = new QFont("Arial", 18);
        label->setStyleSheet(QString("background-color: rgb(30, 31, 34); color: white; border: 1px solid black;"));
        label->setText("Filename");
        label->setFont(*font);
        parent_layout->addWidget(label, 0, 1);
    }

    void addEnding() {
        auto* spacer = new QSpacerItem(20, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
        button_add.setText("+");
        button_add.setFixedSize(30, 30);
        QObject::connect(&button_add, &QPushButton::clicked, [&]() {
            QMenu menu;
            QAction* action_class = menu.addAction("Add class");
            QAction* action_struct = menu.addAction("Add structure");
            QAction* action_component = menu.addAction("Add component");
            QAction* action_module = menu.addAction("Add module");

            QString styleSheet = "QMenu { font-size: 14px; background-color: rgb(43, 45, 48); border: 1px solid black; }"
                                 "QMenu::item:selected { background-color: rgb(46, 67, 110); }";
            menu.setStyleSheet(styleSheet);

            QPoint pos = button_add.mapToGlobal(button_add.rect().topRight());

            if (select_mode == 0) {
                QAction* selectedAction = menu.exec(pos);
                if (selectedAction == action_class) {
                    enterName("class");
                }
                else if (selectedAction == action_component) {
                    enterName("component");
                }
                else if (selectedAction == action_module) {
                    enterName("module");
                }
                else if (selectedAction == action_struct) {
                    enterName("structure");
                }
            }
        });
        content_layout.addWidget(&button_add); // add btn
        for (int i = 0; i < 8; ++i) { // add 7 empty lines to the end
            auto* label = new QLabel;
            auto* font = new QFont("Arial", 14);
            label->setFont(*font);
            content_layout.addWidget(label);
        }
        lines_panel.addItem(spacer);
        content_layout.addItem(spacer); // fill the last line
    }

    void enterName(const std::string& type) {
        auto* dialog = new QDialog();
        auto* line_edit = new QLineEdit();
        auto* ok_button = new QPushButton("OK");
        dialog->setWindowTitle("Enter name of the element");
        dialog->setMinimumWidth(220);

        QObject::connect(ok_button, &QPushButton::clicked, [&]() {
            if (!line_edit->text().toStdString().empty()) {
                addElement(type, line_edit->text());
                dialog->accept();
            }
        });

        QVBoxLayout layout(dialog);
        layout.addWidget(line_edit);
        layout.addWidget(ok_button);

        dialog->exec();
    }

    void initScrollArea() {
        scroll_area.setStyleSheet("background-color: rgb(44, 50, 60);");
        scroll_area.setWidgetResizable(true);
    }

    void initLinesPanel() {
        lines_panel.setSpacing(0);
        lines_panel.setContentsMargins(0, 0, 0, 0);
    }

    void initContent() {
        content_layout.setSpacing(0);
        content_layout.setContentsMargins(0, 0, 0, 0);
    }

    void initWorkspace() {
        auto* separator = new QWidget();
        auto* empty = new QWidget();

        separator->setFixedWidth(1);
        separator->setStyleSheet("background-color: rgb(55, 71, 79)");
        empty->setFixedWidth(20);

        workspace_layout = std::make_shared<QGridLayout>(&workspace_widget); // CONTAINS lines and content
        workspace_layout->addLayout(&lines_panel, 0, 0);
        workspace_layout->addWidget(empty, 0, 1);
        workspace_layout->addWidget(separator, 0, 2);
        workspace_layout->addLayout(&content_layout, 0, 3);
        workspace_layout->setColumnStretch(3, 1);
    }
};

#endif //ARTCHITECT_WORKSPACE_HPP
