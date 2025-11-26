#include "BudgetManager.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
using namespace std;

// Constructor, initializes fixed categories
BudgetManager::BudgetManager()
{
    categoryList =
        {
            // Income
            "Income", "Savings", "Refunds", "Gift", "Other (Income)",
            // Expenses
            "Rent", "Utilities", "Groceries", "Transportation",
            "Food (Dining Out)", "Entertainment", "Health", "Other (Expense)",
        };
}

void BudgetManager::addTransaction(const Transaction& t) {
    transactions.push_back(t);
}

void BudgetManager::deleteTransaction(int index)
{
    if (index < 0 || index >= static_cast<int>(transactions.size())) return;
    transactions.erase(transactions.begin() + index);
}

void BudgetManager::editTransaction(int index, const Transaction& t)
{
    if (index < 0 || index >= static_cast<int>(transactions.size())) return;
    transactions[index] = t;
}

void BudgetManager::listTransactions() const {
    cout << "\n--- Transactions ---\n";
    if (transactions.empty()) {
        cout << "No transactions yet.\n";
        return;
    }

    int index = 1;
    for (const auto& t : transactions) {
        cout << index << "). " << t.date << " | "
             << setw(10) << left << t.category
             << " | " << setw(8) << left << t.type
             << " | $" << fixed << setprecision(2) << t.amount
             << " | " << t.description << "\n";
        index++;
    }
}

const vector<Transaction>& BudgetManager::getTransactions() const
{
    return transactions;
}

// ===== Totals =====

double BudgetManager::totalIncome() const {
    double sum = 0;
    for (const auto& t : transactions)
        if (t.type == "Income") sum += t.amount;
    return sum;
}

double BudgetManager::totalExpense() const {
    double sum = 0;
    for (const auto& t : transactions)
        if (t.type == "Expense") sum += t.amount;
    return sum;
}

double BudgetManager::balance() const {
    return totalIncome() - totalExpense();
}

// ===== File I/O =====

void BudgetManager::saveToFile(const string& filename) const {
    ofstream file(filename);
    for (const auto& t : transactions)
        file << t.toCSV() << "\n";
}

void BudgetManager::loadFromFile(const string& filename) {
    ifstream file(filename);
    string line;
    while (getline(file, line)) {
        if (!line.empty())
            transactions.push_back(Transaction::fromCSV(line));
    }
}

// ===== Categories =====

void BudgetManager::printCategories() const
{
    cout << endl << "=== Categories ===" << endl;

    int width = 20;

    for (size_t i = 0; i < categoryList.size(); ++i)
    {
        cout << setw(2) << right << (i + 1) << "). "
             << left << setw(width) << categoryList[i];

        if ((i + 1) % 2 == 0) cout << endl;
    }
    if (categoryList.size() % 2 != 0) cout << endl;
}

string BudgetManager::getCategoryIndex(int i) const
{
    if (i < 0 || i >= static_cast<int>(categoryList.size())) {
        return "Other";
    }
    return categoryList[i];
}

int BudgetManager::getCategoryCount() const
{
    return static_cast<int>(categoryList.size());
}

// ===== “View by ...” search helpers =====

void BudgetManager::transactionsByCategory(const string& category) const
{
    cout << "=== Transactions in " << category << endl;
    bool found = false;

    for (const auto& t : transactions)
    {
        if (t.category == category)
        {
            found = true;
            cout << t.date << " | " << setw(10) << left << t.category << " | "
                 << setw(8) << left << t.type << " | " << fixed
                 << setprecision(2) << t.amount << " | "
                 << t.description << endl;
        }
    }
    if (!found)
    {
        cout << "No transactions in " << category << " found." << endl;
    }
}

void BudgetManager::transactionsByDay(const string& date) const
{
    cout << "=== Transactions on " << date << " ===" << endl;
    bool found = false;

    for (const auto& t: transactions)
    {
        if (t.date == date)
        {
            found = true;
            cout << t.date << " | " << setw(12) << left << t.category
                 << " | " << setw(8) << left << t.type << " | $"
                 << fixed << setprecision(2) << t.amount << " | "
                 << t.description << endl;
        }
    }

    if (!found)
    {
        cout << "No transactions found on " << date << endl;
    }
}

void BudgetManager::transactionsByMonth(int month, int year) const
{
    cout << "=== Transactions on " << month  << " " << year << " ===" << endl;
    bool found = false;

    for (const auto& t: transactions)
    {
        int m, d, y;
        istringstream iss(t.date);

        if (!(iss >> m >> d >> y)) continue;
        if (m == month && y == year)
        {
            found = true;
            cout << t.date << " | " << setw(12) << left << t.category
                 << " | " << setw(8) << left << t.type << " | $"
                 << fixed << setprecision(2) << t.amount << " | "
                 << t.description << endl;
        }
    }

    if (!found)
    {
        cout << "No transactions found on " << month << "/" << year << "." << endl;
    }
}

void BudgetManager::transactionsByYear(int year) const
{
    cout << "=== Transactions in " << year << " ===" << endl;
    bool found = false;

    for (const auto& t: transactions)
    {
        int m, d, y;
        istringstream iss(t.date);

        if (!(iss >> m >> d >> y)) continue;
        if (y == year)
        {
            found = true;
            cout << t.date << " | " << setw(12) << left << t.category
                 << " | " << setw(8) << left << t.type << " | $"
                 << fixed << setprecision(2) << t.amount << " | "
                 << t.description << endl;
        }
    }

    if (!found)
    {
        cout << "No transactions found on " << year << "." << endl;
    }
}

// ===== Budget limit functions =====

void BudgetManager::setMonthlyBudget(double limit)
{
    monthlyBudgetLimit = limit;
}

double BudgetManager::getMonthlyBudget() const
{
    return monthlyBudgetLimit;
}

void BudgetManager::setCategoryBudget(const string& category, double limit)
{
    categoryBudgets[category] = limit;
}

double BudgetManager::getCategoryBudget(const string& category) const
{
    auto it = categoryBudgets.find(category);
    if (it != categoryBudgets.end())
    {
        return it->second;
    }
    return 0.0;
}

// ===== Monthly helpers for pie chart, etc. =====

double BudgetManager::totalExpenseForMonth(int month, int year) const
{
    double sum = 0.0;

    for (const auto& t : transactions)
    {
        if (t.type != "Expense") continue;

        int m, d, y;
        istringstream iss(t.date);
        if (!(iss >> m >> d >> y)) continue;

        if (m == month && y == year)
        {
            sum += t.amount;
        }
    }
    return sum;
}

map<string, double> BudgetManager::expenseByCategoryForMonth(int month, int year) const
{
    map<string, double> result;

    // Initialize with categories (optional, keeps ordering stable)
    for (const auto& cat : categoryList)
    {
        result[cat] = 0.0;
    }

    for (const auto& t : transactions)
    {
        if (t.type != "Expense") continue;

        int m, d, y;
        istringstream iss(t.date);
        if (!(iss >> m >> d >> y)) continue;

        if (m == month && y == year)
        {
            result[t.category] += t.amount;
        }
    }

    return result;
}
