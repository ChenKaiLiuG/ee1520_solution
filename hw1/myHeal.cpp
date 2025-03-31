#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <filesystem> 
#include <cmath>

#ifdef _MSC_VER
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace std;
#ifdef _MSC_VER
namespace fs = std::filesystem;
#elif __cplusplus >= 201703L
namespace fs = std::filesystem;
#else
namespace fs {
    inline bool exists(const std::string& name) {
        return (access(name.c_str(), F_OK) != -1);
    }
    inline uintmax_t file_size(const std::string& name) {
        struct stat stat_buf;
        if (stat(name.c_str(), &stat_buf) == 0) {
            return stat_buf.st_size;
        }
        return static_cast<uintmax_t>(-1);
    }
}
#endif


int main(int argc, char *argv[]) 
{
    if (argc != 5) 
    {
        cerr << "usage: " << argv[0] << " <destination> <prefix> <chunk size (K)> <number of chunks>" << endl;
        return 1;
    }

    string destination_filename = argv[1];
    string prefix = argv[2];
    long long expected_chunk_size_kb;
    long long num_chunks;

    try {
        expected_chunk_size_kb = stoll(argv[3]);
        num_chunks = stoll(argv[4]);
        if (expected_chunk_size_kb <= 0 || num_chunks <= 0) {
            cerr << "Error: Chunk size and number of chunks must be positive integers." << endl;
            return 1;
        }
    } catch (const std::invalid_argument& e) {
        cerr << "Error: Invalid chunk size or number of chunks provided." << endl;
        return 1;
    } catch (const std::out_of_range& e) {
        cerr << "Error: Chunk size or number of chunks is too large." << endl;
        return 1;
    }

    long long expected_chunk_size_bytes = expected_chunk_size_kb * 1024;
    ofstream destination_file(destination_filename, ios::binary);

    if (!destination_file.is_open()) 
    {
        cerr << "Error: Could not open destination file: " << destination_filename << endl;
        return 1;
    }

    stringstream ss;
    vector<char> buffer(expected_chunk_size_bytes);
    bool missing_chunk = false;
    bool incorrect_chunk_size = false;

    for (long long i = 0; i < num_chunks; ++i) 
    {
        ss.str("");
        ss << prefix << "." << setfill('0') << setw(32) << i;
        string chunk_filename = ss.str();

        if (!fs::exists(chunk_filename)) 
        {
            cerr << "Error: Missing chunk file: " << chunk_filename << endl;
            missing_chunk = true;
        } 
        else 
        {
            cout << "putting " << chunk_filename << endl;
            ifstream chunk_file(chunk_filename, ios::binary);
            if (chunk_file.is_open()) {
                chunk_file.read(buffer.data(), expected_chunk_size_bytes);
                streamsize bytes_read = chunk_file.gcount();
                destination_file.write(buffer.data(), bytes_read);
                chunk_file.close();

                if (i < num_chunks - 1 && bytes_read != expected_chunk_size_bytes) {
                    cerr << "Warning: Incorrect chunk size for file: " << chunk_filename
                         << ". Expected: " << expected_chunk_size_bytes << ", Actual: " << bytes_read << endl;
                    incorrect_chunk_size = true;
                }
            } else {
                cerr << "Error: Could not open chunk file: " << chunk_filename << endl;
                missing_chunk = true;
            }
        }
    }

    destination_file.close();

    if (missing_chunk) {
        cerr << "Error: One or more chunk files are missing. Reassembly failed." << endl;
        remove(destination_filename.c_str());
        return 1;
    }

    if (incorrect_chunk_size) {
        cerr << "Warning: One or more chunk files have an incorrect size. The reassembled file might be incomplete or corrupted." << endl;
        return 1;
    }

    cout << "done... [" << num_chunks << "] chunks produced for " << destination_filename << endl;

    return 0;
}