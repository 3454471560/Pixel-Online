#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>

int main(int argc, char* argv[]) 
{
    if (argc != 4) 
    {
        std::cerr << "Usage: bin2c.exe <input_binary> <output_c> <array_name>\n";
        return 1;
    }

    const char* inputFile = argv[1];
    const char* outputFile = argv[2];
    const char* arrayName = argv[3];

    std::ifstream in(inputFile, std::ios::binary);
    if (!in) 
    {
        std::cerr << "Error: Cannot open input file " << inputFile << "\n";
        return 1;
    }

    std::vector<unsigned char> data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    in.close();

    std::ofstream out(outputFile);
    if (!out) 
    {
        std::cerr << "Error: Cannot create output file " << outputFile << "\n";
        return 1;
    }

    out << "#include <stddef.h>\n\n";
    out << "const unsigned char " << arrayName << "[] = {\n    ";
    for (size_t i = 0; i < data.size(); ++i) 
    {
        out << "0x" << std::hex << std::setw(2) << std::setfill('0') << (int)data[i];
        if (i != data.size() - 1) out << ", ";
        if ((i + 1) % 16 == 0 && i + 1 < data.size()) out << "\n    ";
    }
    out << "\n};\n\n";
    out << "const size_t " << arrayName << "_len = " << std::dec << data.size() << ";\n";

    std::cout << "Generated " << outputFile << "\n";
    return 0;
}