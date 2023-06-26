from crossref.restful import Works
import shutil
import sys
import re

print("You are running the script", sys.argv[0])
filename = sys.argv[1]
ind = filename.index(".bib")
filename_orig = filename[:ind] + "_orig" + filename[ind:]

print("Modifying file", filename)
print("Its content has been copied into file", filename_orig)

shutil.copy2(filename, filename_orig)

#& Read file line by line, and store the lines to modify
with open(filename, encoding = "utf8", mode = "r") as bibFile:
	line = bibFile.readline()
	info_dc = dict()
	author_counter = -1
	counter = 0 # Python starts counting from 0, and the zeroth line (i.e., first) has just been read

	while line != "": # This is the equivalent of End Of File (empty lines are represented by '\n'): https://docs.python.org/3/tutorial/inputoutput.html#methods-of-file-objects
		art_auth_yr = dict() # Article line, author line, year line for given doi. Note that this dictionary is reset after each article detected
		if "@article" in line:
		#? ------ Starting an article entry in bibliography
			art_auth_yr.update({"key_line": counter})
			flag_read_authors = False
			flag_read_doi = False
			flag_read_year = False
			while line != "}\n":
				if ("author = {" in line) and flag_read_authors:
					print("Corrupted bib file, there is more than one author field within an article")
					break
				if ("author = {" in line) and not flag_read_authors:
					art_auth_yr.update({"author": counter})
					flag_read_authors = True
				
				if ("year = {" in line) and flag_read_year:
					print("Corrupted bib file, there is more than one year field within an article")
					break
				if ("year = {" in line) and not flag_read_year:
					art_auth_yr.update({"year": counter})
					flag_read_year = True
				
				if ("doi = {" in line) and flag_read_doi:
					print("Corrupted bib file, there is more than one doi field within an article")
					break
				if ("doi = {" in line) and not flag_read_doi:
					doi = re.search('{(.+?)}', line)
					flag_read_doi = True
				
				line = bibFile.readline()
				counter += 1
			#? ------ Ending an article entry in bibliography
			if flag_read_doi:
				info_dc.update({doi.group(1): art_auth_yr}) # Updating the dictionary doi: (article, author, year) lines
		line = bibFile.readline()
		counter += 1

#& Read entire file
with open(filename, encoding = "utf8", mode = "r") as bibFile:
	wholeFile = bibFile.readlines()

#& Modify the citation keys, authors, and years lines
for doi in info_dc:
	metadata_article = Works().doi(doi)
	if metadata_article is None:
		print("No information found for doi", doi)
		continue
	
	authors = metadata_article['author']
	author_1 = authors[0]['family'].replace(" ", "").lower().capitalize()
	year = metadata_article['issued']['date-parts'][0][0]
	citation_key = author_1 + "." + str(year)
	
	entry = "author = {"
	for author in authors:
		entry += author['family'] + ", " + author['given'] + " and "
	entry = entry.rsplit(" and ", 1)[0] # Remove the last " and "
	entry += "}, \n"

	key_line = info_dc[doi]["key_line"]
	wholeFile[key_line] = "@article{" + citation_key + ", \n"

	author_line = info_dc[doi]["author"]
	wholeFile[author_line] = entry

	year_line = info_dc[doi]["year"]
	wholeFile[year_line] = "year = {" + str(year) + "}, \n"

#& Write into new file
with open(filename, encoding = "utf8", mode = "w") as bibFile:
	bibFile.writelines(wholeFile)
