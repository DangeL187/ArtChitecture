#ifndef ARTCHITECT_UPDATABLEWIDGETWINDOW_HPP
#define ARTCHITECT_UPDATABLEWIDGETWINDOW_HPP

#include <QWidget>

template <typename T>
class UpdatableWidgetWindow: public QWidget {
public:
    explicit UpdatableWidgetWindow(T* owner) {
        t_owner = owner;
        setStyleSheet("background-color: rgb(43, 45, 48);");
    }
protected:
    void resizeEvent(QResizeEvent *event) override {
        QWidget::resizeEvent(event);
        t_owner->updateWidth();
    }
private:
    T* t_owner;
};

#endif //ARTCHITECT_UPDATABLEWIDGETWINDOW_HPP
