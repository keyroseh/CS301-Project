#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QMessageBox>
#include <QHeaderView>
#include <QGroupBox>
#include <QDate>
#include <QAbstractItemView>
#include <QColor>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), dataFile("budget_data.csv") {
    setupUI();
    loadData();
    refreshTransactionTable();
    refreshSummary();
}

MainWindow::~MainWindow() {
    saveData();
}

void MainWindow::setupUI() {
    setWindowTitle("Budget Tracker");
    resize(900, 600);

    // Central widget and main layout
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // Tab widget
    tabWidget = new QTabWidget(this);
    mainLayout->addWidget(tabWidget);

    // ===== TAB 1: Add Transaction =====
    QWidget *addTab = new QWidget();
    QVBoxLayout *addLayout = new QVBoxLayout(addTab);

    QGroupBox *inputGroup = new QGroupBox("Add New Transaction");
    QFormLayout *formLayout = new QFormLayout();

    dateEdit = new QDateEdit(QDate::currentDate());
    dateEdit->setCalendarPopup(true);
    dateEdit->setDisplayFormat("MM dd yyyy");
    formLayout->addRow("Date:", dateEdit);

    // Category dropdown populated from BudgetManager
    categoryCombo = new QComboBox();
    for (const auto& cat : manager.getCategoryList()) {
        categoryCombo->addItem(QString::fromStdString(cat));
    }
    formLayout->addRow("Category:", categoryCombo);

    amountEdit = new QLineEdit();
    amountEdit->setPlaceholderText("0.00");
    formLayout->addRow("Amount:", amountEdit);

    descriptionEdit = new QLineEdit();
    descriptionEdit->setPlaceholderText("Brief description");
    formLayout->addRow("Description:", descriptionEdit);

    addButton = new QPushButton("Add Transaction");
    addButton->setStyleSheet(
        "background-color: #4CAF50; color: white; padding: 10px; font-weight: bold;");
    connect(addButton, &QPushButton::clicked, this, &MainWindow::addTransaction);

    inputGroup->setLayout(formLayout);
    addLayout->addWidget(inputGroup);
    addLayout->addWidget(addButton);
    addLayout->addStretch();

    tabWidget->addTab(addTab, "Add Transaction");

    // ===== TAB 2: Edit / Delete Transaction =====
    QWidget *editTab = new QWidget();
    QVBoxLayout *editLayout = new QVBoxLayout(editTab);

    // Table to choose which transaction to edit or delete
    editTable = new QTableWidget();
    editTable->setColumnCount(5);
    editTable->setHorizontalHeaderLabels(
        {"Date", "Category", "Type", "Amount", "Description"});
    editTable->horizontalHeader()->setStretchLastSection(true);
    editTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    editTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    editTable->setAlternatingRowColors(true);

    editLayout->addWidget(editTable);

    // Group box with edit form + buttons
    QGroupBox *editGroup = new QGroupBox("Edit / Delete Selected Transaction");
    QFormLayout *editFormLayout = new QFormLayout();

    // Date
    editDateEdit = new QDateEdit(QDate::currentDate());
    editDateEdit->setCalendarPopup(true);
    editDateEdit->setDisplayFormat("MM dd yyyy");
    editFormLayout->addRow("Date:", editDateEdit);

    // Category
    editCategoryCombo = new QComboBox();
    for (const auto &cat : manager.getCategoryList()) {
        editCategoryCombo->addItem(QString::fromStdString(cat));
    }
    editFormLayout->addRow("Category:", editCategoryCombo);

    // Amount
    editAmountEdit = new QLineEdit();
    editAmountEdit->setPlaceholderText("0.00");
    editFormLayout->addRow("Amount:", editAmountEdit);

    // Description
    editDescriptionEdit = new QLineEdit();
    editDescriptionEdit->setPlaceholderText("Brief description");
    editFormLayout->addRow("Description:", editDescriptionEdit);

    // Buttons row
    QHBoxLayout *buttonsLayout = new QHBoxLayout();

    editSaveButton = new QPushButton("Save Changes");
    editSaveButton->setStyleSheet(
        "background-color: #0275d8; color: white; padding: 8px; font-weight: bold;");
    buttonsLayout->addWidget(editSaveButton);

    editDeleteButton = new QPushButton("Delete Selected");
    editDeleteButton->setStyleSheet(
        "background-color: #d9534f; color: white; padding: 8px; font-weight: bold;");
    buttonsLayout->addWidget(editDeleteButton);

    editFormLayout->addRow(buttonsLayout);

    editGroup->setLayout(editFormLayout);
    QHBoxLayout *centeredLayout = new QHBoxLayout();
    centeredLayout->addStretch();
    centeredLayout->addWidget(editGroup);
    centeredLayout->addStretch();
    editLayout->addLayout(centeredLayout);

    editLayout->addStretch();

    // Connect signals
    connect(editTable, &QTableWidget::itemSelectionChanged,
            this, &MainWindow::loadEditTransaction);

    connect(editSaveButton, &QPushButton::clicked,
            this, &MainWindow::saveEditTransaction);

    connect(editDeleteButton, &QPushButton::clicked,
            this, &MainWindow::deleteTransaction);

    // Add tab
    tabWidget->addTab(editTab, "Edit / Delete");

    // ===== TAB 3: View All Transactions =====
    QWidget *viewTab = new QWidget();
    QVBoxLayout *viewLayout = new QVBoxLayout(viewTab);

    transactionTable = new QTableWidget();
    transactionTable->setColumnCount(5);
    transactionTable->setHorizontalHeaderLabels(
        {"Date", "Category", "Type", "Amount", "Description"});
    transactionTable->horizontalHeader()->setStretchLastSection(true);
    transactionTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    transactionTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    transactionTable->setAlternatingRowColors(true);

    viewLayout->addWidget(transactionTable);
    tabWidget->addTab(viewTab, "View All Transactions");

    // ===== TAB 4: Dashboard =====
    QWidget *dashboardTab = new QWidget();
    QVBoxLayout *dashLayout = new QVBoxLayout(dashboardTab);

    QGroupBox *summaryGroup = new QGroupBox("Financial Summary");
    QVBoxLayout *summaryLayout = new QVBoxLayout();

    incomeLabel = new QLabel("Total Income: $0.00");
    incomeLabel->setStyleSheet("font-size: 18px; color: green; font-weight: bold;");
    summaryLayout->addWidget(incomeLabel);

    expenseLabel = new QLabel("Total Expense: $0.00");
    expenseLabel->setStyleSheet("font-size: 18px; color: red; font-weight: bold;");
    summaryLayout->addWidget(expenseLabel);

    balanceLabel = new QLabel("Balance: $0.00");
    balanceLabel->setStyleSheet("font-size: 20px; font-weight: bold;");
    summaryLayout->addWidget(balanceLabel);

    summaryGroup->setLayout(summaryLayout);
    dashLayout->addWidget(summaryGroup);
    dashLayout->addStretch();

    tabWidget->addTab(dashboardTab, "Dashboard");
}

