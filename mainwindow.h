#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVector>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QString calculation(bool *ok);

private slots:
    void btnNumClicked();
    void btnBinaryOperatorClicked();
    void on_btnPeroid_clicked();
    void on_btnDel_clicked();
    void on_btnClearAll_clicked();
    void on_btnEqual_clicked(); // btnEqual自动槽

private:
    Ui::MainWindow *ui;
    QString operand;           // 当前正在输入的数字
    QVector<double> operands;  // 存放操作数
    QVector<QString> opcodes;  // 存放运算符
};
#endif // MAINWINDOW_H
