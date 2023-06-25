from crossref.restful import Works
import re

works = Works()

#& Read file line by line, and store the lines to modify
with open('article.bib', encoding = "utf8", mode = "r") as bibFile:
	line = bibFile.readline()
	counter_dc = dict()
	counter_ls = []
	counter = 0 # Python starts counting from 0, and the zeroth line (i.e., first) has just been read
	iter_doi = 0

	while line != "": # This is the equivalent of End Of File (empty lines are represented by '\n'): https://docs.python.org/3/tutorial/inputoutput.html#methods-of-file-objects
		if "@article" in line:
		#? ------ Starting an article entry in bibliography
			flag_read_authors = False
			flag_read_doi = False
			while line != "}\n":
				if ("author = {" in line) and flag_read_authors:
					print("Corrupted bib file, there is more than one author field within an article")
					break
				if ("author = {" in line) and not flag_read_authors:
					counter_ls.append(counter)
					flag_read_authors = True
				if ("doi = {" in line) and flag_read_doi:
					print("Corrupted bib file, there is more than one doi field within an article")
					break
				if ("doi = {" in line) and not flag_read_doi:
					doi = re.search('{(.+?)}', line)
					flag_read_doi = True
				line = bibFile.readline()
				counter += 1
			#? ------ Ending an article entry in bibliography
			counter_dc.update({counter_ls[iter_doi]: doi.group(1)}) # Updating the dictionary author line number: doi
			iter_doi += 1
		line = bibFile.readline()
		counter += 1

#& Read entire file
with open('article.bib', encoding = "utf8", mode = "r") as bibFile:
	wholeFile = bibFile.readlines()

#& Modify the authors lines
for key_lineno in counter_dc:
	doi = counter_dc[key_lineno]
	metadata_article = works.doi(doi)
	if metadata_article is None:
		print("No information found for authors line", key_lineno, "with doi", doi)
		continue
	authors = metadata_article['author']
	year = metadata_article['issued']['date-parts'][0][0]
	author_1 = authors[0]['family'].replace(" ", "").lower().capitalize()
	citation_key = author_1 + "." + str(year)
	entry = "author = {"
	for author in authors:
		entry += author['family'] + ", " + author['given'] + " and "
	entry = entry.rsplit(" and ", 1)[0] # Remove the last " and "
	entry += "}, \n"
	wholeFile[key_lineno] = entry

#& Modify the citation keys lines


#& Modify the years lines


#& Write into new file
with open('article_new.bib', encoding = "utf8", mode = "w") as bibFile:
	bibFile.writelines(wholeFile)


metadata_article.keys()