#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 设置窗口属性
    setWindowTitle("UnravelChat - 登录");
    setFixedSize(400, 500);

    setupConnections();

    // 设置焦点
    ui->usernameEdit->setFocus();

    qDebug() << "UnravelChat 登录界面初始化完成";
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupConnections()
{
    // 按钮连接
    connect(ui->loginBtn, &QPushButton::clicked,
            this, &MainWindow::onLoginClicked);
    connect(ui->registerBtn, &QPushButton::clicked,
            this, &MainWindow::onRegisterClicked);
    connect(ui->forgotPasswordBtn, &QPushButton::clicked,
            this, &MainWindow::onForgotPasswordClicked);

    // 输入框连接
    connect(ui->usernameEdit, &QLineEdit::textChanged,
            this, &MainWindow::onInputChanged);
    connect(ui->passwordEdit, &QLineEdit::textChanged,
            this, &MainWindow::onInputChanged);
    connect(ui->passwordEdit, &QLineEdit::returnPressed,
            this, &MainWindow::onLoginClicked);

    // 复选框连接
    connect(ui->autoLoginCheckBox, &QCheckBox::toggled, [this](bool checked) {
        if (checked) {
            ui->rememberCheckBox->setChecked(true);
        }
    });
}

void MainWindow::onInputChanged()
{
    // 检查是否可以启用登录按钮
    bool canLogin = !ui->usernameEdit->text().trimmed().isEmpty() &&
                    !ui->passwordEdit->text().isEmpty();
    ui->loginBtn->setEnabled(canLogin);
}

bool MainWindow::validateInput()
{
    QString username = ui->usernameEdit->text().trimmed();
    QString password = ui->passwordEdit->text();

    if (username.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入用户名或邮箱");
        ui->usernameEdit->setFocus();
        return false;
    }

    if (password.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入密码");
        ui->passwordEdit->setFocus();
        return false;
    }

    if (username.length() < 3) {
        QMessageBox::warning(this, "输入错误", "用户名至少需要3个字符");
        ui->usernameEdit->setFocus();
        return false;
    }

    if (password.length() < 6) {
        QMessageBox::warning(this, "输入错误", "密码至少需要6个字符");
        ui->passwordEdit->setFocus();
        return false;
    }

    return true;
}

void MainWindow::onLoginClicked()
{
    if (!validateInput()) {
        return;
    }

    QString username = ui->usernameEdit->text().trimmed();
    QString password = ui->passwordEdit->text();

    qDebug() << "尝试登录:" << username;

    // 模拟登录验证
    if (username == "admin" && password == "123456") {
        QMessageBox::information(this, "登录成功",
                                 QString("欢迎回来，%1！\n\n登录成功，即将进入聊天界面...").arg(username));
        setWindowTitle(QString("UnravelChat - %1").arg(username));
    } else {
        QMessageBox::warning(this, "登录失败",
                             "用户名或密码错误！\n\n提示：测试账号 admin/123456");
    }
}

void MainWindow::onRegisterClicked()
{
    QMessageBox::information(this, "注册",
                             "注册功能正在开发中...\n\n敬请期待！");
}

void MainWindow::onForgotPasswordClicked()
{
    QMessageBox::information(this, "忘记密码",
                             "忘记密码功能正在开发中...\n\n请联系管理员重置密码。");
}
