#include <fstream>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <chrono>

using namespace std;
namespace fs = filesystem;
string filePath;
unordered_map<string, int> fileLineCounts;
string processedFolderPath = "D:\\folder&file manipulation\\Processed";
string indexFolderPath = "D:\\folder&file manipulation\\index";
string UnprocessedfolderPath = "D:\\folder&file manipulation\\Unprocessed-Passwords";

string calculate_md5(const string& data) {
    unsigned char result[MD5_DIGEST_LENGTH];
    MD5((const unsigned char*)data.c_str(), data.length(), result);

    stringstream ss;
    ss << hex << setfill('0');
    for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
        ss << setw(2) << static_cast<int>(result[i]);
    }
    return ss.str();
}

string calculate_sha1(const string& data) {
    unsigned char result[SHA_DIGEST_LENGTH];
    SHA1((const unsigned char*)data.c_str(), data.length(), result);

    stringstream ss;
    ss << hex << setfill('0');
    for (int i = 0; i < SHA_DIGEST_LENGTH; ++i) {
        ss << setw(2) << static_cast<int>(result[i]);
    }
    return ss.str();
}

string calculate_sha256(const string& data) {
    unsigned char result[SHA256_DIGEST_LENGTH];
    SHA256((const unsigned char*)data.c_str(), data.length(), result);

    stringstream ss;
    ss << hex << setfill('0');
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << setw(2) << static_cast<int>(result[i]);
    }
    return ss.str();
}

void processLine(const string& line, const string& outputFileName, unordered_set<string>& existingLines) {
    string processedLine = line;

    // Process the first character
    if (!isalpha(processedLine[0])) {
        char c = processedLine[0];
        if (c == '\\' || c == '/' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|') {
            processedLine[0] = '_'; // Replace invalid characters with underscores
        }
    } else {
        processedLine[0] = tolower(processedLine[0]); // Convert to lowercase
    }

    // Write the line to the appropriate file if it doesn't exist already
    if (existingLines.find(processedLine) == existingLines.end()) {
           ofstream outputFileStream(outputFileName, ios::app);
        if (outputFileStream) {
            outputFileStream << processedLine << endl;
            existingLines.insert(processedLine);
        } else {
             cerr << "Error writing to file: " << outputFileName << endl;
        }
    }
}

void processFolder(const string& folderPath, const string& processedFolderPath) {
      unordered_set<string> existingLines;

    // Iterate over each file in the folder
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            ifstream inputFile(entry.path());
            if (!inputFile) {
                cerr << "Error opening file: " << entry.path() << endl;
                continue;
            }
               string fileName = entry.path().filename().stem().string();
               string outputFileName = processedFolderPath + "/" + fileName + ".txt";
            // Read each line of the file
               string line;
            while (getline(inputFile, line)) {
                if (!line.empty()) {
                    processLine(line, outputFileName, existingLines);
                }
            }
        }
    }
}
void index(const string& folderPath) {
    // Iterate over each file in the folder
    for (const auto& entry : fs::directory_iterator(folderPath)) {
        if (entry.is_regular_file()) {
            ifstream inputFile(entry.path());
            if (!inputFile) {
                cerr << "Error opening file: " << entry.path() << endl;
                continue;
            }
              // Read each line of the file
            string line;
            while (getline(inputFile, line)) {
                    // Write the line to the appropriate file
            char firstChar = line[0];
                    
          // Calculate hash values
                    string md5_hash = calculate_md5(line);
                    string sha1_hash = calculate_sha1(line);
                    string sha256_hash = calculate_sha256(line);

          // Determine the subfolder path based on the first character
          string subfolderPath = indexFolderPath + "\\" + firstChar;

          // Create the subfolder if it doesn't exist
          fs::create_directory(subfolderPath);

          // Construct the file path
          string filePath = subfolderPath + "/output_" + firstChar + ".txt";

          // If the file contains 10,000 lines, create a new file
          if (fileLineCounts[filePath] > 500 - 1) {
            int count = fileLineCounts[filePath] / 500;
            do {
              count++;
              filePath = subfolderPath + "/output_" + firstChar + "_" + to_string(count) + ".txt";
            } while (fileLineCounts[filePath] > 500 - 1);
          }

          ofstream outputFile(filePath, ios::app);
          if (outputFile) {
            outputFile << line + "|" + md5_hash + "|" + sha1_hash + "|" + sha256_hash + "|" + entry.path().filename().string() << endl;
            // Increment line count for the file
            fileLineCounts[filePath]++;
          } else {
            cerr << "Error writing to file: " << filePath << endl;
          }
        }
                }}}


void Search(string &password) {
    char c = password[0];
    if (!isalpha(c)) {
        if (c == '\x5C' || c == '/' || c == ':' || c == '*' || c == '?' || c == '"' || c == '<' || c == '>' || c == '|') {
            password[0] = '_';
        }
    } else {
        password[0] = tolower(password[0]);
    }
    string subfolderPath = indexFolderPath + "\\" + password[0];
    for (const auto &entry : fs::directory_iterator(subfolderPath)) {
        if (entry.is_regular_file()) {
            ifstream inputFile(entry.path());
            if (inputFile) {
                string line;
                while (getline(inputFile, line)) {
                    if (!line.empty() && line.substr(0, line.find('|')) == password) {
                        cout << "Found" << entry.path() << endl;
                        return;
                    }
                }
            } else {
                cerr << "Error opening file: " << entry.path() << endl;
            }
        }
    }

     // Calculate hash values
    string md5_hash = calculate_md5(password);
    string sha1_hash = calculate_sha1(password);
    string sha256_hash = calculate_sha256(password);

    fs::create_directory(subfolderPath);
    string filePath = subfolderPath + "/output_" + c + ".txt";
    if (fileLineCounts[filePath] >= 500) {
        int count = fileLineCounts[filePath] / 500;
        do {
            count++;
            filePath = subfolderPath + "/output_" + c + "_" + to_string(count) + ".txt";
        } while (fileLineCounts[filePath] >= 500);
    }
        string filename = fs::path(filePath).filename().string();
    ofstream outputFile(filePath, ios::app);
    if (outputFile) {
         outputFile << password + "|" + md5_hash + "|" + sha1_hash + "|" + sha256_hash + "|" + filename << endl;
        cout << "Password Not Found Recording...." << endl;
        fileLineCounts[filePath]++;
    } else {
        cerr << "Error writing to file: " << filePath << endl;
    }
}

int main() {
     
    processFolder(UnprocessedfolderPath, processedFolderPath);
     index(processedFolderPath);
  
     string pass;
     float total;
     for(int i = 1; i <= 10; i++){
        cout << "Enter Password to be searched: " << endl;
        cin >> pass;
        if (pass == "x") break;

        auto start = chrono::high_resolution_clock::now(); // Start timing

        Search(pass);

        auto end = chrono::high_resolution_clock::now(); // End timing
        chrono::duration<double> duration = end - start;
        
        cout << i << ".Time taken to search: " << duration.count() << " seconds" << endl;
        total = duration.count() + total;
        }
       float avg = total/10;
       cout << "Average Search time: "<< avg << endl;

  return 0;
}