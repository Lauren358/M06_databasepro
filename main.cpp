#include "sqlite3.h"
#include <iostream>
#include <string>
#include <iomanip>
#include <climits>

using namespace std;

/* Program name: Sakila revisited
*  Author: Lauren Davis
*  Date last updated: 5/9/2026
* Purpose: With this program the user is able to view information from the Sakila database. The user can choose to view customer information or view the rentals for a customer. If the user wishes to exit the program from the starting page they type in -1, otherwise -1 will have the user go back a page. If they want to the next page they enter 0. The user is also able to chose how many results they want to see per page and which customer's rental they would like to view.
*/
//Note: AI was used to ensure there were no synatx errors and formatting is done correctly 

void printMainMenu();
void viewRental(sqlite3 *);
void viewCustomer(sqlite3 *);
int mainMenu();
void printCustomerPage(sqlite3_stmt *, int, int);
void printRentalPage(sqlite3_stmt *, int, int);
	
int main()
{
	int choice;

	sqlite3 *mydb;

	int rc; 

	  // Opens database
    rc = sqlite3_open("sakila.db", &mydb);

    if (rc != SQLITE_OK)
    {
        cout << "Error opening database: "
             << sqlite3_errmsg(mydb) << endl;

        return 1;
    }

	
	cout << "Welcome to Sakila" << endl;
	choice = mainMenu();
	while (true)
	{
		switch (choice) 
		{
			case 1: 
			    viewRental(mydb); 
			    break;
			
			
			case 2: 
			    viewCustomer(mydb);
			    break;
			
			
			case 3: 
			    addRental(mydb);
			    break;
			
			//closes the database and exits program
			case -1: 
			    sqlite3_close(mydb);
			    return 0;
			
			default: 
			    cout << "That is not a valid choice." << endl;
		}
		cout << "\n\n";
		choice = mainMenu();
	}
	
} 

void printMainMenu()
{
	cout << "Please choose an option (enter -1 to quit):  " << endl;
	cout << "1. View the rentals for a customer" << endl;
	cout << "2. View Customer Information" << endl;
	cout << "Enter Choice: ";
}

int mainMenu()
{
	int choice = 0;

	printMainMenu();
	cin >> choice;
	while ((!cin || choice < 1 || choice > 3) && choice != -1)
	{
		if (!cin)
		{
			cin.clear();
			cin.ignore(INT_MAX, '\n');
		}
		cout << "That is not a valid choice." << endl
			 << endl;
		printMainMenu();
		cin >> choice;
	}
	return choice;
}

