#include <iostream>
#include <string>
 #include "sqlite3.h"
#include <time.h>  
 using namespace std;

 int main (int argc, const char * argv[]) {
clock_t t=clock();
    sqlite3 *db;
    sqlite3_open("mydb.db", & db);

  string createQuery = "CREATE TABLE IF NOT EXISTS hpctext (firstword TEXT,secondword TEXT);";
    sqlite3_stmt *createStmt;
    cout << "Creating Table Statement" << endl;
    sqlite3_prepare(db, createQuery.c_str(), createQuery.size(), &createStmt, NULL);
    cout << "Stepping Table Statement" << endl;
    if (sqlite3_step(createStmt) != SQLITE_DONE) cout << "Didn't Create Table!" << endl;
int i=0;
    while(i<1000000){
	string insertQuery = "INSERT INTO hpctext VALUES ('viswanath is a boy in "+string(std::to_string(i))+"','tanya is his wife');"; // WORKS!
    sqlite3_stmt *insertStmt;
    //cout << "Creating Insert Statement" << endl;
    sqlite3_prepare(db, insertQuery.c_str(), insertQuery.size(), &insertStmt, NULL);
    //cout << "Stepping Insert Statement" << endl;
    //cout<<i<<" ";
    if(i%1000==0)
cout<<endl<<"Time taken for "<<i<<" insertions: " <<(clock()-t)/(double)CLOCKS_PER_SEC<<endl;
    if (sqlite3_step(insertStmt) != SQLITE_DONE) cout << "Didn't Insert Item!" << endl;  
i++;
 sqlite3_finalize(insertStmt);
}

cout<<endl<<"Time taken: " <<(clock()-t)/(double)CLOCKS_PER_SEC<<endl;
         return 0;
     }
