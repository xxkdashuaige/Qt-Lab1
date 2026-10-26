#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QKeyEvent>
#include <QPushButton>
#include <QStatusBar>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , currentInput("0")
{
    ui->setupUi(this);

    this->setStyleSheet(R"(
    QMainWindow {
        background-color: #111827;
    }

    QWidget {
        color: #F9FAFB;
        font-family: "Microsoft YaHei";
    }

    QLineEdit {
        background-color: #1F2937;
        color: #FFFFFF;
        border: 1px solid #374151;
        border-radius: 12px;
        padding: 12px;
        font-size: 30px;
        selection-background-color: #2563EB;
    }

    QPushButton {
        background-color: #273449;
        color: #F9FAFB;
        border: 1px solid #374151;
        border-radius: 10px;
        padding: 8px;
        font-size: 19px;
        min-height: 38px;
    }

    QPushButton:hover {
        background-color: #475569;
        border: 1px solid #64748B;
    }

    QPushButton:pressed {
        background-color: #64748B;
    }

    QStatusBar {
        background-color: #111827;
        color: #9CA3AF;
    }

    QStatusBar::item {
        border: none;
    }
)");

    setWindowTitle(QStringLiteral("带键盘事件的计算器"));
    setMinimumSize(360, 420);

    // 显示框只用于显示，避免鼠标和键盘直接修改显示内容。
    ui->display->setReadOnly(true);
    ui->display->setFocusPolicy(Qt::NoFocus);

    const QList<QPushButton *> numberButtons = {
        ui->btnNum0, ui->btnNum1, ui->btnNum2, ui->btnNum3, ui->btnNum4,
        ui->btnNum5, ui->btnNum6, ui->btnNum7, ui->btnNum8, ui->btnNum9
    };
    for (QPushButton *button : numberButtons) {
        button->setFocusPolicy(Qt::NoFocus);
        connect(button, &QPushButton::clicked, this, [this, button]() {
            handleToken(button->text());
        });
    }

    const QList<QPushButton *> commandButtons = {
        ui->btnPlus, ui->btnMinus, ui->btnMultiple, ui->btnDivide,
        ui->btnPeroid, ui->btnDel, ui->btnClear, ui->btnClearAll,
        ui->btnEqual, ui->btnPercentage, ui->btnInverse, ui->btnSqrt,
        ui->btnSquare, ui->btnSign
    };
    for (QPushButton *button : commandButtons) {
        button->setFocusPolicy(Qt::NoFocus);
        connect(button, &QPushButton::clicked, this, [this, button]() {
            handleToken(button->text());
        });
    }

    // 统一设置样式，保持按钮尺寸和视觉风格一致。
    setStyleSheet(R"(
        QMainWindow { background: #f3f5f8; }
        QLineEdit#display {
            background: #ffffff; color: #172033; border: 1px solid #d6dce5;
            border-radius: 8px; padding: 12px; font-size: 26px; min-height: 42px;
        }
        QPushButton {
            background: #ffffff; color: #243047; border: 1px solid #dce2ea;
            border-radius: 8px; padding: 8px; min-height: 30px;
            font-size: 16px;
        }
        QPushButton:hover { background: #e8eef7; }
        QPushButton:pressed { background: #d5e2f5; }
        QPushButton#btnPlus, QPushButton#btnMinus,
        QPushButton#btnMultiple, QPushButton#btnDivide {
            background: #e7efff; color: #174ea6; font-weight: bold;
        }
        QPushButton#btnEqual {
            background: #2463eb; color: white; font-weight: bold;
        }
        QPushButton#btnClear, QPushButton#btnClearAll, QPushButton#btnDel {
            background: #edf0f5;
        }
        QStatusBar { color: #475569; }
    )");

    updateDisplay();
    statusBar()->showMessage(QStringLiteral("就绪：可使用鼠标或键盘输入"));
    setFocus();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::handleToken(const QString &token)
{
    if (token.size() == 1 && token.at(0).isDigit()) {
        inputDigit(token);
    } else if (token == "." ) {
        inputDecimalPoint();
    } else if (token == "+" || token == "-" || token == "×" || token == "÷"
               || token == "*" || token == "/") {
        QString op = token;
        if (op == "×") op = "*";
        if (op == "÷") op = "/";
        chooseOperator(op);
    } else if (token == "=") {
        calculateResult();
    } else if (token == "⌫") {
        backspace();
    } else if (token == "CE") {
        clearEntry();
    } else if (token == "C") {
        clearAll();
    } else if (token == "±") {
        applyUnaryOperator("sign");
    } else if (token == "%") {
        applyUnaryOperator("%");
    } else if (token == "1/x") {
        applyUnaryOperator("inverse");
    } else if (token == "x^2" || token == "x²") {
        applyUnaryOperator("square");
    } else if (token == "√x" || token == "sqrt") {
        applyUnaryOperator("sqrt");
    }
}

void MainWindow::inputDigit(const QString &digit)
{
    if (errorState) clearAll();

    // 等待新操作数时，首个数字替换旧显示；否则在当前数字后追加。
    if (waitingForOperand || resultShown) {
        currentInput = digit;
        waitingForOperand = false;
        resultShown = false;
    } else if (currentInput == "0") {
        currentInput = digit;
    } else if (currentInput == "-0") {
        currentInput = "-" + digit;
    } else {
        currentInput += digit;
    }
    updateDisplay();
}

void MainWindow::inputDecimalPoint()
{
    if (errorState) clearAll();
    if (waitingForOperand || resultShown) {
        currentInput = "0.";
        waitingForOperand = false;
        resultShown = false;
    } else if (!currentInput.contains('.')) {
        currentInput += '.';
    }
    updateDisplay();
}

void MainWindow::chooseOperator(const QString &op)
{
    if (errorState) return;

    bool ok = false;
    const double currentValue = currentInput.toDouble(&ok);
    if (!ok) {
        showError(QStringLiteral("输入数字无效"));
        return;
    }

    // 连续输入运算符时替换待处理运算符；已有第二操作数时先完成上次运算。
    if (!pendingOperator.isEmpty() && !waitingForOperand) {
        double result = 0.0;
        if (!calculate(pendingValue, currentValue, pendingOperator, &result)) return;
        currentInput = QString::number(result, 'g', 15);
        pendingValue = result;
    } else if (pendingOperator.isEmpty()) {
        pendingValue = currentValue;
    }

    pendingOperator = op;
    waitingForOperand = true;
    resultShown = false;
    updateDisplay();
    statusBar()->showMessage(QStringLiteral("已选择运算符 %1").arg(op));
}

bool MainWindow::calculate(double left, double right, const QString &op, double *result)
{
    if (op == "+") *result = left + right;
    else if (op == "-") *result = left - right;
    else if (op == "*") *result = left * right;
    else if (op == "/") {
        if (std::abs(right) < 1e-12) {
            showError(QStringLiteral("除数不能为 0，请重新输入"));
            return false;
        }
        *result = left / right;
    } else {
        showError(QStringLiteral("未知运算符"));
        return false;
    }

    if (!std::isfinite(*result)) {
        showError(QStringLiteral("计算结果超出有效范围"));
        return false;
    }
    return true;
}

void MainWindow::calculateResult()
{
    if (errorState || pendingOperator.isEmpty()) {
        statusBar()->showMessage(QStringLiteral("请先输入完整的运算表达式"));
        return;
    }

    // 若运算符后直接按等号，不把空操作数当成有效数字。
    if (waitingForOperand) {
        statusBar()->showMessage(QStringLiteral("请输入第二个操作数"));
        return;
    }

    bool ok = false;
    const double right = currentInput.toDouble(&ok);
    if (!ok) {
        showError(QStringLiteral("输入数字无效"));
        return;
    }

    const QString op = pendingOperator;
    const double left = pendingValue;
    double result = 0.0;
    if (!calculate(left, right, op, &result)) return;

    currentInput = QString::number(result, 'g', 15);
    pendingOperator.clear();
    pendingValue = result;
    waitingForOperand = true;
    resultShown = true;
    updateDisplay();
    statusBar()->showMessage(QStringLiteral("计算完成：%1 %2 %3 = %4")
                                 .arg(left).arg(op).arg(right).arg(currentInput));
}

void MainWindow::applyUnaryOperator(const QString &op)
{
    if (errorState) return;
    bool ok = false;
    double value = currentInput.toDouble(&ok);
    if (!ok) {
        showError(QStringLiteral("输入数字无效"));
        return;
    }

    if (op == "sign") value = -value;
    else if (op == "%") value /= 100.0;
    else if (op == "inverse") {
        if (std::abs(value) < 1e-12) {
            showError(QStringLiteral("0 不能求倒数"));
            return;
        }
        value = 1.0 / value;
    } else if (op == "square") value *= value;
    else if (op == "sqrt") {
        if (value < 0) {
            showError(QStringLiteral("负数不能进行实数平方根运算"));
            return;
        }
        value = std::sqrt(value);
    }

    currentInput = QString::number(value, 'g', 15);
    waitingForOperand = true;
    resultShown = true;
    updateDisplay();
    statusBar()->showMessage(QStringLiteral("已完成单目运算"));
}

void MainWindow::backspace()
{
    if (errorState) {
        clearAll();
        return;
    }
    if (waitingForOperand || resultShown) {
        currentInput = "0";
        waitingForOperand = false;
        resultShown = false;
    } else {
        if (currentInput.size() > 1) {
            currentInput.chop(1);
            if (currentInput == "-" || currentInput.isEmpty()) currentInput = "0";
        } else {
            currentInput = "0";
        }
    }
    updateDisplay();
    statusBar()->showMessage(QStringLiteral("已删除末位字符"));
}

void MainWindow::clearEntry()
{
    currentInput = "0";
    waitingForOperand = true;
    resultShown = false;
    errorState = false;
    updateDisplay();
    statusBar()->showMessage(QStringLiteral("已清除当前操作数"));
}

void MainWindow::clearAll()
{
    currentInput = "0";
    pendingOperator.clear();
    pendingValue = 0.0;
    waitingForOperand = true;
    resultShown = false;
    errorState = false;
    updateDisplay();
    statusBar()->showMessage(QStringLiteral("已清空，可重新计算"));
}

void MainWindow::updateDisplay()
{
    ui->display->setText(errorState ? QStringLiteral("错误") : currentInput);
}

void MainWindow::showError(const QString &message)
{
    errorState = true;
    currentInput = "0";
    pendingOperator.clear();
    pendingValue = 0.0;
    waitingForOperand = true;
    resultShown = false;
    updateDisplay();
    statusBar()->showMessage(message);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    const int key = event->key();
    if (key >= Qt::Key_0 && key <= Qt::Key_9) {
        handleToken(QString(QChar(static_cast<ushort>('0' + key - Qt::Key_0))));
    } else {
        // 数字小键盘通常也会产生 Key_0 至 Key_9，并附带 KeypadModifier。
        switch (key) {
        case Qt::Key_Plus: handleToken("+"); break;
        case Qt::Key_Minus: handleToken("-"); break;
        case Qt::Key_Asterisk: handleToken("*"); break;
        case Qt::Key_Slash: handleToken("/"); break;
        case Qt::Key_Period:
        case Qt::Key_Comma: handleToken("."); break;
        case Qt::Key_Enter:
        case Qt::Key_Return:
        case Qt::Key_Equal: handleToken("="); break;
        case Qt::Key_Backspace: handleToken("⌫"); break;
        case Qt::Key_Delete: handleToken("CE"); break;
        case Qt::Key_Escape: handleToken("C"); break;
        default:
            QMainWindow::keyPressEvent(event);
            return;
        }
    }
    event->accept();
}
