/*
Recommend value setup
128
2
1024
10
*/
#include <iostream>
#include <cmath>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <random>
#include <string>
#include <sstream>
#include <iomanip>
#include <bitset>
#include <random>
#include <unistd.h>
#include <ctime>
#include <cstdlib>
using namespace std;

float hitTimes = 0;
float missTimes = 0;
int countTLBLayer = 0;
int countPhysicalMemoryLayer = 0;

struct TLBEntry{
    string virtualPage;
    int physicalPage;
};

struct PageTableEntry{
    bool valid;
    bool dirty;
    int physicalPage;
};

struct PhysicalMemory{
    string virtualPage;
    string data;
};

int getRandomNumber(mt19937 &gen, int min, int max) {
    // Define the distribution
    uniform_int_distribution<int> distrib(min, max);

    // Generate and return the random number
    return distrib(gen);
}

string gen_random(const int len) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; i++) {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    return tmp_s;
}

int binaryToDecimal(string binary) {
    int decimal = 0;
    int power = 0;

    // Iterate through the binary string from right to left
    for(int i = binary.length() - 1; i >= 0; i--){
        // If the current bit is '1', add 2^power to the decimal value
        if(binary[i] == '1'){
            decimal += pow(2, power);
        }
        // Increment the power for the next bit
        power++;
    }

    return decimal;
}

string decimalToHexadecimal(int decimal){
    stringstream ss;
    ss << hex << decimal; // Convert decimal to hexadecimal using std::hex
    return ss.str(); // Return the hexadecimal string
}

string hexadecimalToBinary(string hexadecimal){
    string binary;
    for(size_t i = 0; i < hexadecimal.length(); i++){
        char hexChar = hexadecimal[i];
        int value;
        if(hexChar >= '0' && hexChar <= '9') {
            value = hexChar - '0';
        }else if (hexChar >= 'a' && hexChar <= 'f'){
            value = 10 + (hexChar - 'a');
        }else{
            // Invalid hexadecimal character
            return "Invalid Hexadecimal Input";
        }
        binary += bitset<4>(value).to_string();
    }
    return binary;
}

string binaryToHexadecimal(string binary) {
    stringstream hexStream;
    bitset<4> bits; // 4 bits at a time for hexadecimal

    // Ensure binary length is multiple of 4
    size_t length = binary.size();
    if(length % 4 != 0){
        length += 4 - (length % 4);
        binary = string(length - binary.size(), '0') + binary;
    }

    for(size_t i = 0; i < binary.size(); i += 4){
        bits = bitset<4>(binary.substr(i, 4));
        hexStream << hex << bits.to_ulong();
    }
    return hexStream.str();
}

string binaryToHexadecimalWithOffsetBits(string binary, int offsetBits){
    stringstream hexStream;
    bitset<4> bits; // 4 bits at a time for hexadecimal

    // Ensure binary length is multiple of 4
    size_t length = binary.size();
    if (length % 4 != 0) {
        length += 4 - (length % 4);
        binary = string(length - binary.size(), '0') + binary;
    }

    // Calculate the number of bits to remove
    size_t bitsToRemove = min((size_t)offsetBits, binary.size());

    // Remove the LSB bits from the binary string
    binary = binary.substr(0, binary.size() - bitsToRemove);

    // Pad with 0s to make length multiple of 4
    size_t padding = 4 - (binary.size() % 4);
    if (padding != 4) {
        binary = string(padding, '0') + binary;
    }

    for (size_t i = 0; i < binary.size(); i += 4) {
        bits = bitset<4>(binary.substr(i, 4));
        hexStream << hex << bits.to_ulong();
    }

    // Get the hexadecimal string
    string hexResult = hexStream.str();

    // Remove leading zero if present
    if (!hexResult.empty() && hexResult[0] == '0') {
        hexResult.erase(0, 1);
    }

    return hexResult;
}

bool TLBCheck(TLBEntry TLB[], int TLBSize, string binary){
    cout << "Page : " << binary << " " << "HEX : " << binaryToHexadecimal(binary) << endl;
    for(int i = 0; i < TLBSize; i++){
        if(TLB[i].virtualPage == binary){
            cout << "TLB Hit! Physical Page: " << TLB[i].physicalPage << endl;
            hitTimes++;
            return true;
        }
    }

    return false;
}

