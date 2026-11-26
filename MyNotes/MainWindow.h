#pragma once

#include <QMainWindow>
#include "ui_MainWindow.h"
#include "Database.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindowClass ui;
    Database database;
};
