#ifndef ARTCHITECT_ELEMENTBUTTON_HPP
#define ARTCHITECT_ELEMENTBUTTON_HPP

#include <deque>
#include <variant>

#include <QPushButton>
#include <QMenu>
#include <QContextMenuEvent>
#include <iostream> // TODO: delete
#include <QDrag>
#include <QMimeData>
#include "CodeGenerator/Class.hpp"


template <typename T>
class ElementButton: public QPushButton {
public:
    explicit ElementButton(T* t, const std::string& type, QWidget* parent = nullptr): QPushButton(parent) {
        setMouseTracking(true);
        t_ptr = t;
        element_type = type;
        if (element_type == "class") {
            code = Class("");
        } else if (element_type == "structure") {
            code = Struct("");
        } else if (element_type == "component") {
            code = Component("");
        } else if (element_type == "module") {
            code = Module("");
        }
    }

    int getMembersAmount() {
        return members;
    }

    void decreaseMembersAmount() {
        members--;
    }

    std::string getType() {
        return element_type;
    }

    void setName(const QString& text) {
        std::visit([text](auto&& arg) { arg.setName(text.toStdString());}, code);
        if (element_type != "member") setText((element_type + " ").c_str() + text);
        else setText(text);
        setFixedWidth(sizeHint().width());
    }

    void setMemberLayout(QGridLayout* layout) {
        member_layout = layout;
    }

    void setMemberParent(ElementButton<T>* parent) {
        member_parent = parent;
    }

    ElementButton<T>* getMemberParent() {
        return member_parent;
    }
protected:
    void contextMenuEvent(QContextMenuEvent *event) override {
        QMenu menu(this);

        QMenu menu_inherit("Inherit", this);
        QAction* action_add_component = menu_inherit.addAction("Add component with specific members...");
        QAction* action_add_parent = menu_inherit.addAction("Inherit all members from...");
        QAction* action_add_selective_parent = menu_inherit.addAction("inherit specific members...");

        QMenu menu_types("Change type", this);
        QAction* action_change_class = menu_types.addAction("Change to Class");
        QAction* action_change_struct = menu_types.addAction("Change to Structure");
        QAction* action_change_component = menu_types.addAction("Change to Component");
        QAction* action_change_module = menu_types.addAction("Change to Module");

        if (element_type == "class") {
            menu_inherit.removeAction(action_add_component);
            menu_types.removeAction(action_change_class);
        } else if (element_type == "structure") {
            menu_inherit.removeAction(action_add_component);
            menu_types.removeAction(action_change_struct);
        } else if (element_type == "component") {
            menu_inherit.removeAction(action_add_component);
            menu_types.removeAction(action_change_component);
        } else if (element_type == "module") {
            menu_types.removeAction(action_change_module);
        }

        QAction* action_menu_inherit = menu.addMenu(&menu_inherit);
        QAction* action_menu_types =  menu.addMenu(&menu_types);
        QAction* action_rename = menu.addAction("Rename");
        QAction* action_delete = menu.addAction("Delete");
        if (element_type == "member") {
            menu.removeAction(action_menu_inherit);
            menu.removeAction(action_menu_types);
        }

        QString styleSheet = "QMenu { font-size: 14px; background-color: rgb(43, 45, 48); border: 1px solid black; }"
                             "QMenu::item:selected { background-color: rgb(46, 67, 110); }";
        menu.setStyleSheet(styleSheet);
        menu_types.setStyleSheet(styleSheet);
        menu_inherit.setStyleSheet(styleSheet);

        if (t_ptr->getSelectMode() == 0) {
            QAction* selectedAction = menu.exec(event->globalPos());
            if (selectedAction == action_add_component) {
                addComponent();
            } else if (selectedAction == action_add_parent) {
                addParent();
            } else if (selectedAction == action_add_selective_parent) {
                addSelectiveParent();
            } else if (selectedAction == action_rename) {
                renameElement();
            } else if (selectedAction == action_delete) {
                if (element_type == "member") member_parent->decreaseMembersAmount();
                setVisible(false);
                t_ptr->removeInvisibleElement();
            } else if (selectedAction == action_change_class) {
                element_type = "class";
                changeType(code, element_type);
                setName((split(text().toStdString(), ' ')[1]).c_str());
            } else if (selectedAction == action_change_struct) {
                element_type = "structure";
                changeType(code, element_type);
                setName((split(text().toStdString(), ' ')[1]).c_str());
            } else if (selectedAction == action_change_component) {
                element_type = "component";
                changeType(code, element_type);
                setName((split(text().toStdString(), ' ')[1]).c_str());
            } else if (selectedAction == action_change_module) {
                element_type = "module";
                changeType(code, element_type);
                setName((split(text().toStdString(), ' ')[1]).c_str());
            }
        }
    }