bool physicalMemoryCheck(PhysicalMemory physicalMemory[], int key, string newBinary){
    if(physicalMemory[key].virtualPage == newBinary){
        return true;
    }
    return false;
}

void showTLBTable(TLBEntry TLB[], int TLBSize){
    cout << "TLB Table:" << endl;
    cout << "___________________________________________________________" << endl;
    cout << setw(10) << "Index |" << setw(31) << "Virtual Page |" << setw(18) << "Physical Page |" << endl;
    cout << "___________________________________________________________" << endl;
    for(int i = 0; i < TLBSize; i++){
        cout << setw(8) << i << " | " << setw(28) << TLB[i].virtualPage << " | " << setw(15) << TLB[i].physicalPage << " | " << endl;
    }
    cout << "___________________________________________________________" << endl;
    cout << endl;
}

void showPageTable(PageTableEntry pageTable[], int virtualMemorySize){
    cout << "Page Table:" << endl;
    cout << "_____________________________________________________________________________________" << endl;
    cout << setw(25) << "Index (Hexadecimal) |" << setw(30) << "Physical Page |" << setw(15) <<  "Valid |" << setw(15) << "Dirty |" << endl;
    cout << "_____________________________________________________________________________________" << endl;

    for(int i = 0; i < virtualMemorySize; i++){
        cout << setw(23) << decimalToHexadecimal(i) << " | " << setw(27) << pageTable[i].physicalPage << " | " << setw(12) << pageTable[i].valid << " | " << setw(12) << pageTable[i].dirty << " | " << endl;
    }
    cout << "_____________________________________________________________________________________" << endl;
    cout << endl;
}

void showPhysicalMemory(PhysicalMemory physicalMemory[], int physicalPageSize){
    cout << "Physical Memory Table:" << endl;
    cout << "_______________________________________________________________________________________________________________" << endl;
    cout << setw(20) << "Physical Page |" << setw(31) << "Virtual Page |" << setw(60) << "Data |" << endl;
    cout << "_______________________________________________________________________________________________________________" << endl;

    for(int i = 0; i < physicalPageSize; i++){
        cout << setw(18) << i << " | " << setw(28) << physicalMemory[i].virtualPage << " | " << setw(57) << physicalMemory[i].data << " | " << endl;
    }
    cout << "_______________________________________________________________________________________________________________" << endl;
    cout << endl;
}

void menu(){
    cout << "+======================================+" << endl;
    cout << "|,--.   ,--.,------.,--.  ,--.,--. ,--.|" << endl;
    cout << "||   `.'   ||  .---'|  ,'.|  ||  | |  ||" << endl;
    cout << "||  |'.'|  ||  `--, |  |' '  ||  | |  ||" << endl;
    cout << "||  |   |  ||  `---.|  | `   |'  '-'  '|" << endl;
    cout << "|`--'   `--'`------'`--'  `--' `-----' |" << endl;
    cout << "+======================================+" << endl;
    cout << "1) Input Data." << endl;
    cout << "2) Start Random And Load Instruction." << endl;
    cout << "3) Read Data." << endl;
    cout << "4) Clear All Load Instruction." << endl;
    cout << "5) Show All Table." << endl;
    cout << "6) Random Data And Load Instruction from Amount." << endl;
    cout << "7) Exit Program." << endl;
    cout << "choose > ";
}

