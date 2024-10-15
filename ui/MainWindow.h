#pragma once

#include <QMainWindow>

namespace Ui
{
    class MainWindow;
}

class QProcess;

class MainWindow final: public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    MainWindow(const MainWindow&) = delete;
    MainWindow(MainWindow&&) = delete;
    MainWindow& operator=(const MainWindow&) = delete;
    MainWindow& operator=(MainWindow&&) = delete;
    ~MainWindow() override;

private slots:
    void doFocusStack();

private:
    Ui::MainWindow* ui = nullptr;
    QProcess* m_algo_process = nullptr;
    QStringList m_files{};
};