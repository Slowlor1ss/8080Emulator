#pragma once

#include <qpainter.h>
#include "ui_QtProj.h"

class Display;
class ConsoleWindow;
class i8080Emulator;

class i8080GUI : public QWidget
{
    Q_OBJECT

public:
    explicit i8080GUI(QWidget *parent = nullptr);
    ~i8080GUI() override;

    void Update8080();
    bool GetIsClosed() const;

private slots:
    void paintEvent(QPaintEvent* pEvent);
    void keyPressEvent(QKeyEvent* pEvent);
    void closeEvent(QCloseEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
    void on_spinBox_valueChanged(int i);
    void on_inputButton_clicked(bool checked);
    void on_disassembleButton_clicked(bool checked);
    void on_checkBox_stateChanged(int state);

private:
    Ui::i8080GUI ui{};
    QString m_Input{""};
    bool m_ConsoleProgram{false};
    i8080Emulator* m_pI8080;
    ConsoleWindow* m_ConsoleWindow;
    bool m_IsClosed{false};

    QPainter m_Painter;
    //No ownership
    Display* m_pDisplay{nullptr};

    uint16_t m_Width;
    uint16_t m_Height;
    uint16_t m_PixelSize;
    uint16_t m_MarginTop{25};
};

