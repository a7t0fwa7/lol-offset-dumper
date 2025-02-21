#include <iostream>
#include <fstream>
#include <chrono> //std::chrono
#include <ctime>  //ctime()
#include <string>
#include <filesystem>

#include "CSV/rapidcsv.h"


#include "Memory/Memory.h"

#define PATTERN_FILE "Patterns.txt"
#define DUMP_FILE "Offsets.h"

std::ofstream output;

enum InputFields
{
	FIELDS_TYPE,
	FIELDS_NAME,
	FIELDS_PATTERN,
	FIELDS_OFFSET,
	FIELDS_TYPESIZE
};

std::string ltrim(const std::string& s)
{
	size_t start = s.find_first_not_of(" ");
	return (start == std::string::npos) ? "" : s.substr(start);
}


bool ParseFileToStruct(std::vector<PatternStruct> &vector)
{

	auto pathAndFile = std::filesystem::current_path().append(PATTERN_FILE).string();
	rapidcsv::Document doc;
	try {
		doc = rapidcsv::Document(pathAndFile, rapidcsv::LabelParams::LabelParams(0, -1), rapidcsv::SeparatorParams(), rapidcsv::ConverterParams(true));
	}
	catch (std::ios::failure) {
;			printf("Patterns.txt not found!\nMake sure its here: %s\n", pathAndFile.c_str());  //theres no way to check if document exists,
			return 0;
	}
	  
	for (int i = -1; i < (int)doc.GetRowCount(); i++)
	{
		std::string type = doc.GetCell<std::string>(0, i);
		if (type != "OFFSET" && type != "ADDRESS" && type != "FUNCTION") {
			continue;
		}

		std::string name = ltrim(doc.GetCell<std::string>(1, i)); //trim space so its not ugly in console
		std::string pattern = doc.GetCell<std::string>(2, i);
		int64_t offset = doc.GetCell<__int64>(3, i);
		int64_t typeSize;
		
		

		try {
			typeSize = doc.GetCell<__int64>(4, i);   //again... theres no way to check if a cell exists with this library.
		}
		catch (std::out_of_range) {
			typeSize = 0;
		}
		
		PatternStruct ps;
		ps.type_size = typeSize;
		ps.name = name;
		ps.pattern = pattern;
		ps.offset = offset;
		
	    if (type == "OFFSET") 
			ps.type = InputType::Offset;
		
		else if (type == "ADDRESS") 
			ps.type = InputType::Address;
		
		else if (type == "FUNCTION") 
			ps.type = InputType::AddressFunction;
		
		
		vector.push_back(ps);
	}

	return true;
}

void CreateDumpFile()
{
	//Create file
	output.open(DUMP_FILE);

	//Get Time Now
	auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

	//Convert time to ctime format
	char str_time[MAXCHAR];
	ctime_s(str_time, MAXCHAR, &now);

	//write in file
    
	output << "#pragma once" << std::endl << std::endl;
	output << "/*" << std::endl;
	output << "Original dumper by @Ph4nton (https://www.unknowncheats.me/forum/members/780190.html)\nUpdated by @Dark (https://www.unknowncheats.me/forum/members/1269962.html)" << std::endl;
	output << str_time;
	output << "*/" << std::endl << std::endl;
	output << "#define BASEADDRESS GetModuleHandle(NULL)" << std::endl;
}

int main(int argc, const char* argv[])
{
	CMemory Memory;
	std::vector<PatternStruct> pattern_struct;

	if (argc < 2) {
		std::cout << "Correct usage: " << argv[0] << " \"League of Legends.exe\"" << std::endl;
		system("pause");
		return 0;
	}
	
	if (!ParseFileToStruct(pattern_struct)) {
		system("pause");
		return 0;
	}

	if (!Memory.Initialize(argv[1])) {
		system("pause");
		return 0;
	}

	//Create output file
	CreateDumpFile();

	for (auto obj : pattern_struct)
	{

		
		//Get address from pattern
		auto address = Memory.Pattern(obj);

		//Save output in file
		output << "#define " << obj.name << " 0x" << std::hex << std::uppercase << address << "\t//" << obj.pattern << std::endl;

		//Print in console
		std::cout << obj.name << ": 0x" << std::hex << std::uppercase << address << std::endl;
	}

	//close file
	output.close();

	system("pause");

	return 0;
}