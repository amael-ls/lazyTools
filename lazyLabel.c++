
//TODO: IMPLEMENT THE SORTING

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <iomanip> // std::setw, std::left

using std::cout;
using std::endl;
using std::string;

typedef std::map<std::string, std::map<std::string, int> > map2d;
void cleanString(std::string &str, std::vector<char> charToClean);

class RefLabel
{
	public :
		RefLabel(std::string const& fileName);
		void run(bool const sortByFile);
		void sort();
		friend std::ostream& operator<<(std::ostream& os, RefLabel const& refLabel);

	private :
	// member data
		std::map<std::string, std::map<std::string, int> > m_listLabel; // map <string nameLabel, map <string fromFile, int count>
		std::map<std::string, std::map<std::string, int> > m_listRef; // map <string nameLabel, map <string fromFile, int nbAssociatedLab>
		std::string m_fileName;
		bool m_sortByFile = false;
		std::map<std::string, std::vector<std::string> > m_listFilesLabels; // map <string fromFile, vector <string> labels>

	// member functions
		void makeList(string nameFile, string const fromFile);
		void addRef(std::string const nameRef, std::string const fromFile, std::string const codeList);
		void associatedLabel();

};

// Constructors
RefLabel::RefLabel(std::string const& fileName): m_fileName(fileName) {}

// Overload operator
std::ostream& operator<<(std::ostream& os, RefLabel const& refLabel)
{
	map2d::const_iterator it;
	std::map<std::string, int>::const_iterator it_map;

	unsigned int countMultiLabel = 0;
	unsigned int countUndefRef = 0;
	// print multilabel
	for (it = refLabel.m_listLabel.begin(); it != refLabel.m_listLabel.end(); ++it)
	{
		if ((it->second).size() != 1)
		{
			++ countMultiLabel;
			os << "Multidefinition of label " << it->first << " :" << endl;
			for (it_map = (it->second).begin(); it_map != (it->second).end(); ++it_map)
				os << "    - " << std::setw(40) << it_map->first << " \t " << it_map->second << endl;
		}
		else
		{
			it_map = (it->second).begin();
			if (it_map->second != 1)
			{
				++ countMultiLabel;
				os << "Multidefinition of label " << it->first << " :" << endl;
				os << "    - " << std::setw(40) << it_map->first << " \t " << it_map->second << endl;
			}
		}
	}

	// print undefined ref
	os << "--------------------" << endl;
	if (!refLabel.m_sortByFile)
	{
		for (it = refLabel.m_listRef.begin(); it != refLabel.m_listRef.end(); ++it)
		{
			if (((it->second).begin())->second == 0)
			{
				++countUndefRef;
				os << "Undefined reference " << it->first << " :" << endl;
				for (it_map = (it->second).begin(); it_map != (it->second).end(); ++it_map)
					os << "    - " << std::setw(40) << it_map->first << endl;
			}
		}
	}
	else
	{
		std::map<std::string, std::vector<std::string> >::const_iterator it_mapSort;
		std::vector<std::string>::const_iterator it_vec;
		for (it_mapSort = refLabel.m_listFilesLabels.cbegin(); it_mapSort != refLabel.m_listFilesLabels.cend(); ++it_mapSort)
		{
			os << "Undefined reference in file " << it_mapSort->first << " :" << endl;
			for (it_vec = (it_mapSort->second).cbegin(); it_vec != (it_mapSort->second).cend(); ++it_vec)
				os << "    - " << std::setw(40) << *it_vec << endl;
		}
	}
	

	if (countMultiLabel == 0)
		os << "No multidefinition of label" << endl;

	if (countUndefRef == 0)
		os << "No undefined ref" << endl;

	return os;
}

// Function addRef, nameRef stands for ref and label, codeList indicates which list is concerned
void RefLabel::addRef(std::string const nameRef, std::string const fromFile, std::string const codeList)
{
	if (codeList == "ref")
		m_listRef[nameRef][fromFile] = 0; // Can be checked only after labels list is complete

	if (codeList == "label")
		m_listLabel[nameRef][fromFile] += 1;
}

