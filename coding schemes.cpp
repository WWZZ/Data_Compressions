/*****************************************************
*** Half-byte Packing and Run Length Coding
*** Encodes and Decodes for any string that doesn't contain '$'
*****************************************************/

#include <iostream>
#include <stdlib.h>
#include <string>
#include <bitset>
#include <sstream> 
#include <cstdlib>

using namespace std;

const char DELIM_CHAR = '$'; 

class baseCode
{
	public:
		baseCode();
		baseCode(string input)
		{
			if(input.find(DELIM_CHAR) != input.npos) //string contains $
			{
				cerr << DELIM_CHAR << " is an invalid char for this code\n";
				exit(1);
			}
			/*
			if(input.empty()) //needed if hardcoding empty string block
			{
				cerr << "Error: String is empty";
				exit(1);
			}
			*/
			toEncode = input;
		}
		void write_it()
		{
			cout << "Coded string is: " << encoded << endl;
			cout << "Decoded string is: " << decoded << endl << endl;
		}
	protected:
		string toEncode;
		string encoded;
		string decoded;
};

class halfByteCode : public baseCode
{
	public:
	halfByteCode(string input) : baseCode(input) {};
	string encodeMsg();
	string decodeMsg();
	
	bool diffPrefix(char prev, char curr);
	char getBitChar(string a);
	string getStringBit(char a);
	string getStringBit(string s);
	string getPacking(string stringBlock);
};

string halfByteCode :: encodeMsg()
{
	char prev = toEncode.at(0), curr;
	string blockEncode;
	blockEncode += prev;
	for (int i = 1; i < toEncode.length(); i++)
	{
		curr = toEncode.at(i);
		if(diffPrefix(prev,curr) || blockEncode.length() == 15)
		{
			if (blockEncode.length() > 3)  // halfbyte packing occurs
			{
				encoded += getPacking(blockEncode);
			}
			else encoded += getStringBit(blockEncode);
			blockEncode.clear();
		}

		blockEncode += curr;
		if (toEncode.length() == i+1) //end of toEncode string
		{
			if (blockEncode.length() > 3)  // halfbyte packing occurs
				encoded += getPacking(blockEncode);
			else encoded += getStringBit(blockEncode);
		}
		prev = curr;
	}
	return encoded;
}

string halfByteCode :: decodeMsg()
{
	int bits_To_Read = 8;
	string toDecode = encoded;
	char curr;
	string prefix;
	
	while (!toDecode.empty()) 
	{
		curr = getBitChar(toDecode.substr(0,bits_To_Read));
		toDecode = toDecode.erase(0, bits_To_Read);
		if (curr == DELIM_CHAR)
		{
			bits_To_Read = 4;
			
			prefix = toDecode.substr(0,bits_To_Read);
			toDecode = toDecode.erase(0,bits_To_Read);
			
			bitset<4> repeat(toDecode.substr(0,bits_To_Read));
			toDecode = toDecode.erase(0,bits_To_Read);
			
			int num_Repeats = repeat.to_ulong();
			
			for (int i = 0; i < num_Repeats; i++)
			{
				decoded += getBitChar(prefix + toDecode.substr(0,bits_To_Read));
				toDecode = toDecode.erase(0,bits_To_Read);
			}
			bits_To_Read = 8;
		}
		else decoded += curr;
	}
	return decoded;
}

bool halfByteCode :: diffPrefix(char prev, char curr) //returns 1 if first 4 bits are different else 0
{
	string a = getStringBit(prev);
	string b = getStringBit(curr);
	return bool(a.substr(0,4).compare(b.substr(0,4)));
}

char halfByteCode::getBitChar(string a) //convert string of bits to ascii char
{
	bitset<8> temp(a);
	//cout << "getBitChar returning: " << char(temp.to_ulong()) << endl;
	return char(temp.to_ulong());
}

string halfByteCode::getStringBit(char a) //convert char to string of bits
{
	bitset<8> temp = a;
	return temp.to_string();
}

