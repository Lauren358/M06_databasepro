#include "sqlite3.h"
#include <iostream>
#include <string>
#include <iomanip>
#include <climits>

using namespace std;

/* Program name: Prepared Statements
*  Author: Lauren Davis
*  Date last updated: 5/9/2026
* Purpose: With this program the user is able to view information from the Sakila database. The user can choose to view customer information or view the rentals for a customer. If the user wishes to exit the program from the starting page they type in -1, otherwise -1 will have the user go back a page. If they want to the next page they enter 0. The user is also able to chose how many results they want to see per page and which customer's rental they would like to view.
*/
//Note: AI was used to ensure there were no synatx errors 

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
			case 1: viewRental(mydb); break;
			case 2: viewCustomer(mydb); break;
			case -1: return 0;
			default: cout << "That is not a valid choice." << endl;
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
			cout << "That is not a valid choice! Try again!" << endl;
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
				cout << "That is not a valid choice! Try again!" << endl;
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
{  
    int totalRows = 0;
    int result;
    int menuNum = 1;
    int choice;
    
    
    string query =
        "SELECT customer_id, last_name, first_name "
        "FROM customer "
        "ORDER BY last_name, first_name";

    sqlite3_stmt *pRes;

    string errorMessage;

    // Prepare first query
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &pRes, NULL) != SQLITE_OK)
    {
        errorMessage = sqlite3_errmsg(db);

        cout << "There was an error: "
             << errorMessage << endl;

        sqlite3_finalize(pRes);

        return;
    }



    // Count rows
    do
    {
        result = sqlite3_step(pRes);

        if (result == SQLITE_ROW)
            totalRows++;

    } while (result == SQLITE_ROW);

    sqlite3_reset(pRes);

    cout << "\nPlease choose the customer you want to see:\n" << endl;

    

    // Display customers
    while ((result = sqlite3_step(pRes)) == SQLITE_ROW)
    {
        cout << menuNum << ". ";

        cout << sqlite3_column_text(pRes, 0) << " - ";

        cout << sqlite3_column_text(pRes, 1) << ", ";

        cout << sqlite3_column_text(pRes, 2) << endl;

        menuNum++;
    }

   

    cout << "\nChoose a customer: ";
    cin >> choice;

    // Input validation
    while (!cin || choice < 1 || choice > totalRows)
    {
        if (!cin)
        {
            cin.clear();
            cin.ignore(INT_MAX, '\n');
        }

        cout << "That is not a valid choice." << endl;
        cout << "Choose a customer: ";

        cin >> choice;
    }

    // Reset statement
    sqlite3_reset(pRes);

    // Move to chosen row
    for (int i = 0; i < choice; i++)
    {
        sqlite3_step(pRes);
    }

    string customerID =
        reinterpret_cast<const char *>(sqlite3_column_text(pRes, 0));

    sqlite3_finalize(pRes);

    // Query customer details
    string detailQuery =
        "SELECT c.first_name, "
        "c.last_name, "
        "a.phone, "
        "a.address, "
        "city.city, "
        "c.email, "
        "c.last_update "
        "FROM customer c "
        "JOIN address a "
        "ON c.address_id = a.address_id "
        "JOIN city "
        "ON a.city_id = city.city_id "
        "WHERE c.customer_id = ?";

    sqlite3_stmt *detailStmt;

    // Prepare detail query
    if (sqlite3_prepare_v2(db,
                           detailQuery.c_str(),
                           -1,
                           &detailStmt,
                           NULL) != SQLITE_OK)
    {
        errorMessage = sqlite3_errmsg(db);

        cout << "There was an error: "
             << errorMessage << endl;

        sqlite3_finalize(detailStmt);

        return;
    }

    // Bind parameter
    sqlite3_bind_int(detailStmt, 1, stoi(customerID));

    // Execute detail query
    if (sqlite3_step(detailStmt) == SQLITE_ROW)
    {
        string firstName =
            reinterpret_cast<const char *>(
                sqlite3_column_text(detailStmt, 0));

        string lastName =
            reinterpret_cast<const char *>(
                sqlite3_column_text(detailStmt, 1));

        string phone =
            reinterpret_cast<const char *>(
                sqlite3_column_text(detailStmt, 2));

        string address =
            reinterpret_cast<const char *>(
                sqlite3_column_text(detailStmt, 3));

        string city =
            reinterpret_cast<const char *>(
                sqlite3_column_text(detailStmt, 4));

        string email =
            reinterpret_cast<const char *>(
                sqlite3_column_text(detailStmt, 5));


        string lastUpdate =
            reinterpret_cast<const char *>(
                sqlite3_column_text(detailStmt, 6));

        cout << "\n----Customer Information----\n" << endl;

        cout << "Name: "
             << firstName << " "
             << lastName << endl;

        cout << "Address: "
             << address << ", "
             << city << endl;
             
        cout << "Phone: "
             << phone << endl;

        cout << "Email: "
             << email << endl;

        cout << "Last Update: "
             << lastUpdate << endl;
    }

    sqlite3_finalize(detailStmt);
}
