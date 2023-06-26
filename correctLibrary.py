#!/usr/bin/python3

import progressbar
from crossref.restful import Works
import shutil
import sys
import re

#### Tool function
## Function to count lines in a file
def buf_count_newlines_gen(fname):
	def _make_gen(reader):
		while True:
			b = reader(2 ** 16)
			if not b: break
			yield b

	with open(fname, "rb") as f:
		count = sum(buf.count(b"\n") for buf in _make_gen(f.raw.read))
	return count

#### Get arguments and copy files
print("You are running the script", sys.argv[0])
filename = sys.argv[1]
ind = filename.index(".bib")
filename_orig = filename[:ind] + "_orig" + filename[ind:]

otherSave = False

if len(sys.argv):
	print("Output also saved under the name", sys.argv[2])
	otherSave = True

print("Modifying file", filename)
print("Its content has been copied into file", filename_orig)

shutil.copy2(filename, filename_orig)

#### Run program
## Get number of lines in file
max_count = buf_count_newlines_gen(filename) + 1

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
			while line != "}\n": # If the file ends by '}' without newline '\n', then max_count should be able to break the infinite loop
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
				if counter > max_count:
					break
			#? ------ Ending an article entry in bibliography
			if flag_read_doi and flag_read_authors and flag_read_year:
				info_dc.update({doi.group(1): art_auth_yr}) # Updating the dictionary doi: (article, author, year) lines
			elif not flag_read_doi:
				print("DOI is missing for:", art_auth_yr)
			else :
				print("Incomplete informations for doi:", doi.group(1))
		line = bibFile.readline()
		counter += 1

#& Read entire file
with open(filename, encoding = "utf8", mode = "r") as bibFile:
	wholeFile = bibFile.readlines()
  
#& Modify the citation keys, authors, and years lines
for doi in progressbar.progressbar(info_dc):
	error_metadata = False
	metadata_article = Works().doi(doi)
	if metadata_article is None:
		print("No information found for doi", doi)
		continue
	
	authors = metadata_article['author']
	author_1 = authors[0]['family'].replace(" ", "").lower().capitalize()
	year = -9999
	flag_read_year = False
	if 'published-print' in metadata_article.keys():
		flag_read_year = True
		year = metadata_article['published-print']['date-parts'][0][0]
	
	elif 'published' in metadata_article.keys():
		flag_read_year = True
		year = metadata_article['published']['date-parts'][0][0]

	else :
		print("No key found to extract the year of publication! Here are all the keys of metadata_article:")
		print(metadata_article.keys())

	if not flag_read_year:
		print("Warning: the year is probably wrong for doi", doi)

	citation_key = author_1 + "." + str(year)
	
	entry = "author = {"
	for author in authors:
		if 'family' not in author.keys() or 'given' not in author.keys():
			print("The entries for doi", doi, "have not been modified because the metadata lack informations on authors' name")
			error_metadata = True
		else :
			entry += author['family'] + ", " + author['given'] + " and "
	
	if error_metadata:
		continue
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

if otherSave:
	with open(sys.argv[2], encoding = "utf8", mode = "w") as bibFile:
		bibFile.writelines(wholeFile)