
#include <iostream>
#include <algorithm>
#include <string>

using namespace std;

int main()
{
	string toConvert = "";
	cout << "copy-paste your string" << endl;
	getline(cin, toConvert);
	transform(toConvert.begin(), toConvert.end(), toConvert.begin(), ::tolower);
	cout << toConvert << endl;
	
	return 0;
}