    void mouseDoubleClickEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton && element_type != "member" && t_ptr->getSelectMode() == 0) {
            auto* dialog = new QDialog();
            auto* line_edit = new QLineEdit();
            auto* ok_button = new QPushButton("OK");
            dialog->setWindowTitle("Create a new member");
            dialog->setMinimumWidth(220);

            QObject::connect(ok_button, &QPushButton::clicked, [&]() {
                if (!line_edit->text().toStdString().empty()) {
                    members++;
                    t_ptr->addElement("member", line_edit->text(), t_ptr->getContentLayout().indexOf(this), this);
                    dialog->accept();
                }
            });

            QVBoxLayout layout(dialog);
            layout.addWidget(line_edit);
            layout.addWidget(ok_button);

            dialog->exec();
        }
    }

    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            if (element_type == "member") {
                dragged_index = t_ptr->getContentLayout().indexOf(member_layout);
            } else {
                dragged_index = t_ptr->getContentLayout().indexOf(this);
            }
        }
    }
    void mouseMoveEvent(QMouseEvent *event) override {
        if (event->buttons() & Qt::LeftButton && dragged_index != -1) {
            QCursor cursor(Qt::ClosedHandCursor);
            QApplication::setOverrideCursor(cursor);
            std::string out;
            std::visit([&out](auto&& arg) { out = arg.getCode(); }, code);
            std::cout << out << " - code\n";
        }
    }
    void mouseReleaseEvent(QMouseEvent *event) override {
        QApplication::changeOverrideCursor(Qt::ArrowCursor);
        if (dragged_index != -1) {
            int dif = -1;
            int new_index = -1;
            for (int i = 0; i < t_ptr->getElementsAmount(); i++) {
                auto* item = t_ptr->getContentLayout().itemAt(i);
                QWidget* widget = item->widget();

                if (item->controlTypes() == QSizePolicy::Label) {
                    continue;
                } else if (item->controlTypes() != QSizePolicy::PushButton) {
                    widget = item->layout()->itemAt(1)->widget();
                }
                QPoint pos = widget->mapToGlobal(widget->rect().center());
                int diff = abs(pos.y() - QCursor::pos().y());
                if (dif != -1 && dif < diff) {
                    break;
                } else {
                    new_index = i;
                    dif = diff;
                }
            }
            auto* item = t_ptr->getContentLayout().itemAt(new_index);
            auto* item2 = t_ptr->getContentLayout().itemAt(dragged_index);
            QWidget* widget = item->widget();

            if (item->controlTypes() != QSizePolicy::Label && item->controlTypes() != QSizePolicy::PushButton &&
                item2->controlTypes() != QSizePolicy::Label && item2->controlTypes() != QSizePolicy::PushButton)
            {
                int from = t_ptr->getContentLayout().indexOf(member_parent);
                if (from < new_index && new_index <= from + member_parent->getMembersAmount()) {
                    if (t_ptr->getSelectMode() == 2 && new_index == dragged_index && selectable) { // look for members
                        T::selectElement(item->layout()->itemAt(1)->widget());
                    } else if (t_ptr->getSelectMode() == 0) {
                        t_ptr->getContentLayout().removeItem(item);
                        t_ptr->getContentLayout().insertItem(dragged_index, item);
                        t_ptr->getContentLayout().removeItem(item2);
                        t_ptr->getContentLayout().insertItem(new_index, item2);
                    }
                }
            } else if (item->controlTypes() == item2->controlTypes()) {
                if (t_ptr->getSelectMode() == 1 && new_index == dragged_index && selectable) { // look for structures
                    T::selectElement(widget);
                } else if (t_ptr->getSelectMode() == 0) {
                    t_ptr->getContentLayout().removeWidget(widget);
                    t_ptr->getContentLayout().insertWidget(dragged_index, widget);
                    t_ptr->getContentLayout().removeWidget(this);
                    t_ptr->getContentLayout().insertWidget(new_index, this);
                }
            }
            dragged_index = -1;
        }
    }
private:
    std::variant<Class, Struct, Component, Module>  code{Struct("")};
    int                                             dragged_index = -1;
    std::string                                     element_type;
    QGridLayout*                                    member_layout = nullptr;
    ElementButton<T>*                               member_parent = nullptr;
    QPushButton*                                    ok_parent_button = nullptr;
    int                                             members = 0;
    std::map<std::string, std::vector<std::string>> parents_and_members;
    bool                                            selectable = true;
    T*                                              t_ptr;

    // I did not have enough time to create a separate class for members...

    void renameElement() {
        auto* dialog = new QDialog();
        auto* line_edit = new QLineEdit();
        auto* ok_button = new QPushButton("OK");
        dialog->setWindowTitle("Rename Element");
        dialog->setMinimumWidth(220);

        QObject::connect(ok_button, &QPushButton::clicked, [&]() {
            if (!line_edit->text().toStdString().empty()) {
                setText(line_edit->text());
                setName(line_edit->text());
                setFixedWidth(sizeHint().width());
                dialog->accept();
            }
        });

        QVBoxLayout layout(dialog);
        layout.addWidget(line_edit);
        layout.addWidget(ok_button);

        dialog->exec();
    }
//TODO: create new type -> CodeElement

