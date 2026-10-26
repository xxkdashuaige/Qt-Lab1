#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class QKeyEvent;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    // 将键盘输入转换成与鼠标按钮相同的命令，复用统一处理逻辑。
    void keyPressEvent(QKeyEvent *event) override;

private:
    void handleToken(const QString &token);
    void inputDigit(const QString &digit);
    void inputDecimalPoint();
    void chooseOperator(const QString &op);
    void calculateResult();
    void applyUnaryOperator(const QString &op);
    void backspace();
    void clearEntry();
    void clearAll();
    bool calculate(double left, double right, const QString &op, double *result);
    void updateDisplay();
    void showError(const QString &message);

    Ui::MainWindow *ui;
    QString currentInput;
    QString pendingOperator;
    double pendingValue = 0.0;
    bool waitingForOperand = true;
    bool resultShown = false;
    bool errorState = false;
};

#endif // MAINWINDOW_H