void viewRental(sqlite3 *db)
{
	string query = "SELECT customer_id, first_name, last_name FROM customer ";
	sqlite3_stmt *pRes;
	string m_strLastError;
	string query2;
	string cusID;
	string cus_fname, cus_lname;
	if (sqlite3_prepare_v2(db, query.c_str(), -1, &pRes, NULL) != SQLITE_OK)
	{
		m_strLastError = sqlite3_errmsg(db);
		sqlite3_finalize(pRes);
		cout << "There was an error: " << m_strLastError << endl;
		return;
	}
	else
	{
		int columnCount = sqlite3_column_count(pRes);
		int i = 0, choice = 0, rowsPerPage, totalRows;
		sqlite3_stmt *pRes2;
		cout << left;
		int res;
		do
		{
			res = sqlite3_step(pRes);
			i++;

		} while (res == SQLITE_ROW);
		totalRows = i - 1;
		sqlite3_reset(pRes);
		cout << "There are " << i - 1 << " rows in the result.  How many do you want to see per page?" << endl;
		cin >> rowsPerPage;
		while (!cin || rowsPerPage < 0)
		{
			if (!cin)
			{
				cin.clear();
				cin.ignore(INT_MAX, '\n');
			}
			cout << "That is not a valid choice, try again." << endl;
			cout << "There are " << i << " rows in the result.  How many do you want to see per page?" << endl;
		}
		if (rowsPerPage > i)
			rowsPerPage = i;
		i = 0;

		while (choice == 0 || choice == -1)
		{
			if (i == 0)
				cout << "Please choose the customer you want to see rentals for (enter 0 to go to the next page):" << endl;
			else if (i + rowsPerPage < totalRows)
				cout << "Please choose the customer you want to see rentals for (enter 0 to go to the next page or -1 to go to the previous page):" << endl;
			else
				cout << "Please choose the customer you want to see rentals for (enter -1 to go to the previous page):" << endl;
			printCustomerPage(pRes, rowsPerPage, i);
			cin >> choice;
		
			while (!(cin) || choice < -1 || choice > totalRows)
			{
				if (!cin)
				{
					cin.clear();
					cin.ignore(INT_MAX, '\n');
				}
				cout << "That is not a valid choice, try again." << endl;
				cin >> choice;
			}
			if (choice == 0)
			{
				i = i + rowsPerPage;

				if (i >= totalRows)
				{
					i = totalRows - rowsPerPage;
					sqlite3_reset(pRes);
					for (int j = 0; j < i; j++)
					{
						sqlite3_step(pRes);
					}
				}
			}
			else if (choice == -1)
			{
				i = i - rowsPerPage;
				if (i < 0)
					i = 0;
				sqlite3_reset(pRes);
				for (int j = 0; j < i; j++)
					sqlite3_step(pRes);
			}
		}
		sqlite3_reset(pRes);
		for (int i = 0; i < choice; i++)
			sqlite3_step(pRes);
		cusID = reinterpret_cast<const char *>(sqlite3_column_text(pRes, 0));
		cus_fname = reinterpret_cast<const char *>(sqlite3_column_text(pRes, 1));
		cus_lname = reinterpret_cast<const char *>(sqlite3_column_text(pRes, 2));
		sqlite3_finalize(pRes);
		query2 = "select rental_id, rental_date, return_date, staff.first_name || ' ' || staff.last_name as 'Staff Name', ";
		query2 += "film.title, film.description, film.rental_rate ";
		query2 += "from rental join staff on rental.staff_id = staff.staff_id ";
		query2 += "join inventory on rental.inventory_id = inventory.inventory_id ";
		query2 += "join film on film.film_id = inventory.film_id ";
		query2 += "where customer_id = " + cusID;

		if (sqlite3_prepare_v2(db, query2.c_str(), -1, &pRes2, NULL) != SQLITE_OK)
		{
			m_strLastError = sqlite3_errmsg(db);
			sqlite3_finalize(pRes2);
			cout << "There was an error: " << m_strLastError << endl;
			return;
		}
		else
		{
			columnCount = sqlite3_column_count(pRes);
			i = 0;
			choice = 0;

			do
			{
				res = sqlite3_step(pRes2);
				i++;

			} while (res == SQLITE_ROW);
			totalRows = i;
			sqlite3_reset(pRes2);
			cout << "There are " << i << " rows in the result.  How many do you want to see per page?" << endl;
			cin >> rowsPerPage;
			while (!cin || rowsPerPage < 0)
			{
				if (!cin)
				{
					cin.clear();
					cin.ignore(INT_MAX, '\n');
				}
				cout << "That is not a valid choice! Try again!" << endl;
				cout << "There are " << i << " rows in the result.  How many do you want to see per page?" << endl;
			}
			if (rowsPerPage > i)
				rowsPerPage = i;
			i = 0;

			while (choice == 0 || choice == -1)
			{
				if (i == 0)
					cout << "Please choose the rental you want to see (enter 0 to go to the next page):" << endl;
				else if (i + rowsPerPage < totalRows)
					cout << "Please choose the rental you want to see (enter 0 to go to the next page or -1 to go to the previous page):" << endl;
				else
					cout << "Please choose the rental you want to see (enter -1 to go to the previous page):" << endl;
				printRentalPage(pRes2, rowsPerPage, i);
				cin >> choice;
			
				while (!(cin) || choice < -1 || choice > totalRows)
				{
					if (!cin)
					{
						cin.clear();
						cin.ignore(INT_MAX, '\n');
					}
					cout << "That is not a valid choice! Try again!" << endl;
					cin >> choice;
				}
				if (choice == 0)
				{
					i = i + rowsPerPage;
					if (i >= totalRows)
					{
						i = totalRows - rowsPerPage;
						sqlite3_reset(pRes2);
						for (int j = 0; j < i; j++)
							sqlite3_step(pRes2);
					}
				}
				else if (choice == -1)
				{
					i = i - rowsPerPage;
					if (i < 0)
						i = 0;
					sqlite3_reset(pRes2); 
					for (int j = 0; j < i; j++)
						sqlite3_step(pRes2);
				}
			}
			sqlite3_reset(pRes2);
			for (int i = 0; i < choice; i++)
				sqlite3_step(pRes2);
		}
		string rentalID = reinterpret_cast<const char *>(sqlite3_column_text(pRes2, 0));
		string rentalDate = reinterpret_cast<const char *>(sqlite3_column_text(pRes2, 1));
		string returnDate;
		if(sqlite3_column_type(pRes2,2) != SQLITE_NULL)
			returnDate = reinterpret_cast<const char *>(sqlite3_column_text(pRes2, 2));
		else 
			returnDate = "";
		string staff = reinterpret_cast<const char *>(sqlite3_column_text(pRes2, 3));
		string filmTitle = reinterpret_cast<const char *>(sqlite3_column_text(pRes2, 4));
		string filmdescription = reinterpret_cast<const char *>(sqlite3_column_text(pRes2, 5));
		string rentalRate = reinterpret_cast<const char *>(sqlite3_column_text(pRes2, 6));
		cout << showpoint << fixed << setprecision(2);
		cout << "Rental Date: " << rentalDate << endl;
		cout << "Staff: " << staff << endl;
		cout << "Customer: " << cus_fname << " " << cus_lname << endl;
		cout << "Film Information:" << endl;
		cout << filmTitle << " - " << filmdescription << " $" << rentalRate << endl;
		cout << "Return Date: " << returnDate << endl;
		sqlite3_finalize(pRes2);
	}
}

