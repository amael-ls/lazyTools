
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

class Citation
{
	public :
		Citation(std::string const& outputBib, std::vector<std::string> const& citePattern);
		void run(std::string nameFile);
		friend std::ostream& operator<<(std::ostream& os, Citation const& cite);

	private :
	// member data
		std::map<std::string, std::map<std::string, int> > m_listRef; // map2d m_listRef; // map <string nameRef, map <string fromFile, int count> >
		std::string m_outputBib;
		bool m_firstCall;
		std::vector<std::string> const m_citePattern;

	// member functions
		void makeList(string nameFile, string const citePattern, string const fromFile);
		bool splitComa(std::string const &str, std::string const &fromFile);
		bool bibFile(string const nameLibrary, std::ios::openmode mode = std::ios::out);
		void addRef(std::string const nameRef, std::string const fromFile);
};



Citation::Citation(std::string const& outputBib, std::vector<std::string> const& citePattern) :
m_outputBib(outputBib), m_firstCall(true), m_citePattern(citePattern)
{
	std::vector<std::string>::const_iterator it = m_citePattern.cbegin();
	std::cout << "Using the following citing patterns:" << std::endl;
	for (; it != m_citePattern.cend(); ++it)
		std::cout << "<" << *it << ">" << std::endl;
}



std::ostream& operator<<(std::ostream& os, Citation const& cite)
{
	map2d::const_iterator it = cite.m_listRef.begin();
	std::map<std::string, int>::const_iterator it_map;
	int count = 0;
	for (it = cite.m_listRef.begin(); it != cite.m_listRef.end(); ++it)
	{
		os << it->first << " :" <<endl;
		for (it_map = (it->second).begin(); it_map != (it->second).end(); ++it_map)
		{
			os << "    - " << std::setw(40) << it_map->first << " \t " << it_map->second << endl;
			count += it_map->second;
		}
	}
	cout << "total number of citations = " << count << endl;
	cout << "number of references = " << (cite.m_listRef).size() << endl;
	return os;
}



void Citation::addRef(std::string const nameRef, std::string const fromFile)
{
	map2d::iterator it = m_listRef.find(nameRef);
	if (it != m_listRef.end())
	{
		std::map<std::string, int>::iterator it_map = (it->second).find(fromFile);
		if (it_map != (it->second).end() )
		{
			it_map->second += 1;
		}
		else
		{
			(it->second).insert(it_map, std::pair<std::string, int>(fromFile, 1));
		}
	}
	else
		m_listRef[nameRef][fromFile] = 1;
}



void Citation::makeList(string nameFile, string const citePattern, string const fromFile)
{
	std::vector<char> vecChar(1,'"');
	cleanString(nameFile, vecChar);
	if (nameFile.size() < 4)
		nameFile += ".tex";
	else if (nameFile.substr(nameFile.size() - 4, 4) != ".tex")
		nameFile += ".tex";

	std::ifstream ifs(nameFile.c_str());
	if (!ifs)
		throw string("*** ERROR (from readFile) *** : cannot open file : ''" + nameFile + "''\nthis input is from ''" + fromFile + "''");

	string nameFile_recursiveCalling;
	string nameRef;
	string readingLine;

	size_t inputPos (0), citePos(0), citePos_sqBracket_opening(0), citePos_sqBracket_closing(0), length_sub_str(0);

	if (m_firstCall) // To skip everything before \beging{document}
	{
		while (getline(ifs, readingLine))
		{
			if (readingLine == "\\begin{document}")
				break;
		}
		m_firstCall = false;
	}

	while ((getline(ifs, readingLine)) && !(ifs.eof()))
	{
		while ((readingLine[0] == '%') && !(ifs.eof())) // to drop all comment lines
			getline(ifs, readingLine);

		while ( (inputPos = readingLine.find("input{", inputPos)) != string::npos) // recursive call
		{
			inputPos += 6;
			length_sub_str = readingLine.find("}", inputPos) - inputPos;
			nameFile_recursiveCalling = readingLine.substr(inputPos, length_sub_str);
			this->makeList(nameFile_recursiveCalling, citePattern, nameFile);
		}

		while ( (citePos = readingLine.find(citePattern, citePos)) != string::npos) // Searches the string for the first occurrence of the sequence specified by its arguments.
		{
			citePos_sqBracket_opening = readingLine.find("[", citePos);
			citePos_sqBracket_closing = readingLine.find("]", citePos);
			citePos = readingLine.find("{", citePos) + 1;

			if ((citePos_sqBracket_opening < citePos) && (citePos < citePos_sqBracket_closing))
				citePos = readingLine.find("{", citePos_sqBracket_closing) + 1;

			length_sub_str = readingLine.find("}", citePos) - citePos;
			nameRef = readingLine.substr(citePos, length_sub_str);

			if (!this->splitComa(nameRef, nameFile))
				this->addRef(nameRef, nameFile);
			citePos++;
		}
		inputPos = 0;
		citePos = 0;
	}
}



