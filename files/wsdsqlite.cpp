#include "utility.hpp"
#include "mpi.h"
#include "zlib-1.2.8/zlib.h"
#include <cstdint>
#include <fstream>
#include "config.hpp"
#include <sqlite3.h> 
using namespace config;

std::unordered_map<string, std::pair<double,int>> stringtoscore_map;
static int callback(void *data, int argc, char **argv, char **azColName){
   //int i;
   //fprintf(stderr, "%s: ", (const char*)data);
   //for(i=0; i<argc; i++){
      //printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	 try {
        stringtoscore_map[argv[0]]=std::make_pair(stod(argv[1]),stoi(argv[2]));
    } catch (const std::invalid_argument&) {
        //cout <<"invalid_argument\n";
    }
   	
   //}
   //printf("\n");
   return 0;
}

void opendb(int &myrank)
{
   char *zErrMsg = 0;
   int  rc;
   string sql;

   /* Open database */
   string dbname = "score" + to_string(myrank)+".db";
   rc = sqlite3_open(dbname.c_str(), &db);
   if( rc ){
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
      return;
   }else{
      //fprintf(stdout, "Opened database successfully\n");
   }
   
}

void closedb()
{
    sqlite3_close(db);
}

//extern sqlite3 *db;
void insert_to_vv( std::set<string> uniquewords, string sentenceid, vector<vector<string>> &vv, int size)
{
	string sentence=sentenceid;
	for(auto &word: uniquewords)
		sentence+=" "+word;
	//cout << endl<<sentence<<endl;
	for(auto &word : uniquewords)
	{	
		vv[std::hash<string>()(word) % size].push_back(sentence);
	}

}
 void extract_words(char *str, int length, vector<vector<string>> &vv, int size)
 {
        
    char *buffer = str;
    // int myrank = MPI::COMM_WORLD.Get_rank(); 
    int current_index=0, index=0;
        
    std::set<string> uniquewords;
    int k=0;

    string sentenceid;
    while(str-buffer < length)
    {
        //index = str.find('\n',index+1);
        char * sentenceend = strchr(str+1,'\n');
        if(sentenceend==NULL) break;
        //if(*(sentenceend+1) == '\n') continue;
        
        string line = string(str, sentenceend-str);//str.substr(current_index,index - current_index);
        //cout << line<<endl;
        boost::char_separator<char> sep("\t");
        boost::tokenizer<boost::char_separator<char>> tokens(line, sep);
        int i=0; 
        string lemma;
        
        vector<string> vtokens;
        std::copy(tokens.begin(), tokens.end(), std::back_inserter(vtokens)); 
        if( vtokens.size() > 4 ){
			sentenceid = vtokens.back();        
			//cout << "Sentenceid: " << sentenceid << endl;
        }
        for ( const auto& token : tokens ) 
        {
            
            if(i==2) 
            {
                if(contains_punctordigit(token))
                    break;
                lemma.assign(token);
            }
            if(i==3)
            {
                if(token.compare("NN") == 0 || token.compare("ADJ") == 0)
                {
                    string uniqueword(lemma+token.at(0));
                    if(uniqueword.length()>STRING_LENGTH-2)break;
                    uniquewords.insert(uniqueword);
                }
            }
            i++;
            //if(i>3) break;
            //if(i==5).assign(token);
        }
        if((*(str+1)=='\n' ||index==length-1) && !uniquewords.empty())
        {   
            insert_to_vv(uniquewords, sentenceid, vv, size);  
            uniquewords.clear();                              
        }
        str=sentenceend;
    }
    str = buffer;
}

int get_max_pos(double * array, int size)
{
    double max=array[0];
    int max_pos=0;

    int i;
    for (i=1; i<size; i++)
    {
        if (max<array[i])
        {
            max=array[i];
            max_pos=i;
        }
    }

    return max_pos;
}