int main(){
    system("Color 3"); //set color

    int choice;
    int physicalPageSize, virtualMemorySize, offsetBits, TLBSize;
    float hitRate = 0, missRate = 0;
    string data;
    string loadInstruction;

    cout << "First Setup Input" << endl;
    cout << "Input Physical Page Size(limit : 128) : ";
    cin >> physicalPageSize;
    cout << "Input Offset Bits : ";
    cin >> offsetBits;
    cout << "Input Virtual Memory Size(limit : 1024) : ";
    cin >> virtualMemorySize;
    cout << "Input TLB Entries(Recommend : 10) : ";
    cin >> TLBSize;
    cout << endl;

    int virtualMemoryRealSize = virtualMemorySize;
    physicalPageSize = physicalPageSize / pow(2, offsetBits);
    virtualMemorySize = virtualMemorySize / pow(2, offsetBits);

    TLBEntry TLB[TLBSize];
    PageTableEntry pageTable[virtualMemorySize];
    PhysicalMemory physicalMemory[physicalPageSize];

    cout << "Physical Page Size : " << physicalPageSize << endl;
    cout << "Offset Bits : " << offsetBits << endl;
    cout << "Virtual Memory Size : " << virtualMemorySize << endl;
    cout << "TLB Entries : " << TLBSize << endl;
    cout << endl;

    //Initialize TLB
    for(int i = 0; i < TLBSize; i++){
        TLB[i].virtualPage = "";
        TLB[i].physicalPage = -1;
    }

    //Initialize pageTable
    for(int i = 0; i < virtualMemorySize; i++){
        pageTable[i].dirty = false;
        pageTable[i].valid = false;
        pageTable[i].physicalPage = -1;
    }

    //Initialize physicalMemory
    for(int i = 0; i < physicalPageSize; i++){
        physicalMemory[i].virtualPage = "";
        physicalMemory[i].data = "";
    }

    // Seed the random number generator
    random_device rd;
    mt19937 gen(rd());

    do{
        if(hitTimes + missTimes != 0){
            hitRate = hitTimes / (hitTimes + missTimes) * 100;
            missRate = missTimes / (hitTimes + missTimes) * 100;
        }
        cout << "HIT RATES : " << hitRate << "%" << endl;
        cout << "MISS RATES : " << missRate << "%" << endl;
        cout << "+======================================+" << endl << endl;
        menu();
        cin >> choice;
        switch(choice){
            case 1:
                cout << "Enter Data : ";
                cin >> data;
                cout << "Enter data successfully!" << endl << endl;
                break;
            case 2:{
                int randomDecNum;
                randomDecNum = getRandomNumber(gen, 0, virtualMemoryRealSize);
                string hexNum = decimalToHexadecimal(randomDecNum);
                cout << "Decimal : " << randomDecNum << " Hexadecimal : " << hexNum << endl << endl;
                string binary = hexadecimalToBinary(hexNum);
                string hex = binaryToHexadecimalWithOffsetBits(binary, offsetBits);
                string newBinary = hexadecimalToBinary(hex);
                if(randomDecNum < pow(2, offsetBits)){
                    newBinary = "00000000";
                    randomDecNum = 0;
                }

                //check TLB
                int newDec = binaryToDecimal(newBinary);

                if(TLBCheck(TLB, TLBSize, newBinary)){
                    hitTimes++;
                    int page = pageTable[newDec].physicalPage;

                    if(physicalMemoryCheck(physicalMemory, page, newBinary)){
                        if(physicalMemory[page].data != data){
                            pageTable[newDec].dirty = true;
                            cout << "Rewrite Data into Physical Page : " << page << " to " << data << endl;
                            physicalMemory[page].data = data;
                        }else{
                            cout << "Read Data from Physical Page : " << page << ", Data : " << physicalMemory[page].data << endl;
                        }
                    }else{
                        cout << "Page fault!" << endl;
                        cout << "Write Data and Virtual Page into Physical Memory..." << endl;
                        physicalMemory[countPhysicalMemoryLayer].data = data;
                        physicalMemory[countPhysicalMemoryLayer].virtualPage = newBinary;
                        countPhysicalMemoryLayer++;
                        if(countPhysicalMemoryLayer == physicalPageSize){
                            countPhysicalMemoryLayer = 0; // Reset index if end of array is reached
                        }
                    }
                }else{
                    cout << "TLB Miss!" << endl;

                    // Check if TLB is full
                    if(countTLBLayer == TLBSize){
                        countTLBLayer = 0; // Reset index if end of array is reached
                    }
                    // Check if physicalMemory is full
                    if (countPhysicalMemoryLayer == physicalPageSize){
                        countPhysicalMemoryLayer = 0; // Reset index if end of array is reached
                    }

                    if(pageTable[newDec].valid){
                        int page = pageTable[newDec].physicalPage;

                        TLB[countTLBLayer].virtualPage = newBinary;
                        TLB[countTLBLayer].physicalPage = pageTable[newDec].physicalPage;
                        cout << "Page Table Hit! Physical Page: " << pageTable[newDec].physicalPage << endl;
                        hitTimes++;

                        if(physicalMemoryCheck(physicalMemory, page, newBinary)){
                            if(physicalMemory[page].data != data){
                                pageTable[newDec].dirty = true;
                                cout << "Rewrite Data into Physical Page : " << page << " to " << data << endl;
                                physicalMemory[page].data = data;
                            }else{
                                cout << "Read Data from Physical Page : " << page << ", Data : " << physicalMemory[page].data << endl;
                            }
                        }else{
                            cout << "Page fault!" << endl;
                            cout << "Write Data and Virtual Page into Physical Memory..." << endl;
                            physicalMemory[countPhysicalMemoryLayer].data = data;
                            physicalMemory[countPhysicalMemoryLayer].virtualPage = newBinary;
                            countPhysicalMemoryLayer++;
                            if(countPhysicalMemoryLayer == physicalPageSize){
                                countPhysicalMemoryLayer = 0; // Reset index if end of array is reached
                            }
                        }

                    }else{
                        TLB[countTLBLayer].virtualPage = newBinary;
                        TLB[countTLBLayer].physicalPage = countPhysicalMemoryLayer;
                        countTLBLayer++;
                        if(countTLBLayer == TLBSize){
                            countTLBLayer = 0; // Reset index if end of array is reached
                        }

                        cout << "Page Table Miss!" << endl;
                        cout << "Loading into Memory..." << endl;
                        pageTable[newDec].valid = true;
                        pageTable[newDec].dirty = false;
                        pageTable[newDec].physicalPage = countPhysicalMemoryLayer;

                        physicalMemory[countPhysicalMemoryLayer].data = data;
                        physicalMemory[countPhysicalMemoryLayer].virtualPage = newBinary;
                        countPhysicalMemoryLayer++;
                        if(countPhysicalMemoryLayer == physicalPageSize){
                                countPhysicalMemoryLayer = 0; // Reset index if end of array is reached
                            }
                        missTimes++;
                    }
                }

                showTLBTable(TLB, TLBSize);
                break;
            }
            case 3:{
                string readHex, readBin;
                int readDec, physicalPage;

                cout << "Enter Address (HEX) : ";
                cin >> readHex;
                cout << "Enter Address successfully!" << endl << endl;

                string newReadHex = "";

                for (auto& x : readHex) {
                    newReadHex += tolower(x);
                }

                readBin = hexadecimalToBinary(newReadHex);
                readDec = binaryToDecimal(readBin);

                physicalPage = pageTable[readDec].physicalPage;

                if(physicalPage == -1){
                    cout << "Data from " << readHex << " is Nothing!" << endl << endl;
                }else{
                    string readData = physicalMemory[physicalPage].data;
                    cout << "Data from " << readHex << " is " << readData << endl << endl;
                }

                break;
            }
            case 4:
                //Initialize TLB
                for(int i = 0; i < TLBSize; i++){
                    TLB[i].virtualPage = "";
                    TLB[i].physicalPage = -1;
                }

                //Initialize pageTable
                for(int i = 0; i < virtualMemorySize; i++){
                    pageTable[i].dirty = false;
                    pageTable[i].valid = false;
                    pageTable[i].physicalPage = -1;
                }

                //Initialize physicalMemory
                for(int i = 0; i < physicalPageSize; i++){
                    physicalMemory[i].virtualPage = "";
                    physicalMemory[i].data = "";
                }

                hitTimes = 0;
                missTimes = 0;
                countTLBLayer = 0;
                countPhysicalMemoryLayer = 0;
                hitRate = 0;
                missRate = 0;

                showTLBTable(TLB, TLBSize);
                showPageTable(pageTable, virtualMemorySize);
                showPhysicalMemory(physicalMemory, physicalPageSize);

                cout << "Completely Clear All Load Instruction!!!" << endl << endl;
                break;
            case 5:
                showTLBTable(TLB, TLBSize);
                showPageTable(pageTable, virtualMemorySize);
                showPhysicalMemory(physicalMemory, physicalPageSize);
                break;
            case 6:{
                int dataAmount;
                int limit;
                cout << "Enter Data Amount  : ";
                cin >> dataAmount;
                cout << "Enter Data Amount Successfully!" << endl;
                cout << "Enter Length of String : ";
                cin >> limit;
                cout << "Enter Length of String Successfully!" << endl << endl;

                for(int i = 0; i < dataAmount; i++){
                    string data = gen_random(limit);

                    int randomDecNum;
                    randomDecNum = getRandomNumber(gen, 0, virtualMemoryRealSize);
                    string hexNum = decimalToHexadecimal(randomDecNum);
                    cout << endl << "Decimal : " << randomDecNum << " Hexadecimal : " << hexNum << endl;
                    string binary = hexadecimalToBinary(hexNum);
                    string hex = binaryToHexadecimalWithOffsetBits(binary, offsetBits);
                    string newBinary = hexadecimalToBinary(hex);
                    if(randomDecNum < pow(2, offsetBits)){
                        newBinary = "00000000";
                        randomDecNum = 0;
                    }

                    //check TLB
                    int newDec = binaryToDecimal(newBinary);

                    if(TLBCheck(TLB, TLBSize, newBinary)){
                        hitTimes++;
                        int page = pageTable[newDec].physicalPage;

                        if(physicalMemoryCheck(physicalMemory, page, newBinary)){
                            if(physicalMemory[page].data != data){
                                pageTable[newDec].dirty = true;
                                cout << "Rewrite Data into Physical Page : " << page << " to " << data << endl;
                                physicalMemory[page].data = data;
                            }else{
                                cout << "Read Data from Physical Page : " << page << ", Data : " << physicalMemory[page].data << endl;
                            }
                        }else{
                            cout << "Page fault!" << endl;
                            cout << "Write Data and Virtual Page into Physical Memory..." << endl;
                            physicalMemory[countPhysicalMemoryLayer].data = data;
                            physicalMemory[countPhysicalMemoryLayer].virtualPage = newBinary;
                            countPhysicalMemoryLayer++;
                            if(countPhysicalMemoryLayer == physicalPageSize){
                                countPhysicalMemoryLayer = 0; // Reset index if end of array is reached
                            }
                        }
                    }else{
                        cout << "TLB Miss! ";

                         // Check if TLB is full
                        if(countTLBLayer == TLBSize){
                            countTLBLayer = 0; // Reset index if end of array is reached
                        }
                        // Check if physicalMemory is full
                        if(countPhysicalMemoryLayer == physicalPageSize){
                            countPhysicalMemoryLayer = 0; // Reset index if end of array is reached
                        }

                        if(pageTable[newDec].valid){
                            int page = pageTable[newDec].physicalPage;

                            TLB[countTLBLayer].virtualPage = newBinary;
                            TLB[countTLBLayer].physicalPage = pageTable[newDec].physicalPage;
                            //countTLBLayer++;
                            cout << "Page Table Hit! Physical Page: " << pageTable[newDec].physicalPage << endl;
                            hitTimes++;

                            if(physicalMemoryCheck(physicalMemory, page, newBinary)){
                                if(physicalMemory[page].data != data){
                                    pageTable[newDec].dirty = true;
                                    cout << "Rewrite Data into Physical Page : " << page << " to " << data << endl;
                                    physicalMemory[page].data = data;
                                }else{
                                    cout << "Read Data from Physical Page : " << page << ", Data : " << physicalMemory[page].data << endl;
                                }
                            }else{
                                cout << "Page fault! Write Data and Virtual Page into Physical Memory..." << endl;
                                physicalMemory[countPhysicalMemoryLayer].data = data;
                                physicalMemory[countPhysicalMemoryLayer].virtualPage = newBinary;
                                countPhysicalMemoryLayer++;
                                if(countPhysicalMemoryLayer == physicalPageSize){
                                    countPhysicalMemoryLayer = 0; // Reset index if end of array is reached
                                }
                            }

                        }else{
                            TLB[countTLBLayer].virtualPage = newBinary;
                            TLB[countTLBLayer].physicalPage = countPhysicalMemoryLayer;
                            countTLBLayer++;

                            cout << "Page Table Miss! Loading into Memory..." << endl;
                            pageTable[newDec].valid = true;
                            pageTable[newDec].dirty = false;
                            pageTable[newDec].physicalPage = countPhysicalMemoryLayer;

                            physicalMemory[countPhysicalMemoryLayer].data = data;
                            physicalMemory[countPhysicalMemoryLayer].virtualPage = newBinary;
                            countPhysicalMemoryLayer++;
                            if(countPhysicalMemoryLayer == physicalPageSize){
                                countPhysicalMemoryLayer = 0; // Reset index if end of array is reached
                            }
                            missTimes++;
                        }
                    }
                }

                break;
            }
            case 7:
                cout << endl << "Exit the program, Bye!";
                break;
        }

    }while(choice != 7);

    return 0;
}
