/*
*************************************************************************************************************************************
 * Single-Level Index program
 * Program simulates creation of a single level index for a provided data file.
 * Program operates in two modes based on first provided command line argument
 * -c indicates that the program should only creates an index
 * -l the other retrieves records from the original file in order using the created index file
 * Name of input text file is expected as second command line argument, file should be located in same directory as cpp file
 * Desired name for output index file is provided as third argument
 * Fourth command line argument indicates number of characters that comprise key used for indexing. Should be between 1 and 24 characters
 * Written by Stanley Varghese 
 ***********************************************************************************************************************************/

#include <iostream>
#include <fstream>
#include <vector> //Vectors provide dynamic array
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <algorithm>
#include <bitset>
using namespace std;

//Class for containing key & offset string variables. Also defines < operator for use in sorting algorithm
class Record
{
    public :
    string key, offset;
    
    Record(string k, string o)
    {
        key = k;
        offset = o;
    }

    //enable sorting vector of records based on key via std::sort. const added since function should not change object
    bool operator <(Record thisObj) const
    {
        return(key < thisObj.key);
    }
    
};

int main(int argc, const char * argv[]) {
    try
    {
        //Variables to house command line parameters for use in program
        string inputFileName;
        string outputFileName;
        int keyLength;
        
        //Conduct input validation to check if expected number of command line parameters were provided
        if(argc != 5)
        {
            cout << "Unexpected number of command line arguments, please try again " << endl;
            cout << "Argument count: " << argc << endl;
            return 0;
        }
        
        fstream myfile;
        int lineCount = 0;
        vector<Record> myVector;
        string programMode = argv[1];
        inputFileName = argv[2];
        outputFileName = argv[3];
        keyLength = atoi(argv[4]); //atoi returns int representation of provided string argument
        
        //Input validation to ensure provided keyLength is within expected range
        if(keyLength < 1 | keyLength > 24)
        {
            cout << "Invalid key length provided, please provide length between 1 and 24. " << endl;
            return 0;
        }
        
        //Input validation to check if provided program mode is valid
        if(programMode != "-c" && programMode != "-l")
        {
            cout << "Invalid program mode provided. Expected either -c or -l. " << endl;
            return 0;
        }
        
        //Attempting to open text file with provided name for input. File should be in same directory as executing code
        myfile.open(inputFileName, ios::in | ios::binary);
        if(myfile)
        {
            //Allocate enough memory for vector to store one Record object for each line found in input text file
            string currentLine;
            while(getline(myfile, currentLine))
            {
                lineCount++;
            }
            myVector.reserve(lineCount);
            
            /*seek back to start of file and read rows from text file into Record objects.
              Key is created via substring function based on desired keylength.
              Offset is determined via binary representation of value returned from tellg before a new line is read*/
            myfile.clear();
            myfile.seekg(0, ios::beg);
            for(int lineCounter = 0; lineCounter < lineCount; lineCounter++)
            {
                bitset<64> binaryOffset(myfile.tellg());
                string binary = binaryOffset.to_string();
                getline(myfile, currentLine);
                myVector.push_back(Record(currentLine.substr(0, keyLength), binary));
            }
                myfile.close();
        }
        else
        {
            cout << "Unable to open file for read " << endl;
            return 0;
        }
        
        /*Sorting vector of objects created from input text file using std::sort.
          defined < operator in Record class since it is used by this sort function */
        sort(myVector.begin(), myVector.end());
        
        /*Writing sorted vector of Record objects to file in binary mode.
          Added trunc to ensure file is overriden if file with desired name already exists in directory */
        myfile.open(outputFileName, ios::out | ios::trunc | ios::binary);
        if(myfile)
        {
            for(int vIterator = 0; vIterator < myVector.size(); vIterator++)
            {
                myfile.write(reinterpret_cast<char *>(&myVector[vIterator]), sizeof(Record));
            }
            myfile.close();
        }
        else
        {
            cout << "Unble to open file for output" << endl;
        }
        
        /* If program mode is -l (list) then open previously created output file in binary mode to list original text file in order.
           This is done by using offset variable of Record objects stored within binary file to lookup location in original input file.
           Retrieval is then done using using seekg */
        vector<Record> readVector;
        if(programMode == "-l")
        {
            ifstream instream(outputFileName, ios::in | ios::binary);
            
            if(instream)
            {
                for(int x = 0; x < lineCount; x++)
                {
                    Record tempReadRecord("", "");
                    instream.read(reinterpret_cast<char *>(&tempReadRecord), sizeof(Record));
                    readVector.push_back(Record(tempReadRecord.key, tempReadRecord.offset));
                }
                instream.close();
            }
            else
            {
                cout << "Unable to read from file!" << endl;
            }
        }
        else
        {
            return 0;
        }
        
        vector<Record> iViewVector;
        ifstream instream(inputFileName, ios::in | ios::binary);
        if(instream)
        {
            for(int vIterator = 0; vIterator < readVector.size(); vIterator++)
            {
                instream.clear();
                instream.seekg(0, ios::beg);
                string outputKey = readVector[vIterator].key; //Take key value as is
                int i_dec = std::stoi (readVector[vIterator].offset,nullptr,2);
                instream.seekg(i_dec, ios::beg);//seek to location provided by offset from start of file
                string tempLine;
                getline(instream, tempLine); //Read in full line from text file
                string recordText = tempLine.substr(keyLength, tempLine.length());//Take substring of line from end of offset to end of line
                iViewVector.push_back(Record(outputKey,recordText));
            }
        }
        else
        {
            cout << "Error reopening input file for lookup!" << endl;
        }
        
        //Display value of Record objects and end program
        cout << "Listing file using index: " << endl;
        for(int vIterator = 0; vIterator < iViewVector.size(); vIterator++)
        {
            cout << iViewVector[vIterator].key << iViewVector[vIterator].offset << endl;
        }
        cout << "Done listing file!" << endl;
        cout << "# of records in file: " << iViewVector.size() << endl;
        cout << "Ending program!" << endl;
        return 0;
    }
    catch(exception ex) //Catch block in case exception encountered at runtime
    {
        cout << "Exception occured: " << ex.what() << endl;
    }
}
