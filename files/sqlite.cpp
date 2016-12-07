#include <sqlite3.h>
#include<iostream>
#include<string>
#include<vector>
using namespace std;
sqlite3 *db;
int main()
{
   char *zErrMsg = 0;
   int  rc;
   string sql;
	 vector <string> twords;
   /* Open database */
   int myrank =0;
   string dbname = "score0.db";
   rc = sqlite3_open(dbname.c_str(), &db);
   if( rc ){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      //return ;
   }else{
      fprintf(stdout, "Opened database successfully\n");
   }




sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, NULL);
sqlite3_stmt *stmt;
    sql = "SELECT cword,score,roothubnum from scoreroothub;";
    rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        //print("error: ", sqlite3_errmsg(db));
        //return;
    }
   for(int j=0;j<700;j++){
  
    int i=0;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        
         //try {
            twords.push_back((const char *)sqlite3_column_text(stmt,0));
            i++;
        //} catch () {
            //cout <<"invalid_argument\n";
        //}
    }
    cout << i << " rows inserted. \n";
    // if (rc != SQLITE_DONE) {
    //     print("error: ", sqlite3_errmsg(db));
    // }
    
  }
  sqlite3_finalize(stmt); 
  sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL);
    sqlite3_close(db);
}
