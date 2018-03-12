#include "optionsdialog.h"
#include "ui_options.h"
#include "mainwindow.h"

OptionsDialog::OptionsDialog(MainWindow *parent, float initThreshold)
    : QDialog(parent),
    ui(new Ui::OptionsDialog)
{
    ui->setupUi(this);
    mainWindow = parent;
    PicThreshold = initThreshold;

    ui->threshold_hSlider->setValue(initThreshold * 100);
    ui->thresholdvalue_Label->setText(QString::number(initThreshold, 'f', 2));

    auto b = connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    Q_ASSERT(b);
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}

float OptionsDialog::GetPictureThreshold() const
{
    return PicThreshold;
}

void OptionsDialog::accept()
{
    PicThreshold = static_cast<float>(ui->threshold_hSlider->value()) / 100.0f;
    QDialog::accept();
}

void OptionsDialog::on_threshold_hSlider_valueChanged(int value)
{
    ui->thresholdvalue_Label->setText(QString::number(static_cast<float>(ui->threshold_hSlider->value()) / 100.0f, 'f', 2));
}