void printCustomerPage(sqlite3_stmt *res, int rowsPerPage, int startNum)
{
	int stop, i = 1;
	do
	{
		stop = sqlite3_step(res);
		if (stop != SQLITE_ROW)
			break;
		cout << i + startNum << ". ";
		if (sqlite3_column_type(res, 0) != SQLITE_NULL)
			cout << sqlite3_column_text(res, 0) << " - ";
		if (sqlite3_column_type(res, 1) != SQLITE_NULL)
			cout << sqlite3_column_text(res, 1) << " ";
		if (sqlite3_column_type(res, 2) != SQLITE_NULL)
			cout << sqlite3_column_text(res, 2) << " ";
		cout << endl;
		i++;

	} while (i <= rowsPerPage);
}

void printRentalPage(sqlite3_stmt *res, int rowsPerPage, int startNum)
{
	int stop, i = 1;
	do
	{
		stop = sqlite3_step(res);
		if (stop != SQLITE_ROW)
			break;
		cout << i + startNum << ". ";
		if (sqlite3_column_type(res, 0) != SQLITE_NULL)
			cout << sqlite3_column_text(res, 0) << " - ";
		if (sqlite3_column_type(res, 1) != SQLITE_NULL)
			cout << sqlite3_column_text(res, 1) << " ";
		cout << endl;
		i++;

	} while (i <= rowsPerPage);
}
//starter code ends
//where the coding begins 
void viewCustomer(sqlite3 *db)
{ string query = "SELECT customer_id, last_name, first_name FROM customer ORDER BY last_name, first_name";
sqlite3_stmt *pRes;
string errorMessage;
cout << "\nPlease choose the customer you want to see:" << endl;
if (sqlite3_prepare_v2(db, query.c_str(), -1, &pRes, NULL) != SQLITE_OK)
{
errorMessage = sqlite3_errmsg(db);
cout << "There was an error: " << errorMessage << endl;
sqlite3_finalize(pRes);
return;
}
// count rows
int i = 0, res;
do { res = sqlite3_step(pRes); i++; } while (res == SQLITE_ROW);
int totalRows = i - 1;
sqlite3_reset(pRes);
cout << "There are " << totalRows << " rows in the result.  How many do you want to see per page?" << endl;
int rowsPerPage;
cin >> rowsPerPage;
while (!cin || rowsPerPage < 0)
{
if (!cin) { cin.clear(); cin.ignore(INT_MAX, '\n'); }
cout << "That is not a valid choice! Try again!" << endl;
cout << "There are " << totalRows << " rows in the result.  How many do you want to see per page?" << endl;
cin >> rowsPerPage;
}
if (rowsPerPage > totalRows) rowsPerPage = totalRows;
i = 0;
int choice = 0;
// paged menu
while (choice == 0 || choice == -1)
{
if (i == 0)
cout << "Please choose the customer you want to see rentals for (enter 0 to go to the next page):" << endl;
else if (i + rowsPerPage < totalRows)
cout << "Please choose the customer you want to see rentals for (enter 0 to go to the next page or -1 to go to the previous page):" << endl;
else
cout << "Please choose the customer you want to see rentals for (enter -1 to go to the previous page):" << endl;
printCustomerPage(pRes, rowsPerPage, i);
cin >> choice;
while (!cin || choice < -1 || choice > totalRows)
{
if (!cin) { cin.clear(); cin.ignore(INT_MAX, '\n'); }
cout << "That is not a valid choice! Try again!" << endl;
cin >> choice;
}
if (choice == 0)
{
i += rowsPerPage;
if (i >= totalRows)
{
i = totalRows - rowsPerPage;
sqlite3_reset(pRes);
for (int j = 0; j < i; j++) sqlite3_step(pRes);
}
}
else if (choice == -1)
{
i -= rowsPerPage;
if (i < 0) i = 0;
sqlite3_reset(pRes);
for (int j = 0; j < i; j++) sqlite3_step(pRes);
}
}
sqlite3_reset(pRes);
for (int j = 0; j < choice; j++) sqlite3_step(pRes);
string customerID = reinterpret_cast<const char *>(sqlite3_column_text(pRes, 0));
sqlite3_finalize(pRes);
// get full customer details using prepared statement
string detailQuery =
"SELECT c.first_name, c.last_name, a.address, ci.city, a.district, a.postal_code, "
"a.phone, c.email, c.active, c.last_update "
"FROM customer c "
"JOIN address a ON c.address_id = a.address_id "
"JOIN city ci ON a.city_id = ci.city_id "
"WHERE c.customer_id = ?";
sqlite3_stmt *detailStmt;
if (sqlite3_prepare_v2(db, detailQuery.c_str(), -1, &detailStmt, NULL) != SQLITE_OK)
{
errorMessage = sqlite3_errmsg(db);
cout << "There was an error: " << errorMessage << endl;
sqlite3_finalize(detailStmt);
return;
}
sqlite3_bind_int(detailStmt, 1, stoi(customerID));
if (sqlite3_step(detailStmt) == SQLITE_ROW)
{
string fname      = reinterpret_cast<const char *>(sqlite3_column_text(detailStmt, 0));
string lname      = reinterpret_cast<const char *>(sqlite3_column_text(detailStmt, 1));
string address    = reinterpret_cast<const char *>(sqlite3_column_text(detailStmt, 2));
string city       = reinterpret_cast<const char *>(sqlite3_column_text(detailStmt, 3));
string district   = reinterpret_cast<const char *>(sqlite3_column_text(detailStmt, 4));
string postal     = reinterpret_cast<const char *>(sqlite3_column_text(detailStmt, 5));
string phone      = reinterpret_cast<const char *>(sqlite3_column_text(detailStmt, 6));
string email      = reinterpret_cast<const char *>(sqlite3_column_text(detailStmt, 7));
int active        = sqlite3_column_int(detailStmt, 8);
string lastUpdate = reinterpret_cast<const char *>(sqlite3_column_text(detailStmt, 9));
cout << "----Customer Information----" << endl;
cout << "Name: " << fname << " " << lname << endl;
cout << "Address: " << address << endl;
cout << city << ", " << district << " " << postal << endl;
cout << "Phone Number: " << phone << endl;
cout << "Email: " << email << endl;
cout << "Active: " << (active ? "Yes" : "No") << endl;
cout << "Last Update: " << lastUpdate << endl;
}
    sqlite3_finalize(detailStmt);
}

