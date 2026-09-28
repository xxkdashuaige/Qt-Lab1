#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QPushButton>
#include <QMessageBox>
#include <cmath>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // 数字按钮
    connect(ui->btnNum0,SIGNAL(clicked()),this,SLOT(btnNumClicked()));
    connect(ui->btnNum1,SIGNAL(clicked()),this,SLOT(btnNumClicked()));
    connect(ui->btnNum2,SIGNAL(clicked()),this,SLOT(btnNumClicked()));
    connect(ui->btnNum3,SIGNAL(clicked()),this,SLOT(btnNumClicked()));
    connect(ui->btnNum4,SIGNAL(clicked()),this,SLOT(btnNumClicked()));
    connect(ui->btnNum5,SIGNAL(clicked()),this,SLOT(btnNumClicked()));
    connect(ui->btnNum6,SIGNAL(clicked()),this,SLOT(btnNumClicked()));
    connect(ui->btnNum7,SIGNAL(clicked()),this,SLOT(btnNumClicked()));
    connect(ui->btnNum8,SIGNAL(clicked()),this,SLOT(btnNumClicked()));
    connect(ui->btnNum9,SIGNAL(clicked()),this,SLOT(btnNumClicked()));
    // 二元运算符
    connect(ui->btnMultiple,SIGNAL(clicked()),this,SLOT(btnBinaryOperatorClicked()));
    connect(ui->btnMinus,SIGNAL(clicked()),this,SLOT(btnBinaryOperatorClicked()));
    connect(ui->btnPlus,SIGNAL(clicked()),this,SLOT(btnBinaryOperatorClicked()));
    connect(ui->btnDivide,SIGNAL(clicked()),this,SLOT(btnBinaryOperatorClicked()));

    operand = "0";
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::calculation(bool *ok)
{
    *ok = false;
    if(operands.size() != 2 || opcodes.size() != 1)
    {
        ui->statusbar->showMessage(QString("操作数:%1,运算符:%2").arg(operands.size()).arg(opcodes.size()));
        return "0";
    }
    double a = operands[0];
    double b = operands[1];
    QString op = opcodes[0];
    double res = 0;

    if(op == "+")
    {
        res = a + b;
    }
    else if(op == "-")
    {
        res = a - b;
    }
    else if(op == "*")
    {
        res = a * b;
    }
    else if(op == "/")
    {
        if(fabs(b) < 1e-9)
        {
            ui->statusbar->showMessage("错误：除数不能为0");
            QMessageBox::warning(this,"运算错误","除数不能为0！");
            operands.clear();
            opcodes.clear();
            return "0";
        }
        res = a / b;
    }
    else
    {
        ui->statusbar->showMessage("未知运算符");
        return "0";
    }
    *ok = true;
    ui->statusbar->showMessage(QString("计算完成 %1 %2 %3 = %4").arg(a).arg(op).arg(b).arg(res));

    operands.clear();
    opcodes.clear();
    operands.push_back(res);
    return QString::number(res);
}

void MainWindow::btnNumClicked()
{
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    QString digit = btn->text();
    if(digit == "0" && operand == "0")
        return;
    if(operand == "0" && digit != "0")
        operand = "";
    operand += digit;
    ui->display->setText(operand);
}

void MainWindow::on_btnPeroid_clicked()
{
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    QString dot = btn->text();
    if(!operand.contains("."))
        operand += dot;
    ui->display->setText(operand);
}

void MainWindow::on_btnDel_clicked()
{
    if(operand.length() > 0)
        operand = operand.left(operand.length()-1);
    if(operand.isEmpty())
        operand = "0";
    ui->display->setText(operand);
}

void MainWindow::on_btnClearAll_clicked()
{
    operand = "0";
    operands.clear();
    opcodes.clear();
    ui->display->setText(operand);
    ui->statusbar->showMessage("已清空");
}

void MainWindow::btnBinaryOperatorClicked()
{
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    QString opcode = btn->text();
    ui->statusbar->showMessage("当前输入：" + operand);

    bool ok;
    double num = operand.toDouble(&ok);
    if(!ok) return;

    if(operands.size() == 1)
    {
        operands.push_back(num);
        QString ret = calculation(&ok);
        operand = ret;
        ui->display->setText(operand);
        opcodes.push_back(opcode);
    }
    else
    {
        operands.push_back(num);
        opcodes.push_back(opcode);
    }
    operand = "0";
}

// ========== btnEqual 等于按钮槽函数 ==========
void MainWindow::on_btnEqual_clicked()
{
    // 必须满足：栈有1个操作数 + 1个运算符，才可以计算
    if(opcodes.size() != 1 || operands.size() != 1)
    {
        ui->statusbar->showMessage("缺少运算符/操作数");
        return;
    }
    bool ok;
    double num = operand.toDouble(&ok);
    if(!ok)
    {
        ui->statusbar->showMessage("输入数字非法");
        return;
    }
    // 当前输入作为第二个操作数入栈
    operands.push_back(num);
    QString result = calculation(&ok);
    ui->display->setText(result);
    operand = result;
}