bool Citation::splitComa(std::string const &str, std::string const &fromFile)
{
	bool comaDetected = false;

	std::string sub_str;
	size_t comaPos (0), oldComaPos (0), length_sub_str (0);
	std::vector<char> charToDel(2); charToDel[0] = ' '; charToDel[1] = ',';

	while ( (comaPos = str.find(",", comaPos)) != string::npos)
	{
		comaDetected = true;
		length_sub_str = comaPos - oldComaPos;
		sub_str = str.substr(oldComaPos, length_sub_str);
		cleanString(sub_str, charToDel);
		this->addRef(sub_str, fromFile);
		oldComaPos = comaPos;
		comaPos++;
	}
	if (comaDetected) // to get the ref after the last coma
	{
		length_sub_str = str.length() - oldComaPos;
		sub_str = str.substr(oldComaPos, length_sub_str);
		cleanString(sub_str, charToDel);
		this->addRef(sub_str, fromFile);
	}
	return comaDetected;
}



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



void Citation::run(std::string nameFile)
{
	bool success = false;
	std::vector<std::string>::const_iterator citePattern_it = m_citePattern.cbegin();
	try
	{
		for (; citePattern_it != m_citePattern.cend(); ++citePattern_it)
			this->makeList(nameFile, *citePattern_it, nameFile);
		cout << *this << endl;
	}
	catch (const string& strErrMakeList)
	{
		std::cerr << strErrMakeList << endl;
	}
	success = this->bibFile("/Users/mistral/Documents/mendeley_bib/library.bib");
	if (!success)
	{
		cout << "******************************************" << endl;
		cout << "Some references have been undetected" << endl;
		cout << "Trying to find them in: `/Users/mistral/ownCloud/library/bibtexMendeley/otherBib.bib´" << endl;
		success = this->bibFile("/Users/mistral/ownCloud/library/bibtexMendeley/otherBib.bib", std::ios::app);
		if (!success)
		{
			cout << "Following references undetected : " << endl;
			cout << *this << endl;
		}
		else
			cout << "They have been found" << endl;

		cout << "******************************************" << endl;
	}
	else
		cout << "Every citation has been found" << endl;

}



bool Citation::bibFile(string const nameLibrary, std::ios::openmode mode)
{
	std::ifstream ifs(nameLibrary.c_str());
	if (!ifs)
		throw string("*** ERROR (from Citation::bibFile) *** : cannot open file : ''" + nameLibrary + "'");

	std::ofstream ofs(m_outputBib.c_str(), mode);
	 if (!ofs)
		throw string("*** ERROR (from Citation::bibFile) *** : cannot create file : ''" + m_outputBib + "'");

	map2d::iterator it;
	string str_at, str;
	size_t posBracket, posComa, length_substr; // because format is always @...{nameRef,

	while (getline(ifs, str_at) && !m_listRef.empty())
	{
		if (str_at[0] == '@')
		{
			it = m_listRef.begin();
			posBracket = str_at.find("{") + 1;
			posComa = str_at.find(",");
			length_substr = posComa - posBracket;
			while (it != m_listRef.end() && str_at.substr(posBracket, length_substr) != it->first)
				++it;

			if (it != m_listRef.end())
			{
				ofs << str_at << endl;
				while(str != "}")
				{
					getline(ifs, str);
					ofs << str << endl;
				}
				m_listRef.erase(it);
			}
			it = m_listRef.begin();
			str = "";
		}
	}
	return (m_listRef.empty());
}



int main(int argc, char *argv[])
{
	try
	{
		if (argc < 3)
			throw string("*** ERROR (from main) : *** 2 args are required (+ optional) \n ./exe main.tex output.bib (citePattern1 citePattern2 ... citePatternZ)");


		std::map<string, string> listRef;
		cout << "you are running " << argv[0] << " with input latex file = " << argv[1] << " and output bibliography file " << argv[2] << endl;
		try
		{
			std::vector<std::string> citingArgs;
			if (argc == 3)
			{
				std::cout << "Using the default citing pattern: \\cite" << std::endl;
				citingArgs.push_back("\\cite");
			}

			if (argc > 3)
			{
				for (unsigned int i = 3; i < argc; ++i)
					citingArgs.push_back(argv[i]);
			}
			Citation cite(argv[2], citingArgs);
			cite.run(argv[1]);
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
