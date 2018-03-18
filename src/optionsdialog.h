#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>

namespace Ui {
class OptionsDialog;
}

class MainWindow;

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    OptionsDialog(MainWindow *parent, float initThreshold = 0.95f);
    ~OptionsDialog();

    float GetPictureThreshold() const;

    virtual void accept();

private slots:
    void on_threshold_hSlider_valueChanged(int value);

private:
    MainWindow *        mainWindow;
    Ui::OptionsDialog * ui;
    float               PicThreshold;
};

#endif // OPTIONSDIALOG_H
