#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <cmath>

using namespace std;

int main(int argc, char *argv[]) {
    if (argc != 4) {
        cerr << "usage: " << argv[0] << " <source> <prefix> <chunk size (K)>" << endl;
        return 1;
    }

    string source_filename = argv[1];
    string prefix = argv[2];
    long long chunk_size_kb;

    try {
        chunk_size_kb = stoll(argv[3]);
        if (chunk_size_kb <= 0) {
            cerr << "Error: Chunk size must be a positive integer." << endl;
            return 1;
        }
    } catch (const std::invalid_argument& e) {
        cerr << "Error: Invalid chunk size provided." << endl;
        return 1;
    } catch (const std::out_of_range& e) {
        cerr << "Error: Chunk size is too large." << endl;
        return 1;
    }

    long long chunk_size_bytes = chunk_size_kb * 1024;
    ifstream source_file(source_filename, ios::binary);

    if (!source_file.is_open()) {
        cerr << "Error: Could not open source file: " << source_filename << endl;
        return 1;
    }

    long long file_size;
    source_file.seekg(0, ios::end);
    file_size = source_file.tellg();
    source_file.seekg(0, ios::beg);

    if (file_size == -1) {
        cerr << "Error: Could not get the size of the source file." << endl;
        source_file.close();
        return 1;
    }

    long long num_chunks = ceil((double)file_size / chunk_size_bytes);
    vector<char> buffer(chunk_size_bytes);
    ofstream output_file;
    stringstream ss;

    for (long long i = 0; i < num_chunks; ++i) {
        ss.str("");
        ss << prefix << "." << setfill('0') << setw(32) << i;
        string output_filename = ss.str();

        cout << "starting " << output_filename << endl;

        output_file.open(output_filename, ios::binary);
        if (!output_file.is_open()) {
            cerr << "Error: Could not open output file: " << output_filename << endl;
            source_file.close();
            return 1;
        }

        source_file.read(buffer.data(), chunk_size_bytes);
        streamsize bytes_read = source_file.gcount();
        output_file.write(buffer.data(), bytes_read);
        output_file.close();

        if (source_file.fail() && !source_file.eof()) {
            cerr << "Error: An error occurred while reading the source file." << endl;
            source_file.close();
            return 1;
        }
    }

    source_file.close();
    cout << "done... [" << num_chunks << "] chunks produced for " << source_filename << endl;

    return 0;
}