//user can add rental
void addRental(sqlite3 *db)
{
    sqlite3_stmt *stmt;

    int choice;
    int customerID;
    int inventoryID;
    int staffID;

    int totalRows = 0;
    int result;

    string query;

    cout << "\n---- Add Rental ----\n" << endl;

    // customer menu 
query =
    "SELECT customer_id, first_name, last_name "
    "FROM customer "
    "ORDER BY first_name, last_name";

if (sqlite3_prepare_v2(db,
                       query.c_str(),
                       -1,
                       &stmt,
                       NULL) != SQLITE_OK)
{
    cout << "Error preparing customer query." << endl;
    return;
}

int rowsPerPage;
int start = 0;

totalRows = 0;

while ((result = sqlite3_step(stmt)) == SQLITE_ROW)
    totalRows++;

sqlite3_reset(stmt);

cout << "There are "
     << totalRows
     << " customers. How many would you like to see per page? ";

cin >> rowsPerPage;

while (!cin || rowsPerPage <= 0)
{
    if (!cin)
    {
        cin.clear();
        cin.ignore(INT_MAX, '\n');
    }

    cout << "Invalid choice. Try again: ";
    cin >> rowsPerPage;
}

choice = 0;

while (choice == 0)
{
    sqlite3_reset(stmt);

    for (int i = 0; i < start; i++)
        sqlite3_step(stmt);

    cout << "\nPlease choose the customer for the rental (enter 0 to go to the next page): "<< endl;

    for (int i = 1; i <= rowsPerPage; i++)
    {
        result = sqlite3_step(stmt);

        if (result != SQLITE_ROW)
            break;

        cout << i + start << ". ";

        cout << sqlite3_column_int(stmt, 0)
             << " - ";

        cout << sqlite3_column_text(stmt, 1)
             << " ";

        cout << sqlite3_column_text(stmt, 2)
             << endl;
    }

    cout << "\nEnter choice: ";
    cin >> choice;

    while (!cin || choice < 0 || choice > totalRows)
    {
        if (!cin)
        {
            cin.clear();
            cin.ignore(INT_MAX, '\n');
        }

        cout << "That is not a valid choice." << endl;
        cin >> choice;
    }

    if (choice == 0)
    {
        start += rowsPerPage;

        if (start >= totalRows)
            start = 0;
    }
}

sqlite3_reset(stmt);

for (int i = 1; i <= choice; i++)
    sqlite3_step(stmt);

customerID = sqlite3_column_int(stmt, 0);

sqlite3_finalize(stmt);

    // movie menu

    
totalRows = 0;
start = 0;

query =
    "SELECT inventory.inventory_id, film.title "
    "FROM inventory "
    "JOIN film "
    "ON inventory.film_id = film.film_id "
    "ORDER BY film.title";

sqlite3_prepare_v2(db,
                   query.c_str(),
                   -1,
                   &stmt,
                   NULL);

while ((result = sqlite3_step(stmt)) == SQLITE_ROW)
    totalRows++;

sqlite3_reset(stmt);

cout << "\nThere are "
     << totalRows
     << " movies. How many would you like to see per page? ";

cin >> rowsPerPage;

while (!cin || rowsPerPage <= 0)
{
    if (!cin)
    {
        cin.clear();
        cin.ignore(INT_MAX, '\n');
    }

    cout << "Invalid choice. Try again: ";
    cin >> rowsPerPage;
}

choice = 0;

while (choice == 0)
{
    sqlite3_reset(stmt);

    for (int i = 0; i < start; i++)
        sqlite3_step(stmt);

    cout << "\nPlease choose the film you want to rent (enter 0 to go to the next page): "<< endl;

    for (int i = 1; i <= rowsPerPage; i++)
    {
        result = sqlite3_step(stmt);

        if (result != SQLITE_ROW)
            break;

        cout << i + start << ". ";

        cout << sqlite3_column_int(stmt, 0)
             << " - ";

        cout << sqlite3_column_text(stmt, 1)
             << endl;
    }

    cout << "\nEnter choice: ";
    cin >> choice;

    while (!cin || choice < 0 || choice > totalRows)
    {
        if (!cin)
        {
            cin.clear();
            cin.ignore(INT_MAX, '\n');
        }

        cout << "That is not a valid choice." << endl;
        cin >> choice;
    }

    if (choice == 0)
    {
        start += rowsPerPage;

        if (start >= totalRows)
            start = 0;
    }
}

sqlite3_reset(stmt);

for (int i = 1; i <= choice; i++)
    sqlite3_step(stmt);

inventoryID = sqlite3_column_int(stmt, 0);

sqlite3_finalize(stmt);

    // staff menu

    totalRows = 0;

    query =
        "SELECT staff_id, first_name, last_name "
        "FROM staff ";

    sqlite3_prepare_v2(db,
                       query.c_str(),
                       -1,
                       &stmt,
                       NULL);

    cout << "\nChoose a staff member:\n" << endl;

    while ((result = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        totalRows++;

        cout << totalRows << ". ";

        cout << sqlite3_column_int(stmt, 0)
             << " - ";

        cout << sqlite3_column_text(stmt, 1)
             << " ";

        cout << sqlite3_column_text(stmt, 2)
             << endl;
    }

    cout << "\nEnter choice: ";
    cin >> choice;

    while (!cin || choice < 1 || choice > totalRows)
    {
        if (!cin)
        {
            cin.clear();
            cin.ignore(INT_MAX, '\n');
        }

        cout << "That is not a valid choice." << endl;
        cout << "Enter choice: ";
        cin >> choice;
    }

    sqlite3_reset(stmt);

    for (int i = 0; i < choice; i++)
        sqlite3_step(stmt);

    staffID = sqlite3_column_int(stmt, 0);

    sqlite3_finalize(stmt);

    // begin transaction

    char *errMsg = nullptr;

    sqlite3_exec(db,
                 "BEGIN TRANSACTION;",
                 NULL,
                 NULL,
                 &errMsg);

    // insert rental

    string rentalQuery =
        "INSERT INTO rental "
        "(rental_date, inventory_id, customer_id, "
        "return_date, staff_id, last_update) "
        "VALUES(datetime('now'), ?, ?, NULL, ?, datetime('now'));";

    sqlite3_stmt *rentalStmt;

    sqlite3_prepare_v2(db,
                       rentalQuery.c_str(),
                       -1,
                       &rentalStmt,
                       NULL);

    sqlite3_bind_int(rentalStmt, 1, inventoryID);
    sqlite3_bind_int(rentalStmt, 2, customerID);
    sqlite3_bind_int(rentalStmt, 3, staffID);

    if (sqlite3_step(rentalStmt) != SQLITE_DONE)
    {
        cout << "Rental insert failed." << endl;

        sqlite3_exec(db,
                     "ROLLBACK;",
                     NULL,
                     NULL,
                     &errMsg);

        sqlite3_finalize(rentalStmt);

        return;
    }

    sqlite3_finalize(rentalStmt);

    // get rental Id

    int rentalID = sqlite3_last_insert_rowid(db);

    // insert payment

    string paymentQuery =
        "INSERT INTO payment "
        "(customer_id, staff_id, rental_id, amount, payment_date) "
        "VALUES(?, ?, ?, 4.99, datetime('now'));";

    sqlite3_stmt *paymentStmt;

    sqlite3_prepare_v2(db,
                       paymentQuery.c_str(),
                       -1,
                       &paymentStmt,
                       NULL);

    sqlite3_bind_int(paymentStmt, 1, customerID);
    sqlite3_bind_int(paymentStmt, 2, staffID);
    sqlite3_bind_int(paymentStmt, 3, rentalID);

    if (sqlite3_step(paymentStmt) != SQLITE_DONE)
    {
        cout << "Payment insert failed." << endl;

        sqlite3_exec(db,
                     "ROLLBACK;",
                     NULL,
                     NULL,
                     &errMsg);

        sqlite3_finalize(paymentStmt);

        return;
    }

   sqlite3_finalize(paymentStmt);

// get payment id

int paymentID = sqlite3_last_insert_rowid(db);

// commit transaction

sqlite3_exec(db,
             "COMMIT;",
             NULL,
             NULL,
             &errMsg);

cout << "\nRental and Payment entered successfully.\n" << "Rental Id: " << rentalID << endl;

cout << "Payment ID: "
     << paymentID << endl;
}