// ===== Slots =====

void MainWindow::addTransaction() {
    QString dateStr = dateEdit->date().toString("M d yyyy");
    QString category = categoryCombo->currentText();     // <-- combo now
    QString amountStr = amountEdit->text().trimmed();
    QString description = descriptionEdit->text().trimmed();

    if (amountStr.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please enter an amount.");
        return;
    }

    bool ok;
    double amount = amountStr.toDouble(&ok);
    if (!ok || amount <= 0) {
        QMessageBox::warning(this, "Input Error", "Please enter a valid positive amount.");
        return;
    }

    // Decide type automatically based on category (same logic as your Menu.cpp)
    QString type;
    if (category == "Income" || category == "Savings"
        || category == "Refunds" || category == "Gift") {
        type = "Income";
    } else {
        type = "Expense";
    }

    Transaction t(dateStr.toStdString(),
                  category.toStdString(),
                  amount,
                  type.toStdString(),
                  description.toStdString());

    manager.addTransaction(t);

    amountEdit->clear();
    descriptionEdit->clear();

    refreshTransactionTable();
    refreshSummary();
    saveData();

    QMessageBox::information(this, "Success", "Transaction added successfully!");
}

void MainWindow::deleteTransaction()
{
    if (!editTable) return;

    auto selection = editTable->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::warning(this, "No Selection",
                             "Please select a transaction to delete.");
        return;
    }

    int row = selection.first().row();

    auto reply = QMessageBox::question(this, "Confirm Delete",
                                       "Are you sure you want to delete this transaction?");
    if (reply != QMessageBox::Yes) {
        return;
    }

    // Delete from the manager (assuming you have this)
    manager.deleteTransaction(row);

    refreshTransactionTable();
    refreshSummary();
    saveData();
}

void MainWindow::loadEditTransaction()
{
    auto selection = editTable->selectionModel()->selectedRows();
    if (selection.isEmpty()) return;

    int row = selection.first().row();
    const auto &transactions = manager.getTransactions();
    if (row < 0 || row >= static_cast<int>(transactions.size())) return;

    const Transaction &t = transactions[static_cast<size_t>(row)];

    QDate date = QDate::fromString(QString::fromStdString(t.date), "M d yyyy");
    if (!date.isValid()) date = QDate::currentDate();
    editDateEdit->setDate(date);

    int catIndex = editCategoryCombo->findText(QString::fromStdString(t.category));
    if (catIndex >= 0) editCategoryCombo->setCurrentIndex(catIndex);

    editAmountEdit->setText(QString::number(t.amount, 'f', 2));
    editDescriptionEdit->setText(QString::fromStdString(t.description));
}