// Function makeList, recursive call
void RefLabel::makeList(string nameFile, string const fromFile)
{
	std::vector<char> vecChar(1,'"');
	std::vector<char> spaceChar(1,' ');
	cleanString(nameFile, vecChar);
	if (nameFile.size() < 4)
		nameFile += ".tex";
	else if (nameFile.substr(nameFile.size() - 4, 4) != ".tex")
		nameFile += ".tex";

	std::ifstream ifs(nameFile.c_str());
	if (!ifs)
		throw string("*** ERROR (from readFile) *** : cannot open file : ''" + nameFile + "''\nthis input is from ''" + fromFile + "''");

	string nameFile_recursiveCalling;
	string nameRef, nameLabel;
	string readingLine;

	size_t inputPos (0), refPos(0), eqrefPos(0), labelPos(0), length_sub_str(0);

	while ((getline(ifs, readingLine)) && !(ifs.eof()))
	{
		while ((readingLine[0] == '%') && !(ifs.eof())) // to drop all of comment lines
			getline(ifs,readingLine);

		while ( (inputPos = readingLine.find("input{", inputPos)) != string::npos) // recursive call
		{
			inputPos += 6;
			length_sub_str = readingLine.find("}", inputPos) - inputPos;
			nameFile_recursiveCalling = readingLine.substr(inputPos, length_sub_str);
			this->makeList(nameFile_recursiveCalling, nameFile);
		}
		// Label

		while ( ((refPos = readingLine.find("\\ref{", refPos)) != string::npos) |
			((eqrefPos = readingLine.find("\\eqref{", eqrefPos)) != string::npos) |
			((labelPos = readingLine.find("\\label{", labelPos)) != string::npos))
		{
			if (refPos != string::npos) // ref or eqref
			{
				refPos = readingLine.find("{", refPos) + 1;
				length_sub_str = readingLine.find("}", refPos) - refPos;
				nameRef = readingLine.substr(refPos, length_sub_str);
				cleanString(nameRef, spaceChar);
				this->addRef(nameRef, nameFile, "ref");
				refPos++;
			}
			if (eqrefPos != string::npos)
			{
				eqrefPos = readingLine.find("{", eqrefPos) + 1;
				length_sub_str = readingLine.find("}", eqrefPos) - eqrefPos;
				nameRef = readingLine.substr(eqrefPos, length_sub_str);
				cleanString(nameRef, spaceChar);
				this->addRef(nameRef, nameFile, "ref");
				eqrefPos++;
			}
			if (labelPos != string::npos) // label
			{
				labelPos = readingLine.find("{", labelPos) + 1;
				length_sub_str = readingLine.find("}", labelPos) - labelPos;
				nameRef = readingLine.substr(labelPos, length_sub_str);
				cleanString(nameRef, spaceChar);
				this->addRef(nameRef, nameFile, "label");
				labelPos++;
			}
		}
		inputPos = 0;
		refPos = 0;
		eqrefPos = 0;
		labelPos = 0;
	}
}

// Clean string
void cleanString(std::string &str, std::vector<char> charToClean)
{
	bool cleaner = true;
	while (cleaner)
	{
		cleaner = false;
		for (unsigned int i=0; i<charToClean.size(); i++)
		{
			while ( *(str.end() - 1) == charToClean[i])
			{
				str = str.substr(0,str.size() - 1);
				cleaner = true;
			}

			while ( *(str.begin()) == charToClean[i])
			{
				str = str.substr(1, str.size());
				cleaner = true;
			}
		}
	}
}

// Check if each ref has an associated label
void RefLabel::associatedLabel()
{
	map2d::iterator it_ref;
	map2d::const_iterator it_lab;
	std::map<std::string, int>::iterator it_map;

	for (it_ref = m_listRef.begin(); it_ref != m_listRef.end(); ++it_ref)
	{
		it_lab = m_listLabel.find(it_ref->first);
		if (it_lab != m_listLabel.cend())
		{
			for (it_map = (it_ref->second).begin(); it_map != (it_ref->second).end(); ++it_map)
				it_map->second = 1;
		}
	}
}

// List labels per files
void RefLabel::sort()
{
	if (m_sortByFile)
	{
		map2d::const_iterator it_ref;
		std::map<std::string, int>::const_iterator it_map;

		for (it_ref = m_listRef.begin(); it_ref != m_listRef.end(); ++it_ref)
		{
			if ( ((it_ref->second).begin())->second == 0 )
				for (it_map = (it_ref->second).begin(); it_map != (it_ref->second).end(); ++it_map)
					m_listFilesLabels[it_map->first].emplace_back(it_ref->first);
		}
	}
}

// Running function
void RefLabel::run(bool const sortByFile)
{
	try
	{
		this->makeList(this->m_fileName, this->m_fileName);
		this->associatedLabel();

		if (sortByFile)
		{
			m_sortByFile = true;
			this->sort();
		}
		cout << *this << endl;
	}
	catch (const string& strErrMakeList)
	{
		std::cerr << strErrMakeList << endl;
	}

}

// Main prog
int main(int argc, char *argv[])
{
	try
	{
		if (argc != 2 && argc != 3)
			throw string("*** ERROR (from main) : *** 1 (2) arg required \n ./exe yourDocument.tex (boolean sortByFile)");

		cout << "you are running " << argv[0] << " on file = " << argv[1] << endl;
		try
		{
			RefLabel cite(argv[1]);
			if (argc == 2)
				cite.run(false);
			if (argc == 3)
				cite.run(true);
		}
		catch (const string& strErrCitation)
		{
			std::cerr << strErrCitation << endl;
		}
	}
	catch (const string& strErrMain)
	{
		std::cerr << strErrMain << endl;
	}
	return 0;
}