void wsd(string &str, vector<std::tuple<string, string, string>> &vtuplestowrite)
{
	boost::char_separator<char> sep(" ");
    boost::tokenizer<boost::char_separator<char>> tokens(str, sep);
    int i=0; 
    string sentenceid;
    vector<string> words;
    for (const auto& token : tokens) {
    	if(i==0)
    	{
    		sentenceid.assign(token);
    		i++;
    		continue;
    	}
    	words.push_back(token);
    }
    
    for(auto &word : words)
    {
	    	/* Create SQL statement */
    	stringtoscore_map.clear();
    	sqlite3_stmt *stmt;
		string sql = "SELECT cword,score,roothubnum from scoreroothub where tword = '" + word + "';";
		int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, NULL);
		if (rc != SQLITE_OK) {
		    //print("error: ", sqlite3_errmsg(db));
		    //return;
		}
		while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		    
		     try {
		        stringtoscore_map[string((const char *)sqlite3_column_text(stmt,0))]=std::make_pair(sqlite3_column_double(stmt,1),sqlite3_column_int(stmt,2));
		    } catch (const std::invalid_argument&) {
		        //cout <<"invalid_argument\n";
		    }
		}
		// if (rc != SQLITE_DONE) {
		//     print("error: ", sqlite3_errmsg(db));
		// }
		sqlite3_finalize(stmt);
	   // string sql = "SELECT * from scoreroothub where tword = '" + word + "';";

	   // /* Execute SQL statement */
	   // int rc = sqlite3_exec(db, sql.c_str(), callback, NULL, NULL);
	   // if( rc != SQLITE_OK ){
	   //    fprintf(stderr, "SQL error:\n");
	   //    //sqlite3_free(zErrMsg);
	   // }else{
	   //    //fprintf(stdout, "Operation done successfully\n");
	   // }
    	if(stringtoscore_map.size()!=0)
    	{
    		Graph *graph = stringtographmap[word];
    		int roothubcount = graph->roothubs.size();
    		if(roothubcount==0)continue;

    		double scorevector[roothubcount];
    		std::fill_n (scorevector, roothubcount, 0);


    		for(auto &word1 : words)
    		{
    			if(word1.compare(word)!=0)
    		 	{
    		 		if(stringtoscore_map.find(word1)!=stringtoscore_map.end())
    		 		{
    		 			auto score_roothubnum = stringtoscore_map[word1];
	    		 		//auto nodeptr = graph->stringtonode_map[word1];
	    		 		//double score = nodeptr->score;
	    		 		//int closeroothub = nodeptr->roothubnum;
	    		 		auto score = score_roothubnum.first;
	    		 		auto closeroothub = score_roothubnum.second;
	    		 		scorevector[closeroothub] += score;
    		 		}
    		 	}
    		}
    		int indexofmaxscore = get_max_pos(scorevector, roothubcount);
    		// int indexofmaxscore = distance(scorevector, max_element(scorevector, scorevector + roothubcount));
    		string wordsense = (graph->roothubs[indexofmaxscore])->str;
    		vtuplestowrite.push_back(std::make_tuple(sentenceid, word, wordsense)); 
    	}

    }

}