string halfByteCode:: getStringBit(string s) //convert string to string of bits
{
	string bits;
	for (char& c : s)
	{
		bitset<8> temp = c;
		bits += temp.to_string();
	}
	return bits;
}

string halfByteCode::getPacking(string stringBlock) //returns the half byte packing of stringBlock
{
	bitset<4> numRepeats = stringBlock.length();
	string prefix = getStringBit(stringBlock.at(0)).substr(0,4);
	string packing;
	packing += getStringBit(DELIM_CHAR) + prefix + numRepeats.to_string();
	for (char& c : stringBlock)
	{
		packing += getStringBit(c).substr(4,4);
	}
	return packing;
}

class runLengthCode : public baseCode
{
	public:
		runLengthCode(string input) : baseCode(input) {};
		string encodeMsg();
		string decodeMsg();
		string compress(string blockEncode);
		string decodeBlock(string block);
};

string runLengthCode::encodeMsg()
{
	string blockEncode;
	char curr;
	char prev = toEncode.at(0);
	blockEncode += prev;
	for(int i = 1; i < toEncode.length(); i++)
	{
		curr = toEncode.at(i);
		
		if (prev == curr)
		{
			blockEncode += curr;
		}
				
		else
		{
			if (blockEncode.length() > 3) //only compress if more than 3 char repeats
				encoded += compress(blockEncode);	
			else encoded += blockEncode;
			
			blockEncode.clear();
			blockEncode += curr;
			prev = curr;
		}
		
		if (i == toEncode.length()-1) //end of toEncode
		{
			if (blockEncode.length() > 3) //only compress if more than 3 char repeats
				encoded += compress(blockEncode);
			else encoded += blockEncode;
		}
	}
	return encoded;
}

string runLengthCode::decodeMsg()
{
	for (int i = 0; i < encoded.length(); i++)
	{
		if (encoded.at(i) == DELIM_CHAR) //'$' found
		{
			decoded += decodeBlock(encoded.substr(i,3));
			i += 2;
		}
		else decoded += encoded.at(i);
	}
	return decoded;
}

string runLengthCode::compress(string blockEncode) 
{
	ostringstream convert;
	convert << blockEncode.length();
	string s;
	s.push_back(DELIM_CHAR);
	s.push_back(blockEncode.at(0));
	return s + convert.str();
}

string runLengthCode::decodeBlock(string block)
{
	int num_repeats = block.at(2) - '0';
	return string(num_repeats, block.at(1));
}

void tester(string input, string output)
{
	if (input == output)
		cout << "Encode and Decode success" << endl;
	else cout << "Encode and Decode failed" << endl;
}

int main()
{
	string input = "ngnaeomMF4444#MM)CM#M@)MNC)@N)MEDMfFFFFFVIJKE)JS)JMVGI)VMEONNFeieovveeeeeee";
	
	halfByteCode test1 (input);
	test1.encodeMsg();
	tester (input , test1.decodeMsg());
	test1.write_it();
	
	runLengthCode test2 (input);
	test2.encodeMsg();
	tester (input, test2.decodeMsg());
	test2.write_it();
	
	string option = "1";
	while (option != "0")
	{
			cout << "Input number of data compression scheme, 0 to quit " << endl;
			cout << "1. Half-byte Packing" << endl;
			cout << "2. Run Length Coding" << endl;
			cin >> option;
			
			while (option < "0" || option > "2")
			{
				cout << "Enter valid, number 1-2 \n";
				cin >> option;
			}
			
			cout << "Enter string to encode and decode" << endl;
			cin >> input;

			if (option == "1")
			{
				halfByteCode test3(input);
				test3.encodeMsg();
				tester (input, test3.decodeMsg());
				test3.write_it();
			}
			
			if (option == "2")
			{
				runLengthCode test4(input);
				test4.encodeMsg();
				tester (input, test4.decodeMsg());
				test4.write_it();
			}
	}
}