// I did not have enough time for "DON'T REPEAT YOURSELF"
// my goal was to release the beta version as soon as possible

    void addParent() {
        ok_parent_button = new QPushButton("OK");
        ok_parent_button->setStyleSheet("background-color: rgb(43, 45, 48);");
        QObject::connect(ok_parent_button, &QPushButton::clicked, [&]() {
            for (int i = 0; i < t_ptr->getElementsAmount(); i++) {
                auto* elem_item = t_ptr->getContentLayout().itemAt(i);
                if (elem_item->controlTypes() != QSizePolicy::PushButton) {
                    continue;
                }
                if (!elem_item->widget()->styleSheet().toStdString().empty()) {
                    auto* widget = elem_item->widget();
                    widget->setStyleSheet("");
                    auto* btn = reinterpret_cast<QPushButton *>(widget);
                    std::visit([btn](auto&& arg) { arg.addParent(split(btn->text().toStdString(), ' ')[1], "public");}, code);
                }
            }
            selectable = true;
            t_ptr->removeInvisibleElement();
            delete ok_parent_button;
            t_ptr->setSelectMode(0);
        });
        t_ptr->getElementsAmount() += 1;
        t_ptr->getContentLayout().insertWidget(t_ptr->getElementsAmount(), ok_parent_button);
        t_ptr->setSelectMode(1);
        selectable = false;
    }
    void addSelectiveParent() {
        ok_parent_button = new QPushButton("OK");
        ok_parent_button->setStyleSheet("background-color: rgb(43, 45, 48);");
        parents_and_members = {};
        QObject::connect(ok_parent_button, &QPushButton::clicked, [&]() {
            for (int i = 0; i < t_ptr->getElementsAmount(); i++) {
                auto* elem_item = t_ptr->getContentLayout().itemAt(i);
                if (elem_item->controlTypes() == QSizePolicy::Label || elem_item->controlTypes() == QSizePolicy::PushButton) {
                    continue;
                }
                auto* elem_widget = elem_item->layout()->itemAt(1)->widget();
                if (!elem_widget->styleSheet().toStdString().empty()) {
                    elem_widget->setStyleSheet("");
                    auto* btn = reinterpret_cast<ElementButton<T>*>(elem_widget);
                    std::string parent_element = split(btn->member_parent->text().toStdString(), ' ')[1];
                    std::string child_element;
                    std::visit([btn, &child_element](auto&& arg) { child_element = btn->text().toStdString();}, code);
                    if (parents_and_members.find(parent_element) != parents_and_members.end()) {
                        parents_and_members[parent_element].push_back(child_element);
                    } else {
                        parents_and_members.insert({parent_element, std::vector<std::string>(1, child_element)});
                    }
                }
            }
            for (auto& pair: parents_and_members) {
                if (pair.first == text().toStdString()) continue; // skip members of selected class
                std::visit([pair](auto&& arg) { arg.addSelectiveParent(pair.first, pair.second);}, code);
            }
            t_ptr->removeInvisibleElement();
            delete ok_parent_button;
            t_ptr->setSelectMode(0);
        });
        t_ptr->getElementsAmount() += 1;
        t_ptr->getContentLayout().insertWidget(t_ptr->getElementsAmount(), ok_parent_button);
        t_ptr->setSelectMode(2);
    }
    void addComponent() { // in fact, this is selective component
        ok_parent_button = new QPushButton("OK");
        ok_parent_button->setStyleSheet("background-color: rgb(43, 45, 48);");
        parents_and_members = {};
        QObject::connect(ok_parent_button, &QPushButton::clicked, [&]() {
            for (int i = 0; i < t_ptr->getElementsAmount(); i++) {
                auto* elem_item = t_ptr->getContentLayout().itemAt(i);
                if (elem_item->controlTypes() == QSizePolicy::Label || elem_item->controlTypes() == QSizePolicy::PushButton) {
                    continue;
                }
                auto* elem_widget = elem_item->layout()->itemAt(1)->widget();
                if (!elem_widget->styleSheet().toStdString().empty()) {
                    elem_widget->setStyleSheet("");
                    auto* btn = reinterpret_cast<ElementButton<T>*>(elem_widget);
                    std::string parent_element = split(btn->member_parent->text().toStdString(), ' ')[1];
                    std::string child_element;
                    std::visit([btn, &child_element](auto&& arg) { child_element = btn->text().toStdString();}, code);
                    if (parents_and_members.find(parent_element) != parents_and_members.end()) {
                        parents_and_members[parent_element].push_back(child_element);
                    } else {
                        parents_and_members.insert({parent_element, std::vector<std::string>(1, child_element)});
                    }
                }
            }
            for (auto& pair: parents_and_members) {
                if (pair.first != text().toStdString()) continue; // skip members that don't belong to selected class
                std::visit([pair](auto&& arg) { arg.addSelectiveComponent(pair.first, pair.second);}, code);
            }
            t_ptr->removeInvisibleElement();
            delete ok_parent_button;
            t_ptr->setSelectMode(0);
        });
        t_ptr->getElementsAmount() += 1;
        t_ptr->getContentLayout().insertWidget(t_ptr->getElementsAmount(), ok_parent_button);
        t_ptr->setSelectMode(2);
    }
};

#endif //ARTCHITECT_ELEMENTBUTTON_HPP
