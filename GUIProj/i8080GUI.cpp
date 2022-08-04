#include "i8080GUI.h"
#include <fstream>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <qpainter.h>
#include <qevent.h>
#include <QTimer>
#include <thread>
#include "8080/ConsoleWindow.h"
#include "8080/i8080Emulator.h"
#include "8080/Display.h"
#include "8080/Keyboard.h"

i8080GUI::i8080GUI(QWidget* parent)
    : QWidget(parent)
    , m_pI8080(new i8080Emulator())
	, m_ConsoleWindow(new ConsoleWindow())
{
    // Connect button signal to appropriate slot
    ui.setupUi(this);
    setFocus();

    m_pDisplay = m_pI8080->GetDisplay();
    m_Width = m_pDisplay->GetWidth();
    m_Height = m_pDisplay->GetHeight();
    m_PixelSize = m_pDisplay->GetPixelSize();
    m_pDisplay->AddDrawCallback([this] { repaint(); }); //will call qt's repaint when display is updated

    ui.spinBox->setValue(m_pI8080->GetClockSpeed());
    ui.disassembleButton->setDisabled(true);

    resize(m_Width * m_PixelSize, m_Height * m_PixelSize + m_MarginTop);
    setMaximumSize(m_Width * m_PixelSize, m_Height * m_PixelSize + m_MarginTop);
    setMinimumSize(m_Width * m_PixelSize, m_Height * m_PixelSize + m_MarginTop);
}

i8080GUI::~i8080GUI()
{
	delete m_pI8080;
    delete m_ConsoleWindow;
}

void i8080GUI::Update8080()
{
    m_pI8080->Update();
}

bool i8080GUI::GetIsClosed() const
{
    return m_IsClosed;
}

void i8080GUI::paintEvent(QPaintEvent*)
{
    // Generate image from chip display data
    //const auto& data = m_pI8080->GetDisplay()->GetPixels();;
    const QImage image{ /*(uchar*)&data[0]*/ (uchar*)m_pDisplay->GetPixels(), m_Width, m_Height, QImage::Format_RGB444};

    //// Set image's colour palette to black and white
    //image.setColorCount(2);
    //image.setColorTable(QList<QRgb>{QColor("black").rgb(), QColor("white").rgb()});

    // Create texture from image
    const auto displayTexture = QPixmap::fromImage(image);

    // Draw
    m_Painter.begin(this);
    m_Painter.setRenderHints(QPainter::Antialiasing); // No AA

    m_Painter.drawPixmap(0, m_MarginTop, m_Width * m_PixelSize, m_Height * m_PixelSize, displayTexture); // Draw virtual machine's display

    m_Painter.end();
}

void i8080GUI::keyPressEvent(QKeyEvent* key)
{
    m_pI8080->GetKeyboard()->KeyDown(key->key());
}

void i8080GUI::keyReleaseEvent(QKeyEvent* key)
{
    m_pI8080->GetKeyboard()->KeyUp(key->key());
}

void i8080GUI::on_spinBox_valueChanged(int i)
{
    m_pI8080->SetClockSpeed(i);
}

void i8080GUI::on_inputButton_clicked(bool checked)
{
    m_Input = QFileDialog::getOpenFileName(this, tr("Open input json file"),
        QString(),
        tr("Select ROM (*.rom; *.com; *.bin)"));

    bool success = m_pI8080->LoadRom(m_ConsoleProgram, m_Input.toStdString().c_str());

    if (success)
    {
        ui.disassembleButton->setDisabled(false);

	    if (m_ConsoleProgram)
		    m_ConsoleWindow->Clear();
    }
}

void i8080GUI::on_disassembleButton_clicked(bool checked)
{
    if (m_ConsoleProgram)
        m_ConsoleWindow->Restore(); //will show console window in case it was minimized

    m_pI8080->PrintDisassembledRom();
}

void i8080GUI::on_checkBox_stateChanged(int state)
{
    m_ConsoleProgram = state;

    if (m_ConsoleProgram)
	    m_ConsoleWindow->Restore(); //will show console window in case it was minimized
}

void i8080GUI::closeEvent(QCloseEvent* event)
{
    m_pI8080->Stop();
    m_IsClosed = true;
    event->accept();
}