void MainWindow::saveEditTransaction()
{
    auto selection = editTable->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        QMessageBox::warning(this, "No Selection",
                             "Please select a transaction to edit.");
        return;
    }

    int row = selection.first().row();

    QString dateStr = editDateEdit->date().toString("M d yyyy");
    QString category = editCategoryCombo->currentText();
    QString amountStr = editAmountEdit->text().trimmed();
    QString description = editDescriptionEdit->text().trimmed();

    if (amountStr.isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please enter an amount.");
        return;
    }

    bool ok;
    double amount = amountStr.toDouble(&ok);
    if (!ok || amount <= 0) {
        QMessageBox::warning(this, "Input Error",
                             "Please enter a valid positive amount.");
        return;
    }

    QString type;
    if (category == "Income" || category == "Savings"
        || category == "Refunds" || category == "Gift") {
        type = "Income";
    } else {
        type = "Expense";
    }

    Transaction t(dateStr.toStdString(),
                  category.toStdString(),
                  amount,
                  type.toStdString(),
                  description.toStdString());

    manager.editTransaction(row, t);   // your existing function
    refreshTransactionTable();
    refreshSummary();
    saveData();

    QMessageBox::information(this, "Saved",
                             "Transaction updated successfully.");
}




void MainWindow::refreshTransactionTable()
{
    const auto &transactions = manager.getTransactions();
    int n = static_cast<int>(transactions.size());

    // ---------- View All tab ----------
    if (transactionTable) {
        transactionTable->setRowCount(n);

        for (int i = 0; i < n; ++i) {
            const Transaction &t = transactions[static_cast<size_t>(i)];

            auto *itemDate = new QTableWidgetItem(QString::fromStdString(t.date));
            auto *itemCat  = new QTableWidgetItem(QString::fromStdString(t.category));
            auto *itemType = new QTableWidgetItem(QString::fromStdString(t.type));
            auto *itemAmt  = new QTableWidgetItem(QString::number(t.amount, 'f', 2));
            auto *itemDesc = new QTableWidgetItem(QString::fromStdString(t.description));

            transactionTable->setItem(i, 0, itemDate);
            transactionTable->setItem(i, 1, itemCat);
            transactionTable->setItem(i, 2, itemType);
            transactionTable->setItem(i, 3, itemAmt);
            transactionTable->setItem(i, 4, itemDesc);

            // color rows for income vs expense
            QColor rowColor = (t.type == "Income")
                                  ? QColor(200, 255, 200)
                                  : QColor(255, 200, 200);
            for (int col = 0; col < 5; ++col) {
                transactionTable->item(i, col)->setBackground(rowColor);
            }
        }
    }


    // Edit/Delete tab
    if (editTable) {
        editTable->setRowCount(n);
        for (int i = 0; i < n; ++i) {
            const Transaction &t = transactions[static_cast<size_t>(i)];

            editTable->setItem(i, 0, new QTableWidgetItem(QString::fromStdString(t.date)));
            editTable->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(t.category)));
            editTable->setItem(i, 2, new QTableWidgetItem(QString::fromStdString(t.type)));
            editTable->setItem(i, 3, new QTableWidgetItem(QString::number(t.amount, 'f', 2)));
            editTable->setItem(i, 4, new QTableWidgetItem(QString::fromStdString(t.description)));
        }
    }
}


void MainWindow::refreshSummary() {
    double income = manager.totalIncome();
    double expense = manager.totalExpense();
    double bal = manager.balance();

    incomeLabel->setText(QString("Total Income: $%1").arg(income, 0, 'f', 2));
    expenseLabel->setText(QString("Total Expense: $%1").arg(expense, 0, 'f', 2));
    balanceLabel->setText(QString("Balance: $%1").arg(bal, 0, 'f', 2));

    if (bal >= 0) {
        balanceLabel->setStyleSheet(
            "font-size: 20px; color: green; font-weight: bold;");
    } else {
        balanceLabel->setStyleSheet(
            "font-size: 20px; color: red; font-weight: bold;");
    }
}

// ===== Save / Load =====

void MainWindow::loadData() {
    manager.loadFromFile(dataFile.toStdString());
}

void MainWindow::saveData() {
    manager.saveToFile(dataFile.toStdString());
}