void disambiguate(int &myrank , int &size)
{
	opendb(myrank);
	sqlite3_exec(db, "BEGIN TRANSACTION", NULL, NULL, NULL);
	clock_t uncompress_clock = clock();
    double uncompress_time = 0;

	typedef uint8_t byte;
	//cout << "Came";
	vector<std::tuple<string, string, string>> vtuplestowrite;
	vector<vector<string>> vv(size);
	ofstream myfile;
  	myfile.open (outputfiles_location + "/wsdonefile"+to_string(myrank)+".txt");
	clock_t wsdtime = clock();
	cout << "P:" << myrank << " " << compressedvector.size()<<endl;
	for(auto &compressedentry : compressedvector)
	{

		auto compressedstr = std::get<0>(compressedentry);

		auto compressedlen = std::get<1>(compressedentry);
		//cout <<" P:" << myrank << " compressedsize: " << compressedlen << endl;
		auto originallen = std::get<2>(compressedentry);
		//if(myrank == 0) cout <<"LISTEN: " << compressedstr[0] << ":" << compressedstr[1] << ":" << compressedstr[2] << ":" <<compressedlen << " " << originallen << endl;

		unsigned char *uncompressbuf = new unsigned char[(int)originallen ];
		long unsigned int uncompresslength = 0;

		if (ALLOW_TIME_LOGGING == 1)
            uncompress_clock = clock();
        
		int result = uncompress(uncompressbuf, &originallen, compressedstr, compressedlen);

		if (ALLOW_TIME_LOGGING == 1)
            uncompress_time += (clock()-uncompress_clock)/(double) CLOCKS_PER_SEC;
        
		//cout <<" P:" << myrank << " uncompressedsize: " << originallen << endl;
		
		//if(myrank == 0) cout << "P:" << myrank << endl << string(outstring) << endl;
		MPI::COMM_WORLD.Barrier();
		delete[] compressedstr;
		extract_words((char*)uncompressbuf, originallen, vv, size);

		char *recvdata = new char[1];
		delete[] uncompressbuf;
		int totalrecvsize=0, totalrecv_strings=0;
		

		for( int i=0 ; i < size ; i++ )
		{
			int sendsize = 0;
			char* sendbuffer = new char[1];
			int sendwords=0;
			if(myrank != i)
			{
				sendwords=vv[i].size();
				sendsize=vv[i].size()*sizeof(int);// + sizeof(int);
				for(auto &entry : vv[i])
					sendsize+=entry.size()+1;

				sendbuffer = new char[ sendsize ];
        		char* memptr = sendbuffer;
        		// int N = vv[i].size();
        		// memcpy( memptr , &N , sizeof(int));
        		// memptr += sizeof(int);
        		for( auto &entry : vv[i] )
		        {
		        	
		        	int b = entry.size()+1;
		            memcpy( memptr , &b , sizeof(int));
		            memptr += sizeof(int);
		            
		            memcpy( memptr , entry.c_str() , entry.size() + 1);            
		            memptr += entry.size()+1;
		        }
			}
			int *allocsizes= new int[size];
			MPI_Reduce(&sendwords ,    &totalrecv_strings,    1,    MPI_INT,    MPI_SUM,  i,    MPI_COMM_WORLD);
        	MPI_Gather(&sendsize,  1,  MPI_INT,  allocsizes,  1,  MPI_INT,  i,  MPI_COMM_WORLD);

        	int *allocdisplacements = new int[size];
	        allocdisplacements[0] = 0;
	        for( int p=0 ; p < size ; p++ )
	        {
	            allocdisplacements[p]=allocdisplacements[p-1]+allocsizes[p-1];
	        }
	         
	        
	        if ( myrank == i )
	        {
	        	totalrecvsize = allocdisplacements[size-1]+ allocsizes[size-1];
	            recvdata = new char[totalrecvsize];
	        }
	        
	        MPI_Gatherv(sendbuffer, sendsize, MPI_CHAR, recvdata,  allocsizes, allocdisplacements, MPI_CHAR , i ,  MPI_COMM_WORLD);
	        
	        delete[](sendbuffer);

		}
		char *memptr = recvdata;
		for(int p = 0 ; p < size ; p++ )
		{
			if(myrank != p)
				vv[p].clear();
		}
		// cout << totalrecv_strings<<endl;
		//if(myrank == 0)
		// for(int j =0; j< 200 ; j ++)
		// {
		// 	cout << recvdata[j];
		// }
			
		// 	cout << endl;
		//if(myrank==0)
		while(totalrecv_strings-- )
		{
			if(memptr - recvdata  > totalrecvsize) break;
			// int count;
			// memcpy( &count , memptr , sizeof(int) );
   //      	memptr += sizeof(int);
   //      	if(myrank==0)cout << "|"<<count<<"|";
   //      	for(int j = 0 ; j < count ; j++ )
   //      	{
        		int strlen;
				memcpy( &strlen , memptr , sizeof(int) );
	        	memptr += sizeof(int);
	        	//cout<< "|" << strlen << "|";

	        	char *str = new char[strlen];
	        	memcpy(str, memptr, strlen);
	        	memptr+=strlen;
	         	//cout << "|" << str << endl;
	         	vv[myrank].push_back(str);
	         	delete []str;
        	//}
		}
		for(auto &sentence : vv[myrank])
		{			
			wsd(sentence, vtuplestowrite);
		}
		delete[] recvdata;
		// //write to file;
		for(auto &entry: vtuplestowrite)
		{
			
			if(WRITE_WSD_TO_FILE == 1)
                myfile << std::get<0>(entry)<<" : " << std::get<1>(entry)<<" : " << std::get<2>(entry)<<"\n";
            else
                cout << std::get<0>(entry)<<" : " << std::get<1>(entry)<<" : " << std::get<2>(entry)<<"\n";
		}
	}
	
	if (ALLOW_TIME_LOGGING == 1)
        if (WRITE_OUTPUT_TO_FILE == 1)
            outputstream <<"P:" << myrank << ". Uncompress time: " << uncompress_time <<endl;
        else   
            cout <<"P:" << myrank << ". Uncompress time: " << uncompress_time << endl;        
    

    if (ALLOW_TIME_LOGGING == 1)
        if (WRITE_OUTPUT_TO_FILE == 1)
            outputstream <<"P:" << myrank <<"WSD Time" << (clock()-wsdtime)/(double) CLOCKS_PER_SEC << "\n";
        else   
            cout <<"P:" << myrank <<"WSD Time" << (clock()-wsdtime)/(double) CLOCKS_PER_SEC << "\n";
       
    
    //if( myrank == size-1 )cout<<endl<<"Time taken for wsd" << (clock()-wsdtime)/(double) CLOCKS_PER_SEC << "\n";
    sqlite3_exec(db, "END TRANSACTION", NULL, NULL, NULL);    
    closedb();
	myfile.close();
}

